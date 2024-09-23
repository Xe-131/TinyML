#include "HardwareSerial.h"
#include <Arduino.h> 
#include <ESP_I2S.h>
#include "common.h"
#include "model.h"

void Task1(void* parameters){
  Serial.printf("\n\n\nTask1 running in %d\n", xPortGetCoreID());

  I2SClass I2S;
  int sample = 0;
  float sample_float = 0;

  // 初始化mic
  //SCK, WS, SDOUT, SDIN, MCLK
  I2S.setPins(41, 42, -1, 2, -1);  
  I2S.begin(I2S_MODE_STD, SAMPLERATE, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, -1);

  while(1){
    // 采集数据
    while(1){
      sample = I2S.read();
      if((sample != 0) && (sample != -1)){
        break;
      }
    }
    sample >>= 8;
    sample_float = (float)sample;
    // 向队列发送数据
    if(xQueueSend(xQueue_waveform, &sample_float, wave_queue_timeout) == pdPASS){
    }
    else{
      Serial.printf("waveform 队列已满超过 %d ms\n", wave_queue_timeout);
    }

    #ifdef TIME_TEST
    // 时间测试
    static int task1_count = 0;
    static int task1_time = 0;
    task1_count++;
    if(task1_count == SAMPLERATE){
      task1_count = 0;
      Serial.printf("Task1 传输1s 数据耗时：%d\n", millis()-task1_time);
      task1_time = millis();
    }
    #endif

  }
}