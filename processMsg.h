#pragma once

void processMsgSv(){
	if(!cl.empty()){
		msg_t msg = cl.get_msg();
		if(msg.type == A67LAN_REWORK_JOIN){
			UserData& user = userData[msg.from];
			user.ip = sv.getIpOfClient(msg.from);
			wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " joined");
		}
		else if(msg.type == A67LAN_REWORK_LEAVE){
			UserData& user = userData[msg.from];
			wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " left");
			userData.erase(msg.from);
		}
		else if(msg.type == A67LAN_REWORK_COMMON){
			stringstream ss(msg.con);
			string msgType;
			ss >> msgType;
			if(msgType == "login"){
				string userName, userToken;
				ss >> userName >> userToken;
				
				
				UserData& user = userData[msg.from];
				
				if(user.isForbidden()){
					cl.send_msg("403", msg.from);
					wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " login, status 403");
				}
				else if(user.status == eUserStatus.didNotLogin){
					if(mpNameToToken.find(userName) != mpNameToToken.end() && userToken == mpNameToToken[userName]){
						user.name = userName;
						cl.send_msg("204 login_succeed", msg.from);
						user.status = eUserStatus.normal;
						user.invalidTokenCount = 0;
						wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " login, status 204 login_succeed");
					}
					else{
						cl.send_msg("401 invalid_token " + to_string(++user.invalidTokenCount), msg.from);
						if(user.invalidTokenCount >= 3) banUser(msg.from);
						wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " login, status 401 invalid_token " 
							+ to_string(user.invalidTokenCount));
					}
				}
				else if(user.status == eUserStatus.normal){
					wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " login, status 400");
					; // ignore
				}
				else if(user.status == eUserStatus.admin){
					wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " login, status 400");
					; // ignore
				}
			}
			else if(msgType == "get"){
				string path, userName, userToken;
				ss >> path >> userName >> userToken;
				UserData& user = userData[msg.from];
				if(user.isForbidden()){
					cl.send_msg("403", msg.from);
					wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " get, status 403");
				}
				else if(user.status == eUserStatus.didNotLogin){
					cl.send_msg("401 did_not_login", msg.from);
					wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " get, status 401 did_not_login");
				}
				else{ // normal / admin
					if(user.name == userName && mpNameToToken.find(userName) != mpNameToToken.end() && userToken == mpNameToToken[userName]){
						// Success.
						File file;
						if(checkNormalPath(path) != 0){
							cl.send_msg("404", msg.from);
							wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " get, status 404");
						}
						else if(!file.loadFromFile("data/" + path)){
							cl.send_msg("204 get_no_content " + path, msg.from);
							wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " get path " + path + 
								", status 204 get_no_content");
						}
						else{
							stringstream ssMsg;
							ssMsg << "200 " << path << "\n";
							for(auto& s: file.lines){
								ssMsg << s << "\n";
							}
							cl.send_msg(ssMsg.str(), msg.from);
							wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " get path " + path + ", status 200");
						}
					}
					else{
						cl.send_msg("401 invalid_token " + to_string(++user.invalidTokenCount), msg.from);
						if(user.invalidTokenCount >= 3) banUser(msg.from);
						wlog("INFO", "user socket " + to_string(msg.from) + " ip " + user.ip + " get, status 401 invalid_token " 
							+ to_string(user.invalidTokenCount));
					}
				}
			}
			// else ignore
		}
	}
}
void processMsgCl(){
	if(!cl.empty()){
		msg_t msg = cl.get_msg();
		if(msg.type == A67LAN_REWORK_JOIN){
			wlog("INFO", "user socket " + to_string(msg.from) + " joined");
		}
		else if(msg.type == A67LAN_REWORK_LEAVE){
			wlog("INFO", "user socket " + to_string(msg.from) + " left");
		}
		else if(msg.type == A67LAN_REWORK_COMMON){
			if(msg.from != svSocket) return; // ignore msg from others.
			stringstream ss(msg.con);
			int status = -1;
			ss >> status;
			if(status == 200){
				string _tmp, path;
				ss >> path;
				getline(ss, _tmp);
				if(path != curPage.path) return;
				curPage.curFile.lines = {};
				while(getline(ss, _tmp)){
					curPage.curFile.lines.push_back(_tmp);
				}
				curPage.type = 0;
				wlog("INFO", "Request for path " + curPage.path + " and return status 200");
			}
			else if(status == 204){
				string msgType;
				ss >> msgType;
				if(msgType == "get_no_content"){
					string path;
					ss >> path;
					if(path != curPage.path) return;
					curPage.curFile.lines = {""};
					curPage.type = 0;
					wlog("INFO", "Request for path " + curPage.path + " and return status 204 get_no_content");
				}
				else if(msgType == "login_succeed"){
					; // no expected
				}
			}
			else if(status >= 400){
				curPage.type = status;
				if(status == 401){
					ss >> curPage.desc;
					if(curPage.desc == "invalid_token") ss >> curPage.invalidTokenCount;
				}
			}
		}
	}
}
void processMsg(){
	if(isServer) processMsgSv();
	else processMsgCl();
}
