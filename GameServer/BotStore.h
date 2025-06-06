#pragma once

#include "user.h"
#include "stdafx.h"
#include "Protocol.h"

#define MAX_BOTSTORE	100

struct BotStoreBodyItems
{
	int num;
	int level;
	int opt;
	bool Enabled;
};

struct BotStoreItems
{
	int ValueZen;
	int ValueSoul;
	int ValueBless;
	int ValueChaos;
	int PCPoints;
	WORD num;
	short Level;
	BYTE Opt;
	BYTE Luck;
	BYTE Skill;
	float Dur;
	BYTE Exc;
	BYTE Anc;
	BYTE Sock[5];
};

struct botStoreStruct
{
	int index;
	int Class;
	int OnlyVip;
	int UseVipMoney;
	char Name[11];
	char StoreName[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	bool Enabled;
	int PCPoints;
	int ColorName;
	BotStoreBodyItems body[9];
	BotStoreItems storeItem[INVENTORY_SIZE-MAIN_INVENTORY_NORMAL_SIZE];
	BYTE ItemCount;
};

class ObjBotStore
{
public:
	bool Enabled;
	void Read(char * FilePath);
	void MakeBot();
	void AddItem(int bIndex,int botNum);
	void gObjTempInventoryItemBoxSet(BYTE * TempMap, int itempos, int xl, int yl, BYTE set_byte);
	BYTE CheckSpace(LPOBJ lpObj, int type, BYTE * TempMap);
	BYTE gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck);
	botStoreStruct bot[MAX_BOTSTORE];
};
extern ObjBotStore BotStore;
