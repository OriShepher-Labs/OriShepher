#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__

#include "main.h"
#include "tim.h"

// 传感器配置
#define SOUND_SPEED 343.0f      // 声速 (m/s) @ 20°C
#define SENSOR_COUNT 3           // 传感器数量
#define SENSOR_INTERVAL_MS 50    // 传感器间间隔 (ms)
#define FILTER_SIZE 5            // 滤波窗口大小

// 阈值配置（可根据实际情况调整）
#define BLIND_ZONE_MM 250        // 盲区 (mm)
#define MAX_RANGE_MM 4500        // 最大量程 (mm)
#define MAX_CHANGE_RATE 1000.0f  // 最大允许变化率 (mm/100ms) - 增大到1000mm
#define MAX_REJECT_COUNT 3       // 最大连续拒绝次数
#define DATA_EXPIRE_TIME_MS 500  // 数据过期时间 (ms)

// 超时配置
#define TIMEOUT_RISE_US 5000     // 上升沿超时 (μs)
#define TIMEOUT_FALL_US 30000    // 下降沿超时 (μs)

// 超声波传感器状态枚举
typedef enum {
    SENSOR_OK = 0,              // 测量成功
    SENSOR_TIMEOUT,             // 超时无信号
    SENSOR_OUT_OF_RANGE         // 超出量程
} Ultrasonic_Result_t;

// 超声波传感器结构体
typedef struct {
    GPIO_TypeDef* echo_port;     // Echo引脚端口
    uint16_t echo_pin;          // Echo引脚编号
    GPIO_TypeDef* trig_port;     // Trig引脚端口
    uint16_t trig_pin;          // Trig引脚编号
    
    uint32_t echo_time_us;      // Echo高电平时长 (μs)
    float distance_mm;          // 计算距离 (mm)
    Ultrasonic_Result_t result;  // 测量结果
    
    // 滤波相关
    float distance_buffer[FILTER_SIZE];  // 滤波缓冲区
    uint8_t filter_index;               // 滤波索引
    uint8_t filter_initialized;         // 滤波初始化标志
    float last_valid_distance;           // 上次有效距离
    uint8_t distance_valid;             // 距离有效标志
    
    // 新增：数据老化和连续拒绝检测
    uint32_t last_update_time;          // 上次更新时间
    uint8_t data_expired;               // 数据是否过期
    uint8_t reject_count;               // 连续拒绝次数
} Ultrasonic_Sensor_t;

// 超声波距离结构体
typedef struct {
    float left;   // 左侧传感器距离 (mm)
    float middle; // 中间传感器距离 (mm)
    float right;  // 右侧传感器距离 (mm)
    uint8_t any_expired;  // 是否有传感器数据过期
} Ultrasonic_Distance_t;

// 微秒级延时
void delay_us(uint32_t us);

// 初始化所有传感器
void Ultrasonic_init(void);

// 测量所有传感器
void Ultrasonic_measureAll(void);

// 获取距离结构体
Ultrasonic_Distance_t Ultrasonic_getDistance(void);

// 获取单个传感器距离
float Ultrasonic_getSensorDistance(uint8_t sensor_id);

// 获取单个传感器的连续拒绝次数
uint8_t Ultrasonic_getRejectCount(uint8_t sensor_id);

// 获取当前正在测量的传感器ID（-1表示未在测量）
int8_t Ultrasonic_getCurrentMeasuringSensor(void);

#endif