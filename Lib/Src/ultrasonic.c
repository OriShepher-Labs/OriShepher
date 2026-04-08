#include "ultrasonic.h"
#include "tim.h"  // CubeMX 生成的 TIM2 配置头文件

// 全局变量：存储三个传感器的距离值 (cm)
float dist1, dist2, dist3;

// 缓存距离数据
Ultrasonic_Distance_t ultrasonic_distances;
volatile uint8_t ultrasonic_data_ready = 0;

// 输入捕获相关变量：用于双边沿捕获
volatile uint32_t echo_start[3] = {0}; // 上升沿捕获时间 (us)
volatile uint32_t echo_end[3] = {0};   // 下降沿捕获时间 (us)
volatile uint8_t echo_captured[3] = {0}; // 捕获状态: 0=未开始, 1=上升沿捕获, 2=下降沿捕获完成

// 异步读取状态机变量
volatile uint8_t read_state = 0; // 0=idle, 1=trigger sensor 0, 2=wait sensor 0, 3=trigger sensor 1, 4=wait sensor 1, 5=trigger sensor 2, 6=wait sensor 2, 7=complete
uint32_t state_start_time = 0;

/**
 * @brief 微秒级延时函数
 * @param us 延时时间 (微秒)
 * @note STM32F1 无 DWT，使用简单循环实现。需要在 O0/O1 下测试校准。
 */
void delay_us(uint32_t us) {
    volatile uint32_t count = us * 8;
    while (count--) {
        __NOP();
    }
}

/**
 * @brief 触发单个超声波传感器
 * @param sensor 传感器编号 (0=左侧, 1=中间, 2=右侧)
 * @note 给 Trig 引脚发送 15us 高电平脉冲，触发超声波发射
 */
void Ultrasonic_triggerSensor(int sensor) {
    GPIO_TypeDef* port = GPIOA; // Trig 引脚都在 GPIOA
    uint16_t pin;
    switch (sensor) {
        case 0: pin = GPIO_PIN_3; break; // Trig_1 (PA3)
        case 1: pin = GPIO_PIN_4; break; // Trig_2 (PA4)
        case 2: pin = GPIO_PIN_5; break; // Trig_3 (PA5)
        default: return;
    }
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET); // 拉高 Trig
    delay_us(15);                               // 保持 15us
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); // 拉低 Trig
}

/**
 * @brief 超声波模块初始化
 * @note 启动 TIM2 输入捕获中断，设置初始极性为上升沿
 */
void Ultrasonic_Init(void) {
    // 启动 TIM2 的三个输入捕获通道 (对应 PA0/1/2)
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // 左侧传感器 Echo (PA0)
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2); // 中间传感器 Echo (PA1)
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3); // 右侧传感器 Echo (PA2)

    // 设置初始捕获极性为上升沿 (检测 Echo 高电平开始)
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);

    // 初始化状态
    read_state = 0;
    ultrasonic_data_ready = 0;
}

/**
 * @brief 启动异步距离读取过程
 * @note 非阻塞启动，实际读取在 Ultrasonic_Process() 中完成
 */
void Ultrasonic_StartRead(void) {
    if (read_state == 0) {
        // 重置捕获状态
        echo_captured[0] = echo_captured[1] = echo_captured[2] = 0;
        ultrasonic_data_ready = 0;
        read_state = 1; // 开始触发第一个传感器
        state_start_time = HAL_GetTick();
    }
}

/**
 * @brief 检查异步读取是否完成
 * @return 1=完成, 0=进行中
 */
uint8_t Ultrasonic_IsReadComplete(void) {
    return ultrasonic_data_ready;
}

/**
 * @brief 获取缓存的距离数据
 * @param distances 指向 Ultrasonic_Distance_t 结构体的指针
 * @note 只有在 Ultrasonic_IsReadComplete() 返回1时才调用
 */
void Ultrasonic_GetDistances(Ultrasonic_Distance_t *distances) {
    *distances = ultrasonic_distances;
}

/**
 * @brief 检查是否准备好执行下一次测距
 * @return 1=准备好（状态机在idle), 0=正在进行中
 * @note 当返回1时，可以安全地调用 Ultrasonic_StartRead() 启动新的一轮测距
 */
uint8_t Ultrasonic_IsReadyForNext(void) {
    return (read_state == 0) ? 1 : 0;
}

/**
 * @brief 异步读取状态机处理函数
 * @return 1=本次完成了一次完整的三传感器测距, 0=读取进行中
 * @note 在主循环中频繁调用此函数以推进读取过程
 *       当返回1时，表示一整套三个传感器的测距已完成，可以准备下一次测距
 */
uint8_t Ultrasonic_Process(void) {
    uint32_t current_time = HAL_GetTick();

    switch (read_state) {
        case 1: // 触发传感器0
            Ultrasonic_triggerSensor(0);
            read_state = 2;
            state_start_time = current_time;
            break;

        case 2: // 等待传感器0完成
            if (echo_captured[0] >= 2 || (current_time - state_start_time) > 50) {
                read_state = 3;
                state_start_time = current_time;
            }
            break;

        case 3: // 传感器0测量完成后等待 50ms 再触发传感器1
            if ((current_time - state_start_time) >= 50) {
                read_state = 4;
            }
            break;

        case 4: // 触发传感器1
            Ultrasonic_triggerSensor(1);
            read_state = 5;
            state_start_time = current_time;
            break;

        case 5: // 等待传感器1完成
            if (echo_captured[1] >= 2 || (current_time - state_start_time) > 50) {
                read_state = 6;
                state_start_time = current_time;
            }
            break;

        case 6: // 传感器1测量完成后等待 50ms 再触发传感器2
            if ((current_time - state_start_time) >= 50) {
                read_state = 7;
            }
            break;

        case 7: // 触发传感器2
            Ultrasonic_triggerSensor(2);
            read_state = 8;
            state_start_time = current_time;
            break;

        case 8: // 等待传感器2完成
            if (echo_captured[2] >= 2 || (current_time - state_start_time) > 50) {
                read_state = 9;
            }
            break;

        case 9: // 完成，填充缓存并标记为一次完整读取
            ultrasonic_distances.left = dist1;      // 左侧传感器距离
            ultrasonic_distances.middle = dist2;    // 中间传感器距离
            ultrasonic_distances.right = dist3;     // 右侧传感器距离
            ultrasonic_data_ready = 1;              // 标记数据已准备好
            read_state = 0;                         // 返回 idle 状态
            return 1;                               // 返回1表示本次已完成一次完整读取

        default:
            break;
    }
    
    return 0; // 返回0表示读取进行中
}

/**
 * @brief TIM2 输入捕获中断回调函数
 * @param htim TIM_HandleTypeDef 指针
 * @note 实现双边沿捕获：上升沿记录开始时间，下降沿记录结束时间，计算 Echo 高电平持续时间
 * @note 距离计算公式：距离(cm) = (时间差(us) * 0.034) / 2
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        // 读取当前捕获值 (TIM2 计数器值，单位 us)
        uint32_t capture = HAL_TIM_ReadCapturedValue(htim, htim->Channel);
        int sensor = -1; // 传感器编号

        // 根据通道确定传感器
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) sensor = 0; // 左侧 (PA0)
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) sensor = 1; // 中间 (PA1)
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) sensor = 2; // 右侧 (PA2)
        else return; // 无效通道

        if (echo_captured[sensor] == 0) {
            // 第一次捕获：上升沿，记录 Echo 高电平开始时间
            echo_start[sensor] = capture;
            // 切换捕获极性为下降沿，准备捕获结束时间
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, htim->Channel, TIM_INPUTCHANNELPOLARITY_FALLING);
            echo_captured[sensor] = 1; // 标记已捕获上升沿
        } else if (echo_captured[sensor] == 1) {
            // 第二次捕获：下降沿，记录 Echo 高电平结束时间
            echo_end[sensor] = capture;
            // 切换回上升沿，为下次测量做准备
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, htim->Channel, TIM_INPUTCHANNELPOLARITY_RISING);
            echo_captured[sensor] = 2; // 标记捕获完成

            // 计算 Echo 高电平持续时间 (us)
            uint32_t diff = echo_end[sensor] - echo_start[sensor];
            // 距离计算：声速 340m/s = 0.034cm/us，往返距离除以 2
            float distance = (diff * 0.034f) / 2.0f;

            // 存储到全局距离变量中
            if (sensor == 0) dist1 = distance;
            else if (sensor == 1) dist2 = distance;
            else if (sensor == 2) dist3 = distance;
        }
    }
}
