使用 HAL 库开发。

名字是用 Origami Shepherd 造的词。

## 开发规范与说明

### 开发环境

使用 [`STM32CubeMX`](https://www.st.com/en/development-tools/stm32cubemx.html) 进行项目构建，勾选使用 [`CMake Tools`](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) 构建系统生成器，使用 [`VS Code`](https://code.visualstudio.com/) + [`STM32CubeIDE for Visual Studio Code`](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension)[ 插件](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension) 进行开发、git 操作、编译烧录等。首次使用 git 时可能要求安装 [`Git`](https://git-scm.cn/install/windows)[ 版本控制系统](https://git-scm.cn/install/windows)。

[`VS Code`](https://code.visualstudio.com/) 插件推荐：[`Git Graph`](https://marketplace.visualstudio.com/items?itemName=mhutchie.git-graph)。

### 项目构建

[`STM32CubeMX`](https://www.st.com/en/development-tools/stm32cubemx.html)

#### 构建系统生成器

1. 打开 **Project Manager** → **Project**
2. **Toolchain / IDE** 选择 **`CMake`** **构建系统生成器**

#### 代码生成器配置

1. 打开 **Project Manager** → **Code Generator**
2. 勾选  **"Generate peripheral initialization as a pair of '.c/.h' files per peripheral"**

   - 作用：每个外设单独生成一对 c/h 文件，方便管理

#### Debug 模式配置

1. 打开 **Pinout & Configuration** → **System Core** → **SYS**
2. **Debug** 选择 **Serial Wire**

#### 时钟配置

1. 打开 **Pinout & Configuration** → **System Core** → **RCC**
2. **High Speed Clock (HSE)**  选择 **Crystal/Ceramic Resonator**（外部晶振）
3. 打开 **Clock Configuration** 标签页
4. 设置 **HCLK** 为 **72 MHz**

#### I2C1 配置（OLED）

1. 打开 **Pinout & Configuration** → **Connectivity** → **I2C1**
2. **Mode** 选择 **I2C**
3. **Configuration** → **I2C Speed Mode** 选择 **Fast Mode**

   - I2C Clock Speed: 400000

#### USART1 配置（RF209S 接收器）

1. 打开 **Pinout & Configuration** → **Connectivity** → **USART1**
2. **Mode** 选择 **Asynchronous**
3. **Configuration** → **Parameter Settings**：

   - **Baud Rate**: 100000 Bits/s
   - **Word Length**: 8 Bits (Including Parity)
   - **Parity**: Even
   - **Stop Bits**: 2
4. 打开 **Pinout & Configuration** → **System Core** → **NVIC**
5. 找到 **USART1 global interrupt** 中断
6. 勾选 **Enabled**

   - Preemption Priority: 0
   - Sub Priority: 0

#### TIM 配置（舵机 PWM）

1. 打开 **Pinout & Configuration** → **Timers** → **TIM3**
2. **Channel1** 选择 **PWM Generation CH1**
3. **Channel2** 选择 **PWM Generation CH2**
4. **Channel3** 选择 **PWM Generation CH3**
5. **Configuration** → **Counter Settings**：

   - **Prescaler (PSC)** : 71（72MHz / (71+1) = 1MHz）
   - **Counter Period (AutoReload Register)** : 19999（1MHz / 20000 = 50Hz，舵机标准频率）

#### GPIO 输入引脚配置

1. 打开 **Pinout & Configuration** → **System Core** → **GPIO**
2. 将所有用于输入的 GPIO 引脚配置为：

   - **GPIO mode**: Input mode
   - **GPIO Pull-up/Pull-down**: Pull-up（或 Pull-down，根据实际需求）

#### 配置汇总

| 外设   | 功能          | 关键参数                            |
| -------- | --------------- | ------------------------------------- |
| SYS    | Debug         | Serial Wire                         |
| RCC    | 外部晶振      | HSE Crystal, HCLK 72MHz             |
| I2C1   | OLED          | Fast Mode 400kHz                    |
| USART1 | RF209S 接收器 | 100k 波特率, 8 位, 偶校验, 2 停止位 |
| TIM3   | 舵机 PWM      | 50Hz, 3 通道                        |
| GPIO   | 输入按钮      | 上拉输入                            |

### 引脚配置

[`STM32CubeMX`](https://www.st.com/en/development-tools/stm32cubemx.html)

部分引脚虽然已经配置但未正式使用，属于预留引脚。

#### 引脚总览

| 引脚编号 | 引脚名称 | 用户引脚标签   | 引脚类型 | 功能备注                                             |
| :--------- | :--------- | :--------------- | :--------- | :----------------------------------------------------- |
| 1        | VBAT     | —             | **电源**         | 备用电池，RTC/后备域供电（通常接 3V 纽扣电池）       |
| 2        | **PC13**         | **RUNNING**               | **GPIO 输出**         | 运行状态指示灯                                       |
| 3        | PC14     | —             | —       | 未使用/保留                                          |
| 4        | PC15     | —             | —       | 未使用/保留                                          |
| 5        | **PD0**         | **RCC_OSC_IN**               | **时钟输入**         | 外部晶振输入（HSE）                                  |
| 6        | **PD1**         | **RCC_OSC_OUT**               | **时钟输出**         | 外部晶振输出（HSE）                                  |
| 7        | NRST     | —             | **复位**         | 复位引脚，外部复位，低电平有效                       |
| 8        | VSSA     | —             | **电源**         | 模拟地 GND（模拟地）                                 |
| 9        | VDDA     | —             | **电源**         | 模拟电源 +3.3V（模拟供电）                           |
| 10       | **PA0**         | **TOF_R_SHUT**               | **GPIO 输出**         | ToF 激光测距模块（右侧）关断控制                     |
| 11       | **PA1**         | **TOF_M_SHUT**               | **GPIO 输出**         | ToF 激光测距模块（中间）关断控制                     |
| 12       | **PA2**         | **TOF_L_SHUT**               | **GPIO 输出**         | ToF 激光测距模块（左侧）关断控制                     |
| 13       | **PA3**         | —             | —       | 未使用/保留                                          |
| 14       | **PA4**         | **POWER_READ**               | **ADC 输入**         | 电源电压检测/ADC                                     |
| 15       | **PA5**         | Servo\_SHUT | **GPIO 输出**         | 舵机开关电路控制                                     |
| 16       | **PA6**         | **Servo_1**               | **PWM 输出**         | 舵机/伺服电机 1                                      |
| 17       | **PA7**         | **Servo_2**               | **PWM 输出**         | 舵机/伺服电机 2                                      |
| 18       | **PB0**         | **Servo_3**               | **PWM 输出**         | 舵机/伺服电机 3                                      |
| 19       | **PB1**         | **Servo_4**               | **PWM 输出**         | 舵机/伺服电机 4                                      |
| 20       | PB2      | —             | —       | 未使用/保留                                          |
| 21       | PB10     | —             | —       | 未使用/保留                                          |
| 22       | **PB11**         | **Bottom_1**               | **GPIO 输入**         | 底部传感器/按键 1                                    |
| 23       | VDD      | —             | **电源**         | 数字电源 +3.3V                                       |
| 24       | VSS      | —             | **电源**         | 数字地 GND                                           |
| 25       | **PB12**         | **Bottom_2**               | **GPIO 输入**         | 底部传感器/按键 2                                    |
| 26       | **PB13**         | **Bottom_3**               | **GPIO 输入**         | 底部传感器/按键 3                                    |
| 27       | **PB14**         | **LED_3**               | **GPIO 输出**         | LED 指示灯 3                                         |
| 28       | **PB15**         | **LED_2**               | **GPIO 输出**         | LED 指示灯 2                                         |
| 29       | **PA8**         | **LED_1**               | **GPIO 输出**         | LED 指示灯 1                                         |
| 30       | **PA9**         | **RC_TX**               | **UART 发送**         | 串口发送（遥控/通信）                                |
| 31       | **PA10**         | **RC_RX**               | **UART 接收**         | 串口接收（遥控/通信）                                |
| 32       | **PA11**         | **USB_DM**               | **USB 差分信号**         | USB 差分信号 D-                                      |
| 33       | **PA12**         | **USB_DP**               | **USB 差分信号**         | USB 差分信号 D+                                      |
| 34       | **PA13**         | **SYS_JTMS-SWDIO**               | **SWD 调试**         | SWD 调试数据（ST-Link 调试接口）                     |
| 35       | VSS      | —             | **电源**         | 数字地 GND                                           |
| 36       | VDD      | —             | **电源**         | 数字电源 +3.3V                                       |
| 37       | PA14     | **SYS_JTCK-SWCLK**               | **SWD 调试**         | SWD 调试时钟（ST-Link 调试接口）                     |
| 38       | PA15     | —             | —       | 未使用/保留                                          |
| 39       | PB3      | —             | —       | 未使用/保留                                          |
| 40       | PB4      | —             | —       | 未使用/保留                                          |
| 41       | PB5      | —             | —       | 未使用/保留                                          |
| 42       | PB6      | —             | —       | 未使用/保留                                          |
| 43       | PB7      | —             | —       | 未使用/保留                                          |
| 44       | **BOOT0**         | —             | **启动模式**         | 启动模式选择，接高电平进入系统存储器启动（串口下载） |
| 45       | **PB8**         | **OLED_SCL**               | **I2C 时钟**         | I2C 时钟线（包括但不限于 I2C 总线设备 OLED 显示屏）  |
| 46       | **PB9**         | **OLED_SDA**               | **I2C 数据**         | I2C 数据线（包括但不限于 I2C 总线设备 OLED 显示屏）  |
| 47       | VSS      | —             | **电源**         | 数字地 GND                                           |
| 48       | VDD      | —             | **电源**         | 数字电源 +3.3V                                       |

#### GPIO 引脚配置详细信息

| 引脚编号 | 引脚名称 | 用户引脚标签 | 信号状态 | GPIO 输出电平 | GPIO 模式        | 上拉/下拉 | 最大输出速度 | 已修改 |
| :--------- | :--------- | :------------- | :--------- | :-------------- | :----------------- | :---------- | :------------- | :------- |
| 10       | PA0      | **TOF_R_SHUT**             | n/a      | Low           | Output Push Pull | 无        | Low          | ☑     |
| 11       | PA1      | **TOF_M_SHUT**             | n/a      | Low           | Output Push Pull | 无        | Low          | ☑     |
| 12       | PA2      | **TOF_L_SHUT**             | n/a      | Low           | Output Push Pull | 无        | Low          | ☑     |
| 29       | PA8      | **LED_1**             | n/a      | Low           | Output Push Pull | 无        | Low          | ☑     |
| 22       | PB11     | **Bottom_1**             | n/a      | n/a           | Input mode       | 无        | n/a          | ☑     |
| 25       | PB12     | **Bottom_2**             | n/a      | n/a           | Input mode       | 无        | n/a          | ☑     |
| 26       | PB13     | **Bottom_3**             | n/a      | n/a           | Input mode       | 无        | n/a          | ☑     |
| 27       | PB14     | **LED_3**             | n/a      | Low           | Output Push Pull | 无        | Low          | ☑     |
| 28       | PB15     | **LED_2**             | n/a      | n/a           | Input mode       | 无        | n/a          | ☑     |
| 2        | PC13     | **RUNNING**             | n/a      | Low           | Output Push Pull | 无        | Low          | ☑     |

#### PWM 配置详细信息

##### TIM2 PWM

| 配置项 | 设置值                    |
| :------- | :-------------------------- |
| **时钟源**       | 内部时钟 (Internal Clock) |
| **从模式**       | 禁用                      |
| **触发源**       | 禁用                      |
| **通道 1~4**       | 全部禁用                  |
| **组合通道**       | 禁用                      |

计数器设置 Counter Settings

| 参数               | 值            | 说明                    |
| :------------------- | :-------------- | :------------------------ |
| 预分频系数 (PSC)   | 72-1          | 72MHz ÷ 72 = **1MHz** 计数频率 |
| 计数模式           | 向上计数 (Up) | —                      |
| 自动重装载值 (ARR) | 65535         | 最大 16 位值            |
| 时钟分频 (CKD)     | 不分频        | —                      |
| 自动重装载预加载   | 禁用          | —                      |

##### TIM3 PWM

| 配置项 | 设置值                    |
| :------- | :-------------------------- |
| **时钟源**       | 内部时钟 (Internal Clock) |
| **从模式**       | 禁用                      |
| **触发源**       | 禁用                      |
| **通道 1~4**       | 均配置为 PWM Generation   |

计数器设置 Counter Settings

| 参数               | 值            | 说明                                  |
| :------------------- | :-------------- | :-------------------------------------- |
| 预分频系数 (PSC)   | 720-1         | 72MHz ÷ 720 = 100kHz 计数频率        |
| 计数模式           | 向上计数 (Up) | —                                    |
| 自动重装载值 (ARR) | 2000-1        | PWM 周期 = 20ms（50Hz，标准舵机频率） |
| 时钟分频 (CKD)     | 不分频        | —                                    |
| 自动重装载预加载   | 禁用          | —                                    |

PWM CH1\~CH4 通道参数

| 参数             | 值                |
| :----------------- | :------------------ |
| 模式             | PWM mode 1        |
| 初始脉宽 (Pulse) | 0                 |
| 输出比较预加载   | 使能              |
| 快速模式         | 禁用              |
| 通道极性         | 高电平有效 (High) |

#### USART 配置详细信息

##### USART1

| 配置项 | 设置值              |
| :------- | :-------------------- |
| **模式**       | 异步 (Asynchronous) |
| **硬件流控**       | 禁用                |

| 参数   | 值                 |
| :------- | :------------------- |
| 波特率 | 100000 Bits/s      |
| 字长   | 8 Bits（含校验位） |
| 校验   | 偶校验 (Even)      |
| 停止位 | 2                  |

在 NVIC 处启用 USART1 的外部中断 global interrupt。

| 参数     | 值             |
| :--------- | :--------------- |
| 数据方向 | 全双工（收发） |
| 过采样   | 16 倍          |

SBUS 协议为 **反向电平**（空闲低电平），需外部反相器或软件取反。SBUS 每帧 25 字节：起始字节 `0x0F` + 16 通道数据（11bit×16， packed）+ 标志位 + 结束字节 `0x00`，100k 波特率下帧周期约 3ms。

### 命名规范

参考 [`STYLE\_RULES.md`](docs\STYLE_RULES.md)。

| 类别       | 格式                 | 备注                | 示例             |
| ------------ | ---------------------- | --------------------- | ------------------ |
| 局部变量   | (class_)snake_case   | (类别)小写下划线    | servo_ids        |
| 全局变量   | g_(class_)snake_case | g_(类别)小写下划线  | g_fuzzy_result   |
| 宏/常量    | ALL_CAPS             | 大写下划线          | RANGE_NEAR_START |
| 枚举类型   | ALL_CAPS             | 类别大写下划线      | SERVO_ID         |
| 枚举值     | ALL_CAPS             | 类别大写下划线      | SERVO_1          |
| 结构体名   | Class_PascalCase     | 类别大驼峰_名大驼峰 | CPG_State        |
| 结构体成员 | (class_)SnakeCase    | 类别小驼峰_名大驼峰 | cpg_State        |
| 局部函数   | snakeCase            | 小驼峰              | mainTaskInit     |
| 全局函数   | Class_pascalCase     | 类别大驼峰_名小驼峰 | CPG_setFrequency |

### 目录结构

仅展示重要的部分，部分文件和新的自定义库文件省略，不一一列出，能大致了解项目结构即可。

```
根目录/
├── Core/              # 
│   ├── Inc/           # 
│   │   └── main.h     # 
│   └── Src/           # 
│       └── main.c     # 程序主入口
├── APP/               # 应用程序代码
│   ├── Inc/           # 应用程序头文件
│   │   └── main_task.h  # 主任务头文件
│   └── Src/           # 应用程序源文件
│       └── main_task.c  # 主任务实现
├── Lib/               # 自定义库
│   ├── Inc/           # 库头文件
│   │   ├── SBUS.h         # SBUS通信协议
│   │   ├── cpg_algorithm.h # CPG算法
│   │   ├── font.h          # 字体定义
│   │   ├── fuzzy_control.h # 模糊控制
│   │   ├── oled.h          # OLED显示
│   │   ├── other_functions.h # 其他功能
│   │   ├── servo_control.h # 伺服控制
│   │   └── ultrasonic.h    # 超声波传感器
│   └── Src/           # 库源文件
│       ├── SBUS.c         # SBUS通信协议实现
│       ├── cpg_algorithm.c # CPG算法实现
│       ├── font.c          # 字体定义实现
│       ├── fuzzy_control.c # 模糊控制实现
│       ├── oled.c          # OLED显示实现
│       ├── other_functions.c # 其他功能实现
│       ├── servo_control.c # 伺服控制实现
│       └── ultrasonic.c    # 超声波传感器实现
├── docs/              # 文档目录
├── CMakeLists.txt     # CMake构建配置
└── Hardware-Test.ioc  # STM32CubeMX配置文件
```

### 其他说明

开发时请尽量按照我们建议的命名方式和代码格式进行开发，包括函数、变量名、提交信息等等。相关规范参考文件在项目的 `docs\STYLE_RULES.md` 中。

不要写屎山代码哇💩！

---

## git 操作规范

### [分支(Branches)](https://github.com/OriShepher-Labs/OriShepher-Core/branches)规范

1. [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main) 分支是**大版本**分支。

   当积累足够多的更新，在适当时机，经过多次测试确保没有明显 bug，并进行代码规范化整理后，可由 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 分支合并到该分支。
2. [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 分支是**主要开发**分支。

   成员在各自的测试分支 `feature-xxx` 中所开发的功能、优化、修改等，经过测试并检查无明显 bug ，进行代码规范化整理后，可合并到该分支。

   必要或关键的修改，建议尽快提交拉取请求到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)，如底层驱动的更新或修改函数接口等。这类所有人都要用到的关键文件，需要尽快同步修改到所有人。因此各成员也需要留意 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 分支是否有代码更新，并及时拉取新代码。
3. `feature/fix-xxx` 分支是各成员的功能开发分支。

   成员自行创建 feature 分支，并在各自的分支进行**开发和调试**。命名方式为 `feature/fix-(功能/内容)`，要使用英文，如 `featrue-CPG`、`feature-oled`、`feature-remote`、`fix-SBUS`、`fix-CPG-function`。或根据项目实际需求命名，实用至上。

   若有多个成员同时开发一个功能，可以共同使用同一个分支(不建议)，或更名，或增加后缀 `feature-remote-B` 来区别。使用多分支时，提交拉取请求前，需先合并到其中一个分支，拉取 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 的最新代码，解决所有代码冲突后，再提交拉取请求合并到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)。

   若修改了底层驱动的更新或修改函数接口等，请尽快提交拉取请求。

### 提交(Commit)规范

参考 [STYLE_RULES.md](docs\STYLE_RULES.md)。

| 中文 | 英文           | 用法                                                                                 |
| ------ | ---------------- | -------------------------------------------------------------------------------------- |
| `[新增]`     | (feat)         | 增加新功能、新驱动、新传感器支持。                                                   |
| `[修复]`     | (fix)          | 修复了 Bug、修正错误、解决程序卡死。                                                 |
| `[优化]`     | (perf)         | 更新算法以提高性能、运行速度、资源利用率。                                           |
| `[重构]`     | (refactor)     | 重构代码逻辑和实现方法、优化文件代码结构和可读性、优化代码组织和命名、删除冗余代码。 |
| `[文档]`     | (docs)         | 修改 README、注释、引脚定义说明。                                                    |
| `[测试]`     | (bsp)          | 为了调试临时添加的代码（通常在合并前要删除）。                                       |
| `[配置]`     | (chore/config) | 修改 CubeMX 配置（.ioc）、时钟配置、编译选项。                                       |
| `[硬件]`     | (hw/hardware)  | 修改原理图、PCB 布局记录或引脚分配更改。                                             |

### [拉取请求](https://github.com/OriShepher-Labs/OriShepher-Core/pulls)(Pull Request)规范

团队未成形，[拉取请求](https://github.com/OriShepher-Labs/OriShepher-Core/pulls)(Pull Request)规范测试中。

1. 自己的分支之间合并自行决定直接合并即可，与其他成员的分支合并则要求使用拉取请求的方式合并。
2. 提交拉取请求前先拉取 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 的最新代码，解决所有代码冲突后，再提交拉取请求合并到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)。
3. 可以在开发完成前先提交**拉取请求草案(Pull request as draft)** ，来得到成员的检查和讨论，期间仍可正常进行提交和修改，参与讨论的成员会在请求提交者提交新的 commit 或有新的讨论消息时收到通知，当一切就绪后再转为准备审核(Ready for review)。
4. 由 `feature-xxx` 发起的拉取请求需有除自己外的其他**至少 1 名成员(除自己)审查**后，才允许合并到 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop)。
5. 大版本更新时，轮流指定一人，由 [`develop`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/develop) 提交拉取请求到 [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main)，需要**所有成员的审查**后，才允许合并到 [`main`](https://github.com/OriShepher-Labs/OriShepher-Core/tree/main)。
6. 拉取请求通过后，如该分支暂无后续开发计划可以选择关闭/删除此 `feature-xxx` 分支。

### 其他说明

可以在提交(commit)和拉取请求(pull request)的提交信息中使用中文，看得懂就行。

不知道什么是分支？不知道什么是拉取请求？不会用 Github？可以到哔哩哔哩或询问 AI 学习。

GitHub 仍有许多有趣的功能等待我们去发现……

---

## 

项目使用 [Apache-2.0 许可证](https://github.com/OriShepher-Labs/OriShepher-Core?tab=Apache-2.0-1-ov-file#)。

<span data-type="text" style="color: var(--b3-font-color5);">所有的说明和规范都尚未完善，它们应在我们的共同讨论中被持续改进……</span>