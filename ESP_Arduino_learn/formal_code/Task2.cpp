#include <Arduino.h> 
#include "common.h"
#include "arduinoFFT.h"

// FFT 操作需要整个窗口的数据
void Task2(void* parameters){
  Serial.printf("\n\n\nTask2 running in %d\n", xPortGetCoreID());

  // FFT 计算所用buffer
  double* vReal = (double*)ps_malloc(WINDOWSIZE * sizeof(double));
  double* vImag = (double*)ps_malloc(WINDOWSIZE * sizeof(double));
  // 寄存一个window 的时域信号
  float* window_wave = (float*)ps_malloc(WINDOWSIZE * sizeof(float));

  ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, WINDOWSIZE, SAMPLERATE);

  while(1){
 
    // // 更新时域信号到window_wave(使用队列)
    // buffer_shift(xQueue_waveform, window_wave, WINDOWSTEP, WINDOWSIZE, wave_queue_timeout);

    // 测试
    static float* animal_p = (float*)dog_wave;
    static int index = 0;
    // 移位
    for(int i = 0; i <= WINDOWSIZE-WINDOWSTEP-1; i++){
      window_wave[i] = window_wave[i + WINDOWSTEP];
    }
    // 赋新值
    for(int i = WINDOWSIZE-WINDOWSTEP; i < WINDOWSIZE; i++){
      window_wave[i] = dog_wave[index];
      index++;
      if(index == 8000){
        index = 0;
      }
    }

    // 复制到vReal
    for(int i = 0; i < WINDOWSIZE; i++){
      vReal[i] = (double)window_wave[i];
    }
    // 初始化虚数
    for(int i = 0; i < WINDOWSIZE; i++){
      vImag[i] = 0;
    }
 

    // 进行FFT 操作
    FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    
    // 测试：直接把真实频谱图的值赋给vReal, 舍弃实际计算的值
    static float* animal_p_2 = (float*)bed;
    static int index_2 = 0;
    for(int i = 0; i < WINDOWSIZE; i++){
      vReal[i] = animal_p_2[index_2];
      index_2++;
      if(index_2 == 8060){
        index_2 = 0;
      }
    }


    // 更新公共频谱图（数据移位）
    buffer_update(vReal, spectrogram, WINDOWSIZE, WINDOWNUM*FREQENCENUM, &xMutexInventory_2);
   


    // 测试时长
    if(temp % WINDOWNUM == 0){
      Serial.println("计算完一秒的数据");
    }
    temp++;

    // Serial.println("\n\n\n\n\n");
    // // 打印频谱图
    // for(int i = 0; i < WINDOWNUM*FREQENCENUM; i++){
    //   if(i % 100 == 0){
    //     Serial.printf("i = %d %f\n", i, spectrogram[i]);
    //   }
    // }     
    

  }
}