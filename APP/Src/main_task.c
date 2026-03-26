#include "main_task.h"
#include "font_kk_simple.h"
#include "oled_kk_simple.h"
#include "stm32f1xx_hal.h"
#include "useless_functions.h"

uint16_t sbus_channels[6]={0}; // SBUS接收的11位通道值数组(0-2047)
CPG_State cpg_State;
double dt = 0.01; // CPG时间步长
SERVO_ID servo_ids[3] = {SERVO_1, SERVO_2, SERVO_3};

// 初始化
void mainTaskInit(void)
{
    // 延时100ms避免屏幕未通电
    HAL_Delay(100);
    OLED_kk_Init();
    Servo_Init();
    CPG_init(&cpg_State);

    // 启动UART1中断接收 （遥控器）
    extern uint8_t sbus_rx_dummy_byte; // extern 告诉编译器“这个变量/函数在其他文件中定义，当前文件只是引用它”，因此编译器不会为其分配内存空间，而是在链接阶段去其他文件中寻找其实际定义。
    HAL_UART_Receive_IT(&huart1, &sbus_rx_dummy_byte, 1);
    HAL_Delay(100);
}

// 主程序
void mainTask(void)
{
    // 检查SBUS帧是否就绪
    if(sbus_frame_ready){                               // SBUS帧接收完毕且数据有效
        sbus_frame_ready = 0;                           // 清除就绪标志
        sbus_decode_channels(sbus_channels);   // 解码SBUS帧为sbus_channels数组
        sbus_clear_frame_buffer();                      // 清空SBUS缓冲区
        
        // 更新CPG状态：根据接收到的通道值控制CPG参数
        CPG_setFrequency(&cpg_State, sbus_channels[2]);    // CH3控制速度
        CPG_setBias(&cpg_State, sbus_channels[3]);         // CH4控制偏置
        // CPG更新舵机角度数据
        CPG_update(&cpg_State, dt);
        
        // 获取舵机角度数据
        double servo_angles[3];
        for (int i = 0; i < 3; i++) {
            servo_angles[i] = CPG_mapAngleToServo(cpg_State.y[i]);
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
        Servo_SetAngle(SERVO_4, mapToRange(55, 145, sbus_channels[1]));     // CH2控制侧鳍舵机角度
        // 速度、偏置、侧鳍 数据
        OLED_PrintASCIIString(8, 1, "a:", &afont8x6, OLED_COLOR_NORMAL);
        OLED_PrintFloat(10, 1, cpg_State.omega[0], 1, &afont8x6, OLED_COLOR_NORMAL);            // 显示浮点数，保留1位小数
        OLED_PrintASCIIString(8, 2, "b:", &afont8x6, OLED_COLOR_NORMAL);
        OLED_PrintFloat(10, 2, cpg_State.bias[0], 1, &afont8x6, OLED_COLOR_NORMAL);             // 显示浮点数，保留1位小数
        OLED_PrintASCIIString(8, 3, "c:", &afont8x6, OLED_COLOR_NORMAL);
        OLED_PrintFloat(10, 3, mapToRange(55, 145, sbus_channels[1]), 1, &afont8x6, OLED_COLOR_NORMAL);           // 侧鳍网络控制角度
    } else {
        // // SBUS帧未就绪，显示等待信息
        OLED_PrintASCIIString(6, 2, "Waiting for", &afont12x6, OLED_COLOR_NORMAL);
        OLED_PrintASCIIString(4, 5, "SBUS signal", &afont16x8, OLED_COLOR_NORMAL);
    }
    running_sign(1000); // 运行状态指示动画
    OLED_ShowFrame(); // 显示帧
    OLED_NewFrame(); // 清除帧缓存
}
