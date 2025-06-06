// IpManager.h: interface for the CIpManager class.
//
//////////////////////////////////////////////////////////////////////
#include "User.h"

#pragma once

struct IP_ADDRESS_INFO
{
	char IpAddress[16];
	char Account[11];
	WORD IpAddressCount;
	WORD IpFloodAttemps;
	WORD IpBlocked;
	DWORD IpBlockedTime;
	DWORD IpTime;
	DWORD IpTime2;
	DWORD IpFloodLastTime;
	DWORD Offattack;
};

class CIpManager
{
public:
	CIpManager();
	virtual ~CIpManager();
	bool CheckIpAddress(char* IpAddress);
	void InsertIpAddress(char* IpAddress, char* Account);
	void RemoveIpAddress(char* IpAddress);
private:
	std::map<std::string,IP_ADDRESS_INFO> m_IpAddressInfo;
};

extern CIpManager gIpManager;
