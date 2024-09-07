#include <ESP_I2S.h>
#include "arduinoFFT.h"

#define WAVESIZE 16384

I2SClass I2S;


void setup() {
  // 串口
  Serial.begin(115200);
  // 麦克风
  I2S.setPins(41, 42, -1, 2, -1);  //SCK, WS, SDOUT, SDIN, MCLK
  I2S.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, -1);
  // 信号采样
  int sample = 0;
  float scaledSample = 0;
  double* waveform = (double*)ps_malloc(WAVESIZE * sizeof(double));
  // FFT
  double* vReal = (double*)ps_malloc(WAVESIZE * sizeof(double));
  double* vImag = (double*)ps_malloc(WAVESIZE * sizeof(double));
  ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, WAVESIZE, 16000);

  // 内存使用情况
  Serial.printf("\nTotal heap: %d\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());


 
}

void loop() {
  
  Serial.printf("\nTotal heap: %d\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
  while(1){

  }
  // for(int i = 0; i < WAVESIZE; i++){
    
  //   while(1){
  //     sample = I2S.read();
  //     if((sample != 0) && (sample != -1)){
  //       break;
  //     }
  //   }
  //   sample >>= 8;
  //   // Convert to float in the range of -1 to 1
  //   scaledSample = sample / 8388607.0; // 2^23 - 1 = 8388607
  //   waveform[i] = scaledSample;
  //   // Serial.println(scaledSample, 6);

  // }

  // for(int i = 0; i < WAVESIZE; i++){
  //   Serial.println(waveform[i], 6);
  // }


}


