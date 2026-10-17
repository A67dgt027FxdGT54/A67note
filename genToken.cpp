#include "TUI.h"
#include "token.h"
using namespace std;
#include "setup.h"

string linkingPwd = "", userName = "";

int main(){
	setup();
	
	tOutput("> Input linking password (don't leak): ", 0, 43);
	tOutput("输入连接口令（请勿泄露）：", 1, 45);
	
	tEnable(T_SCISSOR_TEST);
	tScissor(3, 0, 1, 120);
	
	tClearInputBuf();
	while(!tInput()){
		tClear({' ', 0x0F});
		tOutput(tGetInputBuf(), 3, 49);
		tSwapBuffer();
	}
	linkingPwd = tGetInputBuf();
	tDisable(T_SCISSOR_TEST);
	
	tOutput(" ", 0, 43);
	tOutput("> Input User Name: ", 5, 43);
	tOutput("输入用户名：", 6, 45);
	
	tEnable(T_SCISSOR_TEST);
	tScissor(8, 0, 10, 120);
	
	while(1){
		tClearInputBuf();
		while(!tInput()){
			tClear({' ', 0x0F});
			tOutput(tGetInputBuf(), 8, 49);
			tSwapBuffer();
		}
		userName = tGetInputBuf();
		
		tDisable(T_SCISSOR_TEST);
		tOutput(" ", 5, 43);
		tEnable(T_SCISSOR_TEST);
		
		tOutput("Token: (仅当天有效/Available today only)", 10, 45);
		
		string token = to_hex(getToken(userName, linkingPwd));
		string paste = token + "（仅当天有效/Available today only）";
		tSetClipboardTextGbk(paste);
		
		tOutput(token, 12, 49); 
		tOutput("Stored the token to the clip board.", 14, 45);
		tOutput("已将 token 存储在剪贴板中。", 15, 45);
		tOutput("Press [1] to generate a new token.", 17, 45);
		tOutput("按 [1] 生成新 token。", 18, 45);
		tSwapBuffer();
		
		tChoose("1");
	}
} 
