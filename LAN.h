
/*

    The MIT License (MIT)

    Copyright (c) 2026 A67dgt027FxdGT54 (https://www.luogu.com.cn/user/1417426)

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/ 

#ifndef __A67dgt027FxdGT54_LAN_H
#define __A67dgt027FxdGT54_LAN_H 1417426

/*
 *   Linker Command:
 *    -liphlpapi -lws2_32
 */
#define _WIN32_WINNT 0x0600
#include <bits/stdc++.h>
#include <Winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <stdexcept>

#pragma comment (lib,"iphlpapi.lib")
#pragma comment (lib,"ws2_32")

#if 1
std::vector<std::string> get_ip() {
    std::vector<std::string> result;
    DWORD dwSize = 0;
    // 第一次调用获取所需缓冲区大小
    GetAdaptersInfo(NULL, &dwSize);
    if (dwSize == 0) return result;

    PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)malloc(dwSize);
    if (!pAdapterInfo) return result;

    if (GetAdaptersInfo(pAdapterInfo, &dwSize) != NO_ERROR) {
        free(pAdapterInfo);
        return result;
    }

    PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
    while (pAdapter) {
        // 遍历该适配器的 IP 地址列表（包括 IPv4）
        PIP_ADDR_STRING pAddr = &(pAdapter->IpAddressList);
        while (pAddr) {
            if (pAddr->IpAddress.String[0] != '\0') {
                std::string ip = pAddr->IpAddress.String;
                // 过滤掉环回地址 127.0.0.1 和空地址/APIPA
                if (ip != "127.0.0.1" && ip != "0.0.0.0" && ip.find("169.254.") != 0) {
                    result.push_back(ip);
                }
            }
            pAddr = pAddr->Next;
        }
        pAdapter = pAdapter->Next;
    }

    free(pAdapterInfo);
    return result;
}
#else
std::vector<std::string> get_ip() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
	    printf("WSAStartup failed\n");
	    exit(-1); 
	}
    std::vector<std::string> result;
    ULONG bufferSize = 0;
    DWORD dwRet = GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &bufferSize);
	if (dwRet != ERROR_BUFFER_OVERFLOW || bufferSize == 0) return result;
    PIP_ADAPTER_ADDRESSES pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(bufferSize);
    if (!pAddresses) return result;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    if (GetAdaptersAddresses(AF_INET, flags, NULL, pAddresses, &bufferSize) == NO_ERROR) {
        for (PIP_ADAPTER_ADDRESSES pAdapter = pAddresses; pAdapter != NULL; pAdapter = pAdapter->Next) {
            if (pAdapter->OperStatus != IfOperStatusUp) continue;
            if (pAdapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
            for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; 
                 pUnicast != NULL; 
                 pUnicast = pUnicast->Next) {
				if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
				    struct sockaddr_in* sa = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
				    char ipStr[INET_ADDRSTRLEN] = {0};
				    if (inet_ntop(AF_INET, &(sa->sin_addr), ipStr, INET_ADDRSTRLEN) != NULL) {
				        result.push_back(std::string(ipStr));
				    }
				}
            }
        }
    }
    free(pAddresses);
    WSACleanup();
    return result;
}
#endif
/*
 * * msg format:
 * <ip> sys (join/leave)
 * <ip> msg <msg>
 */

#define A67LAN_default 1
#define A67LAN_custom 2

#ifndef port
# define port 9999
# define A67LAN_port A67LAN_default
#else
# define A67LAN_port A67LAN_custom
#endif

#ifndef buf_size
#define buf_size 32767
# define A67LAN_buf_size A67LAN_default
#else
# define A67LAN_buf_size A67LAN_custom
#endif

struct client_data_t{
	SOCKET sock;
	std::string ip;
};
struct server_msg_t{
	std::string msg;
	std::string target;
};
class server_t{
public:
	int create(){ // MT 1.1
        if(open_socket()) return 1; // MT 1.2
    	SMT=std::thread([this](){SMT_f();});
//    	SMT.detach(); // MT 1.3
    	return 0;
    }
	server_t():running(1),server(INVALID_SOCKET){}
    ~server_t(){
    	running=0;
    	closesocket(server);
    	if(SST.joinable()) SST.join();
    	if(SMT.joinable()) SMT.join();
	}
	int people_limit=10; // people
//	std::unordered_map<SOCKET,std::thread> SRT;
private:
    std::queue<server_msg_t> send_buf; // sent
    SOCKET server;
    std::unordered_map<SOCKET,client_data_t> clients;
    std::mutex clients_mutex;
    std::mutex send_buf_mutex;
    std::atomic<bool> running;
	std::thread SST,SMT;
    void SRT_f(SOCKET sclient){
        int reg; 
        std::string tmMsg(size_t(buf_size), '\0');
        while(running){ // SRT 3.1
            tmMsg=std::string(size_t(buf_size), '\0');
            reg=recv(sclient,&tmMsg[0],buf_size,0); // SRT 3.2
            if(reg<=0) break;
            std::string ip;
			server_msg_t msg;
            {
            	std::lock_guard<std::mutex> lock(clients_mutex);
            	auto it=clients.find(sclient);
            	if(it!=clients.end()) ip=it->second.ip;
            	else break;
            }
			std::string data(tmMsg.data(), reg);
			std::stringstream ss(data);
            ss >> msg.target;
            std::getline(ss, msg.msg);
        	msg.msg=ip+" msg "+msg.msg;
        	{
        		std::lock_guard<std::mutex> lock(send_buf_mutex);
				send_buf.push(msg);
			} 
        }
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
        	auto it=clients.find(sclient);
        	if(it!=clients.end()) {
        		server_msg_t msg;
				msg.msg = it->second.ip+" sys leave";
				msg.target = "all";
        		{
	        		std::lock_guard<std::mutex> lock(send_buf_mutex);
					send_buf.push(msg);
				} 
        		clients.erase(it);
			}
		}
        closesocket(sclient);
//        if(online_cnt==0) exit(-1); 
        return ;
    }
    void SST_f(){
        while(running){
        	server_msg_t msg;
			{
        		std::lock_guard<std::mutex> lock_send(send_buf_mutex);
        		if(!send_buf.empty()){
					msg=send_buf.front();
					send_buf.pop();
				} 
			} 
			if(msg.msg!=""){
		        std::lock_guard<std::mutex> lock_clients(clients_mutex);
	            for(auto &pr:clients){
	            	if(msg.target != "all" && pr.second.ip != msg.target) continue;
	            	auto sock=pr.first;
					send(sock,msg.msg.c_str(),msg.msg.length(),0);
				}
	        }
        	
	        Sleep(10);
		} 
        return ;
    }
    void find_socket(){ // find the clients
#ifdef DEBUG_OUTPUT
		std::cout<<"Debug: Server is listening on port \n"<<port<<"\n";
#endif
        sockaddr_in client;
        int client_len=sizeof(client);
        SST=std::thread([this](){SST_f();});
//        SST.detach(); // SMT 1.4
        while(running){ // SMT 2.1
        	int clients_size;
			{
            	std::lock_guard<std::mutex> lock(clients_mutex);
            	clients_size=clients.size();
            }
            if(clients_size>=people_limit){
                puts("People over the limit. Please Wait for a moment...\n\n");
                while(clients_size>=people_limit){
                	Sleep(200);
					{
		            	std::lock_guard<std::mutex> lock(clients_mutex);
		            	clients_size=clients.size();
		            }
				} 
            }
            SOCKET ns=accept(server,(sockaddr FAR*)&client,&client_len); // SMT 2.2 & 2.5
            if(ns==(SOCKET)(~0)){puts("Fail to Link!\n");continue;}
            char ip[INET_ADDRSTRLEN]={0};
            inet_ntop(AF_INET,&(client.sin_addr),ip,INET_ADDRSTRLEN);
            {
            	std::lock_guard<std::mutex> lock(clients_mutex);
            	clients[ns]={ns,std::string(ip)};
			}
//            join_callback(ip);
            std::string iTmMsg=(std::string)(ip);
            iTmMsg+=" sys join";
            server_msg_t msg;
            msg.msg = iTmMsg;
            msg.target = "all";
            
			{
        		std::lock_guard<std::mutex> lock(send_buf_mutex);
				send_buf.push(msg); // SMT 2.6
			} 
            std::thread SRT([this,ns](){SRT_f(ns);}); // SMT 2.8
            SRT.detach(); // keep it
        }
    }
    bool open_socket(){ // try to open a server, return 0=succeed, 1=localhost has one now
        WSADATA wsaData;
        if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0){
        	puts("fail to open Winsock");
			exit(-1);
		} 
        server=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(server==(SOCKET)(~0)){
            puts("fail to init Winsock");
            WSACleanup();
            exit(-1); 
        }
        sockaddr_in bindser;
        bindser.sin_family=AF_INET;
        bindser.sin_port=htons(port);
        bindser.sin_addr.s_addr=htonl(INADDR_ANY);
        if(bind(server,(const struct sockaddr*)&bindser,sizeof(bindser))==(SOCKET)(~0)){
            puts("localhost has one server now"); 
            WSACleanup();
            closesocket(server);
            return 1;
        }
        if(listen(server,people_limit)==(SOCKET)(~0)){
            puts("fail to listen");
            WSACleanup();
            closesocket(server);
            exit(-1); 
        }
        return 0;
    }
	void SMT_f(){
		find_socket(); // SMT 1.3
	}
};

struct msg_t{
	std::string ip;
	std::string type; 
	std::string con;
	bool valid=1;
};
class client_t{
public:
	client_t():running(1),server(INVALID_SOCKET){}
	void create(std::string ip){// MT 2.2
	    WSADATA wsaData;
	    if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0) exit(-1);
	    server=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	    if(server==(SOCKET)(~0)){
	    	WSACleanup();exit(-1); 
		} 
	    mip=ip;
	    CMT=std::thread([this](){CMT_f();});
//	    CMT.detach();// MT 2.3
	}
	void send_msg(const std::string& msg, const std::string& target = "all"){
		{
    		std::lock_guard<std::mutex> lock(send_buf_mutex);
			send_buf.push(target + " " + msg);
		} 
	}
	~client_t(){
		running=0;
	    closesocket(server);
	    if(CMT.joinable()) CMT.join();
	    if(CST.joinable()) CST.join();
	    WSACleanup();
	}
	bool empty(){
		std::lock_guard<std::mutex> lock(msgs_mutex);
		return msgs.empty();
	}
	msg_t get_msg(){
		std::lock_guard<std::mutex> lock(msgs_mutex);
		msg_t msg=msgs.front();
		msgs.pop();
		return msg;
	}
private:
	SOCKET server;
	std::string mip;
	std::queue<std::string> send_buf;
	std::mutex send_buf_mutex;
	std::atomic<bool> running;
	std::thread CMT,CST;
	std::queue<msg_t> msgs; 
	std::mutex msgs_mutex;
	msg_t decode(char* rc){
		msg_t m;
		int lvl=0;
		int len=strlen(rc);
		for(int i=0;i<len;i++){
			if(rc[i]==' ') lvl++;
			else if(lvl==0) m.ip+=rc[i];
			else if(lvl==1) m.type+=rc[i];
			else if(lvl==2) {
				m.con+=(rc+i);
				break;
			}
		}
		if(mip==m.ip) m.valid=0;
		return m;
	}
	void CST_f(){
		while(running){
			Sleep(50);
			{
        		std::lock_guard<std::mutex> lock(send_buf_mutex);
				if(!send_buf.empty()){
		            int rag=0;
			        rag=send(server,send_buf.front().c_str(),send_buf.front().length(),0);
			        if(rag==-1) break;
		            send_buf.pop();
		        }
			} 
			
		}
        return ;
	}
	void Client(){ // CMT 3.5
	    char rc[buf_size];
	    memset(rc,0,sizeof(rc));
	    while(running){ // CMT 3.6
	        Sleep(10);
	        memset(rc,0,sizeof(rc));
	        int rag=recv(server,rc,buf_size,0); // CMT 3.7
	        if(rag==-1){puts("Server Console Closed!\n");closesocket(server);break;}
	        if(rag == 0) break;
	        msg_t m=decode(rc);
	        if(!m.valid) continue;
	        else {
        		std::lock_guard<std::mutex> lock(msgs_mutex);
				msgs.push(m);
			} 
	    }
	}
	void CMT_f(){
	    sockaddr_in conser;
	    conser.sin_family=AF_INET;
	    conser.sin_port=htons(port);
	    conser.sin_addr.S_un.S_addr=inet_addr(mip.c_str());
	    for(int i=1;i<=10;i++){
	        if(connect(server,(LPSOCKADDR)&conser,sizeof(conser))==(SOCKET)(~0)){ // CMT 2.4
	            if(i==10){
	                puts("Wrong IP!\n");
	                exit(-1); 
	            }
	        }
	        else break;
	        Sleep(100);
	    }
	    CST=std::thread([this](){CST_f();});
//	    CST.detach(); // CMT 2.5
	    Client(); 
	}
};

void disable_quickedit(){
	HANDLE hStdin=GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin,&mode);
	mode&=~ENABLE_QUICK_EDIT_MODE;
	mode&=~ENABLE_INSERT_MODE;
	mode&=~ENABLE_MOUSE_INPUT;
	SetConsoleMode(hStdin,mode);
}

std::string encode_base64(std::string& orig){
	std::string ret="";
	int r=orig.length()%3,
		q=orig.length()/3;
	for(int i=0;i<3*q;i+=3){
		unsigned int block=((unsigned char)(orig[i])<<16)|((unsigned char)(orig[i+1])<<8)|(unsigned char)(orig[i+2]),
					 tmp;
		for(int j=0;j<4;j++){
			tmp=(block>>((3-j)*6))&63; 
			ret+=tmp<=25?(65+tmp):(tmp<=51?(71+tmp):(tmp<=61?(tmp-4):(tmp==62?'+':'/')));
		}
	}
	if(r==1){
		unsigned int block=((unsigned char)(orig[3*q])<<4),
					 tmp;
		for(int j=0;j<2;j++){
			tmp=(block>>((1-j)*6))&63; 
			ret+=tmp<=25?(65+tmp):(tmp<=51?(71+tmp):(tmp<=61?(tmp-4):(tmp==62?'+':'/')));
		}
		ret+="==";
	}
	if(r==2){
		unsigned int block=((unsigned char)(orig[3*q])<<10)|((unsigned char)(orig[3*q+1])<<2),
					 tmp;
		for(int j=0;j<3;j++){
			tmp=(block>>((2-j)*6))&63; 
			ret+=tmp<=25?(65+tmp):(tmp<=51?(71+tmp):(tmp<=61?(tmp-4):(tmp==62?'+':'/')));
		}
		ret+="=";
	}
	return ret;
}
std::string decode_base64(std::string& orig){
	std::string ret="";
	for(int i=0;i<orig.length();i+=4){
		unsigned int block=0,tmp;
		for(int j=i;j<i+4;j++){
			tmp=orig[j];
			block<<=6;
			block|=tmp=='='?0:(tmp=='+'?62:(tmp=='/'?63:(tmp>96?tmp-71:(tmp>64?tmp-65:tmp+4))));
		}
		
		ret+=block>>16;
		if(orig[i+2]!='='){
			ret+=(block>>8)&255;
			if(orig[i+3]!='='){
				ret+=block&255;
			}
		}
	}
	return ret;
}

#if A67LAN_port == A67LAN_default
# undef port
#endif

#if A67LAN_buf_size == A67LAN_default
# undef buf_size
#endif

#undef A67LAN_port
#undef A67LAN_buf_size
#undef A67LAN_default
#undef A67LAN_custom

#endif // __A67dgt027FxdGT54_LAN_H
