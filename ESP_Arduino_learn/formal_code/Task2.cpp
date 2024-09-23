#include "HardwareSerial.h"
#include <Arduino.h> 
#include "common.h"
#include "arduinoFFT.h"
#include "model.h"

// 保留的频率索引
int mel_index[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
  22, 23, 24, 25, 27, 28, 30, 31, 33, 34, 36, 38, 40, 41, 43, 45, 47, 50, 52, 54, 56, 59, 61, 63,
  66, 69, 71, 74, 77, 80, 83, 86, 90, 93, 96, 100, 104, 107, 111, 115, 120, 124};

// 将一个窗口的数据进行FFT
void Task2(void* parameters){
  Serial.printf("\n\n\nTask2 running in %d\n", xPortGetCoreID());

  // FFT 计算所用buffer
  double* vReal = (double*)ps_malloc(WINDOWSIZE * sizeof(double));
  double* vImag = (double*)ps_malloc(WINDOWSIZE * sizeof(double));
  // 寄存一个window 的时域信号
  float* window_wave = (float*)ps_malloc(WINDOWSIZE * sizeof(float));
  // FFT 初始化
  ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, WINDOWSIZE, SAMPLERATE);

  while(1){
 
    // 将麦克风音频从队列中读出
    // 移位
    for(int i = 0; i <= WINDOWSIZE-WINDOWSTEP-1; i++){
      window_wave[i] = window_wave[i + WINDOWSTEP];
    }
    // 赋新值
    for(int i = WINDOWSIZE-WINDOWSTEP; i < WINDOWSIZE; i++){
      if(xQueueReceive(xQueue_waveform ,window_wave+i , wave_queue_timeout) == pdPASS){
      }
      else{
        Serial.printf("waveform 队列已空超过 %d ms\n", wave_queue_timeout);
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
    
    // // 只保留部分频率桶（梅尔）：未归一化版本
    // static float mel_frequence[MELNUM] = {0};
    // for(int i = 0; i < MELNUM; i++){
    //   mel_frequence[i] = (float)vReal[mel_index[i]];
    // }

    // 归一化版本
    static float mel_frequence[MELNUM] = {0};
    float min_val = vReal[mel_index[0]];
    float max_val = vReal[mel_index[0]];
    for (int i = 0; i < MELNUM; i++) {
        float value = (float)vReal[mel_index[i]];
        mel_frequence[i] = value;
        if (value < min_val) {
            min_val = value;
        }
        if (value > max_val) {
            max_val = value;
        }
    }
    // 对 mel_frequence 数组进行归一化
    for (int i = 0; i < MELNUM; i++) {
        mel_frequence[i] = (mel_frequence[i] - min_val) / (max_val - min_val + 1e-6);
    }

    // 将MLENUM 个频率发送到队列:大坑，注意队列默认一次传输4 个字节，而vReal 是double 类型（8 个字节）
    for(int i = 0; i < MELNUM; i++){
      if(xQueueSend(xQueue_spectrogram, mel_frequence+i, spectrogram_queue_timeout) == pdPASS){
      }
      else{
        Serial.printf("spectrogram 队列已满超过 %d ms\n", spectrogram_queue_timeout);
      }         
    }

    #ifdef TIME_TEST
    // 时间测试
    static int task2_count = 0;
    static int task2_time = 0;
    task2_count++;
    if(task2_count == WINDOWNUM){
      task2_count = 0;
      Serial.printf("Task2 生成一张频谱图耗时：%d\n", millis()-task2_time);
      task2_time = millis();
    }
    #endif    

  }
}