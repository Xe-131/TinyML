// 需要先包含Arduino.h 这里面包括了freeRTos 以及一些基本类型定义
#include <Arduino.h> 
#include "common.h"

#define MODEL_QUANT_TFLITE_8000_LEN 1632408


void Task3(void* parameters){
  Serial.printf("\n\n\nTask3 running in %d\n", xPortGetCoreID());


  // 获取 PSRAM 的大小（以字节为单位）
  Serial.printf("PSRAM Size: %u bytes\n", ESP.getPsramSize());
  // 将模型从FLASH 中加载到PSRAM
  unsigned char* MODEL_QUANT_TFLITE_8000_PSRAM = (unsigned char*)ps_malloc(MODEL_QUANT_TFLITE_8000_LEN*sizeof(unsigned char));
  for(int i = 0; i < MODEL_QUANT_TFLITE_8000_LEN; i++){
    MODEL_QUANT_TFLITE_8000_PSRAM[i] = MODEL_QUANT_TFLITE_8000_FLASH_1[i];
  }
  // 获取 PSRAM 剩余内存
  Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());

  while(1){

  }
}

