#pragma once

void processMsgSv(){
	if(!cl.empty()){
		msg_t msg = cl.get_msg();
		if(msg.type == "sys"){
			if(msg.con == "join"){
				wlog("INFO", "user Ip " + msg.ip + " joined");
			}
			else if(msg.con == "leave"){
				wlog("INFO", "user Ip " + msg.ip + " left");
			}
		}
		else if(msg.type == "msg"){
			stringstream ss(msg.con);
			string msgType;
			ss >> msgType;
			if(msgType == "login"){
				string userName, userToken;
				ss >> userName >> userToken;
				
				
				UserData& user = userData[msg.ip];
				
				if(user.status == eUserStatus.didNotLogin){
					userData[msg.ip].name = userName;
					string correctToken = to_hex(getToken(userName, linkingPwd));
					if(userToken == correctToken){
						cl.send_msg("204", msg.ip);
						user.status = eUserStatus.normal;
						user.invalidTokenCount = 0;
					}
					else{
						cl.send_msg("401 invalid_token " + to_string(++user.invalidTokenCount), msg.ip);
						if(user.invalidTokenCount >= 3) user.status = eUserStatus.forbidden;
					}
				}
				else if(user.status == eUserStatus.normal){
					userData[msg.ip].name = userName;
					; // ignore
				}
				else if(user.status == eUserStatus.admin){
					userData[msg.ip].name = userName;
					; // ignore
				}
				else if(user.status == eUserStatus.forbidden){
					cl.send_msg("403", msg.ip);
				}
			}
		}
	}
}
void processMsgCl(){
	if(!cl.empty()){
		msg_t msg = cl.get_msg();
		if(msg.type == "sys"){
			if(msg.con == "join"){
				wlog("INFO", "user Ip " + msg.ip + " joined");
			}
			else if(msg.con == "leave"){
				wlog("INFO", "user Ip " + msg.ip + " left");
			}
		}
		else if(msg.type == "msg"){
			if(msg.ip != svIp) return; // ignore msg from others.
			;
		}
	}
}
void processMsg(){
	if(isServer) processMsgSv();
	else processMsgCl();
}
