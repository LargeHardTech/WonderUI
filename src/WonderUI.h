#ifndef WONDERUI_H
#define WONDERUI_H

#include <U8g2lib.h>
#include <Arduino.h>
#include <vector>

class WonderUI {
public:
    // 构造函数：传入U8G2对象和6个按键引脚（默认与原始硬件一致）
    // 引脚：上, 下, 左, 右, 确认(OK), 返回(Set)
    WonderUI(U8G2 &display, 
             int upPin = 12, int downPin = 13, int leftPin = 14, 
             int rightPin = 27, int okPin = 33, int setPin = 26);

    // 初始化（设置按键引脚模式，U8G2初始化等）
    void begin(uint8_t contrast = 255);

    // 播放启动动画
    void startAnimation();

    // 字符串输入（全键盘）
    // title: 显示在顶部的标题（可选）
    // maxLength: 最大字符数
    String inputString(const String& title = "", int maxLength = 25);

    // 数字输入
    // maxLength: 最大位数
    // base: 'D' 十进制键盘（0-9，清除，退格），'B' 二进制键盘（0,1，清除，退格）
    String inputNumber(int maxLength = 10, char base = 'D');

    // 通用列表选择（支持任意多个选项）
    // title: 对话框标题
    // options: 选项列表（字符串）
    // 返回选中项的索引（0起），若用户按 Set 退出则返回 -1
    int listChoose(const String& title, const std::vector<String>& options);

    // 数值增减调节
    // title: 标题
    // minVal, maxVal: 最小/最大值
    // initial: 初始值
    int numericUpDown(const String& title, int minVal, int maxVal, int initial = 0);

    // 消息框（带确认/取消）
    // title: 标题
    // line1~line3: 显示的三行文本
    // line4: 底部按钮提示，默认 "OK:继续   Set:退出"
    bool messageBox(const String& title, const String& line1, 
                    const String& line2 = "", const String& line3 = "", 
                    const String& line4 = "");

    // 简单提示框（只有 OK 按钮）
    void alert(const String& title, const String& line1, 
               const String& line2 = "", const String& line3 = "");

private:
    U8G2& u8g2;
    int _pinUp, _pinDown, _pinLeft, _pinRight, _pinOk, _pinSet;

    // 内部按键读取（直接读取引脚）
    bool getButton(char key);

    // 辅助绘制函数
    void drawListFrame(int y, const String& text, bool selected);
    void drawScrollIndicator(int page, int totalPages);

    // 原始输入逻辑的移植
    String inputRaw(int maxl, bool numericMode, char base);
    int listChooseRaw(const String& title, const std::vector<String>& options);
};

#endif
