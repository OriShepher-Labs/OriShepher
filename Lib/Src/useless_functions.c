#include "useless_functions.h"
#include "oled_kk_simple.h"

/**
 * @brief 在屏幕右下角绘制旋转加载动画
 * @param refresh_ms 动画旋转的速度（毫秒/帧）
 */
void running_sign(uint16_t refresh_ms) {
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