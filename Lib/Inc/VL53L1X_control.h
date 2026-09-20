#ifndef VL53L1X_CONTROL_H
#define VL53L1X_CONTROL_H

#include "VL53L1X_api.h"
#include "main.h"  // 必须引入，用于自动获取 CubeMX 生成的 TOF_X_SHUT 宏定义

// 重新定义 8 位 I2C 目标地址
#define VL53L1X_ADDR_R       (0x30 << 1)  // 右方传感器
#define VL53L1X_ADDR_M       (0x32 << 1)  // 中间(前方)传感器
#define VL53L1X_ADDR_L       (0x34 << 1)  // 左方传感器
#define VL53L1X_DEFAULT_ADDR (0x29 << 1)  // 芯片出厂默认地址

// 全新的 R M L 位置枚举
typedef enum {
    VL53L1X_DIR_M = 0,  // Middle 中间(前方)
    VL53L1X_DIR_L,      // Left   左方
    VL53L1X_DIR_R       // Right  右方
} VL53L1X_Direction;

// 初始化多传感器系统
void VL53L1X_multiInit(void);

// 高鲁棒性非阻塞自愈式轮询
void VL53L1X_multiPoll(void);

// 获取指定方向的距离数据（单位：mm）
uint16_t VL53L1X_getDistance(VL53L1X_Direction dir);

#endif /* VL53L1X_CONTROL_H */