#include "WonderUI.h"

// ==================== 构造与初始化 ====================
WonderUI::WonderUI(U8G2 &display,
                   int upPin, int downPin, int leftPin, int rightPin,
                   int okPin, int setPin)
    : u8g2(display),
      _pinUp(upPin), _pinDown(downPin), _pinLeft(leftPin), _pinRight(rightPin),
      _pinOk(okPin), _pinSet(setPin) {
}

void WonderUI::begin(uint8_t contrast) {
    // 设置按键引脚为输入上拉
    pinMode(_pinUp, INPUT_PULLUP);
    pinMode(_pinDown, INPUT_PULLUP);
    pinMode(_pinLeft, INPUT_PULLUP);
    pinMode(_pinRight, INPUT_PULLUP);
    pinMode(_pinOk, INPUT_PULLUP);
    pinMode(_pinSet, INPUT_PULLUP);

    u8g2.begin();
    u8g2.setContrast(contrast);
    u8g2.enableUTF8Print();
}

// ==================== 按键读取（内部） ====================
bool WonderUI::getButton(char key) {
    int pin;
    switch (key) {
        case 'U': pin = _pinUp; break;
        case 'D': pin = _pinDown; break;
        case 'L': pin = _pinLeft; break;
        case 'R': pin = _pinRight; break;
        case 'O': pin = _pinOk; break;
        case 'S': pin = _pinSet; break;
        default: return false;
    }
    // 低电平表示按键按下（INPUT_PULLUP 模式）
    return digitalRead(pin) == LOW;
}

// ==================== 启动动画 ====================
void WonderUI::startAnimation() {
    // 渐变显示 "LargeHard"
    for (int i = 1; i <= 100; i += 5) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_helvB18_tf);
            u8g2.setFontPosTop();
            u8g2.setContrast(i);
            u8g2.setCursor(0, 20);
            u8g2.print("LargeHard");
        } while (u8g2.nextPage());
    }
    delay(300);

    // 椭圆动画
    for (int i = 1; i <= 20; i++) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_timR08_tf);
            u8g2.setFontPosTop();
            u8g2.drawEllipse(64, 32, 20, i, U8G2_DRAW_ALL);
        } while (u8g2.nextPage());
        delay(50);
    }
    delay(150);

    // 圆环扩散
    for (int i = 1; i <= 10; i++) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_timR08_tf);
            u8g2.drawCircle(64 + i * 2, 32, 20 - i, U8G2_DRAW_ALL);
        } while (u8g2.nextPage());
        delay(50);
    }
    delay(80);

    // 最后的品牌名
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvR14_tf);
        u8g2.setFontPosTop();
        u8g2.setCursor(5, 23);
        u8g2.print("Wonder");
        u8g2.setFont(u8g2_font_helvR18_tf);
        u8g2.setCursor(95, 22);
        u8g2.print("S");
    } while (u8g2.nextPage());
    delay(150);
}

// ==================== 字符串输入（全键盘） ====================
String WonderUI::inputString(const String& title, int maxLength) {
    String otp = "";
    int x = 0, y = 0;
    bool caps = false;

    char a[4][10] = {
        {'1','2','3','4','5','6','7','8','9','0'},
        {'a','b','c','d','e','f','g','h','i','j'},
        {'k','l','m','n','o','p','q','r','s','t'},
        {'u','v','w','x','y','z',',','.','^','<'}
    };
    char b[4][10] = {
        {'A','B','C','D','E','F','G','H','I','J'},
        {'K','L','M','N','O','P','Q','R','S','T'},
        {'U','V','W','X','Y','Z','?','!','(',')'},
        {':','@','+','-','*','/','\'','"','^','<'}
    };

    delay(200);
    while (true) {
        u8g2.firstPage();
        do {
            u8g2.drawFrame(1, 1, 127, 12);
            u8g2.setFont(u8g2_font_timR08_tf);
            u8g2.setFontPosTop();
            u8g2.setCursor(2, 2);
            if (title.length() > 0) {
                u8g2.print(title + ":");
            }
            u8g2.print(otp);

            u8g2.setFont(u8g2_font_helvR12_tf);
            if (!caps) {
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 4; j++) {
                        u8g2.setCursor(i * 12 + 3, 13 + j * 12);
                        u8g2.print(a[j][i]);
                        if (i == x && j == y) {
                            u8g2.drawFrame(i * 12 + 3, 13 + j * 12, 11, 14);
                        }
                    }
                }
            } else {
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 4; j++) {
                        u8g2.setCursor(i * 12 + 3, 13 + j * 12);
                        u8g2.print(b[j][i]);
                        if (i == x && j == y) {
                            u8g2.drawFrame(i * 12 + 3, 13 + j * 12, 11, 14);
                        }
                    }
                }
            }
        } while (u8g2.nextPage());

        bool flag = false;
        if (getButton('S')) {
            break;
        } else if (getButton('U')) {
            y = (y - 1 + 4) % 4;
            flag = true;
        } else if (getButton('D')) {
            y = (y + 1) % 4;
            flag = true;
        } else if (getButton('L')) {
            x = (x - 1 + 10) % 10;
            flag = true;
        } else if (getButton('R')) {
            x = (x + 1) % 10;
            flag = true;
        } else if (getButton('O')) {
            flag = true;
            char ch = caps ? b[y][x] : a[y][x];
            if (ch == '^') {
                caps = !caps;
            } else if (ch == '<') {
                if (otp.length() > 0) otp.remove(otp.length() - 1);
            } else {
                if (otp.length() < maxLength) otp += ch;
            }
        }
        if (flag) delay(200);
    }
    return otp;
}

// ==================== 数字输入 ====================
String WonderUI::inputNumber(int maxLength, char base) {
    String otp = "";
    int x = 0, y = 0;

    delay(200);
    if (base == 'D') {
        const char a[2][6] = {
            {'1','2','3','4','5','C'},
            {'6','7','8','9','0','<'}
        };
        while (true) {
            u8g2.firstPage();
            do {
                u8g2.drawFrame(1, 1, 127, 12);
                u8g2.setFont(u8g2_font_timR08_tf);
                u8g2.setFontPosTop();
                u8g2.setCursor(2, 2);
                u8g2.print(otp);
                u8g2.setFont(u8g2_font_helvB24_tf);
                for (int i = 0; i < 6; i++) {
                    for (int j = 0; j < 2; j++) {
                        u8g2.setCursor(i * 21 + 1, 14 + j * 25);
                        u8g2.print(a[j][i]);
                        if (i == x && j == y) {
                            u8g2.drawFrame(i * 21, 15 + j * 28, 20, 24);
                        }
                    }
                }
            } while (u8g2.nextPage());

            bool flag = false;
            if (getButton('S')) break;
            else if (getButton('U')) { y = (y - 1 + 2) % 2; flag = true; }
            else if (getButton('D')) { y = (y + 1) % 2; flag = true; }
            else if (getButton('L')) { x = (x - 1 + 6) % 6; flag = true; }
            else if (getButton('R')) { x = (x + 1) % 6; flag = true; }
            else if (getButton('O')) {
                flag = true;
                char ch = a[y][x];
                if (ch == 'C') otp = "";
                else if (ch == '<') { if (otp.length()) otp.remove(otp.length() - 1); }
                else if (otp.length() < maxLength) otp += ch;
            }
            if (flag) delay(200);
        }
    } else if (base == 'B') {
        const char a[4] = {'0','1','C','<'};
        while (true) {
            u8g2.firstPage();
            do {
                u8g2.drawFrame(1, 1, 127, 12);
                u8g2.setFont(u8g2_font_timR08_tf);
                u8g2.setFontPosTop();
                u8g2.setCursor(2, 2);
                u8g2.print(otp);
                u8g2.setFont(u8g2_font_helvB24_tf);
                for (int i = 0; i < 4; i++) {
                    u8g2.setCursor(i * 21 + 1, 14);
                    u8g2.print(a[i]);
                    if (i == x) u8g2.drawFrame(i * 21, 14, 20, 24);
                }
            } while (u8g2.nextPage());

            bool flag = false;
            if (getButton('S')) break;
            else if (getButton('L')) { x = (x - 1 + 4) % 4; flag = true; }
            else if (getButton('R')) { x = (x + 1) % 4; flag = true; }
            else if (getButton('O')) {
                flag = true;
                char ch = a[x];
                if (ch == 'C') otp = "";
                else if (ch == '<') { if (otp.length()) otp.remove(otp.length() - 1); }
                else if (otp.length() < maxLength) otp += ch;
            }
            if (flag) delay(200);
        }
    }
    return otp;
}

// ==================== 通用列表选择 ====================
int WonderUI::listChoose(const String& title, const std::vector<String>& options) {
    if (options.empty()) return -1;
    int page = 0;
    int maxPage = options.size() - 1;
    bool update = false;

    while (true) {
        if (getButton('D')) {
            page++;
            if (page > maxPage) page = 0;
            update = true;
        } else if (getButton('U')) {
            page--;
            if (page < 0) page = maxPage;
            update = true;
        } else if (getButton('O')) {
            return page;
        } else if (getButton('S')) {
            return -1;
        }

        u8g2.firstPage();
        do {
            u8g2.drawHLine(0, 9, 128);
            u8g2.setFont(u8g2_font_timR08_tf);
            u8g2.setFontPosTop();
            u8g2.setCursor(0, 0);
            u8g2.print(title + " - " + String(page + 1) + "/" + String(maxPage + 1));
            u8g2.drawVLine(125, 9, 56);

            u8g2.setFont(u8g2_font_wqy12_t_gb2312);
            u8g2.setFontPosTop();
            u8g2.drawFrame(2, 11, 121, 13);
            u8g2.setCursor(12, 13);
            u8g2.print(options[page]);
            u8g2.setFontPosBottom();
            u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
            u8g2.drawGlyph(3, 13 + 8, 129);
        } while (u8g2.nextPage());

        if (update) {
            delay(200);
            update = false;
        }
    }
}

// ==================== 数值增减调节 ====================
int WonderUI::numericUpDown(const String& title, int minVal, int maxVal, int initial) {
    int value = initial;
    while (true) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_timR08_tf);
            u8g2.setFontPosTop();
            u8g2.setCursor(0, 0);
            u8g2.print(title);
            u8g2.drawHLine(0, 9, 128);
            u8g2.setFont(u8g2_font_wqy12_t_gb2312);
            u8g2.setFontPosTop();
            u8g2.setCursor(0, 11);
            u8g2.print(String(value));
            u8g2.setCursor(0, 52);
            u8g2.print("U/D:调整   O:确认   S:取消");
        } while (u8g2.nextPage());

        if (getButton('U')) {
            value++;
            if (value > maxVal) value = maxVal;
            delay(150);
        } else if (getButton('D')) {
            value--;
            if (value < minVal) value = minVal;
            delay(150);
        } else if (getButton('O')) {
            return value;
        } else if (getButton('S')) {
            return initial;
        }
        delay(50);
    }
}

// ==================== 消息框 ====================
bool WonderUI::messageBox(const String& title, const String& line1, const String& line2,
                          const String& line3, const String& line4) {
    String bottom = line4.length() ? line4 : "OK:继续   Set:退出";
    while (true) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_timR08_tf);
            u8g2.setFontPosTop();
            u8g2.setCursor(0, 0);
            u8g2.print(title);
            u8g2.drawHLine(0, 9, 128);
            u8g2.setFont(u8g2_font_wqy12_t_gb2312);
            u8g2.setFontPosTop();
            u8g2.setCursor(5, 13);
            u8g2.print(line1);
            u8g2.setCursor(5, 25);
            u8g2.print(line2);
            u8g2.setCursor(5, 37);
            u8g2.print(line3);
            u8g2.setCursor(0, 52);
            u8g2.print(bottom);
        } while (u8g2.nextPage());

        if (getButton('O')) return true;
        if (getButton('S')) return false;
        delay(50);
    }
}

void WonderUI::alert(const String& title, const String& line1, const String& line2, const String& line3) {
    while (true) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_timR08_tf);
            u8g2.setFontPosTop();
            u8g2.setCursor(0, 0);
            u8g2.print(title);
            u8g2.drawHLine(0, 9, 128);
            u8g2.setFont(u8g2_font_wqy12_t_gb2312);
            u8g2.setFontPosTop();
            u8g2.setCursor(5, 13);
            u8g2.print(line1);
            u8g2.setCursor(5, 25);
            u8g2.print(line2);
            u8g2.setCursor(5, 37);
            u8g2.print(line3);
            u8g2.setCursor(0, 52);
            u8g2.print("OK:继续");
        } while (u8g2.nextPage());
        if (getButton('O')) break;
        delay(50);
    }
}
