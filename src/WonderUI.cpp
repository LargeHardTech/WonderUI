#include "WonderUI.h"
#include "PyData.h"
#include <stdarg.h>
#include <math.h>

WonderUI::WonderUI(U8G2 &u8g2,
				   uint8_t pinU, uint8_t pinD, uint8_t pinL,
				   uint8_t pinR, uint8_t pinO, uint8_t pinS)
: _u8g2(&u8g2),
_pinU(pinU), _pinD(pinD), _pinL(pinL),
_pinR(pinR), _pinO(pinO), _pinS(pinS) {
}

void WonderUI::begin() {
	// 启用UTF8打印，以便显示中文
	_u8g2->enableUTF8Print();
	
	// 按钮引脚设为输入上拉（原代码模式）
	pinMode(_pinU, INPUT_PULLUP);
	pinMode(_pinD, INPUT_PULLUP);
	pinMode(_pinL, INPUT_PULLUP);
	pinMode(_pinR, INPUT_PULLUP);
	pinMode(_pinO, INPUT_PULLUP);
	pinMode(_pinS, INPUT_PULLUP);
}
void WonderUI::configureButtons(uint8_t pinU, uint8_t pinD, uint8_t pinL,
								uint8_t pinR, uint8_t pinO, uint8_t pinS) {
	_pinU = pinU;
	_pinD = pinD;
	_pinL = pinL;
	_pinR = pinR;
	_pinO = pinO;
	_pinS = pinS;
	
	pinMode(_pinU, INPUT_PULLUP);
	pinMode(_pinD, INPUT_PULLUP);
	pinMode(_pinL, INPUT_PULLUP);
	pinMode(_pinR, INPUT_PULLUP);
	pinMode(_pinO, INPUT_PULLUP);
	pinMode(_pinS, INPUT_PULLUP);
}
// ---- 全局按键音回调（app 注册） ----
KeySoundFn keySoundCb = nullptr;

bool WonderUI::GetButton(char x) {
	bool pressed;
	switch (x) {
		case 'U': pressed = !digitalRead(_pinU); break;
		case 'D': pressed = !digitalRead(_pinD); break;
		case 'L': pressed = !digitalRead(_pinL); break;
		case 'R': pressed = !digitalRead(_pinR); break;
		case 'O': pressed = !digitalRead(_pinO); break;
		case 'S': pressed = !digitalRead(_pinS); break;
		default: return false;
	}
	// 边沿触发按键音（OK=la, SET=do）
	if (x == 'O' || x == 'S') {
		static bool prevO = false, prevS = false;
		if (x == 'O' && pressed && !prevO && keySoundCb) keySoundCb('f');
		if (x == 'S' && pressed && !prevS && keySoundCb) keySoundCb('a');
		if (x == 'O') prevO = pressed;
		if (x == 'S') prevS = pressed;
	}
	return pressed;
}
//-------------------------------------------
// 开机动画
//-------------------------------------------
void WonderUI::start() {
	for (int i = 1; i <= 100; i += 5) {
		_u8g2->firstPage();
		do {
			_u8g2->setFont(u8g2_font_helvB18_tf);
			_u8g2->setFontPosTop();
			_u8g2->setContrast(i);
			_u8g2->setCursor(0, 20);
			_u8g2->print("LargeHard");
		} while (_u8g2->nextPage());
	}
	delay(300);
	
	for (int i = 1; i <= 20; i++) {
		_u8g2->firstPage();
		do {
			start1(i);
		} while (_u8g2->nextPage());
		delay(50);
	}
	delay(150);
	
	for (int i = 1; i <= 10; i++) {
		_u8g2->firstPage();
		do {
			start2(i * 2, i);
		} while (_u8g2->nextPage());
		delay(50);
	}
	delay(80);
	
	_u8g2->firstPage();
	do { start2(20, 10); } while (_u8g2->nextPage());
	delay(80);
	
	_u8g2->firstPage();
	do { start2(20, 10); start2(20, 11); } while (_u8g2->nextPage());
	delay(80);
	
	_u8g2->firstPage();
	do { start2(20, 10); start2(20, 11); start2(20, 12); } while (_u8g2->nextPage());
	delay(150);
	
	_u8g2->firstPage();
	do {
		start2(20, 10);
		start2(20, 11);
		start2(20, 12);
		start3();
		start4();
	} while (_u8g2->nextPage());
}

//-------------------------------------------
// 启动动画2：打字机式
//   ① 光标竖线从屏幕上方二次方缓动（越落越快）落到文字行
//   ② 向左移到打字起点（ease-in-out）
//   ③ 闪两下
//   ④ 逐字打出 "WonderU"（每字符后光标右移一个字符宽）
//   ⑤ U 右侧光标再闪两下 → 常亮（视觉上充当 "UI" 的 "I"）
//   ⑥ 定格后函数返回
//-------------------------------------------
void WonderUI::startTypewriter() {
	const int   TEXT_Y   = 23;              // 文字顶部 y（屏幕视觉中心）
	const int   CUR_W    = 2;               // 光标宽 px（原 4，收窄 2）
	const int   CUR_H    = 20;              // 光标高 px（原 18，向下伸长 2）
	const int   CUR_DY   = 0;               // 光标相对文字基线偏移 px（再上移 3px）
	const int   X_DROP   = 64;              // 下落起始 x（屏幕中线）
	const char* TEXT     = "WonderU";       // 最后光标常亮充当 I，视觉为 "WonderUI"
	const int   TEXT_LEN = 7;

	_u8g2->setFont(u8g2_font_helvB18_tf);
	_u8g2->setFontPosTop();

	// 预计算每个字符的 x（光标落点 = 已打字符总宽）
	// 居中基准用原光标宽 2，再整体左移 3px（先左移 5，后又右移 2）
	int charX[TEXT_LEN + 1];
	charX[0] = (_u8g2->getDisplayWidth() - _u8g2->getStrWidth("WonderU") - 2) / 2 - 3;
	for (int i = 0; i < TEXT_LEN; i++) {
		char s[2] = { TEXT[i], 0 };
		charX[i + 1] = charX[i] + _u8g2->getStrWidth(s);
	}

	// ---- 角标署名（光标落到屏幕中心时同步开始，两句节奏不同、不一定同时结束，之后常亮）----
	_u8g2->setFont(u8g2_font_5x8_tf);
	const char* SIGN_L  = "Powered by";
	const char* SIGN_R  = "LargeHardTech";
	const int   SIGN_LX = 2;                                     // 左上角
	const int   SIGN_LY = 1;
	const int   SIGN_RX = 128 - _u8g2->getStrWidth(SIGN_R) - 2;  // 右下角
	const int   SIGN_RY = 55;
	const unsigned long BLINK_L = 80;     // Powered by 闪烁半周期（闪得快）
	const unsigned long BLINK_R = 110;    // LargeHardTech 闪烁半周期（稍慢，结束也不同步）

	// 时间轴（ms）
	const unsigned long D_DROP    = 400;    // 下落
	const unsigned long D_MOVE    = 350;    // 左移
	const unsigned long D_BLINK   = 480;    // 闪两下（亮120灭120 循环）
	const unsigned long D_FIRST   = 200;    // 首字符前的停顿
	const unsigned long D_TYPE    = 120;    // 每字符间隔
	const unsigned long D_BLINK2  = 480;    // U 后闪两下
	const unsigned long D_HOLD    = 600;    // 常亮定格
	const unsigned long T_MOVE_END  = D_DROP + D_MOVE;
	const unsigned long T_BLINK_END = T_MOVE_END + D_BLINK;
	const unsigned long T_TYPE_END  = T_BLINK_END + D_FIRST + D_TYPE * (TEXT_LEN - 1);
	const unsigned long T_BLINK2_END = T_TYPE_END + D_BLINK2;
	const unsigned long T_END        = T_BLINK2_END + D_HOLD;

	unsigned long t0 = millis();
	while (true) {
		unsigned long t = millis() - t0;
		if (t >= T_END) break;

		int  curX, curY;
		bool curVisible = true;
		int  shown = 0;                      // 已打出的字符数

		if (t < D_DROP) {
			// ① 下落：二次方缓动 y = y0 + dy·p²（越落越快），从屏幕外开始
			float p = (float)t / D_DROP;
			curX = X_DROP;
			curY = -(CUR_H + CUR_DY) + (int)((TEXT_Y + CUR_H + CUR_DY) * p * p);
		}
		else if (t < T_MOVE_END) {
			// ② 左移：ease-in-out quad
			float p = (float)(t - D_DROP) / D_MOVE;
			float e = p < 0.5f ? 2 * p * p : 1 - (-2 * p + 2) * (-2 * p + 2) / 2;
			curX = X_DROP + (int)((charX[0] - X_DROP) * e);
			curY = TEXT_Y;
		}
		else if (t < T_BLINK_END) {
			// ③ 闪两下
			curX = charX[0]; curY = TEXT_Y;
			curVisible = (((t - T_MOVE_END) / 120) % 2) == 0;
		}
		else if (t < T_TYPE_END) {
			// ④ 逐字打出：首字符 200ms，之后每 120ms 一个
			unsigned long tt = t - T_BLINK_END;
			if (tt >= D_FIRST) {
				int n = 1 + (int)((tt - D_FIRST) / D_TYPE);
				if (n > TEXT_LEN) n = TEXT_LEN;
				shown = n;
			}
			curX = charX[shown]; curY = TEXT_Y;
		}
		else if (t < T_BLINK2_END) {
			// ⑤ U 右侧闪两下
			shown = TEXT_LEN;
			curX = charX[TEXT_LEN]; curY = TEXT_Y;
			curVisible = (((t - T_TYPE_END) / 120) % 2) == 0;
		}
		else {
			// ⑥ 常亮定格
			shown = TEXT_LEN;
			curX = charX[TEXT_LEN]; curY = TEXT_Y;
		}

		// 绘制一帧（firstPage/nextPage 自动清缓冲）
		_u8g2->firstPage();
		do {
			_u8g2->setFont(u8g2_font_helvB18_tf);
			_u8g2->setFontPosTop();
			for (int i = 0; i < shown; i++) {
				char s[2] = { TEXT[i], 0 };
				_u8g2->setCursor(charX[i], TEXT_Y);
				_u8g2->print(s);
			}
			if (curVisible) {
				_u8g2->drawBox(curX, curY + CUR_DY, CUR_W, CUR_H);
			}

			// ---- 角标署名：光标刚到中心（开始左移）时同步启动，各闪两下后常亮 ----
			if (t >= D_DROP) {
				unsigned long st = t - D_DROP;
				_u8g2->setFont(u8g2_font_5x8_tf);
				_u8g2->setFontPosTop();
				bool showL = (st >= BLINK_L * 4) || (((st / BLINK_L) % 2) == 0);
				bool showR = (st >= BLINK_R * 4) || (((st / BLINK_R) % 2) == 0);
				if (showL) { _u8g2->setCursor(SIGN_LX, SIGN_LY); _u8g2->print(SIGN_L); }
				if (showR) { _u8g2->setCursor(SIGN_RX, SIGN_RY); _u8g2->print(SIGN_R); }
			}
		} while (_u8g2->nextPage());

		delay(2);
	}
}

void WonderUI::start1(int y) {
	_u8g2->setFont(u8g2_font_timR08_tf);
	_u8g2->setFontPosTop();
	_u8g2->drawEllipse(64, 32, 20, y, U8G2_DRAW_ALL);
}

void WonderUI::start2(int x, int b) {
	_u8g2->setFont(u8g2_font_timR08_tf);
	_u8g2->setFontPosTop();
	_u8g2->drawCircle(64 + x, 32, 20 - b, U8G2_DRAW_ALL);
}

void WonderUI::start3() {
	_u8g2->setFont(u8g2_font_helvR18_tf);
	_u8g2->setFontPosTop();
	_u8g2->setCursor(95, 22);
	_u8g2->print("S");
}

void WonderUI::start4() {
	_u8g2->setFont(u8g2_font_helvR14_tf);
	_u8g2->setFontPosTop();
	_u8g2->setCursor(5, 23);
	_u8g2->print("Wonder");
}

//-------------------------------------------
// 拼音查找：在 PY_DATA 中搜索匹配前缀的汉字
//-------------------------------------------
static String lookupPinyin(const String& prefix) {
    if (prefix.length() == 0) return "";
    String result;
    for (int i = 0; i < PY_COUNT; i++) {
        uint16_t off = PY_OFFSET[i];              // ESP32 flash 直接访问
        char py[8];
        int j = 0;
        while (j < 7 && PY_DATA[off + j] != 0) {
            py[j] = PY_DATA[off + j];
            j++;
        }
        py[j] = 0;
        if (String(py).startsWith(prefix)) {
            off += j + 1;
            uint16_t len = PY_CHARS[i];
            for (int k = 0; k < len; k++)
                result += (char)PY_DATA[off + k];
        }
    }
    return result;
}

//-------------------------------------------
// 字符输入键盘
//-------------------------------------------
// ---- 字符串可视宽度（中文 12px, ASCII 6px）----
static int visualWidth(const String& s, int fontW = 12) {
    int w = 0;
    for (int i = 0; i < (int)s.length(); ) {
        unsigned char c = s[i];
        if (c > 0x7F) { w += fontW; i += 3; }
        else { w += fontW / 2; i += 1; }
    }
    return w;
}

String WonderUI::input(int maxl, const String& init) {
    String otp = init;
    // 确保多字节 offset 在 sprint 内
    while (otp.length() > (unsigned)maxl * 3) otp = otp.substring(0, otp.length() - 1);
    String spel = "";
    int x = 0, y = 0;
    int mode = 0;            // 0=小写 1=大写 2=拼音
    float fx = 0.0f, fy = 0.0f;                 // 平滑光标位置（动画）
    unsigned long lastKey = 0;
    const unsigned long KEY_COOL = 130;         // 方向键冷却，长按可连续移动
    String pyCands = "";
    int pyScroll = 0;

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
    char pl[3][10] = {
        {'a','b','c','d','e','f','g','h','i','j'},
        {'k','l','m','n','o','p','q','r','s','t'},
        {'u','v','w','x','y','z',1,2,3,4}
    };

    delay(200);
    while (true) {
        // ---- 平滑光标逼近目标格（动画核心）----
        fx += ((float)x - fx) * 0.35f;
        if (fabsf((float)x - fx) < 0.02f) fx = (float)x;
        fy += ((float)y - fy) * 0.35f;
        if (fabsf((float)y - fy) < 0.02f) fy = (float)y;

        String display = otp;
        if (mode == 2) display += spel;

        _u8g2->firstPage();
        do {
            _u8g2->drawFrame(1, 1, 127, 13);
            _u8g2->setFont(u8g2_font_wqy12_t_gb2312);
            _u8g2->setFontPosTop();
            int vw = visualWidth(display);
            int sx = 0;
            if (vw > 122) sx = vw - 122;  // 超宽就滚
            _u8g2->setCursor(2 - sx, 3);
            _u8g2->print(display);

            // 键盘用 wqy12 字体（已在上面设置）
            _u8g2->setFont(u8g2_font_wqy12_t_gb2312);
            _u8g2->setFontPosTop();

            if (mode < 2) {
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 4; j++) {
                        _u8g2->setCursor(i * 12 + 3, 14 + j * 12);
                        char ch = (mode == 0) ? a[j][i] : b[j][i];
                        _u8g2->print(ch);
                    }
                }
            } else {
                int totalChars = pyCands.length() / 3;
                for (int i = 0; i < 10; i++) {
                    _u8g2->setCursor(i * 12 + 3, 16);
                    int ci = pyScroll + i;
                    if (ci < totalChars) {
                        int start = ci * 3;
                        if (start + 3 <= pyCands.length())
                            _u8g2->print(pyCands.substring(start, start + 3));
                    }
                }
                for (int j = 1; j <= 3; j++) {
                    for (int i = 0; i < 10; i++) {
                        _u8g2->setCursor(i * 12 + 3, 14 + j * 12);
                        char ch = pl[j-1][i];
                        if (ch == 1) _u8g2->print("\xE3\x80\x82");         // 。
                        else if (ch == 2) _u8g2->print("\xEF\xBC\x8C");     // ，
                        else if (ch == 3) _u8g2->print("^");
                        else if (ch == 4) _u8g2->print("<");
                        else { char s[2] = {ch, 0}; _u8g2->print(s); }
                    }
                }
            }

            // 平滑光标框（三种模式统一，画在字符之上；滑过去而不是瞬移）
            {
                int hlx = 3 + (int)(fx * 12);
                int hly;
                if (mode < 2) hly = 14 + (int)(fy * 12);
                else          hly = (fy < 0.5f) ? 16 : 14 + (int)(fy * 12);
                _u8g2->drawFrame(hlx, hly, 11, 14);
            }
        } while (_u8g2->nextPage());

        bool flag = false;
        unsigned long now = millis();
        if (GetButton('S')) {
            while (GetButton('S')) delay(10);
            if (mode == 2 && pyCands.length() > 0 && y == 0) {
                mode = 0; spel = ""; pyCands = ""; pyScroll = 0; x = 0; y = 0;
            } else break;
        } else if (GetButton('O')) {
            while (GetButton('O')) delay(10);
            flag = true;
            if (mode == 2 && y == 0) {
                int totalChars = pyCands.length() / 3;
                int ci = pyScroll + x;
                if (ci < totalChars) {
                    int start = ci * 3;
                    if (start + 3 <= pyCands.length()) {
                        otp += pyCands.substring(start, start + 3);
                    }
                }
                spel = ""; pyCands = ""; pyScroll = 0; x = 0; y = 0;
            } else if (mode < 2) {
                char ch = (mode == 0) ? a[y][x] : b[y][x];
                if (ch == '^') {
                    int oldMode = mode;
                    mode = (mode + 1) % 3;
                    if (mode == 2 || oldMode == 2) { x = 0; y = 0; }  // 进出拼音才重置
                    spel = ""; pyCands = ""; pyScroll = 0;
                }
                else if (ch == '<') otp = otp.substring(0, otp.length() - 1);
                else if (otp.length() < maxl * 3) otp += ch;  // maxl 按字符数，UTF-8 中文 3 字节/字
            } else if (y > 0) {
                int row = y - 1;
                int mi = pl[row][x];
                if (mi == 3) { mode = 0; x = 0; y = 0; spel = ""; pyCands = ""; pyScroll = 0; }
                else if (mi == 4) {
                    if (spel.length() > 0) {
                        spel = spel.substring(0, spel.length() - 1);
                        pyCands = lookupPinyin(spel); pyScroll = 0;
                    } else if (otp.length() > 0) {
                        // 无拼音 → 删 otp 最后一个字符（中文3字节，ASCII 1字节）
                        int n = 1;
                        if (otp.length() >= 3) {
                            char last = otp[otp.length() - 1];
                            if ((last & 0x80) != 0) n = 3;  // UTF-8 tail byte → 中文 3 字节
                        }
                        otp = otp.substring(0, otp.length() - n);
                    }
                } else if (mi == 1) otp += "\xE3\x80\x82";   // 。（与渲染一致，原为逗号，反了）
                else if (mi == 2) otp += "\xEF\xBC\x8C";     // ，（与渲染一致，原为句号，反了）
                else { spel += (char)mi; pyCands = lookupPinyin(spel); pyScroll = 0; }
            }
        } else if (now - lastKey > KEY_COOL) {
            if (GetButton('U')) { y--; if (y < 0) y = 3; lastKey = now; flag = true; }
            else if (GetButton('D')) { y++; if (y > 3) y = 0; lastKey = now; flag = true; }
            else if (GetButton('L')) {
                x--; if (x < 0) {
                    if (mode == 2 && y == 0) { if (pyScroll > 0) { pyScroll -= 10; if (pyScroll < 0) pyScroll = 0; x = 0; } else x = 9; }
                    else x = 9;
                } lastKey = now; flag = true;
            } else if (GetButton('R')) {
                x++; if (x >= 10) {
                    if (mode == 2 && y == 0) { int maxC = pyCands.length() / 3; if (pyScroll + 10 < maxC) { pyScroll += 10; x = 0; } else x = 9; }
                    else x = 0;
                } lastKey = now; flag = true;
            }
        }
        delay(10);   // 动画帧间隔（方向键改用时间戳冷却，长按可连续移动）
    }
    return otp;
}
String WonderUI::inputnum(int maxl, char base, const String& init) {
	String otp = init;
	// 确保不超限
	while (otp.length() > (unsigned)maxl) otp = otp.substring(0, otp.length() - 1);
	int x = 0, y = 0;
	float fx = 0.0f, fy = 0.0f;                 // 平滑光标位置（动画）
	unsigned long lastKey = 0;
	const unsigned long KEY_COOL = 130;
	delay(200);

	if (base == 'D') {
		const char a[2][6] = {
			{'1','2','3','4','5','C'},
			{'6','7','8','9','0','<'}
		};
		while (true) {
			// 平滑光标逼近
			fx += ((float)x - fx) * 0.35f;
			if (fabsf((float)x - fx) < 0.02f) fx = (float)x;
			fy += ((float)y - fy) * 0.35f;
			if (fabsf((float)y - fy) < 0.02f) fy = (float)y;

			_u8g2->firstPage();
			do {
			_u8g2->drawFrame(1, 1, 127, 12);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			int nsx = otp.length() * 8;
			if (nsx > 122) nsx = nsx - 122; else nsx = 0;
			_u8g2->setCursor(2 - nsx, 2);
			_u8g2->print(otp);
				_u8g2->setFont(u8g2_font_helvB24_tf);
				_u8g2->setFontPosTop();
				for (int i = 0; i < 6; i++) {
					for (int j = 0; j < 2; j++) {
						_u8g2->setCursor(i * 21 + 1, 14 + j * 25);
						_u8g2->print(a[j][i]);
					}
				}
				// 平滑光标框（滑过去而非瞬移）
				_u8g2->drawFrame((int)(fx * 21), 15 + (int)(fy * 28), 20, 24);
			} while (_u8g2->nextPage());

			unsigned long now = millis();
			if (GetButton('S')) { while (GetButton('S')) delay(10); break; }  // 等 SET 松手
			else if (now - lastKey > KEY_COOL) {
				if (GetButton('U')) { y--; if (y < 0) y = 1; lastKey = now; }
				else if (GetButton('D')) { y++; if (y >= 2) y = 0; lastKey = now; }
				else if (GetButton('L')) { x--; if (x < 0) x = 5; lastKey = now; }
				else if (GetButton('R')) { x++; if (x >= 6) x = 0; lastKey = now; }
				else if (GetButton('O')) {
					lastKey = now;
					if (a[y][x] == 'C') otp = "";
					else if (a[y][x] == '<') otp = otp.substring(0, otp.length() - 1);
					else if (otp.length() <= maxl) otp += a[y][x];
				}
			}
			delay(10);
		}
	} else if (base == 'B') {
		const char a[4] = {'0','1','C','<'};
		while (true) {
			// 平滑光标逼近
			fx += ((float)x - fx) * 0.35f;
			if (fabsf((float)x - fx) < 0.02f) fx = (float)x;

			_u8g2->firstPage();
			do {
			_u8g2->drawFrame(1, 1, 127, 12);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			int nsx = otp.length() * 8;
			if (nsx > 122) nsx = nsx - 122; else nsx = 0;
			_u8g2->setCursor(2 - nsx, 2);
			_u8g2->print(otp);
				_u8g2->setFont(u8g2_font_helvB24_tf);
				_u8g2->setFontPosTop();
				for (int i = 0; i < 4; i++) {
					_u8g2->setCursor(i * 21 + 1, 14);
					_u8g2->print(a[i]);
				}
				_u8g2->drawFrame((int)(fx * 21), 14, 20, 24);
			} while (_u8g2->nextPage());

			unsigned long now = millis();
			if (GetButton('S')) { while (GetButton('S')) delay(10); break; }  // 等 SET 松手
			else if (now - lastKey > KEY_COOL) {
				if (GetButton('L')) { x--; if (x < 0) x = 3; lastKey = now; }
				else if (GetButton('R')) { x++; if (x >= 4) x = 0; lastKey = now; }
				else if (GetButton('O')) {
					lastKey = now;
					if (a[x] == 'C') otp = "";
					else if (a[x] == '<') otp = otp.substring(0, otp.length() - 1);
					else if (otp.length() <= maxl) otp += a[x];
				}
			}
			delay(10);
		}
	}
	return otp;
}

//-------------------------------------------
// 四选项列表选择
//-------------------------------------------
int WonderUI::listchoose4(String x, String y, String z, String a, String title) {
	int p = 0;
	const int mp = 3;
	const int N  = 4;
	String items[4] = { x, y, z, a };
	float slide = 0.0f;                        // 页切换纵向滑动
	unsigned long lastKey = 0;
	const unsigned long KEY_COOL = 150;

	// 渲染一帧（edy = 入场纵向偏移，0 为正常位置）
	auto renderList = [&](int edy) {
		int dy = (int)slide + edy;
		_u8g2->firstPage();
		do {
			_u8g2->drawHLine(0, 9 + edy, 128);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0 + edy);
			_u8g2->print(title + "-Page" + String(p + 1));
			_u8g2->drawVLine(125, 9 + edy, 56);

			// 4 行统一按 dy 偏移绘制（原 4 个 if 块参数化，选中项跟随滑动）
			for (int i = 0; i < N; i++) {
				int y = 11 + i * 14 + dy;
				if (y < 9 || y > 55) continue;
				_u8g2->drawFrame(2, y, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, y + 2);
				_u8g2->print(items[i]);
				if (i == p) {
					_u8g2->setFontPosBottom();
					_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
					_u8g2->drawGlyph(3, y + 2 + 1 * 8, 129);
				}
			}
		} while (_u8g2->nextPage());
	};

	// ---- 入场：从底部往上弹出（ease-out，8 帧）----
	for (int f = 0; f <= 7; f++) {
		float ep = (float)f / 7.0f;
		float ee = 1 - (1 - ep) * (1 - ep);
		renderList((int)(64 * (1 - ee)));
	}

	while (true) {
		unsigned long now = millis();

		// ---- 按键（冷却 + 触发滑动）----
		if (now - lastKey > KEY_COOL) {
			if (GetButton('D')) { p++; if (p > mp) p = 0; slide = 14.0f; lastKey = now; }
			else if (GetButton('U')) { p--; if (p < 0) p = mp; slide = -14.0f; lastKey = now; }
		}
		if (GetButton('O')) {
			while (GetButton('O')) delay(10);
			// 确认进入下一层：整页向下滑出
			for (int f = 1; f <= 6; f++) {
				float ep = (float)f / 6.0f;
				renderList((int)(64 * ep * ep));
			}
			return p;
		}

		// ---- 滑动收敛（ease-out）----
		slide += (0.0f - slide) * 0.35f;
		if (fabsf(slide) < 0.4f) slide = 0.0f;

		renderList(0);
		delay(10);
	}
	return -1;
}

//-------------------------------------------
// 三选项列表选择
//-------------------------------------------
int WonderUI::listchoose3(String x, String y, String z, String title) {
	int p = 0;
	const int mp = 2;
	const int N  = 3;
	String items[3] = { x, y, z };
	float slide = 0.0f;                        // 页切换纵向滑动
	unsigned long lastKey = 0;
	const unsigned long KEY_COOL = 150;

	// 渲染一帧（edy = 入场纵向偏移，0 为正常位置）
	auto renderList = [&](int edy) {
		int dy = (int)slide + edy;
		_u8g2->firstPage();
		do {
			_u8g2->drawHLine(0, 9 + edy, 128);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0 + edy);
			_u8g2->print(title + "-Page" + String(p + 1));
			_u8g2->drawVLine(125, 9 + edy, 56);

			// 3 行统一按 dy 偏移绘制（原 3 个 if 块参数化）
			for (int i = 0; i < N; i++) {
				int y = 11 + i * 14 + dy;
				if (y < 9 || y > 55) continue;
				_u8g2->drawFrame(2, y, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, y + 2);
				_u8g2->print(items[i]);
				if (i == p) {
					_u8g2->setFontPosBottom();
					_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
					_u8g2->drawGlyph(3, y + 2 + 1 * 8, 129);
				}
			}
		} while (_u8g2->nextPage());
	};

	// ---- 入场：从底部往上弹出（ease-out，8 帧）----
	for (int f = 0; f <= 7; f++) {
		float ep = (float)f / 7.0f;
		float ee = 1 - (1 - ep) * (1 - ep);
		renderList((int)(64 * (1 - ee)));
	}

	while (true) {
		unsigned long now = millis();

		// ---- 按键（冷却 + 触发滑动）----
		if (now - lastKey > KEY_COOL) {
			if (GetButton('D')) { p++; if (p > mp) p = 0; slide = 14.0f; lastKey = now; }
			else if (GetButton('U')) { p--; if (p < 0) p = mp; slide = -14.0f; lastKey = now; }
		}
		if (GetButton('O')) {
			while (GetButton('O')) delay(10);
			// 确认进入下一层：整页向下滑出
			for (int f = 1; f <= 6; f++) {
				float ep = (float)f / 6.0f;
				renderList((int)(64 * ep * ep));
			}
			return p;
		}

		// ---- 滑动收敛（ease-out）----
		slide += (0.0f - slide) * 0.35f;
		if (fabsf(slide) < 0.4f) slide = 0.0f;

		renderList(0);
		delay(10);
	}
	return -1;
}

//-------------------------------------------
// 不定长列表选择（可变参数，上限 8 项，一屏 4 项光标式）
//-------------------------------------------
int WonderUI::listchoose(String title, int count, ...) {
	const int MAX = 8;
	if (count < 1) return -1;
	if (count > MAX) count = MAX;

	// 读取可变参数（字符串字面量为 const char*）
	String items[MAX];
	va_list args;
	va_start(args, count);
	for (int i = 0; i < count; i++) {
		const char* s = va_arg(args, const char*);
		items[i] = s ? s : "";
	}
	va_end(args);

	int   p      = 0;         // 当前选中项
	int   offset = 0;         // 逻辑可视区顶部
	float scroll = 0.0f;      // 平滑滚动位置（单位：项）
	float focus  = 0.0f;      // 平滑光标位置（单位：项）
	const int VISIBLE = 4;
	unsigned long lastKey = 0;
	const unsigned long KEY_COOL = 150;   // 按键冷却，长按可连续滚动

	// 渲染一帧（edy = 入场纵向偏移，0 为正常位置）
	auto renderList = [&](int edy) {
		_u8g2->firstPage();
		do {
			_u8g2->drawHLine(0, 9 + edy, 128);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0 + edy);
			_u8g2->print(title);
			_u8g2->drawVLine(125, 9 + edy, 56);

			// 列表：按 scroll 连续偏移绘制（上下各多画一项，过渡时无缝）
			int base = (int)floorf(scroll);
			for (int i = -1; i <= VISIBLE; i++) {
				int idx = base + i;
				if (idx < 0 || idx >= count) continue;
				int y = 11 + edy + (int)((idx - scroll) * 14.0f);
				if (y < 9 || y > 55) continue;            // 裁剪出可视区
				_u8g2->drawFrame(2, y, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, y + 2);
				_u8g2->print(items[idx]);
			}
			// 光标：按 focus 平滑位置单独绘制（滑过去而不是瞬移）
			{
				int hy = 11 + edy + (int)((focus - scroll) * 14.0f);
				if (hy >= 9 && hy <= 55) {
					_u8g2->setFontPosBottom();
					_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
					_u8g2->drawGlyph(3, hy + 2 + 1 * 8, 129);
				}
			}
		} while (_u8g2->nextPage());
	};

	// ---- 入场：从底部往上弹出（ease-out，8 帧）----
	for (int f = 0; f <= 7; f++) {
		float ep = (float)f / 7.0f;
		float ee = 1 - (1 - ep) * (1 - ep);
		renderList((int)(64 * (1 - ee)));
	}

	while (true) {
		unsigned long now = millis();

		// ---- 按键 ----
		if (now - lastKey > KEY_COOL) {
			if (GetButton('D')) { p++; if (p >= count) p = 0; lastKey = now; }
			else if (GetButton('U')) { p--; if (p < 0) p = count - 1; lastKey = now; }
		}
		if (GetButton('O')) {
			while (GetButton('O')) delay(10);
			// 确认进入下一层：整页向下滑出
			for (int f = 1; f <= 6; f++) {
				float ep = (float)f / 6.0f;
				renderList((int)(64 * ep * ep));
			}
			return p;
		}
		if (GetButton('S')) {
			while (GetButton('S')) delay(10);
			// 取消：整页向下滑出（与入场对称）
			for (int f = 1; f <= 6; f++) {
				float ep = (float)f / 6.0f;
				renderList((int)(64 * ep * ep));
			}
			return -1;
		}

		// ---- 可视区目标（选中项越出时平移）----
		if (p < offset) offset = p;
		else if (p >= offset + VISIBLE) offset = p - VISIBLE + 1;
		if (offset > count - VISIBLE) offset = count - VISIBLE;
		if (offset < 0) offset = 0;

		// ---- 动画核心：scroll / focus 每帧向目标逼近（ease-out 手感）----
		scroll += ((float)offset - scroll) * 0.35f;
		if (fabsf((float)offset - scroll) < 0.02f) scroll = (float)offset;
		focus  += ((float)p - focus) * 0.35f;
		if (fabsf((float)p - focus) < 0.02f) focus = (float)p;

		renderList(0);
		delay(10);   // 动画帧间隔
	}
	return -1;
}

//-------------------------------------------
// 数组版列表选择（上限 20 项，一屏 4 项光标式）
//-------------------------------------------
int WonderUI::listchooseArr(const char* const* items, int count, const char* title) {
	return listchooseArr(items, nullptr, count, title);   // 无图标版转调带图标版
}

int WonderUI::listchooseArr(const char* const* items, const int* icons, int count, const char* title, int startIdx) {
	const int MAX = 20;
	if (count < 1 || items == NULL) return -1;
	if (count > MAX) count = MAX;

	int   p      = (startIdx > 0 && startIdx < count) ? startIdx : 0;   // 起始选中项
	int   offset = 0;         // 逻辑可视区顶部
	float scroll = 0.0f;      // 平滑滚动位置（单位：项）
	float focus  = (float)p;  // 平滑光标位置（单位：项）
	const int VISIBLE = 4;
	unsigned long lastKey = 0;
	const unsigned long KEY_COOL = 150;

	// 渲染一帧（edy = 入场纵向偏移，0 为正常位置）
	auto renderList = [&](int edy) {
		_u8g2->firstPage();
		do {
			_u8g2->drawHLine(0, 9 + edy, 128);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0 + edy);
			_u8g2->print(title);
			_u8g2->drawVLine(125, 9 + edy, 56);

			// 列表：按 scroll 连续偏移绘制
			int base = (int)floorf(scroll);
			for (int i = -1; i <= VISIBLE; i++) {
				int idx = base + i;
				if (idx < 0 || idx >= count) continue;
				int y = 11 + edy + (int)((idx - scroll) * 14.0f);
				if (y < 9 || y > 55) continue;
				_u8g2->drawFrame(2, y, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, y + 2);
				_u8g2->print(items[idx] ? items[idx] : "");
			}
			// 光标 / 图标：统一按 focus 平滑位置绘制（页内移动滑过去而不是瞬移）
			// 传了 icons 则画选中项自己的图标，否则画默认光标 129
			{
				int  hy    = 11 + edy + (int)((focus - scroll) * 14.0f);
				int  glyph = (icons != nullptr) ? icons[p] : 129;
				if (hy >= 9 && hy <= 55 && glyph > 0) {
					_u8g2->setFontPosBottom();
					_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
					_u8g2->drawGlyph(3, hy + 2 + 1 * 8, glyph);
				}
			}
		} while (_u8g2->nextPage());
	};

	// ---- 入场：从底部往上弹出（ease-out，8 帧）----
	for (int f = 0; f <= 7; f++) {
		float ep = (float)f / 7.0f;
		float ee = 1 - (1 - ep) * (1 - ep);
		renderList((int)(64 * (1 - ee)));
	}

	while (true) {
		unsigned long now = millis();

		// ---- 按键（冷却，长按连续滚）----
		if (now - lastKey > KEY_COOL) {
			if (GetButton('D')) { p++; if (p >= count) p = 0; lastKey = now; }
			else if (GetButton('U')) { p--; if (p < 0) p = count - 1; lastKey = now; }
		}
		if (GetButton('O')) {
			while (GetButton('O')) delay(10);
			// 确认进入下一层：整页向下滑出
			for (int f = 1; f <= 6; f++) {
				float ep = (float)f / 6.0f;
				renderList((int)(64 * ep * ep));
			}
			return p;
		}
		if (GetButton('S')) {
			while (GetButton('S')) delay(10);
			// 退出：整页向下滑出（与入场对称，ease-in 加速离场）
			for (int f = 1; f <= 6; f++) {
				float ep = (float)f / 6.0f;
				renderList((int)(64 * ep * ep));
			}
			return -1;
		}

		// ---- 可视区目标 ----
		if (p < offset) offset = p;
		else if (p >= offset + VISIBLE) offset = p - VISIBLE + 1;
		if (offset > count - VISIBLE) offset = count - VISIBLE;
		if (offset < 0) offset = 0;

		// ---- 动画：平滑逼近 ----
		scroll += ((float)offset - scroll) * 0.35f;
		if (fabsf((float)offset - scroll) < 0.02f) scroll = (float)offset;
		focus  += ((float)p - focus) * 0.35f;
		if (fabsf((float)p - focus) < 0.02f) focus = (float)p;

		renderList(0);
		delay(10);
	}
	return -1;
}

//-------------------------------------------
// 数值上下调节
//-------------------------------------------
int WonderUI::numericUpDown(int maxn, int minn, String title) {
	int   otp   = 0;
	int   prev  = 0;            // 上一个值（滚出动画用）
	float slide = 0.0f;         // 数字纵向偏移，每帧向 0 收敛
	unsigned long lastKey = 0;
	const unsigned long KEY_COOL = 130;

	while (true) {
		unsigned long now = millis();

		// ---- 按键（冷却，长按连续加减）----
		if (now - lastKey > KEY_COOL) {
			if (GetButton('R')) {
				prev = otp; otp++; slide = 12.0f;    // 加：新值自下而上滚入
				lastKey = now;
			} else if (GetButton('L')) {
				prev = otp; otp--; slide = -12.0f;   // 减：新值自上而下滚入
				lastKey = now;
			}
		}
		// 限幅（限幅后若变化，同步 prev 避免视觉跳变）
		if (otp < minn) { otp = minn; prev = otp; slide = 0.0f; }
		else if (otp > maxn) { otp = maxn; prev = otp; slide = 0.0f; }

		// ---- 滑动收敛（ease-out）----
		slide += (0.0f - slide) * 0.30f;
		if (fabsf(slide) < 0.4f) slide = 0.0f;

		_u8g2->firstPage();
		do {
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0);
			_u8g2->print(title);
			_u8g2->drawHLine(0, 9, 128);

			// 数字滚动：当前值 + 旧值反向滑出
			_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 11 + (int)slide);
			_u8g2->print(String(otp));
			if (slide != 0.0f) {
				int prevY = 11 + (int)slide + (slide > 0 ? -12 : 12);
				if (prevY > 9 && prevY < 40) {
					_u8g2->setCursor(0, prevY);
					_u8g2->print(String(prev));
				}
			}

			_u8g2->setCursor(0, 52);
			_u8g2->print("R:加  L:减");

			// 右侧方向指示图标
			_u8g2->setFontPosBottom();
			_u8g2->setFont(u8g2_font_open_iconic_all_6x_t);
			if (GetButton('R'))      _u8g2->drawGlyph(80, 11 + 48, 75);
			else if (GetButton('L')) _u8g2->drawGlyph(80, 11 + 48, 74);
			else                     _u8g2->drawGlyph(80, 11 + 48, 71);
		} while (_u8g2->nextPage());

		if (GetButton('O')) { while (GetButton('O')) delay(10); break; }  // 等 OK 松手再返回
		delay(10);
	}
	return otp;
}

//-------------------------------------------
// 确认/信息框
//-------------------------------------------
bool WonderUI::check(String title, String line1, String line2, String line3, String line4) {
	float slide = 64.0f;                    // 整页从屏幕下方滑入

	// 渲染一帧（dy = 纵向偏移，0 为正常位置）
	auto renderFrame = [&](int dy) {
		_u8g2->firstPage();
		do {
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0 + dy);
			_u8g2->print(title);
			_u8g2->setFont(u8g2_font_timB12_tf);
			_u8g2->setFontPosTop();
			_u8g2->drawHLine(0, 9 + dy, 128);
			_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(5, 13 + dy);
			_u8g2->print(line1);
			_u8g2->setCursor(5, 25 + dy);
			_u8g2->print(line2);
			_u8g2->setCursor(5, 37 + dy);
			_u8g2->print(line3);
			_u8g2->setCursor(0, 52 + dy);
			_u8g2->print(line4);
		} while (_u8g2->nextPage());
	};

	// 退出：整页向下滑出（ease-in 加速离场，5 帧）
	auto slideOut = [&]() {
		for (int f = 1; f <= 5; f++) {
			float ep = (float)f / 5.0f;
			renderFrame((int)(64 * ep * ep));
		}
	};

	while (true) {
		if (GetButton('O')) {
			while (GetButton('O')) delay(10);  // 等 OK 松手，避免残留触发
			slideOut();
			return true;
		}
		else if (GetButton('S')) {
			while (GetButton('S')) delay(10);  // 等 SET 松手
			slideOut();
			return false;
		}

		// 滑入收敛（ease-out）
		slide += (0.0f - slide) * 0.30f;
		if (fabsf(slide) < 0.4f) slide = 0.0f;
		int dy = (int)slide;

		renderFrame(dy);

		delay(10);
	}
	return true;
}

//-------------------------------------------
// 文本框：自动换行(10字/行)，4行/页，上下翻页
//-------------------------------------------
void WonderUI::textBox(String title, String text) {
	const int LINE_WIDTH  = 20;  // 每行 20 单位宽（英文=1, 中文=2）
	const int CH_PER_PAGE  = 4;

	// 1. 拆行：\n 强制换行；其余按显示宽度换行（中文 2 单位, 英文 1 单位, 满 20 换行）
	String lines[320];
	int totalLines = 0;
	const char* raw = text.c_str();
	int len = text.length();
	int bytePos = 0;
	while (bytePos < len && totalLines < 320) {
		int lineStart = bytePos;
		int lineWidth = 0;
		bool hardBreak = false;
		while (bytePos < len) {
			unsigned char c = raw[bytePos];
			if (c == '\n') { bytePos++; hardBreak = true; break; }   // 强制换行（不含 \n 本身）
			int charW = (c > 0x7F) ? 2 : 1;   // 中文宽 2, ASCII 宽 1
			int charBytes = (c > 0x7F) ? 3 : 1;  // UTF-8: 中文 3 字节, ASCII 1 字节
			if (lineWidth + charW > LINE_WIDTH) break;
			lineWidth += charW;
			bytePos += charBytes;
		}
		lines[totalLines++] = text.substring(lineStart, hardBreak ? bytePos - 1 : bytePos);
	}
	if (totalLines == 0) {
		lines[0] = "";
		totalLines = 1;
	}

	int totalPages = (totalLines + CH_PER_PAGE - 1) / CH_PER_PAGE;
	int curPage = 0;
	float slide = 0.0f;                     // 翻页滑动偏移（px），每帧向 0 收敛
	unsigned long lastKey = 0;
	const unsigned long KEY_COOL = 150;

	// 渲染一帧（edy = 入场纵向偏移，0 为正常位置）
	auto renderFrame = [&](int edy) {
		_u8g2->firstPage();
		do {
			// 标题栏
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0 + edy);
			String titleBar = title + " [" + String(curPage + 1) + "/" + String(totalPages) + "]";
			_u8g2->print(titleBar);
			_u8g2->drawHLine(0, 9 + edy, 128);

			// 内容：按 slide 偏移绘制，上下各多画一行保证过渡无缝
			_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
			_u8g2->setFontPosTop();
			int startLine = curPage * CH_PER_PAGE;
			for (int i = -1; i <= CH_PER_PAGE; i++) {
				int idx = startLine + i;
				if (idx < 0 || idx >= totalLines) continue;
				int y = 11 + edy + (int)(i * 13 + slide);
				if (y < 9 || y > 58) continue;         // 裁剪
				_u8g2->setCursor(0, y);
				_u8g2->print(lines[idx]);
			}
		} while (_u8g2->nextPage());
	};

	// ---- 入场：从底部往上弹出（ease-out，8 帧）----
	for (int f = 0; f <= 7; f++) {
		float ep = (float)f / 7.0f;
		float ee = 1 - (1 - ep) * (1 - ep);
		renderFrame((int)(64 * (1 - ee)));
	}

	while (true) {
		unsigned long now = millis();

		// ---- 翻页（冷却 + 设定滑入方向）----
		if (totalPages > 1 && now - lastKey > KEY_COOL) {
			if (GetButton('U')) {
				curPage--; if (curPage < 0) curPage = totalPages - 1;
				slide = -13.0f;              // 上翻：内容从上方滑入
				lastKey = now;
			} else if (GetButton('D')) {
				curPage++; if (curPage >= totalPages) curPage = 0;
				slide = 13.0f;               // 下翻：内容从下方滑入
				lastKey = now;
			}
		}
		if (GetButton('O')) {
			while (GetButton('O')) delay(10);
			// 退出：整页向下滑出
			for (int f = 1; f <= 6; f++) {
				float ep = (float)f / 6.0f;
				renderFrame((int)(64 * ep * ep));
			}
			return;
		}
		if (GetButton('S')) {
			while (GetButton('S')) delay(10);
			for (int f = 1; f <= 6; f++) {
				float ep = (float)f / 6.0f;
				renderFrame((int)(64 * ep * ep));
			}
			return;
		}

		// ---- 滑动回位（ease-out）----
		slide += (0.0f - slide) * 0.35f;
		if (fabsf(slide) < 0.3f) slide = 0.0f;

		renderFrame(0);

		delay(10);
	}
}

//-------------------------------------------
// 标准页：标题栏 + 内容区，每帧轮询 draw(dy)
// draw 返回 true 请求退出；SET 键统一退出（anim 开时带滑出动画）
//-------------------------------------------
void WonderUI::page(const String& title, std::function<bool(int dy)> draw, bool anim) {
	// 渲染一帧（dy = 纵向偏移，0 为正常位置），返回 draw 的退出请求
	auto renderPage = [&](int dy) -> bool {
		bool quit = false;
		_u8g2->firstPage();
		do {
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0 + dy);
			_u8g2->print(title);
			_u8g2->drawHLine(0, 9 + dy, 128);
			quit = draw(dy);
		} while (_u8g2->nextPage());
		return quit;
	};

	// ---- 入场：从底部往上弹出（ease-out，8 帧）----
	if (anim) {
		for (int f = 0; f <= 7; f++) {
			float ep = (float)f / 7.0f;
			float ee = 1 - (1 - ep) * (1 - ep);
			renderPage((int)(64 * (1 - ee)));
		}
	}

	while (true) {
		bool quit = (GetButton('S'));
		if (quit) {
			while (GetButton('S')) delay(10);
		} else {
			quit = renderPage(0);
		}
		if (quit) {
			if (anim) {
				// 退出：整页向下滑出（ease-in 加速离场，6 帧）
				for (int f = 1; f <= 6; f++) {
					float ep = (float)f / 6.0f;
					renderPage((int)(64 * ep * ep));
				}
			}
			return;
		}
		delay(10);
	}
}
