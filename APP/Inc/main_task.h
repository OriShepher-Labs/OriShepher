#ifndef __MAIN_TASK_H__
#define __MAIN_TASK_H__

// C 标准库
#include <math.h>           // 包含数学函数
#include <stdio.h>          // 包含输入输出函数
#include <string.h>         // 包含字符串函数
#include <stdlib.h>         // 包含随机数函数

// STM32硬件抽象与底层配置库 HAL Layer
#include "main.h"
#include "stm32f1xx_hal.h"  // 包含HAL库函数
#include "usart.h"          // 包含串口函数

// 用户自定义与功能逻辑库 (User Application Layer)
#include "oled.h"
#include "cpg_algorithm.h"
#include "fuzzy_control.h"
#include "servo_control.h"
#include "SBUS.h"
#include "other_functions.h"

void mainTaskInit(void);
void mainTask(void);

void remoteDataProcess(void);
void fuzzyControl(void);

void screenDataDisplay(void);
void fuzzyControlModDisplay(void);
float randomDistanceGenerator(float min, float max);
void fuzzyTestProcess(void);



#endif
