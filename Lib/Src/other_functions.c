#include "other_functions.h"

/**
 * @brief 在屏幕右下角绘制旋转加载动画
 * @param refresh_ms 动画旋转的速度（毫秒/帧）
 */
void OF_runningSign(uint16_t refresh_ms) {
    // static uint32_t last_tick = 0;
    static uint8_t step = 0;
    const char frames[] = {'|', '|', '/', '/', '-', '-', '\\', '\\'}; // 旋转动画的4帧

    // 此方法弊端：造成明显频闪
    // 检查时间是否到了，避免频繁刷新导致总线过载
    // if (HAL_GetTick() - last_tick < refresh_ms) {
    //     return;
    // }
    // last_tick = HAL_GetTick();

    // 1. 绘制当前帧
    OLED_PrintASCIIChar(121, 57, frames[step], &afont8x6, OLED_COLOR_NORMAL);
    // OLED_Print_u8_Num(1, 5, &step, &afont8x6, OLED_COLOR_NORMAL); // 不会超出int8长度
    
    // 2. 更新帧索引
    step = (step + 1) % 8;
}

/**
 * @brief 将SBUS通道值映射到目标范围，支持负值传递
 * @param sbus_value SBUS原始值（范围353-1695）
 * @param target_min 目标范围最小值
 * @param target_max 目标范围最大值
 * @return double 映射后的目标值
 * @details 中间值1024将映射到目标范围的中点
 */
double OF_mapToRange(double target_min, double target_max, double sbus_value) {
    // SBUS原始范围参数
    const int16_t SBUS_MIN = 353;
    const int16_t SBUS_MAX = 1695;
    // const int16_t SBUS_MID = 1024;
    
    // 限制输入值在有效范围内
    if (sbus_value < SBUS_MIN) sbus_value = SBUS_MIN;
    if (sbus_value > SBUS_MAX) sbus_value = SBUS_MAX;
    
    return (sbus_value - SBUS_MIN) * (target_max - target_min) / (SBUS_MAX - SBUS_MIN) + target_min;
    // // 计算归一化比例（0-1）
    // double normalized = (double)(sbus_value - SBUS_MIN) / (SBUS_MAX - SBUS_MIN);
    // // 映射到目标范围（支持正负范围）
    // return target_min + normalized * (target_max - target_min);
}

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
char OF_getDirectionChar(double fuzzy_result) {
    if (fuzzy_result < -2.0) {
        return '-';
    } else if (fuzzy_result < -1.0) {
        return '\\';
    } else if (fuzzy_result < 1.0) {
        return '|';
    } else if (fuzzy_result < 2.0) {
        return '/';
    } else {
        return '-';
    }
}