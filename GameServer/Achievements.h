#pragma once

//#include "Protocol.h"

#define ACHIEVEMENTS_COUNT 50

struct PSBMSG_HEAD1
{
	void set(BYTE head,BYTE subh,BYTE size) // OK
	{
		this->type = 0xC1;
		this->size = size;
		this->head = head;
		this->subh = subh;
	}

	void setE(BYTE head,BYTE subh,BYTE size) // OK
	{
		this->type = 0xC3;
		this->size = size;
		this->head = head;
		this->subh = subh;
	}

	BYTE type;
	BYTE size;
	BYTE head;
	BYTE subh;
};

struct PSWMSG_HEAD1
{
	void set(BYTE head,BYTE subh,WORD size) // OK
	{
		this->type = 0xC2;
		this->size[0] = HIBYTE(size);
		this->size[1] = LOBYTE(size);
		this->head = head;
		this->subh = subh;
	}

	void setE(BYTE head,BYTE subh,WORD size) // OK
	{
		this->type = 0xC4;
		this->size[0] = HIBYTE(size);
		this->size[1] = LOBYTE(size);
		this->head = head;
		this->subh = subh;
	}

	BYTE type;
	BYTE size[2];
	BYTE head;
	BYTE subh;
};

// ----------------------------------------------------------------------------------------------

struct ACHIEVEMENTS_INFO
{
	int Index;
	int Count;
	BYTE RewardType;
	int RewardValue;
	char TextName[64];
};

struct ACH_SETTING_USER_DATA
{
	unsigned int counter;
	char Name[64];
	BYTE RewardType;
	int RewardValue;
};

// ----------------------------------------------------------------------------------------------
// User
struct ACHIEVEMENTS_USER_DATA
{
	bool bLoaded;
	int InGameTime;
	int InOfflineAttackTime;
	int CSSwitchTime;
	//ACHIEVEMENTS_DATA mission;
	unsigned int counter[50];
};
// ----------------------------------------------------------------------------------------------
// GameServer <-> Client
struct PMSG_GC_ACH_SETTINGS_DATA
{
	PSWMSG_HEAD1 h;
	//ACHIEVEMENTS_DATA cfg;
	//unsigned int counter[50];
	ACH_SETTING_USER_DATA Data[50];
};

struct PMSG_GC_ACH_PLAYER_DATA
{
	PSBMSG_HEAD1 h;
	bool bWinOpen;
	//ACHIEVEMENTS_DATA usr;
	unsigned int counter[50];
};

// ----------------------------------------------------------------------------------------------
// GameServer <-> DataServer
struct PMSG_GDREQ_ACH_LOAD
{
	PSBMSG_HEAD1 h;
	int aIndex;
	char szName[11];
};

struct PMSG_DGANS_ACH_LOAD
{
	PSBMSG_HEAD1 h;
	int aIndex;
	bool bResult;
	//ACHIEVEMENTS_DATA usr;
	unsigned int counter[50];
};

struct PMSG_GDREQ_ACH_SAVE
{
	PSBMSG_HEAD1 h;
	char szName[11];
	//ACHIEVEMENTS_DATA usr;
	unsigned int counter[50];
};
// ----------------------------------------------------------------------------------------------

struct PMSG_CG_ACH_WINOPEN
{
	PSBMSG_HEAD1 h;
};
// ----------------------------------------------------------------------------------------------

struct PMSG_GC_ACH_POWER
{
	PSBMSG_HEAD1 h;
	int iDamage;
	int iDamageMax;
	int iDefence;
	int iDefenceMax;
	int iLife;
	int iLifeMax;
	int iExellentDamage;
	int iExellentDamageMax;
	int iCriticalDamage;
	int iCriticalDamageMax;
};
// ----------------------------------------------------------------------------------------------

class CAchievements
{
public:
		 CAchievements();
		 ~CAchievements();

	void Init();
	void Load();
	void Read(char* path);
	void GDPlayerLoad(int aIndex);
	void DGPlayerLoad(PMSG_DGANS_ACH_LOAD* aRecv);
	void GDPlayerSave(int aIndex);
	void GCSettingsData(int aIndex);
	void GCPlayerData(int aIndex, bool bWin);
	void CGWindowOpen(PMSG_CG_ACH_WINOPEN* aRecv, int aIndex);
	//void PlayerReward(int aIndex, int mission_need, int mission, int ireward);
	void PlayerReward(int aIndex, int missionIndex);

	void MonsterKill(int aIndex, int aMonsterIndex);
	void PlayerKill(int aIndex, int aTargetIndex);
	void PickUp(int aIndex, int iItemNum, int iLevel, int iExl, int iAnc, int iDur);
	void UsedJewel(int aIndex);
	void TimeInGame(int aIndex);
	void SuccessChaosMix(int aIndex);
	void FailedChaosMix(int aIndex);
	void LevelUp(int aIndex);
	void ResetUp(int aIndex);
	void GrandResetUp(int aIndex);
	void MootopVoteUp(int aIndex);
	void TeleportInCC(int aIndex);
	void WinCastleSiege(char* szGuildName);
	void WinCastleSiegeNew(char* szGuildName);
	void BuyDonateShopItem(int aIndex, int iCost);
	void GuildWarWin(int aIndex);
	void DuelWin(int aIndex);
	void CompleteBC(int aIndex);
	void WinCC(int aIndex);
	void PickUpZen(int aIndex, int money);
	bool GetItemDrop(int aIndex, int ItemNumber);

	void CalculatorPower(int aIndex);

	int m_DivDamage;
	int m_DivDefence;
	int m_DivLife;
	int m_DivExellentPercent;
	int m_DivCriticalPercent;

private:
	bool bEnable;
	//ACHIEVEMENTS_DATA m_config;
	//ACHIEVEMENTS_DATA m_reward;

	//std::map<int,ACHIEVEMENTS_INFO> m_AchInfo;
	std::vector<ACHIEVEMENTS_INFO> m_AchInfo;

	int iDamageMax;
	int iDefenceMax;
	int iLifeMax;
	int iExellentDamageMax;
	int iCriticalDamageMax;
};
extern CAchievements  gAchievements;
// ----------------------------------------------------------------------------------------------