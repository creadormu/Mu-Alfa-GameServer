#pragma once

#include "user.h"
#include "Protocol.h"


// -------------------------------------------------------------------------------
struct MUA_VIP_OPTION_DATA
{
	int			Index;

	int			MUA_VIP_SAT_THUONG;
	int			MUA_VIP_SAT_THUONG_X2;
	int			MUA_VIP_PHONG_THU;
	int			MUA_VIP_THE_LUC;
	int			MUA_VIP_SD;
};
// -------------------------------------------------------------------------------
struct BUY_MUA_VIP_REQ
{
	PSBMSG_HEAD h;
	int			Number;
};
// -------------------------------------------------------------------------------
class MuaVip
{
public:
	// ----
	void		Init();
	// ----
	MuaVip();
	virtual ~MuaVip();
	// ----
	void		BUY_MUA_VIP(LPOBJ lpObj, BUY_MUA_VIP_REQ *aRecv);
	// ----
	void		Load(char* path);
	void		SET_MUA_VIP_INFO(MUA_VIP_OPTION_DATA info);
	void		MUA_VIP_OPTION(LPOBJ lpObj, bool flag);
	// ----
	MUA_VIP_OPTION_DATA M_OPTION_MUA_VIP[5];
	// ----
}; extern MuaVip gMuaVip;

// -------------------------------------------------------------------------------