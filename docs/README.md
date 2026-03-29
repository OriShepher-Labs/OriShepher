## 项目简介

> **OriShepher: An Origami-structured Shepherd for Marine Ranching.**

OriShepher 基于 STM32F103C8T6 芯片，主要解决水下仿生鱼的运动控制与多传感器数据融合问题。

项目使用[Apache-2.0 许可证](https://github.com/OriShepher-Labs/OriShepher-Core?tab=Apache-2.0-1-ov-file#)。

### 开发环境

- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) + [VS Code](https://code.visualstudio.com/)

- HAL库

### 硬件设备

- 处理器 STM32F103C8T6
- 遥控器 天地飞ET16S
- 接收机 RF209S
- 舵机 RDS3218-20KG ×1 | QFS0035-35KG ×1

### 核心算法

- CPG（Central Pattern Generator中央模式发生器），用于运动控制。
- Fuzzy Control 模糊控制，用于自主避障。
- S.BUS协议遥控通信，用于遥控控制。