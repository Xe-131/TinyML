/* 此文件仅存放主文件的全局变量声明和宏定义*/

#ifndef _COMMON_H_
#define _COMMON_H_

#define SAMPLERATE 8000
// 频谱图样本数 = WINDOWSIZE + (STEPNUM-1) * WINDOWSTEP
#define WINDOWSIZE 128   // 一次FFT 的样本数，必须是2 的幂
#define WINDOWSTEP 64  // 步长
#define WINDOWNUM 124   // 总窗个数
#define FREQENCENUM 64 // 单窗的频率分量个数，WINDOWSIZE/2 为有效量，轴对称

// 时域信号(队列)
extern StaticQueue_t waveform_QueueControlBlock;
extern QueueHandle_t xQueue_waveform;
extern uint8_t* waveform;
#define wave_queue_timeout 1000

// 频域信号
extern unsigned int* spectrogram;

// 声明互斥锁
extern SemaphoreHandle_t xMutexInventory_1;
extern SemaphoreHandle_t xMutexInventory_2;
extern TickType_t timeout;

// 运行时间
extern unsigned int time_os;

// 临时 
extern int temp;

/* 将from 数组左移，丢弃最左边的数据，最右边更新新的数据*/
void buffer_update(double* from, unsigned int* to, int from_size, int to_size, SemaphoreHandle_t* key);
void buffer_shift(QueueHandle_t queue, int* buffer, int shift_size, int buffer_size, int queue_timeout);

#endif
