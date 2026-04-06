#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#include "main.h"

// 超声波距离结构体：封装三个传感器的距离数据
typedef struct {
    float left;   // 左侧传感器距离 (cm)
    float middle; // 中间传感器距离 (cm)
    float right;  // 右侧传感器距离 (cm)
} Ultrasonic_Distance_t;

// 全局缓存变量
extern Ultrasonic_Distance_t ultrasonic_distances; // 缓存的距离数据
extern volatile uint8_t ultrasonic_data_ready;     // 数据是否准备好标志

// 模块接口函数
void Ultrasonic_Init(void);                                    // 初始化超声波模块
void Ultrasonic_StartRead(void);                               // 启动异步距离读取
uint8_t Ultrasonic_IsReadComplete(void);                       // 检查读取是否完成
void Ultrasonic_GetDistances(Ultrasonic_Distance_t *distances); // 获取缓存的距离数据
void Ultrasonic_Process(void);                                 // 异步读取状态机处理（在主循环中调用）

#endif
