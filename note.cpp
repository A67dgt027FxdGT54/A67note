constexpr int _port = 6767;
#define port _port
#include "LAN.h"
#undef port
#include "TUI.h" 
#include "token.h"
using namespace std;

void wlog(std::string level,std::string msg){
	std::ofstream _log("note.log",std::ios::app);
    std::time_t now = std::time(nullptr);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    _log << "[" << time_buf << "] [" <<level<<"] "<<msg<<std::endl;
}

void setup();
void showSetupUI();
void processMsg();
void processPage();
int checkNormalPath(const std::string& path);
string getTimesZh(int times);
string getTimesEn(int times);

string myName = "", myIp = "127.0.0.1", linkingPwd = "", svIp = "127.0.0.1", token = "";

server_t sv;
client_t cl;

bool isServer = false;

/*	Message Format
 *	- from client to server:
 *		- login <userName> <token>
 *		- get <path> <userName> <token>
 *	- from server to client:
 *		- 204
 *		- 401 did_not_login
 *		- 401 invalid_token <invalidTokenCount>
 *		- 403 
 *		- 404
 */

struct EUserStatus{
	const int didNotLogin = 0, normal = 1, admin = 2, forbidden = 3;
}eUserStatus;
struct UserData{
	string name = "Unknown";
	int status = eUserStatus.didNotLogin;
	int invalidTokenCount = 0;
};

map<string/*Ip*/, UserData> userData;

struct File{
	vector<string> lines = {""};
	bool loadFromFile(string path){
		lines.clear();
		ifstream ifsFile(path);
		string sTempLine;
		if(!ifsFile.is_open()){
			lines = {""};
			return false;
		} 
		while(getline(ifsFile, sTempLine)){
			lines.push_back(sTempLine);
		}
		if(lines.size() == 0) lines = {""};
		return true;
	}
};

void requestForPage(const string& path){
	stringstream ss;
	ss << "get " << path << " " << myName << " " << token;
	cl.send_msg(ss.str(), svIp);
}
struct CurPageData{
	int type = 2, invalidTokenCount = 0;
	/*   0 = normal
	 *   1 = manager
	 *   2 = loading normal page
	 *   401 = unauthorized (desc: did_not_login | invalid_token)
	 *   403 = forbidden
	 *   404 = notfound
	 */
	string path = "index";
	string desc = "";
	File curFile;
	
	bool loadFileFromCurPath(){
		if(!isServer || type != 0){
			return false;
		}
		curFile.loadFromFile("data/" + path);
		return true;
	}
	void getPageWithCurPath(){
		if(path == "#manager"){
			if(isServer){
				type = 1;
			}
			else{
				type = 403;
			}
		}
		else if(!checkNormalPath(path)){
			if(isServer){
				type = 0;
				loadFileFromCurPath();
			}
			else{
				type = 2;
				requestForCurPage();
			}
		}
		else{
			type = 404;
		}
	}
	void requestForCurPage(){
		requestForPage(path);
	}
}curPage;

int main(){
	setup();
	showSetupUI();
	
	while(1){
		processMsg();
		processPage();
		Sleep(10);
	}
	
	tPause();
}

#include "check.h"
#include "setup.h"
#include "setup_ui.h"
#include "processMsg.h"
#include "processPage.h"
