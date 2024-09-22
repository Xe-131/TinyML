#include "HardwareSerial.h"
#include <Arduino.h> 
#include <ESP_I2S.h>
#include "common.h"
#include "model.h"

// 麦克风一次收集一个STEP 的数据量
void Task1(void* parameters){
  Serial.printf("\n\n\nTask1 running in %d\n", xPortGetCoreID());

  I2SClass I2S;
  int sample = 0;
  float sample_float = 0;
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
      sample_float = (float)sample;
      // Serial.printf("{mic}%10f\n", sample_float);
      // Serial.printf("{mic}%d\n", sample);
      // Serial.println(-3000);
      // Serial.printf("%d\n", sample);
      // Serial.println(3000);
      // 向队列发送数据
      if(xQueueSend(xQueue_waveform, &sample_float, wave_queue_timeout) == pdPASS){
      }
      else{
        Serial.printf("waveform 队列已满超过 %d ms\n", wave_queue_timeout);
      }
    }

    // // 测试直接音频输入
    // for(int i = 0; i < 8000; i++){
    //   while(1){
    //     sample = I2S.read();
    //     if((sample != 0) && (sample != -1)){
    //       break;
    //     }
    //   }
    //   sample >>= 8;
    //   sample_float = (float)sample;
    //   // 向队列发送数据
    //   if(xQueueSend(xQueue_waveform, bird_wave + i, wave_queue_timeout) == pdPASS){
    //   }
    //   else{
    //     Serial.printf("waveform 队列已满超过 %d ms\n", wave_queue_timeout);
    //   }
    // }
    // Serial.println("Task1 完成1s 传输");

  }
}