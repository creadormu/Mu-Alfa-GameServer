
#pragma once

#define MAX_BOTTRADER	100

struct MixesItems
{
	int Type;
	BYTE LevelMin;
	BYTE LevelMax;
	BYTE OptionMin;
	BYTE OptionMax;
	BYTE Luck;
	BYTE Skill;
	BYTE Exc;
	BYTE Dur;
};

struct BotTraderBodyItems
{
	int num;
	int level;
	int opt;
	bool Enabled;
};

struct Mixes
{
	int Index;
	char Name[256];
	int SuccessRate;
	MixesItems i_Need[64];	
	MixesItems i_Success[64];
	int i_NeedCount;
	int i_SuccessCount;
};

struct botTraderStruct
{
	int index;
	BYTE Class;
	BOOL OnlyVip;
	char Name[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	int ActiveGensFamily;
	int ReqContribution;
	int GensFamily;
	bool Enabled;
	int ChangeColorName;
	int Money;
	int PCPoint;
	BotTraderBodyItems body[9];
	int MixesIndex[50];
	int MixesCount;
};

class ObjBotTrader
{
public:
	void Read(char * FilePath);
	void MakeBot();
	int GetBotIndex(int aIndex);
	BOOL IsInTrade(int aIndex);
	BOOL TradeOpen(int index, int nindex);
	void TradeOk(int aIndex);
	void TradeCancel(int aIndex);
	BYTE gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck);
private:
	bool Enabled;
	BYTE TradeMix(int aIndex,int BotNum);
	bool SearchSameIDNumber(int mIndex, int To);
	botTraderStruct bot[MAX_BOTTRADER];
	Mixes Mix[1024];
	int MixCount;
	int MixNumber(int index);
	int ItemCount(int aIndex);
};

extern ObjBotTrader BotTrader;

