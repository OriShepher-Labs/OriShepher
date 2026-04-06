# RCWL-1605 超声波测距模块使用说明

## 概述

本模块实现了基于STM32F103C8T6的RCWL-1605超声波测距功能，支持三个传感器同时工作，用于机器人避障系统。采用轮询触发方式避免回声干扰，双边沿输入捕获精确测量距离。

## 硬件连接

### 传感器配置
- **传感器数量**: 3个 (左侧、中间、右侧)
- **型号**: RCWL-1605 (GPIO模式)

### 引脚连接
| 传感器 | Trig 引脚 | Echo 引脚 | GPIO 端口 |
|--------|-----------|-----------|-----------|
| 左侧   | PA3      | PA0      | GPIOA    |
| 中间   | PA4      | PA1      | GPIOA    |
| 右侧   | PA5      | PA2      | GPIOA    |

### TIM2 配置 (CubeMX)
- **时钟源**: 72MHz APB1
- **预分频器 (Prescaler)**: 71 (计数频率 1MHz, 1tick = 1us)
- **计数周期 (Period)**: 65535
- **通道配置**:
  - TIM2_CH1 (PA0): 输入捕获，上升沿/下降沿
  - TIM2_CH2 (PA1): 输入捕获，上升沿/下降沿
  - TIM2_CH3 (PA2): 输入捕获，上升沿/下降沿

### Trig 引脚配置
- **模式**: 输出推挽 (Output Push-Pull)
- **初始状态**: 低电平

## 软件架构

### 文件结构
```
Lib/
├── Inc/
│   └── ultrasonic.h    // 头文件：结构体定义和API声明
└── Src/
    └── ultrasonic.c    // 源文件：实现测距逻辑
```

### 核心组件
1. **输入捕获中断**: `HAL_TIM_IC_CaptureCallback`
2. **微秒延时**: `delay_us()` (近似实现)
3. **轮询触发**: `Ultrasonic_TriggerSensor()`
4. **距离计算**: 基于声速公式

## API 接口

### 数据结构
```c
typedef struct {
    float left;   // 左侧传感器距离 (cm)
    float middle; // 中间传感器距离 (cm)
    float right;  // 右侧传感器距离 (cm)
} Ultrasonic_Distance_t;
```

### 函数接口

#### 初始化
```c
void Ultrasonic_Init(void);
```
- **功能**: 初始化超声波模块
- **调用时机**: 系统启动时，在 `mainTaskInit()` 中调用
- **实现细节**:
  - 启动 TIM2 输入捕获中断
  - 设置初始捕获极性为上升沿

#### 读取距离
```c
void Ultrasonic_ReadDistances(Ultrasonic_Distance_t *distances);
```
- **功能**: 读取三个传感器的距离数据
- **参数**: `distances` - 指向距离结构体的指针
- **返回值**: 无，直接修改传入的结构体
- **实现细节**:
  - 轮询触发三个传感器 (间隔30ms)
  - 等待捕获完成 (超时100ms)
  - 计算距离并填充结构体

### 内部函数
```c
void delay_us(uint32_t us);                    // 微秒级延时
void Ultrasonic_TriggerSensor(int sensor);     // 触发单个传感器
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim); // 输入捕获回调
```

## 使用流程

### 1. 系统初始化
```c
#include "ultrasonic.h"

// 在 main_task.c 的 mainTaskInit() 中
Ultrasonic_Init();
```

### 2. 读取距离数据
```c
Ultrasonic_Distance_t dist;

// 在需要距离数据的地方调用
Ultrasonic_ReadDistances(&dist);

// 使用距离数据
float left_dist = dist.left;
float middle_dist = dist.middle;
float right_dist = dist.right;
```

### 3. 集成到模糊控制
```c
// 在 fuzzyControl() 函数中
Ultrasonic_Distance_t dist;
Ultrasonic_ReadDistances(&dist);

// 传入模糊控制系统
CPG_setBias(&cpg_State, Fuzzy_update(dist.left, dist.middle, dist.right), false);
```

## 技术细节

### 测距原理
1. **触发阶段**: 给 Trig 引脚发送 15us 高电平脉冲
2. **发射阶段**: 传感器发射超声波
3. **接收阶段**: Echo 引脚输出高电平，持续时间与距离成正比
4. **捕获阶段**: 使用 TIM2 输入捕获测量 Echo 高电平持续时间
5. **计算阶段**: 距离 = (时间(us) × 0.034) / 2

### 双边沿捕获机制
- **上升沿**: 记录 Echo 高电平开始时间
- **下降沿**: 记录 Echo 高电平结束时间
- **时间差**: 计算高电平持续时间
- **状态机**: 使用 `echo_captured[]` 数组管理捕获状态

### 轮询触发策略
- **问题**: 同时触发多个传感器会导致回声干扰
- **解决方案**: 依次触发传感器，间隔30ms
- **顺序**: 左侧 → 中间 → 右侧
- **等待**: 每次触发后等待捕获完成

### 距离计算公式
```
距离(cm) = (Echo高电平时间(us) × 声速(cm/us)) / 2
声速 = 340m/s = 0.034cm/us
```

### 误差处理
- **超时机制**: 读取距离时设置100ms超时
- **范围限制**: 实际使用时应限制距离范围 (30-150cm)
- **异常值**: 未捕获到信号时距离为0

## 注意事项

### 硬件注意事项
1. **电源**: RCWL-1605 需要稳定5V电源
2. **接地**: 确保所有传感器共地
3. **干扰**: 避免强电磁干扰环境
4. **角度**: 传感器应水平安装，避免倾斜

### 软件注意事项
1. **调用频率**: 不要过于频繁调用 `Ultrasonic_ReadDistances()`，建议间隔>100ms
2. **中断安全**: 回调函数使用 volatile 变量，确保线程安全
3. **调试**: 可以通过串口打印距离值进行调试
4. **校准**: 实际使用时可能需要校准距离公式

### 性能参数
- **测量范围**: 2cm - 450cm (理论值)
- **测量精度**: ±3mm
- **响应时间**: ~100ms (三个传感器轮询)
- **功耗**: 低功耗模式

## 故障排除

### 常见问题
1. **距离为0**: 检查传感器连接和电源
2. **距离不准**: 校准声速参数或检查安装角度
3. **无响应**: 检查 TIM2 配置和中断使能
4. **干扰大**: 增加触发间隔或使用屏蔽

### 调试方法
1. **串口输出**: 在回调中打印捕获时间
2. **LED指示**: 使用LED显示捕获状态
3. **示波器**: 观察 Trig 和 Echo 波形
4. **单步调试**: 逐个传感器测试

## 扩展功能

### 可能的改进
1. **多线程**: 使用RTOS任务优化读取流程
2. **滤波算法**: 添加中值滤波减少噪声
3. **温度补偿**: 根据环境温度校准声速
4. **自动校准**: 实现距离自动校准功能

### 兼容性
- **芯片**: STM32F1系列
- **HAL版本**: STM32Cube HAL库
- **编译器**: arm-none-eabi-gcc

## 版本历史

- **v1.0**: 初始版本，实现基本测距功能
- 支持三个RCWL-1605传感器
- 轮询触发避免干扰
- 双边沿输入捕获

---

*本文档基于项目代码自动生成，如有更新请及时修改。*