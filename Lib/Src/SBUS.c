#include "SBUS.h"  

// SBUS接收缓冲区
uint8_t sbus_frame_buffer[SBUS_RX_BUFFER_SIZE];
// SBUS接收字节计数器
uint8_t sbus_byte_count = 0;
// SBUS帧同步标志
uint8_t sbus_frame_synced = 0;
// SBUS帧就绪标志
uint8_t sbus_frame_ready = 0;
// UART接收中断dummy字节（必须保留以维持中断机制）
uint8_t sbus_rx_dummy_byte = 0;

/**
 * @brief UART接收完成中断回调函数
 * @note 处理UART1的SBUS帧接收
 * 
 * 状态机流程：
 * 1. 等待帧起始字节(0x0F)
 * 2. 接收帧数据字节(共25字节)
 * 3. 设置帧就绪标志，准备下一帧接收
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart->Instance == USART1) {
        // 读取接收到的字节
        uint8_t rx_byte = (uint8_t)(huart->Instance->DR & 0x00FF);
        
        // 帧同步与数据接收状态机
        if(sbus_frame_synced == 0) {
            // 【状态1】等待帧起始字节
            if(rx_byte == 0x0F) {
                sbus_frame_synced = 1;              // 标记帧已同步
                sbus_byte_count = 0;
                sbus_frame_buffer[sbus_byte_count++] = rx_byte;
            }
        } else {
            // 【状态2】帧同步中，接收数据字节
            if(sbus_byte_count < SBUS_FRAME_LENGTH) {
                sbus_frame_buffer[sbus_byte_count++] = rx_byte;
                
                // 检查是否接收完一帧数据
                if(sbus_byte_count == SBUS_FRAME_LENGTH) {
                    // 帧接收完成，设置就绪标志
                    sbus_frame_ready = 1;
                    sbus_frame_synced = 0;           // 准备接收下一帧
                }
            } else {
                // 【错误处理】接收溢出，重置状态机
                sbus_frame_synced = 0;
                sbus_byte_count = 0;
            }
        }
        
        // 重启接收中断（继续接收下一个字节）
        HAL_UART_Receive_IT(&huart1, &sbus_rx_dummy_byte, 1);
    }
}

/**
 * @brief 解码SBUS帧为6个通道值
 * @param[out] channels 输出的6个通道值数组（11位精度，0-2047）
 * @return 无
 * 
 * @details
 * SBUS帧格式：
 * - 字节0:   帧头(0x0F)
 * - 字节1-22: 11bit×6通道数据（压缩存储）
 * - 字节23:   校验/标志字节
 * - 字节24:   帧尾(0x00或0x04)
 * 
 * 各通道位映射：
 * - CH1: 字节[1:2] bits[7:0] + bits[2:0]
 * - CH2: 字节[2:3] bits[4:0] + bits[7:3]
 * - CH3: 字节[3:5] bits[7:6] + bits[7:0] + bits[1:0]
 * - CH4: 字节[5:6] bits[6:0] + bits[3:0]
 * - CH5: 字节[6:7] bits[2:0] + bits[7:4]
 * - CH6: 字节[7:9] bits[8:0] + bits[7] + bits[1:0]
 */
void sbus_decode_channels(uint16_t* channels)
{
    // 【第一步】验证帧的有效性（检查帧尾字节）
    if (sbus_frame_buffer[SBUS_FRAME_LENGTH - 1] != 0x00 && sbus_frame_buffer[SBUS_FRAME_LENGTH - 1] != 0x04) {
        // 帧尾无效：清空所有通道数据
        for (uint8_t i = 0; i < 6; i++) {
            channels[i] = 0;
        }
        return;
    }

    // 【第二步】逐通道解码11位数据
    uint16_t channel_value = 0;

    // 【CH1】通道1解码
    channel_value = 0;
    channel_value = (sbus_frame_buffer[2] & 0x07) << 8;  // 高3位
    channel_value = channel_value | sbus_frame_buffer[1];  // 低8位
    channels[0] = channel_value;

    // 【CH2】通道2解码
    channel_value = 0;
    channel_value = (sbus_frame_buffer[3] & 0x3f) << 5;  // 高6位
    channel_value = channel_value | (sbus_frame_buffer[2] >> 3);  // 低5位
    channels[1] = channel_value;

    // 【CH3】通道3解码
    channel_value = 0;
    channel_value = (sbus_frame_buffer[5] & 0x01) << 10;  // 高1位
    channel_value = channel_value | (sbus_frame_buffer[4] << 2);  // 中8位
    channel_value = channel_value | (sbus_frame_buffer[3] >> 6);  // 低2位
    channels[2] = channel_value;

    // 【CH4】通道4解码
    channel_value = 0;
    channel_value = (sbus_frame_buffer[6] & 0x0f) << 7;  // 高4位
    channel_value = channel_value | (sbus_frame_buffer[5] >> 1);  // 低7位
    channels[3] = channel_value;

    // 【CH5】通道5解码
    channel_value = 0;
    channel_value = (sbus_frame_buffer[7] & 0x7f) << 4;  // 高7位
    channel_value = channel_value | (sbus_frame_buffer[6] >> 4);  // 低4位
    channels[4] = channel_value;

    // 【CH6】通道6解码
    channel_value = 0;
    channel_value = (sbus_frame_buffer[9] & 0x03) << 9;  // 高2位
    channel_value = channel_value | (sbus_frame_buffer[8] << 1);  // 中8位
    channel_value = channel_value | (sbus_frame_buffer[7] >> 7);  // 低1位
    channels[5] = channel_value;
}

/**
 * @brief 清空SBUS帧缓冲区
 * @return 无
 * 
 * @note 用于初始化或错误恢复时清空缓冲区中的旧数据
 */
void sbus_clear_frame_buffer(void)
{
    // 遍历整个缓冲区，逐字节清空为0
    for(uint8_t i = 0; i < SBUS_FRAME_LENGTH; i++){
        sbus_frame_buffer[i] = 0;
    }
}
