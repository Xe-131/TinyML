#include "Task2.h"
#include "Task1.h"
#include "common.h"

// 初始化模型数组
#include "MODEL_QUANT_TFLITE_8000.h"

// 时域信号(队列)
StaticQueue_t waveform_QueueControlBlock;
QueueHandle_t xQueue_waveform;
uint8_t* waveform = NULL;

// 频域信号 (队列)
StaticQueue_t spectrogram_QueueControlBlock;
QueueHandle_t xQueue_spectrogram;
uint8_t* spectrogram = NULL;



// 运行时间
unsigned int time_os;

// 临时 
int temp = 0;

// 保留的频率索引
int mel_index[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
 22, 23, 24, 25, 27, 28, 30, 31, 33, 34, 36, 38, 40, 41, 43, 45, 47, 50, 52, 54, 56, 59, 61, 63,
  66, 69, 71, 74, 77, 80, 83, 86, 90, 93, 96, 100, 104, 107, 111, 115, 120, 124};

void setup() {
  // 串口
  Serial.begin(115200);
  // 打印频谱图信息
  Serial.println("\nspectrogram infomation:");
  Serial.printf("window size = %d\n", WINDOWSIZE);
  Serial.printf("window step = %d\n", WINDOWSTEP);
  Serial.printf("step number = %d\n", WINDOWNUM);
  Serial.printf("freqence number = %d\n", FREQENCENUM);
  Serial.printf("spectrogram size = %d\n", WINDOWNUM*MELNUM);


  // 全局变量的内存分配
  waveform = (uint8_t*)ps_malloc(WINDOWSTEP*2 * sizeof(float)); // 时域信号(队列)
  spectrogram = (uint8_t*)ps_malloc(WINDOWNUM*MELNUM*2 * sizeof(float)); // 频域信号（变量）
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


  // xTaskCreatePinnedToCore(Task1, "iis mic", 1024*10, NULL, 1, NULL, 1);
  // vTaskDelay(30); // 先让mic 采集数据
  xTaskCreatePinnedToCore(Task2, "FFT", 1024*10, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(Task3, "modle inference", 1024*6, NULL, 1, NULL, 0);

  // 结束setloop 任务，减少资源消耗
  vTaskDelay(1000);
  vTaskDelete(NULL);
}



// /* 将from 数组左移，丢弃最左边的数据，最右边更新新的数据*/
// void buffer_update(double* from, float* to, int from_size, int to_size, SemaphoreHandle_t* key){
//   if(from_size >= to_size){
//     Serial.println("buffer_update went wrong !");
//     return;
//   }

//   // 转移自己内存
//   for(int i = from_size; i < to_size; i++){
//     to[i-from_size] = to[i];
//   }
//   // 加入新的数据(公共区)
//   if(xSemaphoreTake(*key, timeout) == pdPASS){
//     while(1){
//       to[to_size-1] = (float)from[from_size-1];
//       to_size--;
//       from_size--;
//       if(from_size == 0){
//         break;
//       }
//     }
//     // 释放钥匙
//     xSemaphoreGive(*key);
//   }
//   else{
//     Serial.println("buffer_update 未获得钥匙");
//   }

// }

// void buffer_shift(QueueHandle_t queue, float* buffer, int shift_size, int buffer_size, int queue_timeout){
//   if(shift_size >= buffer_size){
//     Serial.println("buffer_shift went wrong !");
//     return;
//   }
  
//   // 转移自己内存
//   for(int i = shift_size; i < buffer_size; i++){
//     buffer[i-shift_size] = buffer[i];
//   }  

//   // 从队列中添加新的数据
//   while(1){
//     if(xQueueReceive(queue, (buffer+buffer_size-1), queue_timeout) == pdPASS){
//     }
//     else{
//       Serial.printf("buffer shift 队列已空超过 %d ms\n", queue_timeout);
//     }

//     buffer_size--;
//     shift_size--;
//     if(shift_size == 0){
//       break;
//     }
    
//   }
// }

void loop() {
}