#include <ESP_I2S.h>
#include "arduinoFFT.h"


// 频谱图样本数 = WINDOWSIZE + (STEPNUM-1) * WINDOWSTEP
#define WINDOWSIZE 256   // 一次FFT 的样本数，必须是2 的幂
#define WINDOWSTEP 128  // 步长
#define WINDOWNUM 124   // 总窗个数
#define FREQENCENUM 129 // 单窗的频率分量个数，WINDOWSIZE/2 为有效量，轴对称
#define SCALE 10000 // 将缩放到1 到 -1 的原始信号二次缩放

// 时域信号
double* waveform = NULL;
// 频域信号
double* spectrogram = NULL;

// 声明互斥锁
SemaphoreHandle_t xMutexInventory_1 = NULL;
SemaphoreHandle_t xMutexInventory_2 = NULL;
TickType_t timeout = 1000;

// 运行时间
unsigned int time_os;

void setup() {
  // 串口
  Serial.begin(115200);
  // 打印频谱图信息
  Serial.println("\nspectrogram infomation:");
  Serial.printf("window size = %d\n", WINDOWSIZE);
  Serial.printf("window step = %d\n", WINDOWSTEP);
  Serial.printf("step number = %d\n", WINDOWNUM);
  Serial.printf("spectrogram size = %d\n", WINDOWSIZE + (WINDOWNUM-1) * WINDOWSTEP);
  // 创建互斥锁
  xMutexInventory_1 = xSemaphoreCreateMutex();
  if(xMutexInventory_1 == NULL){
    Serial.println("No Enough Ram For Mutex");
  }
  // 创建任务(core 1)
  xTaskCreatePinnedToCore(task1, "iis mic", 1024*3, NULL, 1, NULL, 1);
  vTaskDelay(30); // 先让mic 采集数据
  xTaskCreatePinnedToCore(task2, "FFT", 1024*6, NULL, 1, NULL, 1);


}

void loop() {
}

// 麦克风一次收集一个STEP 的数据量
void task1(void* parameters){
  Serial.printf("task1 running in %d\n", xPortGetCoreID());

  I2SClass I2S;
  int sample = 0;
  double scaledSample = 0;
  // 分配到SRAM
  double* waveform_copy = (double*)ps_malloc(WINDOWSTEP * sizeof(double));
  waveform = (double*)ps_malloc(WINDOWSTEP * sizeof(double));
  // 初始化mic
  I2S.setPins(41, 42, -1, 2, -1);  //SCK, WS, SDOUT, SDIN, MCLK
  I2S.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, -1);

  while(1){
    // 采集信号到任务私有数组
    for(int i = 0; i < WINDOWSTEP; i++){
      while(1){
        sample = I2S.read();
        if((sample != 0) && (sample != -1)){
          break;
        }
      }
      sample >>= 8;
      scaledSample = (sample / 8388607.0)*SCALE; // 2^23 - 1 = 8388607
      waveform_copy[i] = scaledSample;
    }

    // 将数据传到公共区域
    if(xSemaphoreTake(xMutexInventory_1, timeout) == pdPASS){
      for(int i = 0; i < WINDOWSTEP; i++){
        waveform[i] = waveform_copy[i];
      }
      // 释放钥匙
      xSemaphoreGive(xMutexInventory_1);     
    }
    else{
      Serial.println("mic写数据 未获得钥匙");
    }

  }
}

// FFT 操作需要整个窗口的数据
void task2(void* parameters){
  Serial.printf("task2 running in %d\n", xPortGetCoreID());
  // FFT 计算所用buffer
  double* vReal = (double*)ps_malloc(WINDOWSIZE * sizeof(double));
  double* vImag = (double*)ps_malloc(WINDOWSIZE * sizeof(double));
  // 寄存一个window 的时域信号
  double* window_wave = (double*)ps_malloc(WINDOWSIZE * sizeof(double));
  // 频域信号（公共）
  spectrogram = (double*)ps_malloc(WINDOWNUM*FREQENCENUM * sizeof(double));

  ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, WINDOWSIZE, 16000);

  while(1){
 

    // 初始化虚数
    for(int i = 0; i < WINDOWSIZE; i++){
      vImag[i] = 0;
    }
    // 更新时域信号到window_wave
    buffer_update(waveform, window_wave, WINDOWSTEP, WINDOWSIZE, &xMutexInventory_1);
    // 复制到vReal
    for(int i = 0; i < WINDOWSIZE; i++){
      vReal[i] = window_wave[i];
    }

 

    // 进行FFT 操作
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    
    // 更新公共频谱图（数据移位）
    buffer_update(vReal, spectrogram, WINDOWSIZE, WINDOWNUM*FREQENCENUM, &xMutexInventory_1);
    


    Serial.println("\n\n\n\n\n");
    // 打印频谱图
    for(int i = 0; i < WINDOWNUM*FREQENCENUM; i++){
      if(i % 100 == 0){
        Serial.printf("i = %d %4f\n", i, spectrogram[i]);
      }
    }     
    
 

  
  }
}

/* 将from 数组左移，丢弃最左边的数据，最右边更新新的数据*/
void buffer_update(double* from, double* to, int from_size, int to_size, SemaphoreHandle_t* key){
  if(from_size >= to_size){
    Serial.println("buffer_update went wrong !");
    return;
  }

  // 转移自己内存
  for(int i = from_size; i < to_size; i++){
    to[i-from_size] = to[i];
  }
  // 加入新的数据(公共区)
  if(xSemaphoreTake(*key, timeout) == pdPASS){
    while(1){
      to[to_size-1] = from[from_size-1];
      to_size--;
      from_size--;
      if(from_size == 0){
        break;
      }
    }
    // 释放钥匙
    xSemaphoreGive(*key);
  }
  else{
    Serial.println("buffer_update 未获得钥匙");
  }

}