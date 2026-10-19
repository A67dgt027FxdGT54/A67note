#pragma once

string getTimesZh(int times){
	if(times == 1) return "第一次";
	else if(times == 2) return "第二次";
	else if(times == 3) return "第三次"; 
	return "";
}
string getTimesEn(int times){
	if(times == 1) return "1st";
	else if(times == 2) return "2nd";
	else if(times == 3) return "3rd";
	return "";
}


void chooseIp(){
	bool invalidChoose = 1;
	auto ips = get_ip();
	if(ips.size()==1){
		myIp = ips[0];
	}
	else{
		while(invalidChoose){
			tClearInputBuf();
			while(!tInput()){
				tClear({' ', 0x0F});
				tOutput("> You have many IP. Choose one in the same LAN: ", 5, 43);
				tOutput("有多个 IP，请选择正在同一局域网中使用的 IP：", 6, 45);
				for(int i = 0; i < ips.size(); i++){
					stringstream ss;
					ss << i << ":" << ips[i];
					tOutput(ss.str(), 10+i, 43);
				}
				tOutput(tGetInputBuf(), 8, 49);
				tSwapBuffer();
			}
			if(checkChooseIp(tGetInputBuf()) || stoi(tGetInputBuf()) >= ips.size()){
				tEnable(T_SCISSOR_TEST);
				tScissor(10, 0, 20, 120);
				tClear({' ', 0x0F});
				tOutput(" ", 5, 43);
				tOutput("> Invalid input. Press [1] to re-input. ", 10, 43);
				tOutput("无效输入。按 [1] 重新输入。", 11, 45);
				tDisable(T_SCISSOR_TEST);
				tSwapBuffer();
				tChoose("1");
			}
			else{
				invalidChoose = 0;
			}
		}
		myIp=ips[stoi(tGetInputBuf())];
	}
}
void inputUserName(){
	bool invalidUserName = 1;
	while(invalidUserName){
		tClearInputBuf();
		while(!tInput()){
			tClear({' ', 0x0F});
			tOutput("> Input user name:", 9, 43);
			tOutput("输入用户名：", 10, 45);
			tOutput(tGetInputBuf(), 15, 49);
			tSwapBuffer();
		}
		if(checkUserName(tGetInputBuf())){
			tOutput(" ", 9, 43);
			tOutput("> Invalid input. Press [1] to re-input. ", 19, 43);
			tOutput("无效输入。按 [1] 重新输入。", 20, 45);
			tSwapBuffer();
			tChoose("1");
		}
		else{
			invalidUserName = 0;
		}
	}
	myName = tGetInputBuf();
}
void createClientAndShowServerInfo(){
	Sleep(100);
	cl.create("127.0.0.1");
	isServer = true;
	
	while(1){
		while(cl.empty()) Sleep(10);
		msg_t msg = cl.get_msg();
		if(msg.type == A67LAN_REWORK_SOCKET_INFO){
			mySocket = msg.from;
			break;
		}
	}
	userData[mySocket].name = myName;
	userData[mySocket].status = eUserStatus.admin;
	userData[mySocket].ip = myIp;
	
	stringstream ss;
	ss << "服务器 IP：" << myIp << " 服务器套接字：" << mySocket;
	tSetClipboardTextGbk(ss.str());
	
	tClear({' ', 0x0F});
	tOutput("Server IP: ", 5, 45);
	tOutput("服务器 IP：", 6, 45);
	tOutput(myIp, 8, 49);
	
	tOutput("Server Socket: ", 10, 45);
	tOutput("服务器套接字：", 11, 45);
	tOutput(to_string(mySocket), 13, 49);
	
	tOutput("Succeed. Stored the information above to the clip board.", 15, 45);
	tOutput("创建成功。已将上述信息存储在剪贴板中。", 16, 45);
	
	tOutput("> Press [1] to continue. ", 18, 43);
	tOutput("按 [1] 继续。", 19, 45);
	
	tSwapBuffer();
	
	tChoose("1");
	
	curPage.type = 0;
	curPage.loadFileFromCurPath();
}
void inputServerIp(){
	tClearInputBuf();
	while(!tInput()){
		tClear({' ', 0x0F});
		tOutput("> Input server IP: ", 5, 43); 
		tOutput("输入服务器 IP：", 6, 45); 
		tOutput(tGetInputBuf(), 8, 49);
		tSwapBuffer();
	} 
	svIp = tGetInputBuf();
}
void inputServerSocket(){
	bool invalidSocket = 1;
	while(invalidSocket){
		tClearInputBuf();
		while(!tInput()){
			tClear({' ', 0x0F});
			tOutput("Input server IP: ", 5, 45);
			tOutput("输入服务器 IP：", 6, 45); 
			tOutput(svIp, 8, 49);
			tOutput("> Input server socket: ", 10, 43);
			tOutput("输入服务器套接字：", 11, 45);
			tOutput(tGetInputBuf(), 13, 49);
			tSwapBuffer();
		}
		stringstream ss(tGetInputBuf());
		ss >> svSocket;
		if(svSocket != (unsigned long long)(-1)) invalidSocket = 0; 
	}
}
void inputToken(){
	tClearInputBuf();
	while(!tInput()){
		tClear({' ', 0x0F});
		tOutput("Input server IP: ", 5, 45);
		tOutput("输入服务器 IP：", 6, 45); 
		tOutput(svIp, 8, 49);
		tOutput("Input server socket: ", 10, 45);
		tOutput("输入服务器套接字：", 11, 45);
		tOutput(to_string(svSocket), 13, 49);
		tOutput("> Please tell the owner of the server that ", 15, 43);
		tOutput("your user name, ask him/her for a token, ", 16, 45);
		tOutput("and input it: ", 17, 45);
		tOutput("请将你的用户名告知服务器的运营者，申请一个", 18, 45);
		tOutput("token 并输入：", 19, 45);
		tOutput(tGetInputBuf(), 21, 49);
		tSwapBuffer();
	}
	token = tGetInputBuf();
}

void showSetupUI(){
	tOutput("A67note", 0, 56);
	tOutput("Developed by A67dgt027FxdGT54, AC Games", 29, 40);
	tOutput("> Press [1] to create an account.", 9, 43);
	tOutput("按 [1] 注册账号", 10, 45);
	tSwapBuffer();
	
	char ch = tChoose("1");
	
	chooseIp();
	inputUserName();
	
	tClear({' ', 0x0F});
	tOutput("> Press [1] to create a server.", 9, 43);
	tOutput("按 [1] 创建服务器", 10, 45);
	
	tOutput("> Press [2] to link to another's server.", 19, 43);
	tOutput("按 [2] 连接到他人的服务器", 20, 45);
	
	tSwapBuffer();
	
	ch = tChoose("12"); 
	
	if(ch == '1'){
		tOutput(" ", 10, 43);
		int ret = sv.create();
		if(ret == 0){
			createClientAndShowServerInfo();
		}
		else if(ret == 1){
			tOutput("Fail: You can create ONE server only. Link to another's server later. ", 15, 45);
			tOutput("创建失败：只能创建一个服务器。即将连接至他人的服务器。", 16, 45);
			tSwapBuffer(); 
			Sleep(1000);
			ch = '2';
		}
		else{
			tOutput("Fail. Link to another's server later. ", 15, 45);
			tOutput("创建失败。即将连接至他人的服务器。", 16, 45);
			tSwapBuffer(); 
			Sleep(1000);
			ch = '2';
		}
	}
	if(ch == '2'){
		inputServerIp();
		inputServerSocket();
		bool invalidToken = 1, clientCreated = 0;
		while(invalidToken){
			inputToken();
			
			tOutput(" ", 15, 43);
			tOutput("> Linking to the server...", 23, 43);
			tOutput("正在连接到服务器...", 24, 45);
			tSwapBuffer();
			
			if(!clientCreated) cl.create(svIp), clientCreated = 1;
			else sendMsgToServer("login " + myName + " " + token);
			
			
			// Request for page after login succ
			
			// wait
			
			while(1){
				while(cl.empty()) Sleep(10);
				msg_t msg = cl.get_msg();
				if(msg.type == A67LAN_REWORK_JOIN){
					stringstream ss;
					ss << "user socket " << msg.from << " IP " << sv.getIpOfClient(msg.from) << " joined.";
					wlog("INFO", ss.str());
				}
				else if(msg.type == A67LAN_REWORK_LEAVE){
					stringstream ss;
					ss << "user socket " << msg.from << " IP " << sv.getIpOfClient(msg.from) << " left.";
					wlog("INFO", ss.str());
				}
				else if(msg.type == A67LAN_REWORK_SOCKET_INFO){
					mySocket = msg.from;
					
					sendMsgToServer("login " + myName + " " + token);
				}
				else if(msg.type == A67LAN_REWORK_COMMON){
					if(msg.from != svSocket) continue; // ignore msg from others.
					stringstream ss(msg.con);
					int iStatus;
					ss >> iStatus;
					if(iStatus == 204){
						invalidToken = 0;
						break;
					}
					else if(iStatus == 401){
						string sErrType;
						int invalidTokenCount;
						ss >> sErrType >> invalidTokenCount;
						if(sErrType == "invalid_token"){
							tOutput(" ", 18, 43);
							tOutput("> 401 Invalid Token (" + getTimesEn(invalidTokenCount) + ", you'll be forbidden after 3 times)", 20, 43);
							tOutput("401 Token 无效（" + getTimesZh(invalidTokenCount) + "，错误三次将被封禁）", 21, 45);
							tOutput("Press [1] to re-input", 22, 45);
							tOutput("按 [1] 重新输入", 23, 45);
							tChoose("1");
						}
						break;
					}
					else if(iStatus == 403){
						tOutput(" ", 18, 43);
						tOutput("> 403 Forbidden", 20, 43);
						tOutput("403 您被封禁", 21, 45);
						tOutput("Ask the owner of the server to unblock, Then press [1] to input the token, or press [2] to exit. ", 22, 45);
						tOutput("请联系服主解封，之后按 [1] 输入 Token，或按 [2] 退出。", 23, 45);
						char ch2 = tChoose("12");
						if(ch2 == '2') exit(0);
						break;
					}
				}
			}
		}
		curPage.type = 2;
		curPage.requestForCurPage();
	}
	
	tDisable(T_SCISSOR_TEST);
}
