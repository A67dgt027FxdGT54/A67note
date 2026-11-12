
std::string to_hex(unsigned long long token){
	std::string res = "";
	for(int i = 60; i >= 0; i -= 4){
		unsigned long long cur = (token >> i) & 0xF;
		if(cur < 10) res += cur + '0';
		else res += cur - 10 + 'a';
	}
	return res;
}

namespace nsProcessManagerPage{
	int curList = 0; // 0 = Members, 1 = Tokens
	
	vector<tButton_t> btns = vector<tButton_t>{
		tButton_t(3, 0, "Members", 0x0F),
		tButton_t(3, 10, "Tokens", 0x0F)
	};
	
	void setupFunctions(){
		for(int i=0; i<btns.size(); i++){
			btns[i].setHoverCallback(
				[i](){
					btns[i].setCol(0x08);
				}
			);
			btns[i].setUnhoverCallback(
				[i](){
					btns[i].setCol(0x0F);
				}
			);
		}
		btns[0].setClickCallback(
			[](){
				curList = 0;
			}
		);
		btns[1].setClickCallback(
			[](){
				curList = 1;
			}
		);
	}
	
	void outputMembersList(){
		tOutput("Id   Name                Socket                   Ip                  Status", 5, 0);
		int id = 1;
		for(auto& g: userData){
			UserData& user = g.second;
			tCol def = (id & 1) ? 0x07 : 0x0F;
			tSetOutputCol(def);
			
			tOutput(to_string(id), id + 5, 0);
			
			if(nameBlacklist.find(user.name) != nameBlacklist.end()) tSetOutputCol(0x04);
			tOutput(user.name, id + 5, 5);
			tSetOutputCol(def);
			
			tOutput(to_string(g.first), id + 5, 25);
			
			if(ipBlacklist.find(user.ip) != ipBlacklist.end()) tSetOutputCol(0x04);
			tOutput(user.ip, id + 5, 50);
			tSetOutputCol(def);
			
			string sStatus;
			tCol statusCol = 0x0A;
			if(user.isForbidden()) sStatus = "Forbidden", statusCol = 0x04;
			else if(user.status == eUserStatus.didNotLogin){
				if(user.invalidTokenCount) 
					sStatus = "Invalid Token " + to_string(user.invalidTokenCount), statusCol = 0x0E;
				else
					sStatus = "Did not Login", statusCol = 0x08;
			}
			else if(user.status == eUserStatus.normal) sStatus = "Normal", statusCol = 0x0A;
			else if(user.status == eUserStatus.admin) sStatus = "Admin", statusCol = 0x09;
			
			tSetOutputCol(statusCol);
			tOutput(sStatus, id + 5, 70);
			tSetOutputCol(def);
			id++;
		}
		tSetOutputCol(0x0F);
	}
	void outputTokensList(){
		tOutput("Id   Token               Name", 5, 0);
		int id = 1;
		for(const auto& g: mpNameToToken){
			const string& name = g.first;
			const string& tok = g.second;
			tSetOutputCol((id & 1) ? 0x07 : 0x0F);
			tOutput(to_string(id), id + 5, 0);
			tOutput(tok, id + 5, 5);
			tOutput(name, id + 5, 25);
			id++;
		}
		tSetOutputCol(0x0F);
	}
	string cliBuf, cliShowback;
	tCol sbc = 0x0F;
	void processCLI(){
		if(tIsMouseInArea(2, 0, 28, 120) && GetAsyncKeyState(1)) focus = 1;
		
		if(focus == 1 && tKbhit()){
			wchar_t wch = tGetwch();
			string s = tWchar_to_gbk(wch);
			char ch = 65;
			if(s.length() == 1) ch = s[0];
			if(ch == '\b'){
				if(cliBuf.length()) cliBuf.pop_back();
			}
			else if(ch == '\r' || ch == '\n'){
				stringstream ss(cliBuf);
				string op, type, tgt;
				ss >> op >> type;
				if(op == "ban" || op == "block"){
					ss >> tgt;
					if(type == "ip"){
						if(tgt.length()) 
						ipBlacklist.insert(tgt);
					}
					else if(type == "name"){
						if(tgt.length()) 
						nameBlacklist.insert(tgt);
					}
					sbc = 0x0C;
					cliShowback = "Succeed to " + op + " " + type + " " + tgt;
				}
				else if(op == "unblock"){
					ss >> tgt;
					if(type == "ip"){
						if(tgt.length()) 
						ipBlacklist.erase(tgt);
					}
					else if(type == "name"){
						if(tgt.length()) 
						nameBlacklist.erase(tgt);
					}
					sbc = 0x0A;
					cliShowback = "Succeed to " + op + " " + type + " " + tgt;
				}
				else if(op == "gen"){
					if(type.length())
						mpNameToToken[type] = to_hex(gtk());
					sbc = 0x0A;
					cliShowback = "Succeed to " + op + " token for " + type;
				}
				else if(op == "delete"){
					if(type.length())
						mpNameToToken.erase(type);
					sbc = 0x0C;
					cliShowback = "Succeed to " + op + " token for " + type;
				}
				cliBuf = "";
			}
			else if(wch == 224){
				;
			}
			else{
				cliBuf += s;
			}
		}
		
		tOutput(string(120, '='), 27, 0);
		tOutput("> " + cliBuf + ((time(0) & 1) ? "_" : ""), 28, 0);
		tSetOutputCol(sbc);
		tOutput(cliShowback, 29, 0);
		tSetOutputCol(0x0F);
	}
	
	void processManagerPage(){
		for(auto& g: btns){
			g.process();
		}
		if(curList == 0) outputMembersList();
		else if(curList == 1) outputTokensList();
		processCLI();
	}
}
