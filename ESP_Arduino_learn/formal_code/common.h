/* 此文件仅存放主文件的全局变量声明和宏定义*/
/* 以及Task 函数的声明*/
#ifndef _COMMON_H_
#define _COMMON_H_

// 采样率，用于初始化mic 和FFT
#define SAMPLERATE 8000
// 频谱图样本数 = WINDOWSIZE + (STEPNUM-1) * WINDOWSTEP
// 一次FFT 的样本数，必须是2 的幂，一次FFT 的数据量 
#define WINDOWSIZE 256   
// 窗口滑动步长，wave 队列大小/2
#define WINDOWSTEP 128  // 步长
// 总窗个数，用于计算spectrogram 大小
#define WINDOWNUM 61   
// 用上频率个数，压缩每列频谱图，计算spectrogram 大小
#define MELNUM 64

// 时域信号(队列)
extern StaticQueue_t waveform_QueueControlBlock;
extern QueueHandle_t xQueue_waveform;
extern uint8_t* waveform;
#define wave_queue_timeout 1000

// 频域信号 (队列)
extern StaticQueue_t spectrogram_QueueControlBlock;
extern QueueHandle_t xQueue_spectrogram;
extern uint8_t* spectrogram;
#define spectrogram_queue_timeout 1000

// Task
void Task2(void* parameters);
void Task1(void* parameters);
void Task3(void* parameters);

// 开启每个任务的时间测试
#define TIME_TEST 1

#endif
