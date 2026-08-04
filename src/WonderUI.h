/*
WonderUI - 通用OLED显示界面库
适用于U8g2，包含输入、列表选择、数值调节、确认框及开机动画。
示例化时传入U8g2对象和按钮引脚，使用默认引脚可省略。
*/

#ifndef WonderUI_h
#define WonderUI_h

#include <Arduino.h>
#include <U8g2lib.h>

class WonderUI {
public:
	// 构造函数，默认按钮引脚与原代码一致
	WonderUI(U8G2 &u8g2,
			 uint8_t pinU = 12,
			 uint8_t pinD = 13,
			 uint8_t pinL = 14,
			 uint8_t pinR = 27,
			 uint8_t pinO = 33,
			 uint8_t pinS = 26);
	
	// 初始化按钮引脚（内部上拉）
	void begin();
	// 重新设置全部按钮引脚（立即使能内部上拉）
	void configureButtons(uint8_t pinU, uint8_t pinD, uint8_t pinL,
						  uint8_t pinR, uint8_t pinO, uint8_t pinS);
	// 开机动画
	void start();
	
	// 字符输入（最大长度）
	String input(int maxl);
	
	// 数字输入（maxl: 最大长度, base: 'D' 大键盘, 'B' 二值横条）
	String inputnum(int maxl, char base);
	
	// 四选项列表选择，返回选中的索引(0~3)
	int listchoose4(String x, String y, String z, String a, String title);
	
	// 三选项列表选择，返回选中的索引(0~2)
	int listchoose3(String x, String y, String z, String title);
	
	// 不定长列表选择，返回选中的索引(0~count-1)，Set 退出返回 -1
	// 用法: listchoose("标题", 3, "选项1", "选项2", "选项3")  （上限 8 项）
	int listchoose(String title, int count, ...);

	// 数组版列表选择（上限 20 项，一屏 4 项光标式），Set 退出返回 -1
	// items 为 const char* 指针数组，count 为项数
	int listchooseArr(const char* const* items, int count, const char* title);
	
	// 数值上下调节（范围 minn ~ maxn）
	int numericUpDown(int maxn, int minn, String title);
	
	// 确认/信息框，OK返回true，Set返回false
	bool check(String title, String line1, String line2, String line3, String line4);
	
	// 读取按钮状态（按下为true，低电平有效）
	// 'U'/'D'/'L'/'R'/'O'/'S' 对应 上下左右OK/SET
	bool GetButton(char x);
	
private:
	U8G2 *_u8g2;
	uint8_t _pinU, _pinD, _pinL, _pinR, _pinO, _pinS;
	
	// 开机动画辅助函数
	void start1(int y);
	void start2(int x, int b);
	void start3();
	void start4();
};

#endif
