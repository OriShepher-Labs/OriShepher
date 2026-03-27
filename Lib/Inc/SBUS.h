#ifndef _SBUS_H
#define _SBUS_H

/*
感谢 LGQWakkk 的开源仓库 https://github.com/LGQWakkk/STM32_SBUS_RX
注意一定要使用三极管硬件信号取反，还有接收机三极管单片机共地之类的问题

1. 三极管基极（B）的连接
- 接收器的 SBUS 信号线 → 1kΩ 电阻的一端 → SS8050 三极管的基极（B）（中间引脚）；

2. 三极管发射极（E）的连接
- SS8050 的发射极（E）（左侧引脚）→ GND 线 ↓；
- GND 线 同时连接：
  - 接收器的 GND；
  - F103C8T6 的 GND 引脚。

3. 三极管集电极（C）的连接
- SS8050 的集电极（C）（右侧引脚）→ 两根线：
  - 第一根线 → 10kΩ 电阻的一端 → F103C8T6 的 3.3V 引脚；
  - 第二根线 → F103C8T6 的 PA10 引脚（USART1_RX）。
*/

#include <stdint.h>

#include "usart.h"

#include "oled.h"

#define SBUS_RX_BUFFER_SIZE  	200  	// SBUS接收缓冲区大小（字节）
#define SBUS_UART1_RX_ENABLE 	1		// 使能（1）/禁止（0）串口1接收
#define SBUS_FRAME_LENGTH 		25  	// SBUS单帧数据长度（字节）

void SBUS_init(void);                    // 初始化SBUS接收
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void SBUS_decodeChannels(uint16_t* channels);  // 解码SBUS帧为6个通道值
void SBUS_clearFrameBuffer(void);              // 清空SBUS帧缓冲区


#endif

