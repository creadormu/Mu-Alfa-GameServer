#pragma once
#include "User.h"
#define MAX_CLASS 7

struct REWARD_ITEM_INFO
{
	int Reset;
	int MReset;
	int UserType[4];
	//--
	int ItemIndex;
	int ItemLevel;
	int ItemDurability;
	int ItemSkill;
	int ItemLuck;
	int ItemOption;
	int ItemExcellent;
	int ItemAncient;
	int ItemJOH;
	int ItemOpEx;
	int ItemSocket1;
	int ItemSocket2;
	int ItemSocket3;
	int ItemSocket4;
	int ItemSocket5;
};

struct REWARD_INFO
{
	std::vector< REWARD_ITEM_INFO > ItemStart;
};

class cRewardSystem
{

public:
	cRewardSystem();
	virtual ~cRewardSystem();
	void Load(char* path);
	bool GetRewardByReset( LPOBJ lpObj );
	bool GetRewardByMReset( LPOBJ lpObj );
public:
	REWARD_INFO m_RewardItemInfo[MAX_CLASS];
};

extern cRewardSystem gRewardSystem;