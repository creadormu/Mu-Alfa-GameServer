#pragma once

#include "User.h"
#include "Protocol.h"

struct PMSG_CHANGECLASS_DATA
{
	PSBMSG_HEAD	Head;
	int PriceType;
	int Price;
};

struct CG_CHANGECLASS_RECV
{
	PSBMSG_HEAD Head;
	int Type;
};

class cChangeClass
{
public:
	cChangeClass();
	virtual ~cChangeClass();
	void Init();
	void Load(char* path);
	void ChangeClass(LPOBJ lpObj, int Class);
	void SendData(int aIndex);
	void RecvChangeClass(CG_CHANGECLASS_RECV* Data, int aIndex);
	static void ChangeClassCallback(LPOBJ lpObj,int Class,DWORD null,DWORD WCoinC,DWORD WCoinP,DWORD GoblinPoint);
	// ----
	int m_Price[MAX_ACCOUNT_LEVEL];
	int m_PriceType[MAX_ACCOUNT_LEVEL];
	int m_ChangeClassSwitch;
	int m_ChangeClassEnable[MAX_ACCOUNT_LEVEL];
	int m_ChangeClassRequireLevel[MAX_ACCOUNT_LEVEL];
	int m_ChangeClassRequireReset[MAX_ACCOUNT_LEVEL];

}; extern cChangeClass gChangeClass;