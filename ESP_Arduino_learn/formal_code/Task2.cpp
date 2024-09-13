#include "HardwareSerial.h"
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
 
    // // // 更新时域信号到window_wave(使用队列)
    // // buffer_shift(xQueue_waveform, window_wave, WINDOWSTEP, WINDOWSIZE, wave_queue_timeout);

    // static int num_count = 0;
    // Serial.println("-----------------");
    // for(int i = 0; i < WINDOWSIZE; i++){
    //   Serial.printf("i = %d  ", i+num_count-256);
    //   Serial.println(window_wave[i], 6);
    // }
    // Serial.println("-----------------");
    // num_count = num_count + 128;
    // vTaskDelay(1000);

    // 测试: 更新音频数据到window_wave
    static int index = 0;
    // 移位
    for(int i = 0; i <= WINDOWSIZE-WINDOWSTEP-1; i++){
      window_wave[i] = window_wave[i + WINDOWSTEP];
    }
    // 赋新值
    for(int i = WINDOWSIZE-WINDOWSTEP; i < WINDOWSIZE; i++){
      window_wave[i] = bird_wave[index];
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
    
    // 只保留部分频率桶（梅尔）
    static float mel_frequence[MELNUM] = {0};
    for(int i = 0; i < MELNUM; i++){
      mel_frequence[i] = (float)vReal[mel_index[i]];
    }

//     // 测试
//     static int j = -1;
// Serial.println("-----------------");
//     for(int i = 0; i < MELNUM; i++){
//       Serial.printf("j = %d i = %d  %6f\n", j, i, vReal[i]);
//     }
//     j++;
// Serial.println("-----------------");
//     vTaskDelay(1000);
   


    // 将MLENUM 个频率发送到队列:大坑，注意队列默认一次传输4 个字节，而vReal 是double 类型（8 个字节）
    for(int i = 0; i < MELNUM; i++){
      if(xQueueSend(xQueue_spectrogram, mel_frequence+i, spectrogram_queue_timeout) == pdPASS){
      }
      else{
        Serial.printf("spectrogram 队列已满超过 %d ms\n", spectrogram_queue_timeout);
      }         
    }

    // // 测试：直接把真实频谱图的值赋给vReal, 舍弃实际计算的值
    // static float* animal_p_2 = (float*)bed;
    // static int index_2 = 0;
    // for(int i = 0; i < MELNUM; i++){
    //   if(xQueueSend(xQueue_spectrogram, animal_p_2 + index_2, spectrogram_queue_timeout) == pdPASS){
    //   }
    //   else{
    //     Serial.printf("spectrogram 队列已满超过 %d ms\n", spectrogram_queue_timeout);
    //   }     
    //   index_2++;

    //   if(index_2 == 3904){
    //     index_2 = 0;
    //   }
    // }





    // 测试时长
    // if(temp % WINDOWNUM == 0){
    //   Serial.println("计算完一秒的数据");
    // }
    // temp++;

    // Serial.println("\n\n\n\n\n");
    // // 打印频谱图
    // for(int i = 0; i < WINDOWNUM*FREQENCENUM; i++){
    //   if(i % 100 == 0){
    //     Serial.printf("i = %d %f\n", i, spectrogram[i]);
    //   }
    // }     
    

  }
}