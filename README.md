# WonderUI

一个为 ESP32 + U8g2 OLED 屏幕设计的通用用户界面库，提供开机动画、文字/数字输入、列表选择、数值调节、确认对话框等常用 UI 组件。所有界面完全还原自“WonderOS”项目的原始显示效果。

## 功能
- 6 键控制（上、下、左、右、OK、返回）
- 全字符键盘输入（支持大小写、符号）
- 数字输入键盘（大键盘 / 二进制横条）
- 四选项 / 三选项列表选择
- 数值上下调节控件
- 确认 / 信息对话框
- 精美的开机动画（圆环、文字）
- 按钮引脚可运行时重新配置

## 依赖
- [U8g2](https://github.com/olikraus/u8g2) （OLED 图形库）
- Arduino 环境（适用于 ESP32，可轻易移植到其他平台）

## 安装
1. 下载本仓库 ZIP 文件，或克隆到 Arduino 的 `libraries` 目录。
2. 重启 Arduino IDE。
3. 在“文件 → 示例 → WonderUI”中找到 `BasicUsage` 示例。

## 硬件连接（默认引脚）
| 按钮 | 功能 | ESP32 引脚 |
|------|------|------------|
| U    | 上   | 12         |
| D    | 下   | 13         |
| L    | 左   | 14         |
| R    | 右   | 27         |
| O    | OK   | 33         |
| S    | 返回 | 26         |

所有按钮均采用 **输入上拉** 方式，按下时引脚为低电平。

## 快速使用
```cpp
#include <U8g2lib.h>
#include "WonderUI.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
WonderUI ui(u8g2);

void setup() {
  u8g2.begin();
  ui.begin();
  ui.start();   // 开机动画
}

void loop() {
  String name = ui.input(10);
  // ... 其他组件
}
```

## API 参考
### 构造函数
`WonderUI(U8G2 &u8g2, uint8_t pinU=12, uint8_t pinD=13, ...);`
- `u8g2`: U8g2 显示对象引用
- `pinU ~ pinS`: 六个按钮引脚，均有默认值

### 基本设置
`void begin()` — 初始化按钮引脚并启用 UTF8 打印（调用一次）。

`void configureButtons(uint8_t pinU, uint8_t pinD, ...)` — 运行时更换按钮引脚，立即生效。

### UI 组件
- `String input(int maxl)` — 字符输入，最大长度 `maxl`。
- `String inputnum(int maxl, char base)` — 数字输入，`base='D'`（大键盘）或 `'B'`（横条）。
- `int listchoose4(String x, y, z, a, String title)` — 四选项列表，返回索引 0~3。
- `int listchoose3(String x, y, z, String title)` — 三选项列表，返回索引 0~2。
- `int numericUpDown(int maxn, int minn, String title)` — 数值调节，返回当前值。
- `bool check(String title, line1, line2, line3, line4)` — 确认框，OK 返回 `true`，Set 返回 `false`。

### 动画
`void start()` — 播放开机动画。

## 许可
本项目采用 MIT 许可证，自由使用。

## 作者
巨硬科技 LHT