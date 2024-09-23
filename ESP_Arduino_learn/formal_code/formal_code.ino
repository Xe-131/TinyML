#include "common.h"
#include "model.h"

// 时域信号(队列)
StaticQueue_t waveform_QueueControlBlock;
QueueHandle_t xQueue_waveform;
uint8_t* waveform = NULL;

// 频域信号 (队列)
StaticQueue_t spectrogram_QueueControlBlock;
QueueHandle_t xQueue_spectrogram;
uint8_t* spectrogram = NULL;

// 打印音频数据信息
void print_message();

void setup() {

  // 串口
  Serial.begin(115200);
  // 打印频谱图信息
  print_message();

  // 给两个队列分配内存
  waveform = (uint8_t*)ps_malloc(WINDOWSTEP*2 * sizeof(float)); 
  spectrogram = (uint8_t*)ps_malloc(WINDOWNUM*MELNUM*2 * sizeof(float));
  xQueue_waveform = xQueueCreateStatic(WINDOWSTEP*2, sizeof(float), waveform, &waveform_QueueControlBlock);
  xQueue_spectrogram = xQueueCreateStatic(WINDOWNUM*MELNUM*2, sizeof(float), spectrogram, &spectrogram_QueueControlBlock);
  if(xQueue_waveform != NULL){
    Serial.println("waveform 队列创建成功");
  }
  else{
    Serial.println("waveform 队列创建失败");
  }
  if(xQueue_spectrogram != NULL){
    Serial.println("spectrogram 队列创建成功");
  }
  else{
    Serial.println("spectrogram 队列创建失败");
  }

  // 任务创建
  xTaskCreatePinnedToCore(Task1, "iis mic", 1024*10, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(Task2, "FFT", 1024*10, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(Task3, "modle inference", 1024*6, NULL, 1, NULL, 0);

  // 结束setloop 任务，减少资源消耗
  vTaskDelay(1000);
  vTaskDelete(NULL);
}


void loop() {
}

void print_message(){
  Serial.println("\nspectrogram infomation:");
  Serial.printf("window size = %d\n", WINDOWSIZE);
  Serial.printf("window step = %d\n", WINDOWSTEP);
  Serial.printf("step number = %d\n", WINDOWNUM);
  Serial.printf("spectrogram size = %d\n", WINDOWNUM*MELNUM);
}