
#ifndef A67LAN_REWORK_EXCEPTS
#define A67LAN_REWORK_EXCEPTS

class A67LAN_rework_networkError: public std::runtime_error{
public:
	explicit A67LAN_rework_networkError(const std::string& msg): std::runtime_error(msg){
	}
};

class A67LAN_rework_FailedToStartupWSA: public A67LAN_rework_networkError{
public:
	explicit A67LAN_rework_FailedToStartupWSA(): A67LAN_rework_networkError("A67LAN_rework: Failed to startup WSA."){
	}
};

class A67LAN_rework_FailedToCreateServerSocket: public A67LAN_rework_networkError{
public:
	explicit A67LAN_rework_FailedToCreateServerSocket(): A67LAN_rework_networkError("A67LAN_rework: Failed to create server socket."){
	}
};

class A67LAN_rework_BadListen: public A67LAN_rework_networkError{
public:
	explicit A67LAN_rework_BadListen(): A67LAN_rework_networkError("A67LAN_rework: Bad listen."){
	}
};

class A67LAN_rework_BadProtocol: public std::runtime_error{
public:
	explicit A67LAN_rework_BadProtocol(): std::runtime_error("A67LAN_rework: Bad protocol."){
	}
};
class A67LAN_rework_FailedToLinkToServer: public A67LAN_rework_networkError{
public:
	explicit A67LAN_rework_FailedToLinkToServer(): A67LAN_rework_networkError("A67LAN_rework: Failed to link to Server."){
	}
};

#if 0  // unused
class A67LAN_rework_BadBindToPort: public A67LAN_rework_networkError{
public:
	explicit A67LAN_rework_BadBindToPort(): A67LAN_rework_networkError("A67LAN_rework: Bad bind to port " + std::to_string(A67LAN_REWORK_PORT) +"."){
	}
};

#endif  // unused

#endif
