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

/**
 * @brief 根据模糊控制结果获取方向指示字符
 * @param fuzzy_result 模糊控制结果数值
 * @return char 方向字符：'\'、'|'、'/'、'-'
 * @details 根据数值范围返回对应的方向符号：
 *          < -2:  '-' (向左水平)
 *          -2 ~ -1: '\' (向左下)
 *          -1 ~ 1:  '|' (垂直)
 *          1 ~ 2:   '/' (向右下)
 *          > 2:     '-' (向右水平)
 */
char OF_getDirectionChar(double fuzzy_result);

#endif