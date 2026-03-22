#include "main_task.h"

uint16_t sbus_channels[6]={0}; // SBUS接收的11位通道值数组(0-2047)
CPGState state;
double dt = 0.01; // CPG时间步长
SERVO_ID servo_ids[3] = {SERVO_1, SERVO_2, SERVO_3};

// 初始化
void mainTaskInit(void)
{
    OLED_kk_Init();
    Servo_Init();
    CPG_Init(&state);

    // 启动UART1中断接收
    extern uint8_t sbus_rx_dummy_byte;
    HAL_UART_Receive_IT(&huart1, &sbus_rx_dummy_byte, 1);
}

// 主程序
void mainTask(void)
{
    OLED_NewFrame(); // 清除帧缓存
    // 检查SBUS帧是否就绪
    if(sbus_frame_ready){                               // SBUS帧接收完毕且数据有效
        sbus_frame_ready = 0;                           // 清除就绪标志
        sbus_decode_channels(sbus_channels);   // 解码SBUS帧为sbus_channels数组
        sbus_clear_frame_buffer();                      // 清空SBUS缓冲区

        // 更新CPG状态：根据接收到的通道值控制CPG参数
        set_cpg_frequency(&state, sbus_channels[2]);    // CH3控制速度
        set_cpg_bias(&state, sbus_channels[3]);         // CH4控制偏置
        // CPG更新舵机角度数据
        cpg_update(&state, dt);

        // 获取舵机角度数据
        double servo_angles[3];
        for (int i = 0; i < 3; i++) {
            servo_angles[i] = map_to_servo(state.y[i]);
        }

        // 显示舵机角度（取整显示），并设置舵机角度
        for (int i = 0; i < 3; i++) {
            // 舵机角度数据
            char servo_label[5];
            sprintf(servo_label, "S%d:", i + 1);
            OLED_PrintASCIIString(1, i + 1, servo_label, &afont8x6, OLED_COLOR_NORMAL);
            OLED_PrintFloat(4, i + 1, servo_angles[i], 0, &afont8x6, OLED_COLOR_NORMAL);
            printf("Servo ID: %d\n", servo_ids[i]);
            Servo_SetAngle(servo_ids[i], servo_angles[i]);             // 分别设定3个水底舵机
        }
        Servo_SetAngle(SERVO_4, map_to_range(55, 145, sbus_channels[1]));     // CH2控制侧鳍舵机角度
        // 速度、偏置、侧鳍 数据
        OLED_PrintASCIIString(8, 1, "a:", &afont8x6, OLED_COLOR_NORMAL);
        OLED_PrintFloat(10, 1, state.omega[0], 1, &afont8x6, OLED_COLOR_NORMAL);            // 显示浮点数，保留1位小数
        OLED_PrintASCIIString(8, 2, "b:", &afont8x6, OLED_COLOR_NORMAL);
        OLED_PrintFloat(10, 2, state.bias[0], 1, &afont8x6, OLED_COLOR_NORMAL);             // 显示浮点数，保留1位小数
        OLED_PrintASCIIString(8, 3, "c:", &afont8x6, OLED_COLOR_NORMAL);
        OLED_PrintFloat(10, 3, map_to_range(55, 145, sbus_channels[1]), 1, &afont8x6, OLED_COLOR_NORMAL);           // 侧鳍网络控制角度
    } else {
        
    }
    OLED_ShowFrame(); // 显示帧
}
