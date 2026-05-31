#include "main_task.h"
// -------------------------------------------------

// static 声明的变量/函数只能在当前文件中使用，不能在其他文件中访问
// extern 告诉编译器“这个变量/函数在其他文件中定义，当前文件只是引用它”，因此编译器不会为其分配内存空间，而是在链接阶段去其他文件中寻找其实际定义。

// SBUS 
static uint16_t sbus_channels[6]={0}; // SBUS接收的11位通道值数组(0-2047)
extern uint8_t g_sbus_frame_ready;    // SBUS帧就绪标志（单帧接收完毕）
// CPG 
static CPG_State cpg_State;           // CPG状态结构体，包含速度、偏置、角度等参数
static double dt = 0.01;              // CPG单位时间步长，用于计算每单位时间步长下的CPG数据变化
// 舵机 
static SERVO_ID servo_ids[3] = {SERVO_1, SERVO_2, SERVO_3}; // 舵机ID数组
static double servo_angles[3];        // 舵机角度数组
// VL53L1X 传感器距离数据（单位：mm）
static uint16_t g_tof_dist_front = 0;
static uint16_t g_tof_dist_left = 0;
static uint16_t g_tof_dist_right = 0;
// 模糊控制结果数据
extern float g_fuzzy_result;

// -------------------------------------------------

// 初始化
void mainTaskInit(void)
{
    HAL_Delay(100); // 延时100ms避免屏幕未通电
    OLED_init();
    Servo_init();
    CPG_init(&cpg_State);
    SBUS_init();
    // 初始化 VL53L1X 多传感器系统
    VL53L1X_multiInit();
    // HAL_TIM_Base_Start(&htim2);
    HAL_Delay(100);
}

// -------------------------------------------------
// 主程序

// 主任务
void mainTask(void)
{
    // SBUS帧已就绪，人工接管
    if(g_sbus_frame_ready){       // SBUS帧接收完毕且数据有效
        g_sbus_frame_ready = 0;   // 清除就绪标志
        // 处理遥控器数据
        remoteControl();
        // 显示遥控器数据
        remoteControlDataDisplay();
    } else {
        // 无遥控器数据，切换到模糊控制模式
        fuzzyControl();
        // 显示模糊控制数据
        fuzzyControlDataDisplay();
    }
    OF_runningSign(1000); // 运行状态指示动画
    OLED_showFrame(); // 显示帧
    OLED_newFrame(); // 清除帧缓存
}

// -------------------------------------------------
// 主功能函数

// 处理遥控器数据
void remoteControl(void) {
    //SBUS数据处理
    SBUS_decodeChannels(sbus_channels); // 解码SBUS帧为sbus_channels数组
    SBUS_clearFrameBuffer();                      // 清空SBUS缓冲区
    
    // 将接收到的遥控器原始数据(各通道值)参数 传入cpg_State的相关参数
    CPG_setFrequency(&cpg_State, sbus_channels[2]);         // CH3控制速度
    CPG_setBias(&cpg_State, sbus_channels[3], true);        // CH4控制偏置
    // 根据cpg_State现有的所有参数随时间步长dt计算更新一次CPG数据。把cpg_State地址传入函数，新的数据将直接写入cpg_State
    CPG_update(&cpg_State, dt);
    
    // 从cpg_State提取CPG最终角度数据，然后映射为舵机角度
    for (int i = 0; i < 3; i++) {
        // servo_angles[i] = CPG_mapAngleToServo(cpg_State.y[i]);
        Servo_setAngle(servo_ids[i], CPG_mapAngleToServo(cpg_State.y[i]));
    }
    // 应用舵机角度
    for (int i = 0; i < 3; i++) {
        // Servo_setAngle(servo_ids[i], servo_angles[i]); // 分别设定3个舵机
    }
    Servo_setAngle(SERVO_4, OF_mapToRange(55, 145, sbus_channels[1])); // CH2控制腹鳍舵机角度
}

// 执行模糊控制
void fuzzyControl(void) {
    // 测距数据获取
    // 轮询检查 VL53L1X 传感器数据 非阻塞
    VL53L1X_multiPoll();
    
    // 从 R M L 接口获取数据
    g_tof_dist_front = VL53L1X_getDistance(VL53L1X_DIR_M); // Middle 对应前
    g_tof_dist_left  = VL53L1X_getDistance(VL53L1X_DIR_L); // Left 对应左
    g_tof_dist_right = VL53L1X_getDistance(VL53L1X_DIR_R); // Right 对应右

    // 模糊控制 更新 & CPG 更新
    CPG_setFrequency(&cpg_State, 1400);    // 默认速度
    CPG_setBias(&cpg_State, Fuzzy_update(g_tof_dist_left / 10.0f, g_tof_dist_front / 10.0f, g_tof_dist_right / 10.0f), false);
    // 根据cpg_State现有的所有参数随时间步长dt计算更新一次CPG数据。把cpg_State地址传入函数，新的数据将直接写入cpg_State
    CPG_update(&cpg_State, dt);
    
    // 应用 CPG 数据 到舵机
    // 从cpg_State提取CPG最终角度数据，然后映射为舵机角度
    for (int i = 0; i < 3; i++) {
        servo_angles[i] = CPG_mapAngleToServo(cpg_State.y[i]);
    }
    // 应用舵机角度
    for (int i = 0; i < 3; i++) {
        Servo_setAngle(servo_ids[i], servo_angles[i]); // 分别设定3个舵机
    }
}

// -------------------------------------------------
// 调试信息与数据显示

// 参数显示
void remoteControlDataDisplay(void) {
    // 控制模式显示
    OLED_printString(1, 8, "Remote Control mode", &afont8x6, OLED_COLOR_NORMAL);


    // 显示舵机角度（取整显示）
    char servo_label[5];  // 四个舵机ID标签数组
    for (int i = 0; i < 3; i++) {
        // 舵机角度数据显示
        sprintf(servo_label, "S%d:", i + 1);
        OLED_printString(1, i + 1, servo_label, &afont8x6, OLED_COLOR_NORMAL);
        OLED_printFloat(4, i + 1, servo_angles[i], 0, &afont8x6, OLED_COLOR_NORMAL);
        printf("Servo ID: %d\n", servo_ids[i]);
    }
    // CPG速度、偏置 和 腹鳍角度 数据显示
    OLED_printString(8, 1, "a:", &afont8x6, OLED_COLOR_NORMAL);
    OLED_printFloat(10, 1, cpg_State.omega[0], 1, &afont8x6, OLED_COLOR_NORMAL);  // 显示浮点数，保留1位小数
    OLED_printString(8, 2, "b:", &afont8x6, OLED_COLOR_NORMAL);
    OLED_printFloat(10, 2, cpg_State.bias[0], 1, &afont8x6, OLED_COLOR_NORMAL);   // 显示浮点数，保留1位小数
    OLED_printString(8, 3, "c:", &afont8x6, OLED_COLOR_NORMAL);
    OLED_printFloat(10, 3, OF_mapToRange(55, 145, sbus_channels[1]), 1, &afont8x6, OLED_COLOR_NORMAL); // 腹鳍网络控制角度
}

// 无遥控器数据提示
void fuzzyControlDataDisplay() {
    // 控制模式显示
    OLED_printString(1, 8, "Fuzzy Control mode", &afont8x6, OLED_COLOR_NORMAL);
    
    // 显示距离数据
    // 按 R(右)、M(前)、L(左) 顺序显示
    // 第一行：R（右方）
    OLED_printString(1, 1, "R:", &afont8x6, OLED_COLOR_NORMAL);
    OLED_printFloat(6, 1, g_tof_dist_right / 10.0, 1, &afont8x6, OLED_COLOR_NORMAL);
    OLED_printString(12, 1, "cm", &afont8x6, OLED_COLOR_NORMAL);
    // 第二行：M（前方/Middle）
    OLED_printString(1, 2, "M:", &afont8x6, OLED_COLOR_NORMAL);
    OLED_printFloat(6, 2, g_tof_dist_front / 10.0, 1, &afont8x6, OLED_COLOR_NORMAL);
    OLED_printString(12, 2, "cm", &afont8x6, OLED_COLOR_NORMAL);
    // 第三行：L（左方）
    OLED_printString(1, 3, "L:", &afont8x6, OLED_COLOR_NORMAL);
    OLED_printFloat(6, 3, g_tof_dist_left / 10.0, 1, &afont8x6, OLED_COLOR_NORMAL);
    OLED_printString(12, 3, "cm", &afont8x6, OLED_COLOR_NORMAL);
    
    // 显示模糊控制结果
    OLED_printFloat(1, 4, cpg_State.bias[0], 3, &afont8x6, OLED_COLOR_NORMAL);
    OLED_printFloat(8, 4, g_fuzzy_result, 5, &afont8x6, OLED_COLOR_NORMAL);
    OLED_PrintASCIIChar(9*6, 5*8, OF_getDirectionChar(g_fuzzy_result), &afont8x6, OLED_COLOR_NORMAL);

}
