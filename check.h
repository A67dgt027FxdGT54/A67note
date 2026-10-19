#pragma once

int checkChooseIp(const std::string& choose){
	if(choose.length() >= 10) return 2;
	if(choose.empty()) return 3;
	for(char c: choose){
		if(!isdigit((unsigned char)c)) return 1;
	}
	return 0;
}

int checkUserName(const std::string& name){
	if(name.length() > 20) return 1;
	if(name.empty()) return 3;
	for(char c: name){
		if(isspace((unsigned char)c)) return 2;
	}
	return 0;
}

int checkNormalPath(const std::string& path){
	if(path.length() > 64) return 1;
	if(path.empty()) return 3;
	for(char c: path){
		if(!isalnum((unsigned char)c) && c != '_') return 2;
	}
	return 0;
}
