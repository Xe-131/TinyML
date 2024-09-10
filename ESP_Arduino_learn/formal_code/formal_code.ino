#include <ESP_I2S.h>
#include "arduinoFFT.h"
// #include "Task1.h"
// #include "common.h"

#define SAMPLERATE 8000
// 频谱图样本数 = WINDOWSIZE + (STEPNUM-1) * WINDOWSTEP
#define WINDOWSIZE 128   // 一次FFT 的样本数，必须是2 的幂
#define WINDOWSTEP 64  // 步长
#define WINDOWNUM 124   // 总窗个数
#define FREQENCENUM 64 // 单窗的频率分量个数，WINDOWSIZE/2 为有效量，轴对称

// 时域信号(队列)
StaticQueue_t waveform_QueueControlBlock;
QueueHandle_t xQueue_waveform;
uint8_t* waveform = NULL;
#define wave_queue_timeout 1000

// 频域信号
unsigned int* spectrogram = NULL;

// 声明互斥锁
SemaphoreHandle_t xMutexInventory_1 = NULL;
SemaphoreHandle_t xMutexInventory_2 = NULL;
TickType_t timeout = 1000;

// 运行时间
unsigned int time_os;

// 临时 
int temp = 0;

void setup() {
  // 串口
  Serial.begin(115200);
  // 打印频谱图信息
  Serial.println("\nspectrogram infomation:");
  Serial.printf("window size = %d\n", WINDOWSIZE);
  Serial.printf("window step = %d\n", WINDOWSTEP);
  Serial.printf("step number = %d\n", WINDOWNUM);
  Serial.printf("freqence number = %d\n", FREQENCENUM);
  Serial.printf("spectrogram size = %d\n", WINDOWSIZE + (WINDOWNUM-1) * WINDOWSTEP);
  // 创建互斥锁
  xMutexInventory_1 = xSemaphoreCreateMutex();
  if(xMutexInventory_1 == NULL){
    Serial.println("No Enough Ram For Mutex");
  }
  xMutexInventory_2 = xSemaphoreCreateMutex();
  if(xMutexInventory_2 == NULL){
    Serial.println("No Enough Ram For Mutex");
  }
  // 全局变量的内存分配
  waveform = (uint8_t*)ps_malloc(WINDOWSTEP*2 * sizeof(int)); // 时域信号(队列)
  spectrogram = (unsigned int*)ps_malloc(WINDOWNUM*FREQENCENUM * sizeof(unsigned int)); // 频域信号（变量）
  xQueue_waveform = xQueueCreateStatic(WINDOWSTEP*2, sizeof(int), waveform, &waveform_QueueControlBlock);
  if(xQueue_waveform != NULL){
    Serial.println("waveform 队列创建成功");
  }
  else{
    Serial.println("waveform 队列创建失败");
  }

  xTaskCreatePinnedToCore(Task1, "iis mic", 1024*3, NULL, 1, NULL, 1);
  vTaskDelay(30); // 先让mic 采集数据
  xTaskCreatePinnedToCore(task2, "FFT", 1024*6, NULL, 1, NULL, 1);


}




// 麦克风一次收集一个STEP 的数据量
void Task1(void* parameters){
  Serial.printf("task1 running in %d\n", xPortGetCoreID());

  I2SClass I2S;
  int sample = 0;

  // 初始化mic
  I2S.setPins(41, 42, -1, 2, -1);  //SCK, WS, SDOUT, SDIN, MCLK
  I2S.begin(I2S_MODE_STD, SAMPLERATE, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, -1);

  while(1){
    // 采集数据
    for(int i = 0; i < WINDOWSTEP; i++){
      while(1){
        sample = I2S.read();
        if((sample != 0) && (sample != -1)){
          break;
        }
      }
      sample >>= 8;
      // 向队列发送数据
      if(xQueueSend(xQueue_waveform, &sample, wave_queue_timeout) == pdPASS){
      }
      else{
        Serial.printf("waveform 队列已满超过 %d ms\n", wave_queue_timeout);
      }
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
  int* window_wave = (int*)ps_malloc(WINDOWSIZE * sizeof(int));

  ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, WINDOWSIZE, SAMPLERATE);

  while(1){
 
    // 更新时域信号到window_wave(使用队列)
    buffer_shift(xQueue_waveform, window_wave, WINDOWSTEP, WINDOWSIZE, wave_queue_timeout);

    // 复制到vReal
    for(int i = 0; i < WINDOWSIZE; i++){
      vReal[i] = (double)window_wave[i];
    }
    // 初始化虚数
    for(int i = 0; i < WINDOWSIZE; i++){
      vImag[i] = 0;
    }
 

    // 进行FFT 操作
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    
    // 更新公共频谱图（数据移位）
    buffer_update(vReal, spectrogram, WINDOWSIZE, WINDOWNUM*FREQENCENUM, &xMutexInventory_2);
    if(temp > 123){
      Serial.println(temp);
    }
    
    temp++;
    // Serial.println("\n\n\n\n\n");
    // // 打印频谱图
    // for(int i = 0; i < WINDOWNUM*FREQENCENUM; i++){
    //   if(i % 100 == 0){
    //     Serial.printf("i = %d %d\n", i, spectrogram[i]);
    //   }
    // }     
    

  }
}

/* 将from 数组左移，丢弃最左边的数据，最右边更新新的数据*/
void buffer_update(double* from, unsigned int* to, int from_size, int to_size, SemaphoreHandle_t* key){
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
      to[to_size-1] = (unsigned int)from[from_size-1];
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

void buffer_shift(QueueHandle_t queue, int* buffer, int shift_size, int buffer_size, int queue_timeout){
  if(shift_size >= buffer_size){
    Serial.println("buffer_shift went wrong !");
    return;
  }
  
  // 转移自己内存
  for(int i = shift_size; i < buffer_size; i++){
    buffer[i-shift_size] = buffer[i];
  }  

  // 从队列中添加新的数据
  while(1){
    if(xQueueReceive(queue, (buffer+buffer_size-1), queue_timeout) == pdPASS){
    }
    else{
      Serial.printf("buffer shift 队列已空超过 %d ms\n", queue_timeout);
    }

    buffer_size--;
    shift_size--;
    if(shift_size == 0){
      break;
    }
    
  }
}

void loop() {
}