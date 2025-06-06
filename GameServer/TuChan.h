#pragma once

#include "StdAfx.h"
#include "user.h"

struct TUCHAN_INFO
{
	int	Enabled;
	int	gemcount;
	int	gemcount1;
	int	gemcount2;
	int	gemcount3;
	int	maxlv;
	int rType;
	int rIndex;
	int rType1;
	int rIndex1;
	int rType2;
	int rIndex2;
	int rType3;
	int rIndex3;
	int rdmg;
	int rdouble;
	int rcrit;
	int rexc;
	int Delay;
	
};


class cTitle
{
public:
	cTitle();
	virtual ~cTitle();
	void	Init();
	void	Load(char *path);
	void	gObjUserTitle(LPOBJ lpObj);
	void	Option(LPOBJ lpObj, bool flag);
	//void RequestParty(PMSG_EVENTTUCHAN_REQ* lpMsg, int aIndex);
	TUCHAN_INFO m_TuChan;
	

};
extern cTitle gTitle;
