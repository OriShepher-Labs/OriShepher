/**
 * @file    VL53L1X_control.c
 * @brief   VL53L1X 多传感器控制模块
 * 
 * @note    三个 VL53L1X 传感器接线说明：
 * @note    ┌─────────────────────────────────────────────┐
 * @note    │ 传感器    方向    I2C_SCL   I2C_SDA   XSHUT │
 * @note    │ TOF_A      右方     PB6       PB7      PA0  │
 * @note    │ TOF_B      前方     PB6       PB7      PA1  │
 * @note    │ TOF_C      左方     PB6       PB7      PA2  │
 * @note    └─────────────────────────────────────────────┘
 * @note    - I2C 总线：三个传感器共用 I2C1（PB6/SCL, PB7/SDA）
 * @note    - XSHUT 引脚：独立控制，用于修改传感器地址
 * @note    - 默认地址：0x29(7位)，修改后分别为 A=0x30, B=0x32, C=0x34
 */

#include "VL53L1X_control.h"

// 内部静态测距数据存储
static uint16_t g_distance_m = 0;
static uint16_t g_distance_l = 0;
static uint16_t g_distance_r = 0;

// 为每个传感器建立独立的自愈时间戳（软件看门狗）
static uint32_t g_last_update_time[3] = {0};

// 地址与方向严格的数组映射
static const uint16_t g_sensor_addrs[3] = {
    VL53L1X_ADDR_M,  // [0] 中
    VL53L1X_ADDR_L,  // [1] 左
    VL53L1X_ADDR_R   // [2] 右
};

static const VL53L1X_Direction g_sensor_dirs[3] = {
    VL53L1X_DIR_M,
    VL53L1X_DIR_L,
    VL53L1X_DIR_R
};

/**
 * @brief  单路传感器的全套初始化配置流
 */
static int8_t VL53L1X_initSingleDevice(uint16_t target_addr) {
    uint8_t byteData = 0;
    uint16_t timeout = 0;

    // 等待芯片固件完成 Boot 引导
    while (byteData == 0) {
        VL53L1X_BootState(target_addr, &byteData);
        HAL_Delay(2);
        if (++timeout > 100) return -1; 
    }

    if (VL53L1X_SensorInit(target_addr) != 0) return -1;
    
    VL53L1X_SetDistanceMode(target_addr, 2);               // 长距离模式
    VL53L1X_SetTimingBudgetInMs(target_addr, 33);          // 33ms 曝光窗口
    VL53L1X_SetInterMeasurementInMs(target_addr, 33);
    
    if (VL53L1X_StartRanging(target_addr) != 0) return -1;

    return 0;
}

/**
 * @brief  初始化多传感器系统（基于 R M L 标签架构）
 */
void VL53L1X_multiInit(void) {
    // 1. 物理复位：利用 CubeMX 标签将所有传感器的 XSHUT 拉低
    HAL_GPIO_WritePin(TOF_R_SHUT_GPIO_Port, TOF_R_SHUT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TOF_M_SHUT_GPIO_Port, TOF_M_SHUT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TOF_L_SHUT_GPIO_Port, TOF_L_SHUT_Pin, GPIO_PIN_RESET);
    HAL_Delay(50); 

    // 2. 配置右方传感器 - R
    HAL_GPIO_WritePin(TOF_R_SHUT_GPIO_Port, TOF_R_SHUT_Pin, GPIO_PIN_SET);
    HAL_Delay(20);
    if (VL53L1X_SetI2CAddress(VL53L1X_DEFAULT_ADDR, VL53L1X_ADDR_R) == 0) {
        VL53L1X_initSingleDevice(VL53L1X_ADDR_R);
    }

    // 3. 配置中间(前方)传感器 - M
    HAL_GPIO_WritePin(TOF_M_SHUT_GPIO_Port, TOF_M_SHUT_Pin, GPIO_PIN_SET);
    HAL_Delay(20);
    if (VL53L1X_SetI2CAddress(VL53L1X_DEFAULT_ADDR, VL53L1X_ADDR_M) == 0) {
        VL53L1X_initSingleDevice(VL53L1X_ADDR_M);
    }

    // 4. 配置左方传感器 - L
    HAL_GPIO_WritePin(TOF_L_SHUT_GPIO_Port, TOF_L_SHUT_Pin, GPIO_PIN_SET);
    HAL_Delay(20);
    if (VL53L1X_SetI2CAddress(VL53L1X_DEFAULT_ADDR, VL53L1X_ADDR_L) == 0) {
        VL53L1X_initSingleDevice(VL53L1X_ADDR_L);
    }

    // 初始化时间戳，防止启动时误触发自愈
    uint32_t startup_tick = HAL_GetTick();
    g_last_update_time[0] = startup_tick;
    g_last_update_time[1] = startup_tick;
    g_last_update_time[2] = startup_tick;
}

/**
 * @brief  高鲁棒性、防死锁、带自愈能力的独立轮询扫描
 */
void VL53L1X_multiPoll(void) {
    uint8_t dataReady = 0;
    uint16_t temp_distance = 0;
    uint32_t current_time = HAL_GetTick();

    // 每一轮循环，独立检查所有 3 个传感器（非阻塞）
    for (uint8_t i = 0; i < 3; i++) {
        uint16_t current_addr = g_sensor_addrs[i];
        dataReady = 0;

        // 读取当前传感器的就绪状态
        VL53L1X_CheckForDataReady(current_addr, &dataReady);

        if (dataReady) {
            // 获取数据并清除中断
            if (VL53L1X_GetDistance(current_addr, &temp_distance) == 0) {
                VL53L1X_ClearInterrupt(current_addr);

                // 更新对应的距离变量
                switch (g_sensor_dirs[i]) {
                    case VL53L1X_DIR_M: g_distance_m = temp_distance; break;
                    case VL53L1X_DIR_L: g_distance_l = temp_distance; break;
                    case VL53L1X_DIR_R: g_distance_r = temp_distance; break;
                }
                // 成功刷新数据，重置该路传感器的“看门狗时间戳”
                g_last_update_time[i] = current_time; 
            }
        } 
        else {
            // 【自愈防挂死逻辑】
            // 如果连续 300ms 都没有读到该传感器的新数据，强行发送清除中断，重新“踢醒”传感器
            if (current_time - g_last_update_time[i] > 300) {
                VL53L1X_ClearInterrupt(current_addr);
                g_last_update_time[i] = current_time; // 刷新时间戳
            }
        }
    }
}

/**
 * @brief  获取指定方向的测距数据
 */
uint16_t VL53L1X_getDistance(VL53L1X_Direction dir) {
    switch (dir) {
        case VL53L1X_DIR_M: return g_distance_m;
        case VL53L1X_DIR_L: return g_distance_l;
        case VL53L1X_DIR_R: return g_distance_r;
        default: return 0;
    }
}
