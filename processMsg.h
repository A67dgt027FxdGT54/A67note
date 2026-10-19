#pragma once

void processMsgSv(){
	if(!cl.empty()){
		msg_t msg = cl.get_msg();
		if(msg.type == A67LAN_REWORK_JOIN){
			UserData& user = userData[msg.from];
			user.ip = sv.getIpOfClient(msg.from);
			wlog("INFO", "user socket " + to_string(msg.from) + "ip" + user.ip + " joined");
		}
		else if(msg.type == A67LAN_REWORK_LEAVE){
			UserData& user = userData[msg.from];
			wlog("INFO", "user socket " + to_string(msg.from) + "ip" + user.ip + " left");
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
				}
				else if(user.status == eUserStatus.didNotLogin){
					if(user.token != TOKEN_PLACEHOLDER && userToken == user.token){
						user.name = userName;
						cl.send_msg("204", msg.from);
						user.status = eUserStatus.normal;
						user.invalidTokenCount = 0;
					}
					else{
						cl.send_msg("401 invalid_token " + to_string(++user.invalidTokenCount), msg.from);
						if(user.invalidTokenCount >= 3) banUser(msg.from);
					}
				}
				else if(user.status == eUserStatus.normal){
					user.name = userName;
					; // ignore
				}
				else if(user.status == eUserStatus.admin){
					user.name = userName;
					; // ignore
				}
			}
		}
		// else ignore
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
			;
		}
	}
}
void processMsg(){
	if(isServer) processMsgSv();
	else processMsgCl();
}
