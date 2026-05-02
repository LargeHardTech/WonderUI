/**
 * WonderUI 自定义按键引脚示例
 * 演示如何更改按键引脚（不同于默认值）
 */

#include <Wire.h>
#include <U8g2lib.h>
#include "WonderUI.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// 自定义按键引脚分配
// 假设使用：上=15, 下=16, 左=17, 右=18, 确认=19, 返回=21
WonderUI ui(u8g2, 15, 16, 17, 18, 19, 21);

void setup() {
  ui.begin();
  ui.startAnimation();

  // 演示简单的数字输入
  ui.messageBox("Demo", "Press OK to input a number", "", "", "OK to start");
  String num = ui.inputNumber(4, 'D');
  ui.alert("Result", "Your number: " + num, "", "");
}

void loop() {
  // 空循环
}