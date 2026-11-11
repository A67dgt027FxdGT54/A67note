
/*

    The MIT License (MIT)

    Copyright (c) 2026 A67dgt027FxdGT54 (https://www.luogu.com.cn/user/1417426)

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/ 

#ifndef __A67_TUI_H
#define __A67_TUI_H 1

#include<bits/stdc++.h>
#include<windows.h>
#include<time.h> 
#include<conio.h>

#define T_QUICKEDIT 0x0000

#define T_SCISSOR_TEST 0x0100
#define T_AUTO_LF 0x0101
#define T_BLEND 0x0102


using tEnum = unsigned int;
using tErr = unsigned int;
using tCol = unsigned short;
using tPx = std::pair<char, tCol>;

int tWindowHeight = 30, tWindowWidth = 120;
int tViewportHeight = 30, tViewportWidth = 120, tViewportRow = 0, tViewportCol = 0;
int tScissorHeight = 30, tScissorWidth = 120, tScissorRow = 0, tScissorCol = 0;
int tUuTmp = 0;

tCol tOutputCol = 0x07;

std::bitset<256> tEnabledBits;
std::vector<std::vector<tPx> > tScreenBuf = std::vector<std::vector<tPx> >(30, std::vector<tPx>(120, std::make_pair(32, 0x07)));

std::string tWchar_to_gbk(wchar_t wc){
	char buf[8]={0};
	int len=WideCharToMultiByte(936,0,&wc,1,buf,7,NULL,NULL);
	if(len<=0) return "";
	return std::string(buf,len);
}
std::string tWstring_to_gbk(const std::wstring& wstr){
	if(wstr.empty()) return "";
	int len=WideCharToMultiByte(936,0,wstr.c_str(),(int)wstr.length(),NULL,0,NULL,NULL);
	if(len<=0) return "";
	std::string result(len,0);
	WideCharToMultiByte(936,0,wstr.c_str(),(int)wstr.length(),&result[0],len,NULL,NULL);
	return result;
}
std::wstring tGbk_to_wstring(const std::string& gbkStr){
	if(!gbkStr.size()) return std::wstring();
	int wLen = MultiByteToWideChar(936, 0, gbkStr.c_str(), int(gbkStr.size()), nullptr, 0);
	if(wLen <= 0) return std::wstring();
	std::wstring wstr(wLen, L'\0');
	int ret = MultiByteToWideChar(936, 0, gbkStr.c_str(), int(gbkStr.size()), &wstr[0], wLen);
	if(ret <= 0) return std::wstring();
	return wstr;
}

void tLog(std::string level,std::string msg){
	std::ofstream _log("TUI.log",std::ios::app);
    std::time_t now = std::time(nullptr);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    _log << "[" << time_buf << "] [" <<level<<"] "<<msg<<std::endl;
}

void t__SetCursorPos(int r, int c){
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD _pos;
	_pos.X=c, _pos.Y=r;
	SetConsoleCursorPosition(hOutput,_pos);
}
void t__SetOutputCol(tCol col){
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOutput, col);
}


namespace t__Kbcompat{
	std::deque<wchar_t> pending;
	
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	
	bool nextKeyEvent(wchar_t& wch, WORD& vscan){
		INPUT_RECORD ir;
		DWORD total = 0, read = 0;
		
		while(GetNumberOfConsoleInputEvents(h, &total) && total > 0){
			if(!ReadConsoleInputW(h, &ir, 1, &read)) return false;
			if(ir.EventType != KEY_EVENT) continue;
			if(!ir.Event.KeyEvent.bKeyDown) continue;
			wch = ir.Event.KeyEvent.uChar.UnicodeChar;
			vscan = ir.Event.KeyEvent.wVirtualScanCode;
			return true;
		}
		return false;
	}
}

int tKbhit(){
	if(!t__Kbcompat::pending.empty()) return 1;
	DWORD total = 0;
	if(!GetNumberOfConsoleInputEvents(t__Kbcompat::h, &total) || total == 0) return 0;
	if(total > 512) total = 512;
	INPUT_RECORD buf[512];
	DWORD n = 0;
	if(!PeekConsoleInputW(t__Kbcompat::h, buf, total, &n)) return 0;
	for(DWORD i=0;i<n;i++)
		if(buf[i].EventType == KEY_EVENT && buf[i].Event.KeyEvent.bKeyDown) return 1;
	return 0;
}

wchar_t tGetwch(){
	if(!t__Kbcompat::pending.empty()){
		wchar_t c = t__Kbcompat::pending.front();
		t__Kbcompat::pending.pop_front();
		return c;
	}
	wchar_t wch = 0;
	WORD wscan = 0;
	if(!t__Kbcompat::nextKeyEvent(wch, wscan)) return 0;
	if(wch != 0) return wch;
#ifndef TUI_GETWCH_EXT
	if(
		wscan != 71 ||
		wscan != 72 ||
		wscan != 75 ||
		wscan != 77 ||
		wscan != 79 ||
		wscan != 80 ||
		wscan != 83 
	) wscan = 0;
#endif
	t__Kbcompat::pending.push_back(static_cast<wchar_t>(wscan));
	return 224;
}

std::queue<char> t__GetchBuf;
int tGetch(){
	if(!t__GetchBuf.empty()){
		int ret = static_cast<int>(t__GetchBuf.front());
		t__GetchBuf.pop();
		return ret;
	}
	else{
		std::string s = tWchar_to_gbk(tGetwch());
		if(s.size() == 0) return 0;
		for(auto c: s){
			t__GetchBuf.push(c);
		}
		int ret = static_cast<int>(t__GetchBuf.front());
		t__GetchBuf.pop();
		return ret;
	}
}

tErr tEnable(tEnum func){
	if(func < 0x0100){
		if(func == T_QUICKEDIT){
			HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
			DWORD mode;
			GetConsoleMode(hInput,&mode);
			mode |= ENABLE_QUICK_EDIT_MODE;
			mode |= ENABLE_INSERT_MODE;
			mode |= ENABLE_MOUSE_INPUT;
			bool fail = !SetConsoleMode(hInput,mode);
			if(fail) return GetLastError();
			return 0;
		}
	}
	else if(func < 0x0200){
		tEnabledBits[func - 0x0100] = 1;
		return 0;
	}
	return ERROR_INVALID_PARAMETER;
}
tErr tDisable(tEnum func){
	if(func < 0x0100){
		if(func == T_QUICKEDIT){
			HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
			DWORD mode;
			GetConsoleMode(hInput,&mode);
			mode &= ~ENABLE_QUICK_EDIT_MODE;
			mode &= ~ENABLE_INSERT_MODE;
			mode &= ~ENABLE_MOUSE_INPUT;
			bool fail = SetConsoleMode(hInput,mode);
			if(fail) return GetLastError();
			return 0;
		}
	}
	else if(func < 0x0200){
		tEnabledBits[func - 0x0100] = 0;
		return 0;
	}
	return ERROR_INVALID_PARAMETER;
}

void tResizeWindow(int h, int w){
	tWindowHeight = h;
	tWindowWidth = w;
	std::string cmd = "mode con cols=" + std::to_string(w) + " lines=" + std::to_string(h);
	system(cmd.c_str());
	tScreenBuf.assign(h, std::vector<tPx>(w, std::make_pair(32,0x07)));
}
void tViewport(int r, int c, int h, int w){
	tViewportHeight = h, tViewportWidth = w, tViewportRow = r, tViewportCol = c;
}
void tScissor(int r, int c, int h, int w){
	tScissorHeight = h, tScissorWidth = w, tScissorRow = r, tScissorCol = c;
}

void tClear(tPx px){
	int sr = 0, er = tWindowHeight, sc = 0, ec = tWindowWidth;
	if(tEnabledBits[T_SCISSOR_TEST - 0x0100]){
		sr = tScissorRow, er = sr + tScissorHeight, sc = tScissorCol, ec = sc + tScissorWidth;
	}
	for(int row = sr; row < er; row++){
		for(int col = sc; col < ec; col++){
			tScreenBuf[row][col] = px;
		}
	}
}
void tSwapBuffer(){
	for(int row = 0; row < tWindowHeight; row++){
		t__SetCursorPos(row, 0);
		for(int col = 0; col < tWindowWidth; col++){
			if(!col || tScreenBuf[row][col-1].second != tScreenBuf[row][col].second){
				t__SetOutputCol(tScreenBuf[row][col].second);
			}
			printf("%c", tScreenBuf[row][col].first);
		}
	}
}
void tHideCursor(){
	CONSOLE_CURSOR_INFO _cur={1,0};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&_cur);
}
void tShowCursor(){
	CONSOLE_CURSOR_INFO _cur={1,1};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&_cur);
}
void tSetOutputCol(tCol col){
	tOutputCol = col;
}
bool t__PxShouldDiscard(int r, int c){
    if(r < 0 || r >= tWindowHeight) return true;
    if(c < 0 || c >= tWindowWidth) return true;
	if(r < tViewportRow || tViewportRow + tViewportHeight <= r) return true;
	if(c < tViewportCol || tViewportCol + tViewportWidth <= c) return true;
	if(tEnabledBits[T_SCISSOR_TEST - 0x0100]){
		if(r < tScissorRow || tScissorRow + tScissorHeight <= r) return true;
		if(c < tScissorCol || tScissorCol + tScissorWidth <= c) return true;
	}
	return false;
}

std::wstring tInputBuf;
bool tInput(){
	if(tKbhit()){
		wchar_t wch = tGetwch();
		char ch = 65;
		std::string wchs = tWchar_to_gbk(wch);
		if(wchs.length() == 1) ch = wchs[0];
		
		if(ch == '\r'){
			return true;
		}
		else if(ch == '\b'){
			if(tInputBuf.length())tInputBuf.pop_back();
		}
		else if(wch >= 32 && wch!=127){
			tInputBuf += wch;
		}
	}
	return false;
}
std::string tGetInputBuf(){
	return tWstring_to_gbk(tInputBuf);
}
void tClearInputBuf(){
	tInputBuf = std::wstring();
}
void tOutput(const std::string& s, int r, int c){
	r = tViewportRow + r;
	int origC = c = tViewportCol + c;
	for(int i = 0; i < s.length(); i++){
		if(!t__PxShouldDiscard(r, c)){
			if(tEnabledBits[T_BLEND - 0x0100])
				tScreenBuf[r][c] = {s[i], tScreenBuf[r][c].second & 0xF0 | tOutputCol & 0x0F};
			else
				tScreenBuf[r][c] = {s[i], tOutputCol};
		} 
		c++; 
		if(tEnabledBits[T_AUTO_LF - 0x0100]) 
			if(c >= tViewportCol + tViewportWidth) r++, c = origC;
	}
}
void tPause(){
	while(1){
		tUuTmp = tUuTmp * 114514 + 7891;
	}
}
char tChoose(const std::string& list){
	char ch = tGetch();
	while(([&]()->bool{
		for(const char& c: list){
			if(c == ch) return false;
		}
		return true;
	})()) ch = _getch();
	return ch;
}

bool tSetClipboardText(const std::wstring& text){
	if(!OpenClipboard(NULL)) return false;
	if(!EmptyClipboard()){
		CloseClipboard();
		return false;
	}
	SIZE_T bytes = (text.size() + 1) * sizeof(wchar_t);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
	if(!hMem){
		CloseClipboard();
		return false;
	}
	void* p = GlobalLock(hMem);
	if(!p){
		GlobalFree(hMem);
		CloseClipboard();
		return false;
	}
	memcpy(p, text.c_str(), bytes);
	GlobalUnlock(hMem);
	if(!SetClipboardData(CF_UNICODETEXT, hMem)){
		GlobalFree(hMem);
		CloseClipboard();
		return false;
	}
	CloseClipboard();
	return true;
}
bool tSetClipboardTextGbk(const std::string& gbkText){
	return tSetClipboardText(tGbk_to_wstring(gbkText));
}
std::wstring tGetClipboardText(){
	std::wstring res;
	if(!OpenClipboard(NULL)) return res;
	if(!IsClipboardFormatAvailable(CF_UNICODETEXT)){
		CloseClipboard();
		return res;
	}
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	if(!hData){
		CloseClipboard();
		return res;
	}
	SIZE_T size = GlobalSize(hData);
	wchar_t* p = static_cast<wchar_t*>(GlobalLock(hData));
	if(p){
		size_t capa = size / sizeof(wchar_t);
		size_t len = 0;
		while(len < capa && p[len] != L'\0') ++len;
		res.assign(p, len);
		GlobalUnlock(hData);
	}
	CloseClipboard();
	return res;
}
std::string tGetClipboardTextGbk(){
	return tWstring_to_gbk(tGetClipboardText());
}


std::pair<int, int> tGetMousePosChar(){
	COORD fontSz = GetConsoleFontSize(GetStdHandle(STD_OUTPUT_HANDLE), 0);
	POINT _pt;
    GetCursorPos(&_pt);
    HWND _h = GetForegroundWindow();
    ScreenToClient(_h, &_pt);
    _pt.x /= fontSz.X;
	_pt.y /= fontSz.X * 2;
    return std::make_pair(_pt.y, _pt.x);
}
bool tIsMouseInArea(int r, int c, int h, int w){
	std::pair<int, int> pr = tGetMousePosChar();
	return (r <= pr.first && pr.first < r + h && c <= pr.second && pr.second < c + w);
}

struct tTempStatus_t{
private:
	tEnum func;
	int value;
	bool invalid;
public:
	tTempStatus_t() = delete;
	tTempStatus_t(tEnum aFunc, int aValue): func(aFunc), invalid(false){
		value = tEnabledBits[aFunc - 0x0100];
		if(aValue == 0) tDisable(aFunc);
		else if(aValue == 1) tEnable(aFunc);
	}
	~tTempStatus_t(){
		if(value == 0) tDisable(func);
		else if(value == 1) tEnable(func);
	}
	tTempStatus_t(const tTempStatus_t& o)=delete;
	tTempStatus_t& operator =(const tTempStatus_t& o)=delete;
	tTempStatus_t(tTempStatus_t&& o) = delete;
	tTempStatus_t& operator =(tTempStatus_t&& o) = delete;
};

struct tButton_t{
private:
	int r = 0, c = 0;
	std::string text;
	tCol col = 0x01;
	std::function<void(void)> click_callback, drag_callback, hover_callback, unclick_callback, undrag_callback, unhover_callback;
	bool lastMouse = 0;
public:
	tButton_t() = default;
	tButton_t(int _r, int _c, const std::string& _t, tCol _col): r(_r), c(_c), text(_t), col(_col){}
	void setClickCallback(const std::function<void(void)>& cb){
		click_callback = cb;
	}
	void setDragCallback(const std::function<void(void)>& cb){
		drag_callback = cb;
	}
	void setHoverCallback(const std::function<void(void)>& cb){
		hover_callback = cb;
	}
	void setUnclickCallback(const std::function<void(void)>& cb){
		unclick_callback = cb;
	}
	void setUndragCallback(const std::function<void(void)>& cb){
		undrag_callback = cb;
	}
	void setUnhoverCallback(const std::function<void(void)>& cb){
		unhover_callback = cb;
	}
	void process(){
		tCol oCol = tOutputCol;
		bool blend = tEnabledBits[T_BLEND - 0x100];
		tSetOutputCol(col);
		tDisable(T_BLEND);
		tOutput(text, r, c);
		tSetOutputCol(oCol);
		if(blend) tEnable(T_BLEND);
		if(tIsMouseInArea(r, c, 1, text.length())){
			if(hover_callback) hover_callback();
//			else tLog("WARN", "no hover callback");
			
			if((GetAsyncKeyState(1) & 0x8000)){
				if(drag_callback) drag_callback();
				
				if(!lastMouse){
					if(click_callback) click_callback();
				}
				else{
					if(unclick_callback) unclick_callback();
				}
				
				lastMouse = 1;
			}
			else{
				if(undrag_callback) undrag_callback();
				lastMouse = 0;
			}
		}
		else{
			if(unhover_callback) unhover_callback();
			lastMouse = 0;
		}
	}
	void setR(int _r){
		lastMouse = 0;
		r = _r;
	}
	void setC(int _c){
		lastMouse = 0;
		c = _c;
	}
	void setText(const std::string& _t){
		lastMouse = 0;
		text = _t;
	}
	void setCol(tCol _col){
		col = _col;
	}
	int getR(){
		return r;
	}
	int getC(){
		return c;
	}
	std::string getText(){
		return text;
	}
	tCol getCol(){
		return col;
	}
}; 


#endif // __A67_TUI_H
