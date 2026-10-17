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

unsigned long long mySocket = -1;
string myName = "", myIp = "127.0.0.1", svIp = "127.0.0.1", token = "";
bool isServer = false;

server_t sv;
client_t cl;


struct EUserStatus{
	const int didNotLogin = 0, normal = 1, admin = 2;
}eUserStatus;

struct UserData{
	string name = "Unknown";
	int status = eUserStatus.didNotLogin;
	int invalidTokenCount = 0;
	string ip = "0.0.0.0";
};
map<unsigned long long, UserData> userData;


int main(){
	setup();
	showSetupUI();
}

#include "setup.h"
#include "setup_ui.h"
