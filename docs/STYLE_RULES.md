## 代码 命名规范

| 类别 | 格式 | 备注 | 示例 |
|---|---|---|---|
|局部变量|(class_)snake_case|(类别)小写下划线|servo_ids|
|全局变量|g_(class_)snake_case|g_(类别)小写下划线|g_fuzzy_result|
|宏/常量|ALL_CAPS|大写下划线|RANGE_NEAR_START|
|枚举类型|ALL_CAPS|类别大写下划线|SERVO_ID|
|枚举值|ALL_CAPS|类别大写下划线|SERVO_1|
|结构体名|Class_PascalCase|类别大驼峰_名大驼峰|CPG_State|
|结构体成员|(class_)SnakeCase|类别小驼峰_名大驼峰|cpg_State|
|局部函数|snakeCase|小驼峰|mainTaskInit||
|全局函数|Class_pascalCase|类别大驼峰_名小驼峰|CPG_setFrequency|

## 提交信息 前缀 参考

| 中文 | 英文 | 用法 |
|------|------|------|
| `[新增]` | (feat) | 增加新功能、新驱动、新传感器支持。 |
| `[修复]` | (fix) | 修复了 Bug、修正错误、解决程序卡死。 |
| `[优化]` | (perf) | 更新算法以提高性能、运行速度、资源利用率。 |
| `[重构]` | (refactor) | 重构代码逻辑和实现方法、优化文件代码结构和可读性、优化代码组织和命名、删除冗余代码。 |
| `[文档]` | (docs) | 修改 README、注释、引脚定义说明。 |
| `[测试]` | (bsp) | 为了调试临时添加的代码（通常在合并前要删除）。 |
| `[配置]` | (chore/config) | 修改 CubeMX 配置（.ioc）、时钟配置、编译选项。 |
| `[硬件]` | (hw/hardware) | 修改原理图、PCB 布局记录或引脚分配更改。 |
<!-- 
| `[驱动]` | (drv/driver) | 专门针对底层外设驱动的修改（I2C, SPI, ADC）。 |
| `[工具]` | (cfg:/config) | 修改烧录脚本、串口调试助手配置文件等。
 -->