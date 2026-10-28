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
	const int   CUR_W    = 4;               // 光标宽 px（原 2，加宽 2）
	const int   CUR_H    = 20;              // 光标高 px（原 18，向下伸长 2）
	const int   CUR_DY   = 3;               // 光标相对文字基线下移 px
	const int   X_DROP   = 64;              // 下落起始 x（屏幕中线）
	const char* TEXT     = "WonderU";       // 最后光标常亮充当 I，视觉为 "WonderUI"
	const int   TEXT_LEN = 7;

	_u8g2->setFont(u8g2_font_helvB18_tf);
	_u8g2->setFontPosTop();

	// 预计算每个字符的 x（光标落点 = 已打字符总宽）
	// 居中基准仍用原光标宽 2，再整体左移 5px（所有元素统一左移）
	int charX[TEXT_LEN + 1];
	charX[0] = (_u8g2->getDisplayWidth() - _u8g2->getStrWidth("WonderU") - 2) / 2 - 5;
	for (int i = 0; i < TEXT_LEN; i++) {
		char s[2] = { TEXT[i], 0 };
		charX[i + 1] = charX[i] + _u8g2->getStrWidth(s);
	}

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
                        if (i == x && j == y)
                            _u8g2->drawFrame(i * 12 + 3, 14 + j * 12, 11, 14);
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
                    if (y == 0 && i == x)
                        _u8g2->drawFrame(i * 12 + 3, 16, 11, 14);
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
                        if (y == j && i == x)
                            _u8g2->drawFrame(i * 12 + 3, 14 + j * 12, 11, 14);
                    }
                }
            }
        } while (_u8g2->nextPage());

        bool flag = false;
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
        } else if (GetButton('U')) { y--; if (y < 0) y = (mode == 2 ? 3 : 3); flag = true; }
        else if (GetButton('D')) { y++; if (y > (mode == 2 ? 3 : 3)) y = 0; flag = true; }
        else if (GetButton('L')) {
            x--; if (x < 0) {
                if (mode == 2 && y == 0) { if (pyScroll > 0) { pyScroll -= 10; if (pyScroll < 0) pyScroll = 0; x = 0; } else x = 9; }
                else x = 9;
            } flag = true;
        } else if (GetButton('R')) {
            x++; if (x >= 10) {
                if (mode == 2 && y == 0) { int maxC = pyCands.length() / 3; if (pyScroll + 10 < maxC) { pyScroll += 10; x = 0; } else x = 9; }
                else x = 0;
            } flag = true;
        }
        if (flag) delay(200);
    }
    return otp;
}
String WonderUI::inputnum(int maxl, char base, const String& init) {
	String otp = init;
	// 确保不超限
	while (otp.length() > (unsigned)maxl) otp = otp.substring(0, otp.length() - 1);
	int x = 0, y = 0;
	delay(200);
	
	if (base == 'D') {
		const char a[2][6] = {
			{'1','2','3','4','5','C'},
			{'6','7','8','9','0','<'}
		};
		while (true) {
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
						if (i == x && j == y) {
							_u8g2->drawFrame(i * 21, 15 + j * 28, 20, 24);
						}
					}
				}
			} while (_u8g2->nextPage());
			
			bool flag = false;
			if (GetButton('S')) { while (GetButton('S')) delay(10); break; }  // 等 SET 松手
			else if (GetButton('U')) { y--; if (y < 0) y = 1; flag = true; }
			else if (GetButton('D')) { y++; if (y >= 2) y = 0; flag = true; }
			else if (GetButton('L')) { x--; if (x < 0) x = 5; flag = true; }
			else if (GetButton('R')) { x++; if (x >= 6) x = 0; flag = true; }
			else if (GetButton('O')) {
				flag = true;
				if (a[y][x] == 'C') otp = "";
				else if (a[y][x] == '<') otp = otp.substring(0, otp.length() - 1);
				else if (otp.length() <= maxl) otp += a[y][x];
			}
			if (flag) delay(200);
		}
	} else if (base == 'B') {
		const char a[4] = {'0','1','C','<'};
		while (true) {
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
					if (i == x) {
						_u8g2->drawFrame(i * 21, 14, 20, 24);
					}
				}
			} while (_u8g2->nextPage());
			
			bool flag = false;
			if (GetButton('S')) { while (GetButton('S')) delay(10); break; }  // 等 SET 松手
			else if (GetButton('L')) { x--; if (x < 0) x = 3; flag = true; }
			else if (GetButton('R')) { x++; if (x >= 4) x = 0; flag = true; }
			else if (GetButton('O')) {
				flag = true;
				if (a[x] == 'C') otp = "";
				else if (a[x] == '<') otp = otp.substring(0, otp.length() - 1);
				else if (otp.length() <= maxl) otp += a[x];
			}
			if (flag) delay(200);
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
	bool upd = false;
	
	while (true) {
		if (GetButton('D')) { p++; if (p > mp) p = 0; upd = true; }
		else if (GetButton('U')) { p--; if (p < 0) p = mp; upd = true; }
		else if (GetButton('O')) { while (GetButton('O')) delay(10); return p; }  // 等 OK 松手再返回
		
		_u8g2->firstPage();
		do {
			_u8g2->drawHLine(0, 9, 128);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0);
			_u8g2->print(title + "-Page" + String(p + 1));
			_u8g2->drawVLine(125, 9, 56);
			
			if (p == 0) {
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->drawFrame(2, 11, 121, 13);
				_u8g2->setCursor(12, 13); _u8g2->print(x);
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
				_u8g2->drawGlyph(3, 13 + 8, 129);
				
				_u8g2->drawFrame(2, 25, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, 27); _u8g2->print(y);
				
				_u8g2->drawFrame(2, 39, 121, 13);
				_u8g2->setCursor(12, 41); _u8g2->print(z);
				
				_u8g2->drawFrame(2, 53, 121, 13);
				_u8g2->setCursor(12, 55); _u8g2->print(a);
			}
			else if (p == 1) {
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->drawFrame(2, 11, 121, 13);
				_u8g2->setCursor(12, 13); _u8g2->print(x);
				_u8g2->drawFrame(2, 25, 121, 13);
				_u8g2->setCursor(12, 27); _u8g2->print(y);
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
				_u8g2->drawGlyph(3, 27 + 8, 129);
				_u8g2->drawFrame(2, 39, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, 41); _u8g2->print(z);
				_u8g2->drawFrame(2, 53, 121, 13);
				_u8g2->setCursor(12, 55); _u8g2->print(a);
			}
			else if (p == 2) {
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->drawFrame(2, 11, 121, 13);
				_u8g2->setCursor(12, 13); _u8g2->print(x);
				_u8g2->drawFrame(2, 25, 121, 13);
				_u8g2->setCursor(12, 27); _u8g2->print(y);
				_u8g2->drawFrame(2, 39, 121, 13);
				_u8g2->setCursor(12, 41); _u8g2->print(z);
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
				_u8g2->drawGlyph(3, 41 + 8, 129);
				_u8g2->drawFrame(2, 53, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, 55); _u8g2->print(a);
			}
			else if (p == 3) {
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->drawFrame(2, 11, 121, 13);
				_u8g2->setCursor(12, 13); _u8g2->print(x);
				_u8g2->drawFrame(2, 25, 121, 13);
				_u8g2->setCursor(12, 27); _u8g2->print(y);
				_u8g2->drawFrame(2, 39, 121, 13);
				_u8g2->setCursor(12, 41); _u8g2->print(z);
				_u8g2->drawFrame(2, 53, 121, 13);
				_u8g2->setCursor(12, 55); _u8g2->print(a);
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
				_u8g2->drawGlyph(3, 55 + 8, 129);
			}
		} while (_u8g2->nextPage());
		
		if (upd) { delay(200); upd = false; }
	}
	return -1;
}

//-------------------------------------------
// 三选项列表选择
//-------------------------------------------
int WonderUI::listchoose3(String x, String y, String z, String title) {
	int p = 0;
	const int mp = 2;
	bool upd = false;
	
	while (true) {
		if (GetButton('D')) { p++; if (p > mp) p = 0; upd = true; }
		else if (GetButton('U')) { p--; if (p < 0) p = mp; upd = true; }
		else if (GetButton('O')) { while (GetButton('O')) delay(10); return p; }  // 等 OK 松手再返回
		
		_u8g2->firstPage();
		do {
			_u8g2->drawHLine(0, 9, 128);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0);
			_u8g2->print(title + "-Page" + String(p + 1));
			_u8g2->drawVLine(125, 9, 56);
			
			if (p == 0) {
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->drawFrame(2, 11, 121, 13);
				_u8g2->setCursor(12, 13); _u8g2->print(x);
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
				_u8g2->drawGlyph(3, 13 + 8, 129);
				_u8g2->drawFrame(2, 25, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, 27); _u8g2->print(y);
				_u8g2->drawFrame(2, 39, 121, 13);
				_u8g2->setCursor(12, 41); _u8g2->print(z);
			}
			else if (p == 1) {
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->drawFrame(2, 11, 121, 13);
				_u8g2->setCursor(12, 13); _u8g2->print(x);
				_u8g2->drawFrame(2, 25, 121, 13);
				_u8g2->setCursor(12, 27); _u8g2->print(y);
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
				_u8g2->drawGlyph(3, 27 + 8, 129);
				_u8g2->drawFrame(2, 39, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, 41); _u8g2->print(z);
			}
			else if (p == 2) {
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->drawFrame(2, 11, 121, 13);
				_u8g2->setCursor(12, 13); _u8g2->print(x);
				_u8g2->drawFrame(2, 25, 121, 13);
				_u8g2->setCursor(12, 27); _u8g2->print(y);
				_u8g2->drawFrame(2, 39, 121, 13);
				_u8g2->setCursor(12, 41); _u8g2->print(z);
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
				_u8g2->drawGlyph(3, 41 + 8, 129);
			}
		} while (_u8g2->nextPage());
		
		if (upd) { delay(200); upd = false; }
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

	while (true) {
		unsigned long now = millis();

		// ---- 按键 ----
		if (now - lastKey > KEY_COOL) {
			if (GetButton('D')) { p++; if (p >= count) p = 0; lastKey = now; }
			else if (GetButton('U')) { p--; if (p < 0) p = count - 1; lastKey = now; }
		}
		if (GetButton('O')) { while (GetButton('O')) delay(10); return p; }
		if (GetButton('S')) { while (GetButton('S')) delay(10); return -1; }

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

		_u8g2->firstPage();
		do {
			_u8g2->drawHLine(0, 9, 128);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0);
			_u8g2->print(title);
			_u8g2->drawVLine(125, 9, 56);

			// 列表：按 scroll 连续偏移绘制（上下各多画一项，过渡时无缝）
			int base = (int)floorf(scroll);
			for (int i = -1; i <= VISIBLE; i++) {
				int idx = base + i;
				if (idx < 0 || idx >= count) continue;
				int y = 11 + (int)((idx - scroll) * 14.0f);
				if (y < 9 || y > 55) continue;            // 裁剪出可视区
				_u8g2->drawFrame(2, y, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, y + 2);
				_u8g2->print(items[idx]);
			}
			// 光标：按 focus 平滑位置单独绘制（滑过去而不是瞬移）
			{
				int hy = 11 + (int)((focus - scroll) * 14.0f);
				if (hy >= 9 && hy <= 55) {
					_u8g2->setFontPosBottom();
					_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
					_u8g2->drawGlyph(3, hy + 2 + 1 * 8, 129);
				}
			}
		} while (_u8g2->nextPage());

		delay(10);   // 动画帧间隔
	}
	return -1;
}

//-------------------------------------------
// 数组版列表选择（上限 20 项，一屏 4 项光标式）
//-------------------------------------------
int WonderUI::listchooseArr(const char* const* items, int count, const char* title) {
	const int MAX = 20;
	if (count < 1 || items == NULL) return -1;
	if (count > MAX) count = MAX;

	int   p      = 0;         // 当前选中项
	int   offset = 0;         // 逻辑可视区顶部
	float scroll = 0.0f;      // 平滑滚动位置（单位：项）
	float focus  = 0.0f;      // 平滑光标位置（单位：项）
	const int VISIBLE = 4;
	unsigned long lastKey = 0;
	const unsigned long KEY_COOL = 150;

	while (true) {
		unsigned long now = millis();

		// ---- 按键（冷却，长按连续滚）----
		if (now - lastKey > KEY_COOL) {
			if (GetButton('D')) { p++; if (p >= count) p = 0; lastKey = now; }
			else if (GetButton('U')) { p--; if (p < 0) p = count - 1; lastKey = now; }
		}
		if (GetButton('O')) { while (GetButton('O')) delay(10); return p; }
		if (GetButton('S')) { while (GetButton('S')) delay(10); return -1; }

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

		_u8g2->firstPage();
		do {
			_u8g2->drawHLine(0, 9, 128);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0);
			_u8g2->print(title);
			_u8g2->drawVLine(125, 9, 56);

			// 列表：按 scroll 连续偏移绘制
			int base = (int)floorf(scroll);
			for (int i = -1; i <= VISIBLE; i++) {
				int idx = base + i;
				if (idx < 0 || idx >= count) continue;
				int y = 11 + (int)((idx - scroll) * 14.0f);
				if (y < 9 || y > 55) continue;
				_u8g2->drawFrame(2, y, 121, 13);
				_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
				_u8g2->setFontPosTop();
				_u8g2->setCursor(12, y + 2);
				_u8g2->print(items[idx] ? items[idx] : "");
			}
			// 光标：按 focus 平滑位置绘制
			{
				int hy = 11 + (int)((focus - scroll) * 14.0f);
				if (hy >= 9 && hy <= 55) {
					_u8g2->setFontPosBottom();
					_u8g2->setFont(u8g2_font_open_iconic_all_1x_t);
					_u8g2->drawGlyph(3, hy + 2 + 1 * 8, 129);
				}
			}
		} while (_u8g2->nextPage());

		delay(10);
	}
	return -1;
}

//-------------------------------------------
// 数值上下调节
//-------------------------------------------
int WonderUI::numericUpDown(int maxn, int minn, String title) {
	int otp = 0;
	while (true) {
		bool flag = false;
		_u8g2->firstPage();
		do {
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0);
			_u8g2->print(title);
			_u8g2->drawHLine(0, 9, 128);
			_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 11);
			_u8g2->print(String(otp));
			_u8g2->setCursor(0, 52);
			_u8g2->print("R:加  L:减");
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			
			if (GetButton('R')) {
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_6x_t);
				_u8g2->drawGlyph(80, 11 + 48, 75);
				otp++; flag = true;
			} else if (GetButton('L')) {
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_6x_t);
				_u8g2->drawGlyph(80, 11 + 48, 74);
				otp--; flag = true;
			} else {
				_u8g2->setFontPosBottom();
				_u8g2->setFont(u8g2_font_open_iconic_all_6x_t);
				_u8g2->drawGlyph(80, 11 + 48, 71);
			}
		} while (_u8g2->nextPage());
		
		if (otp < minn) otp = minn;
		else if (otp > maxn) otp = maxn;
		
		if (flag) delay(150);
		if (GetButton('O')) { while (GetButton('O')) delay(10); break; }  // 等 OK 松手再返回
	}
	return otp;
}

//-------------------------------------------
// 确认/信息框
//-------------------------------------------
bool WonderUI::check(String title, String line1, String line2, String line3, String line4) {
	while (true) {
		if (GetButton('O')) { while (GetButton('O')) delay(10); break; }  // 等 OK 松手，避免残留触发
		else if (GetButton('S')) { while (GetButton('S')) delay(10); return false; }  // 等 SET 松手
		
		_u8g2->firstPage();
		do {
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0);
			_u8g2->print(title);
			_u8g2->setFont(u8g2_font_timB12_tf);
			_u8g2->setFontPosTop();
			_u8g2->drawHLine(0, 9, 128);
			_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(5, 13);
			_u8g2->print(line1);
			_u8g2->setCursor(5, 25);
			_u8g2->print(line2);
			_u8g2->setCursor(5, 37);
			_u8g2->print(line3);
			_u8g2->setCursor(0, 52);
			_u8g2->print(line4);
		} while (_u8g2->nextPage());
	}
	return true;
}

//-------------------------------------------
// 文本框：自动换行(10字/行)，4行/页，上下翻页
//-------------------------------------------
void WonderUI::textBox(String title, String text) {
	const int LINE_WIDTH  = 20;  // 每行 20 单位宽（英文=1, 中文=2）
	const int CH_PER_PAGE  = 4;

	// 1. 按显示宽度拆行（中文 2 单位, 英文 1 单位, 满 20 换行）
	String lines[320];
	int totalLines = 0;
	const char* raw = text.c_str();
	int len = text.length();
	int bytePos = 0;
	while (bytePos < len && totalLines < 320) {
		int lineStart = bytePos;
		int lineWidth = 0;
		while (bytePos < len) {
			unsigned char c = raw[bytePos];
			int charW = (c > 0x7F) ? 2 : 1;   // 中文宽 2, ASCII 宽 1
			int charBytes = (c > 0x7F) ? 3 : 1;  // UTF-8: 中文 3 字节, ASCII 1 字节
			if (lineWidth + charW > LINE_WIDTH) break;
			lineWidth += charW;
			bytePos += charBytes;
		}
		lines[totalLines++] = text.substring(lineStart, bytePos);
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
			return;
		}
		if (GetButton('S')) {
			while (GetButton('S')) delay(10);
			return;
		}

		// ---- 滑动回位（ease-out）----
		slide += (0.0f - slide) * 0.35f;
		if (fabsf(slide) < 0.3f) slide = 0.0f;

		_u8g2->firstPage();
		do {
			// 标题栏
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(0, 0);
			String titleBar = title + " [" + String(curPage + 1) + "/" + String(totalPages) + "]";
			_u8g2->print(titleBar);
			_u8g2->drawHLine(0, 9, 128);

			// 内容：按 slide 偏移绘制，上下各多画一行保证过渡无缝
			_u8g2->setFont(u8g2_font_wqy12_t_gb2312);
			_u8g2->setFontPosTop();
			int startLine = curPage * CH_PER_PAGE;
			for (int i = -1; i <= CH_PER_PAGE; i++) {
				int idx = startLine + i;
				if (idx < 0 || idx >= totalLines) continue;
				int y = 11 + (int)(i * 13 + slide);
				if (y < 9 || y > 58) continue;         // 裁剪
				_u8g2->setCursor(0, y);
				_u8g2->print(lines[idx]);
			}
		} while (_u8g2->nextPage());

		delay(10);
	}
}
