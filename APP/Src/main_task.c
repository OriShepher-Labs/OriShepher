#include "main_task.h"
// #include "font.h"  // OLED字体库，已禁用
// #include "oled.h"   // OLED驱动，已禁用


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

// 距离数据
extern float dist_left, dist_middle, dist_right;
// 模糊控制结果数据
extern float g_fuzzy_result;

// -------------------------------------------------

// 初始化
void mainTaskInit(void)
{
    // HAL_Delay(100); // 延时100ms避免屏幕未通电 - OLED已禁用
    // OLED_init();     // OLED初始化，已禁用
    Servo_init();
    CPG_init(&cpg_State);
    SBUS_init();
    // HAL_Delay(100);
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
        remoteDataProcess();
        // 显示数据
        screenDataDisplay();
    } else {
        // 无遥控器数据，切换到模糊控制模式
        fuzzyControl();
        fuzzyControlModDisplay();
        // HAL_Delay(100);
    }
    // OF_runningSign(1000); // 运行状态指示动画 - OLED已禁用
    // OLED_showFrame(); // 显示帧 - OLED已禁用
    // OLED_newFrame(); // 清除帧缓存 - OLED已禁用
}

// -------------------------------------------------
// 主功能函数

// 处理遥控器数据
void remoteDataProcess(void) {
    //SBUS数据处理
    SBUS_decodeChannels(sbus_channels); // 解码SBUS帧为sbus_channels数组
    SBUS_clearFrameBuffer();                      // 清空SBUS缓冲区
    
    // CH5舵机锁检测：CH5为负（拨杆向前）时所有舵机归零、Servo_switch输出低电平；为正时Servo_switch输出高电平
    if (sbus_channels[4] < 1024) {
        for (int i = 0; i < 3; i++) {
            Servo_setAngle(servo_ids[i], 0.0f);
        }
        Servo_setAngle(SERVO_4, 0.0f);
        HAL_GPIO_WritePin(Servo_switch_GPIO_Port, Servo_switch_Pin, GPIO_PIN_RESET);
        return;     // 跳出函数，终止后续处理
    } else {
        HAL_GPIO_WritePin(Servo_switch_GPIO_Port, Servo_switch_Pin, GPIO_PIN_SET);
    }
    
    // CH6速度档位调节：正=全速(×1.0)，零=降一档(×0.8)，负=降两档(×0.6)
    double speed_multiplier;
    if (sbus_channels[5] > 1300) {
        speed_multiplier = 1.0;
    } else if (sbus_channels[5] > 700) {
        speed_multiplier = 0.8;
    } else {
        speed_multiplier = 0.6;
    }

    // 将接收到的遥控器原始数据(各通道值)参数 传入cpg_State
    CPG_setFrequency(&cpg_State, (double)sbus_channels[2] * speed_multiplier);    // CH3控制速度
    CPG_setBias(&cpg_State, sbus_channels[3], true);   // CH4控制偏置（need_mapping=true：原始通道值需映射）
    // 根据cpg_State的所有参数随时间步长dt计算更新一次CPG数据。把cpg_State地址传入函数，新的数据将直接写入cpg_State
    CPG_update(&cpg_State, dt);
    
    // 从cpg_State提取CPG最终角度数据，然后映射为舵机角度
    for (int i = 0; i < 3; i++) {
        servo_angles[i] = CPG_mapAngleToServo(cpg_State.y[i]);
    }
    // 应用舵机角度
    for (int i = 0; i < 3; i++) {
        Servo_setAngle(servo_ids[i], servo_angles[i] - 13); // 分别设定3个舵机
    }
    Servo_setAngle(SERVO_4, OF_mapToRange(55, 145, sbus_channels[1])); // CH2控制腹鳍舵机角度
}

// 执行模糊控制
void fuzzyControl(void) {
    // dist_left += randomDistanceGenerator(-2, 2);
    // dist_middle += randomDistanceGenerator(-2, 2);
    // dist_right += randomDistanceGenerator(-2, 2);

    if(dist_left < 150) dist_left += 1;
    if(dist_middle < 150 && dist_left > 149) dist_middle += 1;
    if(dist_right < 150 && dist_middle > 149 && dist_left > 149) dist_right += 1;

    if(dist_left > 150) dist_left = 150;
    if(dist_middle > 150) dist_middle = 150;
    if(dist_right > 150) dist_right = 150;
    if(dist_left < 30) dist_left = 30;
    if(dist_middle < 30) dist_middle = 30;
    if(dist_right < 30) dist_right = 30;

    CPG_setBias(&cpg_State, Fuzzy_update(dist_left, dist_middle, dist_right), false);
}

// -------------------------------------------------
// 调试信息与数据显示

// 参数显示
void screenDataDisplay(void) {
    // OLED显示已禁用，改用串口输出调试信息
    for (int i = 0; i < 3; i++) {
        printf("Servo ID: %d, Angle: %.1f\n", servo_ids[i], servo_angles[i]);
    }
    printf("CPG Omega: %.1f, Bias: %.1f\n", cpg_State.omega[0], cpg_State.bias[0]);
    printf("Ventral Fin Angle: %.1f\n", OF_mapToRange(55, 145, sbus_channels[1]));
}

// 无遥控器数据提示
void fuzzyControlModDisplay() {
    // OLED显示已禁用，改用串口输出调试信息
    printf("Fuzzy Control Mode\n");
    printf("Distance: L=%.0f, M=%.0f, R=%.0f\n", dist_left, dist_middle, dist_right);
    printf("CPG Bias: %.3f, Fuzzy Result: %.5f\n", cpg_State.bias[0], g_fuzzy_result);
}

// 距离数据随机生成
float randomDistanceGenerator(float min, float max) {
    int range = (int)(max - min);
    return min + (float)(rand() % (range + 1));
}