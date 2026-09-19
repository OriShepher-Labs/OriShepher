# 项目简介

> **OriShepher: An Origami-structured Shepherd for Marine Ranching.**

OriShepher 基于 STM32F103C8T6 芯片，主要解决水下仿生鱼的运动控制与多传感器数据融合问题。

项目使用 [Apache-2.0 许可证](https://github.com/OriShepher-Labs/OriShepher-Core?tab=Apache-2.0-1-ov-file#)。

## 最新进展

最新进展请关注 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 和 [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main) 分支。

### 功能一览

1. 遥控 / 自主避障双模式自动切换
- 单片机接收到遥控器信号时，切换为遥控器操作，退出模糊控制；
- 遥控器信号消失时，立即切回模糊控制自主避障模式。

2. 遥控器基础动作控制
- 支持前进、转向、浮沉等基础动作；
- 锁定鱼尾舵机保持伸直状态；鱼尾摆动换挡变速。

3. 红外测距自主避障
- 基于三颗红外测距模块与模糊控制算法实现基本自主避障；
- 经实测，左右靠近墙体时可自主调整至面向远离墙体的方向。

### 已知问题与限制

- 三颗测距传感器存在互相干扰，超声传感器暂无法启用，
  现阶段以红外测距传感器替代验证；
- 红外测距模块水下不可用，模糊控制自主避障目前仅限空气环境验证。

### 未来更新计划

- 压力传感、惯性测量、水下声学测距、视觉。

## 开发环境

- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) + [VS Code](https://code.visualstudio.com/)
- HAL 库，CMake 构建系统生成器

## 硬件设备

- 处理器 STM32F103C8T6
- 遥控器 天地飞 ET16S
- 接收机 RF209S
- 舵机 RDS3218-20KG | QFS0035-35KG
- 红外测距 TOF400C-VL53L1X
- 超声测距 RCWL-1605

## 核心算法

- CPG（Central Pattern Generator 中央模式发生器），用于运动控制。
- Fuzzy Control 模糊控制，用于自主避障。
- S.BUS 协议遥控通信，用于遥控控制。
