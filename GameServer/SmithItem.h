#include "stdafx.h"
#include "Protocol.h"
#define	MAX_TYPE_ITEMS	12
#define	MAX_SUBTYPE_ITEMS	512

struct SDHP_SENDSVSHOP
{
	PSBMSG_HEAD h;	// C1:01
	BYTE MaxLevel;
	BYTE MaxSkill;
	BYTE MaxLuck;
	BYTE MaxOpt;
	BYTE MaxExc;
	int pLevel;
	int pSkill;
	int pLuck;
	int pOpt;
	int pExc;
	int	pExc1;
	int	pExc2;
	int	pExc3;
	int	pExc4;
	int	pExc5;
	int TotalPrice;
};

struct SDHP_BUYSVSHOP
{
	PSBMSG_HEAD h;	// C1:01
	int	ItemType;
	int ItemIndex;
	BYTE mLevel;
	BYTE mSkill;
	BYTE mLuck;
	BYTE mOpt;
	BYTE mExc;
	BYTE ExcOption;
	BYTE ExcOption1;
	BYTE ExcOption2;
	BYTE ExcOption3;
	BYTE ExcOption4;
	BYTE ExcOption5;
	BYTE Days;
};

struct SHOPITEM
{
	int Price_1Day;
	int Price_7Days;
	int Price_30Days;
	bool Enabled;
};


class SCFVipShop
{
public:
	void Init(char * Ini, char * FilePath);
	void SendInfo(int aIndex);
	void RecvBuy(int aIndex,SDHP_BUYSVSHOP * lpMsg);
//Vars:
	SHOPITEM item[MAX_TYPE_ITEMS*MAX_SUBTYPE_ITEMS];
	BOOL Enabled;
	BYTE IsForeverFFFE;
	int TypeCount;

private:
	void Read(char * FilePath);
//Vars:
	BYTE MaxLevel;
	BYTE MaxSkill;
	BYTE MaxLuck;
	BYTE MaxOpt;
	BYTE MaxExc;

	int pLevel;
	int pSkill;
	int pLuck;
	int pOpt;
	int pExc;
	int	pExc1;
	int	pExc2;
	int	pExc3;
	int	pExc4;
	int	pExc5;
	int TotalPrice;
};

extern SCFVipShop SVShop;
