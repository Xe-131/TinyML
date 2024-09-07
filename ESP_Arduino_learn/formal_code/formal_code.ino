#include <ESP_I2S.h>
#include "arduinoFFT.h"

// 总窗大小
#define WAVESIZE 16384
// 频谱图样本数 = WINDOWSIZE + (STEPNUM-1) * WINDOWSTEP
#define WINDOWSIZE 256 // 一次FFT 的样本数，必须是2 的幂
#define WINDOWSTEP 128 // 步长
#define STEPNUM 124
// 时域信号
double* waveform = NULL;

// 声明互斥锁
SemaphoreHandle_t xMutexInventory = NULL;
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
  Serial.printf("step number = %d\n", STEPNUM);
  Serial.printf("simple num = %d\n", WINDOWSIZE + (STEPNUM-1) * WINDOWSTEP);
  // 创建互斥锁
  xMutexInventory = xSemaphoreCreateMutex();
  if(xMutexInventory == NULL){
    Serial.println("No Enough Ram For Mutex");
  }
  // 创建任务
  xTaskCreate(task1, "iis mic", 1024*3, NULL, 1, NULL);
  xTaskCreate(task2, "FFT", 1024*6, NULL, 1, NULL);


}

void loop() {
}

void task1(void* parameters){
  I2SClass I2S;
  int sample = 0;
  float scaledSample = 0;
  // 分配到SRAM
  waveform = (double*)ps_malloc(WAVESIZE * sizeof(double));
  // 初始化mic
  I2S.setPins(41, 42, -1, 2, -1);  //SCK, WS, SDOUT, SDIN, MCLK
  I2S.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, -1);

  // 采集信号到数组
  while(1){
    if(xSemaphoreTake(xMutexInventory, timeout) == pdPASS){
      // Serial.printf("task1 time: %d\n", millis() - time_os);
      
      for(int i = 0; i < WAVESIZE; i++){
        while(1){
          sample = I2S.read();
          if((sample != 0) && (sample != -1)){
            break;
          }
        }
        sample >>= 8;
        scaledSample = sample / 8388607.0; // 2^23 - 1 = 8388607
        waveform[i] = scaledSample;
      }
      // 释放钥匙
      // time_os = millis();
      xSemaphoreGive(xMutexInventory);
      vTaskDelay(1);
    }
    else{
      // 在timeout 内数组没有FFT 完成
      Serial.println("task1 get no key");
      
    }
  }
}

void task2(void* parameters){
  double* vReal = (double*)ps_malloc(WAVESIZE * sizeof(double));
  double* vImag = (double*)ps_malloc(WAVESIZE * sizeof(double));
  ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, WAVESIZE, 16000);

  while(1){
    if(xSemaphoreTake(xMutexInventory, timeout) == pdPASS){
      for(int i = 0; i < WAVESIZE; i++){
      vImag[i] = 0;
      // 被两个任务同时读和写
      vReal[i] = waveform[i];
      }
      // 释放钥匙
      xSemaphoreGive(xMutexInventory);
    }
    else{
      Serial.println("task2 get no key");
      continue;
    }
    
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    double x = FFT.majorPeak();
    Serial.println(x, 6);
                

  }


}