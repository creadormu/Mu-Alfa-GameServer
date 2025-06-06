#pragma once
#include "User.h"

struct DROP_REWARD_INFO
{
	int Index;
	int ItemIndex;
	int DropZen;
	int DropRate;
	int Message;
	int Fireworks;
};

struct DROP_REWARD_STATS
{
	int Index;
	int RuudMin;
	int RuudMax;
	int WCoinCMin;
	int WCoinCMax;
	int WCoinPMin;
	int WCoinPMax;
	int GoblinPMin;
	int GoblinPMax;
};
class cDropReward
{

public:
	cDropReward();
	virtual ~cDropReward();
	void load(char* path);
	bool DropRewardByItemIndex(int ItemIndex, LPOBJ lpObj);

public:
	std::map<int, DROP_REWARD_INFO> m_CustomItemDropInfo;
	std::map<int, DROP_REWARD_STATS> m_CustomItemDropReward;

};extern cDropReward gDropReward;