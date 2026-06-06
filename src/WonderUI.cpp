#include "WonderUI.h"

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
bool WonderUI::GetButton(char x) {
	switch (x) {
		case 'U': return !digitalRead(_pinU);
		case 'D': return !digitalRead(_pinD);
		case 'L': return !digitalRead(_pinL);
		case 'R': return !digitalRead(_pinR);
		case 'O': return !digitalRead(_pinO);
		case 'S': return !digitalRead(_pinS);
	}
	return false;
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
// 字符输入键盘
//-------------------------------------------
String WonderUI::input(int maxl) {
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
		_u8g2->firstPage();
		do {
			_u8g2->drawFrame(1, 1, 127, 12);
			_u8g2->setFont(u8g2_font_timR08_tf);
			_u8g2->setFontPosTop();
			_u8g2->setCursor(2, 2);
			_u8g2->print(otp);
			
			_u8g2->setFont(u8g2_font_helvR12_tf);
			_u8g2->setFontPosTop();
			
			if (!caps) {
				for (int i = 0; i < 10; i++) {
					for (int j = 0; j < 4; j++) {
						_u8g2->setCursor(i * 12 + 3, 13 + j * 12);
						_u8g2->print(a[j][i]);
						if (i == x && j == y) {
							_u8g2->drawFrame(i * 12 + 3, 13 + j * 12, 11, 14);
						}
					}
				}
			} else {
				for (int i = 0; i < 10; i++) {
					for (int j = 0; j < 4; j++) {
						_u8g2->setCursor(i * 12 + 3, 13 + j * 12);
						_u8g2->print(b[j][i]);
						if (i == x && j == y) {
							_u8g2->drawFrame(i * 12 + 3, 13 + j * 12, 11, 14);
						}
					}
				}
			}
		} while (_u8g2->nextPage());
		
		bool flag = false;
		if (GetButton('S')) {
			break;
		} else if (GetButton('U')) {
			y--;
			if (y < 0) y = 3;
			flag = true;
		} else if (GetButton('D')) {
			y++;
			if (y >= 4) y = 0;
			flag = true;
		} else if (GetButton('L')) {
			x--;
			if (x < 0) x = 9;
			flag = true;
		} else if (GetButton('R')) {
			x++;
			if (x >= 10) x = 0;
			flag = true;
		} else if (GetButton('O')) {
			flag = true;
			if (a[y][x] == '^') {
				caps = !caps;
			} else if (a[y][x] == '<') {
				otp = otp.substring(0, otp.length() - 1);
			} else {
				if (otp.length() <= maxl) {
					otp += (caps ? b[y][x] : a[y][x]);
				}
			}
		}
		
		if (flag) delay(200);
	}
	return otp;
}

//-------------------------------------------
// 数字输入键盘
//-------------------------------------------
String WonderUI::inputnum(int maxl, char base) {
	String otp = "";
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
				_u8g2->setCursor(2, 2);
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
			if (GetButton('S')) break;
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
				_u8g2->setCursor(2, 2);
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
			if (GetButton('S')) break;
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
		else if (GetButton('O')) { return p; }
		
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
		else if (GetButton('O')) { return p; }
		
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
		if (GetButton('O')) break;
	}
	return otp;
}

//-------------------------------------------
// 确认/信息框
//-------------------------------------------
bool WonderUI::check(String title, String line1, String line2, String line3, String line4) {
	while (true) {
		if (GetButton('O')) break;
		else if (GetButton('S')) return false;
		
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
