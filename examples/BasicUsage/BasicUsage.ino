/*
  WonderUI 基础使用示例
  硬件：ESP32 + SSD1306 128x64 OLED (I2C)
  功能：依次展示库中的各个 UI 组件
*/

#include <U8g2lib.h>
#include "WonderUI.h"

// 创建 U8g2 实例（请根据你的屏幕型号调整构造函数）
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// 创建 WonderUI 实例，默认引脚（上、下、左、右、OK、返回）
WonderUI ui(u8g2);

// 如果想运行时更换引脚，可以这样：
// WonderUI ui(u8g2, 12, 13, 14, 27, 33, 26);

void setup() {
  Serial.begin(115200);

  // 初始化 u8g2
  u8g2.begin();
  // 初始化 WonderUI（按钮引脚、UTF8）
  ui.begin();

  // 播放开机动画
  ui.start();

  // 示例：运行时修改按钮引脚（可省略，若使用默认引脚）
  // ui.configureButtons(12, 13, 14, 27, 33, 26);

  Serial.println("WonderUI 示例开始");
}

void loop() {
  // ---- 1. 文字输入 ----
  Serial.println("请输入您的名字（最长10个字符）：");
  String name = ui.input(10);
  Serial.print("你输入的名字：");
  Serial.println(name);

  // ---- 2. 数字输入（大键盘） ----
  Serial.println("请输入一个数字（大键盘，最长5位）：");
  String numStr = ui.inputnum(5, 'D');
  Serial.print("你输入的数字：");
  Serial.println(numStr);

  // ---- 3. 二进制输入（2键横条） ----
  Serial.println("请输入一个二进制数（B模式，最长8位）：");
  String binStr = ui.inputnum(8, 'B');
  Serial.print("你输入的二值串：");
  Serial.println(binStr);

  // ---- 4. 四选项列表选择 ----
  int choice4 = ui.listchoose4("苹果", "香蕉", "橘子", "葡萄", "水果选择");
  Serial.print("你选了第 ");
  Serial.print(choice4);
  Serial.println(" 项水果");

  // ---- 5. 三选项列表选择 ----
  int choice3 = ui.listchoose3("开始", "暂停", "退出", "游戏菜单");
  Serial.print("你选了第 ");
  Serial.print(choice3);
  Serial.println(" 项");

  // ---- 6. 数值上下调节 ----
  int volume = ui.numericUpDown(100, 0, "设置音量");
  Serial.print("音量设置为：");
  Serial.println(volume);

  // ---- 7. 确认对话框 ----
  bool confirmed = ui.check("确认", "是否保存设置？", "", "", "OK:保存  Set:取消");
  if (confirmed) {
    Serial.println("用户选择保存");
  } else {
    Serial.println("用户取消保存");
  }

  // 循环一次后重启示例（真实产品中可连续运行）
  delay(2000);
  Serial.println("===== 新一轮演示 =====");
}