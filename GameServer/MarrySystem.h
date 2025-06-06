#pragma once

#include "User.h"
#include "Map.h"

#define MAXLEVEL 400

struct SDHP_RECV_MARRY
{
	PBMSG_HEAD h;
	BYTE SubCode;
	char Name[11];
	int CostDivorce;
	int CostForceDivorce;
	int TraceCost;
	BYTE MAP;
	BYTE ProposeX;
	BYTE ProposeY;
	BYTE AcceptX;
	BYTE AcceptY;
	int MarryMaxTime;
};

class Marry
{
public:
	
	void LoadData(char * FilePath);
	void SendInfo(int aIndex);
	bool GetSex(LPOBJ lpObj);
	int GetPos(LPOBJ lpObj);
	void CommandCore(LPOBJ lpObj,BYTE Command);

	int Enabled;
	BYTE MarryMap;
	int MarryMaxTime;
	BYTE ProposeX;
	BYTE ProposeY;
	BYTE AcceptX;
	BYTE AcceptY;
	int CostDivorce;
	int CostForceDivorce;
	int ShopEnabled;
	int CommandAF;
	WORD Maps[MAX_MAP];
	int MapIndex[MAX_MAP];
};

extern Marry gMarry;
