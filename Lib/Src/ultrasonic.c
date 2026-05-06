#include "ultrasonic.h"
#include <math.h>

// 传感器数组
static Ultrasonic_Sensor_t g_sensors[SENSOR_COUNT];

// 距离结构体
static Ultrasonic_Distance_t g_distances;

// 当前正在测量的传感器ID（-1表示未在测量）
static int8_t g_current_measuring_sensor = -1;

// 微秒级延时（修复定时器溢出问题）
void delay_us(uint32_t us)
{
    uint32_t start_tick = __HAL_TIM_GET_COUNTER(&htim2);
    uint32_t current_tick;
    uint32_t elapsed = 0;
    
    while(elapsed < us) {
        current_tick = __HAL_TIM_GET_COUNTER(&htim2);
        
        if(current_tick < start_tick) {
            elapsed = (UINT32_MAX - start_tick) + current_tick + 1;
        } else {
            elapsed = current_tick - start_tick;
        }
    }
}

// 距离滤波
static float filterDistance(Ultrasonic_Sensor_t* sensor, float new_distance)
{
    if(!sensor->filter_initialized) {
        for(uint8_t i = 0; i < FILTER_SIZE; i++) {
            sensor->distance_buffer[i] = new_distance;
        }
        sensor->filter_initialized = 1;
        return new_distance;
    }
    
    sensor->distance_buffer[sensor->filter_index] = new_distance;
    sensor->filter_index = (sensor->filter_index + 1) % FILTER_SIZE;
    
    float sum = 0;
    for(uint8_t i = 0; i < FILTER_SIZE; i++) {
        sum += sensor->distance_buffer[i];
    }
    
    return sum / FILTER_SIZE;
}

// 单个传感器测距
static Ultrasonic_Result_t measureSensor(Ultrasonic_Sensor_t* sensor)
{
    uint32_t timeout_counter;
    
    // 发送触发信号
    HAL_GPIO_WritePin(sensor->trig_port, sensor->trig_pin, GPIO_PIN_SET);
    delay_us(12);
    HAL_GPIO_WritePin(sensor->trig_port, sensor->trig_pin, GPIO_PIN_RESET);
    
    // 等待上升沿
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while(HAL_GPIO_ReadPin(sensor->echo_port, sensor->echo_pin) == GPIO_PIN_RESET) {
        timeout_counter = __HAL_TIM_GET_COUNTER(&htim2);
        if(timeout_counter > TIMEOUT_RISE_US) {
            return SENSOR_TIMEOUT;
        }
    }
    
    // 等待下降沿
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while(HAL_GPIO_ReadPin(sensor->echo_port, sensor->echo_pin) == GPIO_PIN_SET) {
        timeout_counter = __HAL_TIM_GET_COUNTER(&htim2);
        if(timeout_counter > TIMEOUT_FALL_US) {
            return SENSOR_OUT_OF_RANGE;
        }
    }
    
    // 计算距离
    sensor->echo_time_us = __HAL_TIM_GET_COUNTER(&htim2);
    sensor->distance_mm = (float)sensor->echo_time_us * SOUND_SPEED / 2000.0f;
    
    return SENSOR_OK;
}

// 处理测量结果（改进版）
/**
 * @brief 处理测量结果函数（改进版）
 * @param sensor 传感器结构体指针
 * @return 无
 * @note 改进点：
 *       1. 增加连续拒绝次数限制，避免数据长时间卡住
 *       2. 添加数据老化机制，检测传感器失效
 *       3. 增大最大变化率阈值，适应快速移动场景
 */
static void processResult(Ultrasonic_Sensor_t* sensor)
{
    // 获取当前时间
    uint32_t current_time = HAL_GetTick();
    
    if(sensor->result == SENSOR_OK) {
        // 测量成功，更新时间戳
        sensor->last_update_time = current_time;
        sensor->data_expired = 0;
        
        if(sensor->distance_mm < BLIND_ZONE_MM || sensor->distance_mm > MAX_RANGE_MM) {
            // 距离超出有效范围
            sensor->reject_count++;
            
            if(sensor->reject_count < MAX_REJECT_COUNT && sensor->distance_valid) {
                // 未达到最大拒绝次数，使用上次有效值
                sensor->distance_mm = sensor->last_valid_distance;
            } else {
                // 达到最大拒绝次数，强制更新（传感器可能已移动到新位置）
                sensor->reject_count = 0;
                sensor->last_valid_distance = sensor->distance_mm;
                sensor->distance_valid = 1;
                sensor->distance_mm = filterDistance(sensor, sensor->distance_mm);
            }
        } else {
            // 距离在有效范围内，检查变化率
            if(sensor->distance_valid && 
               fabs(sensor->distance_mm - sensor->last_valid_distance) > MAX_CHANGE_RATE) {
                // 变化率过大
                sensor->reject_count++;
                
                if(sensor->reject_count < MAX_REJECT_COUNT) {
                    // 未达到最大拒绝次数，使用上次有效值
                    sensor->distance_mm = sensor->last_valid_distance;
                } else {
                    // 达到最大拒绝次数，强制接受新数据
                    sensor->reject_count = 0;
                    sensor->last_valid_distance = sensor->distance_mm;
                    sensor->distance_valid = 1;
                    sensor->distance_mm = filterDistance(sensor, sensor->distance_mm);
                }
            } else {
                // 数据有效且变化合理，正常更新
                sensor->reject_count = 0;
                sensor->last_valid_distance = sensor->distance_mm;
                sensor->distance_valid = 1;
                sensor->distance_mm = filterDistance(sensor, sensor->distance_mm);
            }
        }
    } else {
        // 测量失败
        if(sensor->distance_valid) {
            // 使用上次有效值
            sensor->distance_mm = sensor->last_valid_distance;
            
            // 检查数据是否过期
            if(current_time - sensor->last_update_time > DATA_EXPIRE_TIME_MS) {
                sensor->data_expired = 1;
            }
        }
    }
}

// 初始化所有传感器
void Ultrasonic_init(void)
{
    // 传感器0 (右): Echo_R = PA0, Trig_R = PB2
    g_sensors[0].echo_port = Echo_R_GPIO_Port;
    g_sensors[0].echo_pin = Echo_R_Pin;
    g_sensors[0].trig_port = Trig_R_GPIO_Port;
    g_sensors[0].trig_pin = Trig_R_Pin;
    
    // 传感器1 (中): Echo_M = PA1, Trig_M = PA5
    g_sensors[1].echo_port = Echo_M_GPIO_Port;
    g_sensors[1].echo_pin = Echo_M_Pin;
    g_sensors[1].trig_port = Trig_M_GPIO_Port;
    g_sensors[1].trig_pin = Trig_M_Pin;
    
    // 传感器2 (左): Echo_L = PA2, Trig_L = PA3
    g_sensors[2].echo_port = Echo_L_GPIO_Port;
    g_sensors[2].echo_pin = Echo_L_Pin;
    g_sensors[2].trig_port = Trig_L_GPIO_Port;
    g_sensors[2].trig_pin = Trig_L_Pin;
    
    // 初始化所有传感器状态
    for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
        g_sensors[i].echo_time_us = 0;
        g_sensors[i].distance_mm = 0;
        g_sensors[i].result = SENSOR_OK;
        g_sensors[i].filter_index = 0;
        g_sensors[i].filter_initialized = 0;
        g_sensors[i].last_valid_distance = 0;
        g_sensors[i].distance_valid = 0;
        
        // 新增字段初始化
        g_sensors[i].last_update_time = 0;
        g_sensors[i].data_expired = 0;
        g_sensors[i].reject_count = 0;
    }
    
    // 初始化距离结构体
    g_distances.left = 0;
    g_distances.middle = 0;
    g_distances.right = 0;
    g_distances.any_expired = 0;
}

// 测量所有传感器
void Ultrasonic_measureAll(void)
{
    uint8_t expired_count = 0;
    
    for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
        // 设置当前正在测量的传感器ID
        g_current_measuring_sensor = i;
        
        g_sensors[i].result = measureSensor(&g_sensors[i]);
        processResult(&g_sensors[i]);
        
        // 检查是否有传感器数据过期
        if(g_sensors[i].data_expired) {
            expired_count++;
        }
        
        if(i < SENSOR_COUNT - 1) {
            HAL_Delay(SENSOR_INTERVAL_MS);
        }
    }
    
    // 测量完成，重置为-1
    g_current_measuring_sensor = -1;
    
    // 更新距离结构体
    g_distances.left = g_sensors[0].distance_mm;
    g_distances.middle = g_sensors[1].distance_mm;
    g_distances.right = g_sensors[2].distance_mm;
    g_distances.any_expired = (expired_count > 0) ? 1 : 0;
}

// 获取距离结构体
Ultrasonic_Distance_t Ultrasonic_getDistance(void)
{
    return g_distances;
}

// 获取单个传感器距离
float Ultrasonic_getSensorDistance(uint8_t sensor_id)
{
    if(sensor_id < SENSOR_COUNT) {
        return g_sensors[sensor_id].distance_mm;
    }
    return 0;
}

// 获取单个传感器的连续拒绝次数
uint8_t Ultrasonic_getRejectCount(uint8_t sensor_id)
{
    if(sensor_id < SENSOR_COUNT) {
        return g_sensors[sensor_id].reject_count;
    }
    return 0;
}

// 获取当前正在测量的传感器ID
int8_t Ultrasonic_getCurrentMeasuringSensor(void)
{
    return g_current_measuring_sensor;
}