#pragma once

#include "user.h"
#include "Protocol.h"


// -------------------------------------------------------------------------------
struct DANH_HIEU_OPTION_DATA
{
	int			Index;

	int			DANH_HIEU_SAT_THUONG;
	int			DANH_HIEU_SAT_THUONG_X2;
	int			DANH_HIEU_PHONG_THU;
	int			DANH_HIEU_THE_LUC;
	int			DANH_HIEU_SD;
};
// -------------------------------------------------------------------------------
struct BUY_DANH_HIEU_REQ
{
	PSBMSG_HEAD h;
	int			Number;
};
// -------------------------------------------------------------------------------
class DanhHieu
{
public:
	// ----
	void		Init();
	// ----
	DanhHieu();
	virtual ~DanhHieu();
	// ----
	void		BUY_DANH_HIEU(LPOBJ lpObj, BUY_DANH_HIEU_REQ *aRecv);
	// ----
	void		Load(char* path);
	void		SET_DANH_HIEU_INFO(DANH_HIEU_OPTION_DATA info);
	void		DANH_HIEU_OPTION(LPOBJ lpObj, bool flag);
	// ----
	DANH_HIEU_OPTION_DATA M_OPTION_DANH_HIEU[20];
	// ----
}; extern DanhHieu gDanhHieu;


// -------------------------------------------------------------------------------