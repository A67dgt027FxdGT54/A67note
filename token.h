#pragma once
#include <bits/stdc++.h>

using u64 = unsigned long long;

u64 a67hash(const void* vt, size_t s){
	u64 ret = 0;
	const unsigned char* t = reinterpret_cast<const unsigned char*>(vt);
	while(s--){
		ret = ret * 0xE8D4A51027ULL + *t;
		t++;
	}
	return ret;
}
u64 lsh(u64 x, u64 y){
	if(y >= 64) return 0LL;
	return x << y;
} 
u64 rsh(u64 x, u64 y){
	if(y >= 64) return 0LL;
	return x >> y;
} 
u64 lsr(u64 x, u64 y){
	y = y & 0x3f;
	return lsh(x, y) | rsh(x, 64 - y);
}
u64 rsr(u64 x, u64 y){
	y = y & 0x3f;
	return rsh(x, y) | lsh(x, 64 - y);
}
size_t getSize(const std::string& s){
	return sizeof(char) * s.length();
}
u64 getCurDay(){
	return (time(0) - 57600LL) / 86400LL;
}
u64 getToken(const std::string& sUserName, const std::string& sLinkingPwd){
	u64 hUserName = a67hash(sUserName.c_str(), getSize(sUserName));
	u64 hTmp = a67hash(sLinkingPwd.c_str(), getSize(sLinkingPwd));
	u64 iDay = getCurDay();
	u64 hDay = a67hash(&iDay, sizeof(u64));
	
	if(hTmp & 0x40) hUserName = lsr(hUserName, hTmp & 0x3F);
	else            hUserName = rsr(hUserName, hTmp & 0x3F);
	
	if(hTmp & 0x4000) hDay = lsr(hDay, (hTmp >> 8) & 0x3F);
	else              hDay = rsr(hDay, (hTmp >> 8) & 0x3F);
	
	hTmp >>= 16;
	u64 hLinkingPwd = a67hash(&hTmp, sizeof(u64));
	return hLinkingPwd ^ hUserName ^ hDay;
}

