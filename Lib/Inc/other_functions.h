#ifndef __OTHER_FUNCTIONS_H__
#define __OTHER_FUNCTIONS_H__

#include "oled.h"

void OF_runningSign(uint16_t refresh_ms);

/**
 * @brief 将SBUS通道值映射到目标范围，支持负值传递
 * @param sbus_value SBUS原始值（范围353-1695）
 * @param target_min 目标范围最小值
 * @param target_max 目标范围最大值
 * @return double 映射后的目标值
 * @details 中间值1024将映射到目标范围的中点
 */
double OF_mapToRange(double target_min, double target_max, double sbus_value);

#endif