/**
 * WonderUI 基本使用示例
 * 演示如何初始化库、使用各种输入和对话框功能
 */

#include <Wire.h>
#include <U8g2lib.h>
#include "WonderUI.h"

// 创建 U8G2 对象（SSD1306, 128x64, I2C）
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// 创建 WonderUI 对象，传入按键引脚（按需修改）
// 默认：上12，下13，左14，右27，确认33，返回26
WonderUI ui(u8g2, 12, 13, 14, 27, 33, 26);

void setup() {
  // 初始化（自动配置引脚模式和U8G2）
  ui.begin();

  // 播放开机动画
  ui.startAnimation();

  // 提示欢迎
  ui.alert("Welcome", "WonderUI Library", "Example v1.0", "");

  // 字符串输入示例
  String name = ui.inputString("Enter your name", 20);

  // 数字输入示例
  String ageStr = ui.inputNumber(3, 'D');
  int age = ageStr.toInt();

  // 列表选择示例
  std::vector<String> options = {"Option A", "Option B", "Option C"};
  int choice = ui.listChoose("Select an option", options);

  // 数值调节示例
  int volume = ui.numericUpDown("Volume", 0, 100, 50);

  // 显示汇总信息
  String msg = "Name: " + name + "\nAge: " + ageStr + "\nOption: " + String(choice + 1) + "\nVolume: " + String(volume);
  ui.messageBox("User Info", msg, "", "", "OK: Exit");

  // 最终提示
  ui.alert("Done", "Example finished", "Reset device to run again", "");
}

void loop() {
  // 空闲
  delay(1000);
}