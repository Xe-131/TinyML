// 需要先包含Arduino.h 这里面包括了freeRTos 以及一些基本类型定义
#include <Arduino.h> 
#include "common.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"


#define MODEL_QUANT_TFLITE_8000_LEN 1632408

// 没有名字的命名空间，限制以下这些变量只能在本文件中使用
namespace {
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;
int inference_count = 0;
// 这个量决定了模型能不能跑起来，分配在内部RAM
constexpr int kTensorArenaSize = 100000;
uint8_t tensor_arena[kTensorArenaSize];
} 

float* input_buffer = NULL;

void Task3(void* parameters){
  Serial.printf("\n\n\nTask3 running in %d\n", xPortGetCoreID());


  // // 获取 PSRAM 的大小（以字节为单位）
  // Serial.printf("PSRAM Size: %u bytes\n", ESP.getPsramSize());
  // // 将模型从FLASH 中加载到PSRAM
  // unsigned char* MODEL_QUANT_TFLITE_8000_PSRAM = (unsigned char*)ps_malloc(MODEL_QUANT_TFLITE_8000_LEN*sizeof(unsigned char));
  // for(int i = 0; i < MODEL_QUANT_TFLITE_8000_LEN; i++){
  //   MODEL_QUANT_TFLITE_8000_PSRAM[i] = MODEL_QUANT_TFLITE_8000_FLASH_1[i];
  // }
  // // 获取 PSRAM 剩余内存
  // Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
  
  // 从FLASH 中构建模型
  model = tflite::GetModel(MODEL_QUANT_TFLITE_8000_FLASH_1);
  if(model->version() != TFLITE_SCHEMA_VERSION){
    Serial.printf("\n\nModel provided is schema version %d not equal to supported version %d.\n\n", model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  // 创建resolver : 如何根据模型的结构添加合适的操作？
  static tflite::MicroMutableOpResolver<7> resolver;
  // if(resolver.AddDepthwiseConv2D() != kTfLiteOk){
  //   Serial.println("resolver.AddDepthwiseConv2D() != KTfLiteOK");
  //   return;
  // }
  if(resolver.AddFullyConnected() != kTfLiteOk){
    Serial.println("resolver.AddFullyConnected() != KTfLiteOK");
    return;    
  }
  // if(resolver.AddSoftmax() != kTfLiteOk){
  //   Serial.println("resolver.AddSoftmax() != KTfLiteOK");
  //   return;
  // }
  if(resolver.AddReshape() != kTfLiteOk){
    Serial.println("resolver.AddReshape() != KTfLiteOK");
    return;    
  }
  if (resolver.AddQuantize() != kTfLiteOk) {  // 添加QUANTIZE操作
    Serial.println("resolver.AddQuantize() != KTfLiteOK");
    return;    
  }
  if (resolver.AddResizeBilinear() != kTfLiteOk) {  // 添加RESIZE_BILINEAR操作
      Serial.println("resolver.AddResizeBilinear() != KTfLiteOK");
      return;    
  }
  if (resolver.AddConv2D() != kTfLiteOk) {  // 添加CONV_2D操作
      Serial.println("resolver.AddConv2D() != KTfLiteOK");
      return;
  }
  if (resolver.AddMaxPool2D() != kTfLiteOk) {  // 添加MAX_POOL_2D操作
    Serial.println("resolver.AddMaxPool2D() != KTfLiteOK");
    return;
  }
  if (resolver.AddDequantize() != kTfLiteOk) {
      Serial.println("resolver.AddDequantize() != KTfLiteOK");
      return;
  }
  // 创建interpreter
  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;
  // 分配张量内存
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if(allocate_status != kTfLiteOk){
    Serial.println("AllocateTensors() failed");
    return;
  }
  // 获取输入输出对象
  input = interpreter->input(0);
  output = interpreter->output(0);
  // 打印输入张量信息
  Serial.printf("\n input->dims->size = %d \n", input->dims->size);
  Serial.printf("\nthe model input shape is (%d, %d, %d, %d)\n",input->dims->data[0], input->dims->data[1], input->dims->data[2], input->dims->data[3]);
  //输入数据类型取决于训练所用输入（量化中的uin8 指的是权重)
  if(input->type != kTfLiteFloat32){
    Serial.println("input->type != kTfLiteFloat32");
  }
  // 确认是.f ???
  input_buffer = input->data.f;

  Serial.println("\n------------------------模型初始化完成------------------------\n\n");

  // 频谱图缓冲区
  float* spectrogram_buffer;
  spectrogram_buffer = (float*)ps_malloc(WINDOWNUM*MELNUM * sizeof(float));
  
  while(1){
    // 向左移位
    for(int i = 0; i <= (WINDOWNUM-1)*MELNUM-1; i++){
      spectrogram_buffer[i] = spectrogram_buffer[i+MELNUM];
    }
    // 从队列读一个mel_num 的数据
    for(int i = (WINDOWNUM-1)*MELNUM; i < WINDOWNUM*MELNUM; i++){
      if(xQueueReceive(xQueue_spectrogram, spectrogram_buffer + i, spectrogram_queue_timeout) == pdPASS){
      }
      else{
        Serial.printf("waveform 队列已满超过 %d ms\n", spectrogram_queue_timeout);
      }
    }
    // 输入
    for(int i = 0; i < WINDOWNUM*MELNUM; i++){
      input->data.f[i] = spectrogram_buffer[i];
    }

    // 测试这边收到的频谱图与隔壁发送的频谱图是否一致
//     static int j = -1;
// Serial.println("-----------------");
//     for(int i = (WINDOWNUM-1)*MELNUM; i < WINDOWNUM*MELNUM; i++){
//       Serial.printf("j = %d i = %d  %6f\n", j, i-3840, spectrogram_buffer[i]);
//     }
//     j++;
// Serial.println("-----------------");
//     vTaskDelay(1000);


    // // 测试直接频谱图输入
    // static int time = 1;
    // static float* temp_p = NULL;
    // if(time == 1){
    //   temp_p = (float*)cat;
    //   time++;
    // }
    // else if(time == 2){
    //   temp_p = (float*)bed;
    //   time++;
    // }
    // else if(time == 3){
    //   temp_p = (float*)dog;
    //   time++;
    // }
    // else if(time == 4){
    //   temp_p = (float*)bird;
    //   time = 1;
    // }
    // for(int i = 0; i < 3904; i++){
    //   input->data.f[i] = temp_p[i];
    // }

    // 推断
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      Serial.println("Invoke failed");
      return;
    }

    // 获取输出形状
    // Serial.printf("\nthe output->dims->size = %d", output->dims->size);
    // Serial.printf("\n output shape is (%d, %d) \n", output->dims->data[0], output->dims->data[1]);
    // if(output->type != kTfLiteFloat32){
    //   Serial.println("output->type != kTfLiteFloat32");
    // }
    


    // 打印输出
    float max_value = -1000000;
    int max_index = 0;
    for (int i = 0; i < 4; i++) {
      if (output->data.f[i] > max_value) {
          max_value = output->data.f[i];
          max_index = i;
      }
    }
    if(max_index == 0){
      Serial.println("bed");
    }
    else if(max_index == 1){
      Serial.println("bird");
    }
    else if(max_index == 2){
      Serial.println("cat");
    }
    else if(max_index == 3){
      Serial.println("dog");
    }
    Serial.printf("%6f %6f %6f %6f\n", output->data.f[0], output->data.f[1], output->data.f[2], output->data.f[3]);

    // 喂狗
    vTaskDelay(1);
  }

}

