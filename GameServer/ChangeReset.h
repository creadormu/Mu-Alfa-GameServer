#pragma once

#include "User.h"
#include "Protocol.h"

enum eChangeReset
{
	CHANGE_RESET_5 = 0,
	CHANGE_RESET_10 = 1,
	CHANGE_RESET_15 = 2,
	CHANGE_RESET_20 = 3,
	CHANGE_RESET_25 = 4,
};

struct CG_CHANGE_RESET_RECV
{
	PSBMSG_HEAD Head;
	int Type;
};

class cChangeReset
{
public:
	cChangeReset();
	virtual ~cChangeReset();
	void Init();
	void ChangeReset(LPOBJ lpObj, int Class);
	void RecvChangeReset(CG_CHANGE_RESET_RECV* Data, int aIndex);
	static void ChangeClassCallback(LPOBJ lpObj,int Class,DWORD null,DWORD WCoinC,DWORD WCoinP,DWORD GoblinPoint);
private:
}; extern cChangeReset gChangeReset;