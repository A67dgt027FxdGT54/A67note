#define A67LAN_REWORK_PORT 6767
#include "LAN_rework.h" 
#include "TUI.h" 
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

unsigned long long mySocket = -1, svSocket = -1;
string myName = "", myIp = "127.0.0.1", svIp = "127.0.0.1", token = "";
bool isServer = false;


/*	Message Format
 *	- from client to server:
 *		- login <userName> <token>
 *		- get <path> <userName> <token>
 *	- from server to client:
 *		- 200 <path> \n line1 \n line2 \n ...
 *		- 204 (login_succeed | get_no_content <path>)
 *		- 401 did_not_login
 *		- 401 invalid_token <invalidTokenCount>
 *		- 403 
 *		- 404
 */

server_t sv;
client_t cl;


struct EUserStatus{
	const int didNotLogin = 0, normal = 1, admin = 2;
}eUserStatus;

//#define TOKEN_PLACEHOLDER "__PLACEHOLDER__"
#define NAME_PLACEHOLDER "__NAME PLACEHOLDER__" 
	// there is a space because stringstream splits by space so that no one can set user name to the placeholder.
#define IP_PLACEHOLDER "__IP_PLACEHOLDER__"

set<string> ipBlacklist, nameBlacklist;

map<string, string> mpNameToToken; 

struct UserData{
	string name = NAME_PLACEHOLDER;
	int status = eUserStatus.didNotLogin;
	int invalidTokenCount = 0;
	string ip = IP_PLACEHOLDER;
	bool isForbidden(){
		return ipBlacklist.find(this->ip) != ipBlacklist.end() || nameBlacklist.find(this->name) != nameBlacklist.end();
	}
};
map<unsigned long long, UserData> userData;

void banUser(unsigned long long sock){
	UserData& user = userData[sock];
	if(user.ip != IP_PLACEHOLDER && !user.ip.empty() && ipBlacklist.find(user.ip) == ipBlacklist.end()) ipBlacklist.insert(user.ip);
	if(user.name != NAME_PLACEHOLDER && nameBlacklist.find(user.name) == nameBlacklist.end()) nameBlacklist.insert(user.name);
}

#define sendMsgToServer(_msg) do{ \
	cl.send_msg(_msg, svSocket); \
}while(0)

vector<int> calcCharType(const std::string& value){
	vector<int> ct(value.length());
	for(int i = 0; i < value.length(); i++){
		if(value[i] >= 0) ct[i] = 0;
		else{
			ct[i] = 1;
			if(i + 1 < value.size()) ct[++i] = 2;
		}
	}
	return ct;
}

struct File{
	vector<string> lines = {""};
	/*  Type of the char.
	 *  0 = ASCII
	 *  1 = GBK(first byte)
	 *  2 = GBK(second byte)
	 */ 
	vector<vector<int> > charType = {{}}; 
	bool loadFromFile(string path){
		lines.clear();
		ifstream ifsFile(path);
		string sTempLine;
		if(!ifsFile.is_open()){
			lines = {""};
			charType = {{}}; 
			return false;
		} 
		while(getline(ifsFile, sTempLine)){
			if(sTempLine.size() && sTempLine.back() == '\r') sTempLine.pop_back();
			lines.push_back(sTempLine);
		}
		if(lines.size() == 0) {lines = {""}; charType = {{}}; return false;}
		initCharType();
		return true;
	}
	void initCharType(){
		charType = vector<vector<int> >(lines.size());
		for(int i = 0; i < lines.size(); i++){
			charType[i] = calcCharType(lines[i]);
		}
	}
	void erase(int start, int count){
		lines.erase(lines.begin() + start, lines.begin() + start + count);
		charType.erase(charType.begin() + start, charType.begin() + start + count);
	}
	void insert(int index, const std::string& value){
		auto ct = calcCharType(value);
		lines.insert(lines.begin() + index, value);
		charType.insert(charType.begin() + index, ct);
	}
};
void requestForPage(const string& path){
	stringstream ss;
	ss << "get " << path << " " << myName << " " << token;
	sendMsgToServer(ss.str());
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

namespace nsProcessNormalPage{void setupFunctions();}

int main(){
	setup();
	nsProcessNormalPage::setupFunctions();
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
