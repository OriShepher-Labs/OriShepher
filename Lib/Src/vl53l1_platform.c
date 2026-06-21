/**
  ******************************************************************************
  * @file  vl53l1_platform.c
  * @brief 完美适配 STM32 HAL 库的 VL53L1X ULD 底层平台移植驱动
  ******************************************************************************
  */

#include "vl53l1_platform.h"
#include "i2c.h"  // 确保引入了 CubeMX 生成的 I2C 句柄头文件

// 声明外部定义的 I2C 句柄（请根据实际使用的 I2C 端口修改，例如 hi2c1 或 hi2c2）
extern I2C_HandleTypeDef hi2c1; 

// 写入连续字节
int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    if (HAL_I2C_Mem_Write(&hi2c1, dev, index, I2C_MEMADD_SIZE_16BIT, pdata, count, 500) == HAL_OK) {
        return 0;
    }
    return -1;
}

// 读取连续字节
int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    if (HAL_I2C_Mem_Read(&hi2c1, dev, index, I2C_MEMADD_SIZE_16BIT, pdata, count, 500) == HAL_OK) {
        return 0;
    }
    return -1;
}

// 写入 1 个字节 (8位)
int8_t VL53L1_WrByte(uint16_t dev, uint16_t index, uint8_t data) {
    if (HAL_I2C_Mem_Write(&hi2c1, dev, index, I2C_MEMADD_SIZE_16BIT, &data, 1, 100) == HAL_OK) {
        return 0;
    }
    return -1;
}

// 写入 2 个字节 (16位) -> 处理大小端转换
int8_t VL53L1_WrWord(uint16_t dev, uint16_t index, uint16_t data) {
    uint8_t buffer[2];
    buffer[0] = (uint8_t)(data >> 8);   // 高位在前
    buffer[1] = (uint8_t)(data & 0x00FF);
    if (HAL_I2C_Mem_Write(&hi2c1, dev, index, I2C_MEMADD_SIZE_16BIT, buffer, 2, 100) == HAL_OK) {
        return 0;
    }
    return -1;
}

// 写入 4 个字节 (32位) -> 处理大小端转换
int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index, uint32_t data) {
    uint8_t buffer[4];
    buffer[0] = (uint8_t)(data >> 24);
    buffer[1] = (uint8_t)(data >> 16);
    buffer[2] = (uint8_t)(data >> 8);
    buffer[3] = (uint8_t)(data & 0x000000FF);
    if (HAL_I2C_Mem_Write(&hi2c1, dev, index, I2C_MEMADD_SIZE_16BIT, buffer, 4, 100) == HAL_OK) {
        return 0;
    }
    return -1;
}

// 读取 1 个字节 (8位)
int8_t VL53L1_RdByte(uint16_t dev, uint16_t index, uint8_t *data) {
    if (HAL_I2C_Mem_Read(&hi2c1, dev, index, I2C_MEMADD_SIZE_16BIT, data, 1, 100) == HAL_OK) {
        return 0;
    }
    return -1;
}

// 读取 2 个字节 (16位) -> 处理大小端转换
int8_t VL53L1_RdWord(uint16_t dev, uint16_t index, uint16_t *data) {
    uint8_t buffer[2];
    if (HAL_I2C_Mem_Read(&hi2c1, dev, index, I2C_MEMADD_SIZE_16BIT, buffer, 2, 100) == HAL_OK) {
        *data = ((uint16_t)buffer[0] << 8) | buffer[1];
        return 0;
    }
    return -1;
}

// 读取 4 个字节 (32位) -> 处理大小端转换
int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index, uint32_t *data) {
    uint8_t buffer[4];
    if (HAL_I2C_Mem_Read(&hi2c1, dev, index, I2C_MEMADD_SIZE_16BIT, buffer, 4, 100) == HAL_OK) {
        *data = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | 
                ((uint32_t)buffer[2] << 8)  | buffer[3];
        return 0;
    }
    return -1;
}

// 底层延时接口
int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms) {
    HAL_Delay(wait_ms);
    return 0;
}