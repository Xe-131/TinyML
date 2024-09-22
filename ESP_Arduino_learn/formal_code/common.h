/* 此文件仅存放主文件的全局变量声明和宏定义*/
/* 以及Task 函数的声明*/
#ifndef _COMMON_H_
#define _COMMON_H_

#define SAMPLERATE 8000
// 频谱图样本数 = WINDOWSIZE + (STEPNUM-1) * WINDOWSTEP
#define WINDOWSIZE 256   // 一次FFT 的样本数，必须是2 的幂
#define WINDOWSTEP 128  // 步长
#define WINDOWNUM 61   // 总窗个数
#define FREQENCENUM 128 // 单窗的频率分量个数，WINDOWSIZE/2 为有效量，轴对称(后续还要进行梅尔优化)
#define MELNUM 64
extern int mel_index[MELNUM];

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


// 运行时间
extern unsigned int time_os;

// 临时 
extern int temp;

// Task
void Task2(void* parameters);
void Task1(void* parameters);
void Task3(void* parameters);



#endif
