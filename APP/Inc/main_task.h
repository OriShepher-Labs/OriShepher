#ifndef __MAIN_TASK_H__
#define __MAIN_TASK_H__

// C 标准库
#include <math.h>
#include <stdio.h>
#include <string.h>

// STM32硬件抽象与底层配置库 HAL Layer
#include "main.h"
#include "stm32f1xx_hal.h"
#include "usart.h"

// 用户自定义与功能逻辑库 (User Application Layer)
#include "oled.h"
#include "cpg_algorithm.h"
#include "servo_control.h"
#include "SBUS.h"
#include "other_functions.h"

void mainTaskInit(void);
void mainTask(void);

static void remoteDataProcess(void);
static void screenDataDisplay(void);
static void screenNoDataDisplay(void);


#endif
