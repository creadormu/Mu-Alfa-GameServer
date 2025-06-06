#include "stdafx.h"
#include "Achievements.h"
#include "Path.h"
#include "GameMain.h"
#include "ObjectManager.h"
#include "Util.h"
#include "Map.h"
#include "CastleSiege.h"
#include "Party.h"
#include "Guild.h"
#include "Union.h"
#include "CashShop.h"
#include "Notice.h"
#include "DSProtocol.h"
#include "Log.h"
#include "GuildClass.h"

CAchievements  gAchievements;

CAchievements::CAchievements()
{
	this->Init();
}

CAchievements::~CAchievements()
{
}

void CAchievements::Init()
{
	this->bEnable = false;
}

void CAchievements::Load()
{

	this->Init();
	this->Read(gPath.GetFullPath("Custom\\Achievements.xml"));

	for(int aIndex = OBJECT_START_USER; aIndex < MAX_OBJECT; aIndex++)
	{
		const LPOBJ lpUser = &gObj[aIndex];
		if(lpUser->Connected == OBJECT_PLAYING && lpUser->Type == OBJECT_USER)
		{
			this->GCSettingsData(aIndex);
		}
	}
}

void CAchievements::Read(char* path)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(path);

	if(res.status != pugi::status_ok)
	{
		ErrorMessageBox("%s file load fail (%s)", path, res.description());
	}

	this->m_AchInfo.clear();

	auto Achievements = file.child("achievements");

	this->bEnable = Achievements.attribute("enable").as_bool(false);

	auto mission = Achievements.child("mission");
	for(auto item = mission.child("info"); item; item=item.next_sibling())
	{
		ACHIEVEMENTS_INFO info;
		info.Index = item.attribute("index").as_int(0);
		info.Count = item.attribute("count").as_int(0);
		info.RewardType = item.attribute("rewardType").as_int(0);
		info.RewardValue = item.attribute("rewardValue").as_int(0);
		strcpy_s(info.TextName,item.attribute("Name").as_string());

		this->m_AchInfo.emplace_back(info);
	}

	auto Power = Achievements.child("power");
	this->iDamageMax = Power.child("damageMax").text().as_int();
	this->iDefenceMax = Power.child("defenseMax").text().as_int();
	this->iLifeMax = Power.child("lifeMax").text().as_int();
	this->iExellentDamageMax = Power.child("excellentDamageMax").text().as_int();
	this->iCriticalDamageMax = Power.child("criticalDamageMax").text().as_int();
	this->m_DivDamage = Power.child("divDamage").text().as_int();
	this->m_DivDefence = Power.child("divDefense").text().as_int();
	this->m_DivLife = Power.child("divLife").text().as_int();
	this->m_DivExellentPercent = Power.child("divExcellentPercent").text().as_int();
	this->m_DivCriticalPercent = Power.child("divCriticalPercent").text().as_int();
}

void CAchievements::GDPlayerLoad(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];
	PMSG_GDREQ_ACH_LOAD pMsg;
	pMsg.h.set(0xFB, 0x00, sizeof(pMsg));
	pMsg.aIndex = aIndex;
	memcpy(pMsg.szName, lpUser->Name, sizeof(pMsg.szName));
	gDataServerConnection.DataSend((BYTE*)&pMsg, sizeof(pMsg));
}

void CAchievements::DGPlayerLoad(PMSG_DGANS_ACH_LOAD* aRecv)
{
	int aIndex = aRecv->aIndex;

	if(!OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	memcpy(&lpUser->ach.counter, &aRecv->counter, sizeof(aRecv->counter));

	lpUser->ach.bLoaded = true;

	this->GCPlayerData(aIndex, false);

	gObjectManager.CharacterCalcAttribute(lpUser->Index);

	GCNewCharacterInfoSend(lpUser);

	GDCharacterInfoSaveSend(lpUser->Index);
	//gObjCalCharacter(aIndex);
}

void CAchievements::GDPlayerSave(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!OBJECT_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	PMSG_GDREQ_ACH_SAVE pMsg;
	pMsg.h.set(0xFB, 0x01, sizeof(pMsg));
	pMsg.szName[10] = 0;
	memcpy(pMsg.szName, lpUser->Name, sizeof(pMsg.szName)-1);
	memcpy(&pMsg.counter, &lpUser->ach.counter, sizeof(pMsg.counter));
	//cDBSMng.Send((char*)&pMsg, sizeof(pMsg));
	gDataServerConnection.DataSend((BYTE*)&pMsg, sizeof(pMsg));
}

void CAchievements::GCSettingsData(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!OBJECT_RANGE(aIndex))
	{
		return;
	}

	PMSG_GC_ACH_SETTINGS_DATA pMsg;
	pMsg.h.set(0xFB, 0x0F, sizeof(pMsg));

	for(int i=0;i<ACHIEVEMENTS_COUNT;i++)
	{
		pMsg.Data[i].counter = this->m_AchInfo.at(i).Count;
		strcpy_s(pMsg.Data[i].Name,this->m_AchInfo.at(i).TextName);
		pMsg.Data[i].RewardType = this->m_AchInfo.at(i).RewardType;
		pMsg.Data[i].RewardValue = this->m_AchInfo.at(i).RewardValue;

	}
	
	DataSend(aIndex, (LPBYTE)&pMsg, sizeof(pMsg));

	this->GDPlayerLoad(aIndex);
}

void CAchievements::GCPlayerData(int aIndex, bool bWin)
{
	if(!OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	PMSG_GC_ACH_PLAYER_DATA pMsg;
	pMsg.h.set(0xFB, 0x0C, sizeof(pMsg));
	pMsg.bWinOpen = bWin;
	memcpy(&pMsg.counter, &lpUser->ach.counter, sizeof(pMsg.counter));
	DataSend(aIndex, (LPBYTE)&pMsg, sizeof(pMsg));
}

void CAchievements::CGWindowOpen(PMSG_CG_ACH_WINOPEN* aRecv, int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	this->GCPlayerData(aIndex, true);
}

void CAchievements::PlayerReward(int aIndex, int missionIndex)
{
	if(!OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->ach.counter[missionIndex] >= this->m_AchInfo.at(missionIndex).Count)
	{
		if (this->m_AchInfo.at(missionIndex).RewardType == 0) // Zen
		{
			lpObj->Money += this->m_AchInfo.at(missionIndex).RewardValue;
			GCMoneySend(lpObj->Index, lpObj->Money);
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,"[Achievements]: +%d Zen",this->m_AchInfo.at(missionIndex).RewardValue);
		}
		else if (this->m_AchInfo.at(missionIndex).RewardType == 1) // WCoinC
		{
			gCashShop.GDCashShopAddPointSaveSend(lpObj->Index, 0, this->m_AchInfo.at(missionIndex).RewardValue, 0, 0,0);
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,"[Achievements]: +%d WCoinC",this->m_AchInfo.at(missionIndex).RewardValue);
		}
		else if (this->m_AchInfo.at(missionIndex).RewardType == 2) // WCoinP
		{
			gCashShop.GDCashShopAddPointSaveSend(lpObj->Index, 0, 0, this->m_AchInfo.at(missionIndex).RewardValue, 0,0);
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,"[Achievements]: +%d WCoinP",this->m_AchInfo.at(missionIndex).RewardValue);
		}
		else if (this->m_AchInfo.at(missionIndex).RewardType == 3) // WCoinG
		{
			gCashShop.GDCashShopAddPointSaveSend(lpObj->Index, 0, 0, 0, this->m_AchInfo.at(missionIndex).RewardValue,0);
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,"[Achievements]: +%d Goblin Points",this->m_AchInfo.at(missionIndex).RewardValue);
		}
		//else if (this->m_AchInfo.at(missionIndex).RewardType == 4) // Credits
		//{
		//	/*lpObj->BankEx.Credits += this->m_AchInfo.at(missionIndex).RewardValue;
		//	gBankEx.GCUpdateBankEx(lpObj->Index);
		//	gBankEx.GDSavePoint(lpObj->Index);
		//	gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,"[Achievements]: +%d Credits",this->m_AchInfo.at(missionIndex).RewardValue);*/
		//}
		else if (this->m_AchInfo.at(missionIndex).RewardType == 5) // SuperPoints
		{
			//lpObj->freePoints+=this->m_AchInfo.at(missionIndex).RewardValue;
			lpObj->LevelUpPoint+=this->m_AchInfo.at(missionIndex).RewardValue;

			gObjectManager.CharacterCalcAttribute(lpObj->Index);

			GCNewCharacterInfoSend(lpObj);

			GDCharacterInfoSaveSend(lpObj->Index);

			//gLog.Output(LOG_SUPER_POINTS,"[SuperPoints]: [%s] [%s] Obtained %d (total %d) points for achievement (id: %d)",lpObj->Account,lpObj->Name, this->m_AchInfo.at(missionIndex).RewardValue,lpObj->iQuestStat,missionIndex);
		}
	}
}

void CAchievements::MonsterKill(int aIndex, int aMonsterIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex) || !gObjIsConnected(aMonsterIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];
	LPOBJ lpMonster = &gObj[aMonsterIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i1_MonsterKill
	if(this->m_AchInfo.at(0).Count > lpUser->ach.counter[0])
	{
		lpUser->ach.counter[0]++;
		this->PlayerReward(aIndex, 0);

	}
	//i2_GoldenKill
	if(this->m_AchInfo.at(1).Count > lpUser->ach.counter[1])
	{
		if( lpMonster->Class == 78  || lpMonster->Class == 53  || lpMonster->Class == 79  || lpMonster->Class == 80  || lpMonster->Class == 82  || 
			lpMonster->Class == 502 || lpMonster->Class == 493 || lpMonster->Class == 494 || lpMonster->Class == 495 || lpMonster->Class == 496 || 
			lpMonster->Class == 497 || lpMonster->Class == 498 || lpMonster->Class == 499 || lpMonster->Class == 500 || lpMonster->Class == 501 ||
			lpMonster->Class == 43	||	//GoldenBudgeDragon
			lpMonster->Class == 44	||	//Dragon
			lpMonster->Class == 54	||	//GoldenBudgeDragon
			lpMonster->Class == 81		//GoldenVepar
			)	
		{
			lpUser->ach.counter[1]++;
			this->PlayerReward(aIndex, 1);
		}
	}
	//i3_KalimaMonserKill
	if(this->m_AchInfo.at(2).Count > lpUser->ach.counter[2])
	{
		if(KALIMA_MAP_RANGE(lpUser->Map))
		{
			lpUser->ach.counter[2]++;
			this->PlayerReward(aIndex, 2);
		}
	}
	//i4_WhiteWizardKill
	if(this->m_AchInfo.at(3).Count > lpUser->ach.counter[3])
	{
		if(lpMonster->Class == 135)
		{
			lpUser->ach.counter[3]++;
			this->PlayerReward(aIndex, 3);
		}
	}
	//i6_RedDragonKill
	if(this->m_AchInfo.at(5).Count > lpUser->ach.counter[5])
	{
		if(lpMonster->Class == 42)
		{
			lpUser->ach.counter[5]++;
			this->PlayerReward(aIndex, 5);
		}
	}
	//i7_OrcKill
	if(this->m_AchInfo.at(6).Count > lpUser->ach.counter[6])
	{
		if(lpMonster->Class == 136 || lpMonster->Class == 137)
		{
			lpUser->ach.counter[6]++;
			this->PlayerReward(aIndex, 6);
		}
	}
	//i8_KillGateBC
	if(this->m_AchInfo.at(7).Count > lpUser->ach.counter[7])
	{
		if(lpMonster->Class == 131)
		{
			lpUser->ach.counter[7]++;
			this->PlayerReward(aIndex, 7);
		}
	}
	//i22_KanturuBossKill
	if(this->m_AchInfo.at(21).Count > lpUser->ach.counter[21])
	{
		if(lpMonster->Class == 361 || lpMonster->Class == 362 || lpMonster->Class == 363)
		{
			lpUser->ach.counter[21]++;
			this->PlayerReward(aIndex, 21);
		}
	}
	//i23_Monster100LvlKill
	if(this->m_AchInfo.at(22).Count > lpUser->ach.counter[22])
	{
		if(lpMonster->Level >= 100)
		{
			lpUser->ach.counter[22]++;
			this->PlayerReward(aIndex, 22);
		}
	}
	//i26_CryWolfEventMonster
	if(this->m_AchInfo.at(25).Count > lpUser->ach.counter[25])
	{
		if(lpMonster->Class == 349 || lpMonster->Class == 340)
		{
			lpUser->ach.counter[25]++;
			this->PlayerReward(aIndex, 25);
		}
	}
	//i27_HellMainKill
	if(this->m_AchInfo.at(26).Count > lpUser->ach.counter[26])
	{
		if(lpMonster->Class == 309)
		{
			lpUser->ach.counter[26]++;
			this->PlayerReward(aIndex, 26);
		}
	}
	//i29_SelupanKill
	if(this->m_AchInfo.at(28).Count > lpUser->ach.counter[28])
	{
		if(lpMonster->Class == 459)
		{
			lpUser->ach.counter[28]++;
			this->PlayerReward(aIndex, 28);
		}
	}
	//i30_KundunKill
	if(this->m_AchInfo.at(29).Count > lpUser->ach.counter[29])
	{
		if(lpMonster->Class == 275)
		{
			lpUser->ach.counter[29]++;
			this->PlayerReward(aIndex, 29);
		}
	}
	//i31_KillStatueBC
	if(this->m_AchInfo.at(30).Count > lpUser->ach.counter[30])
	{
		//if(lpMonster->Class == 131)
		if(lpMonster->Class == 132 || lpMonster->Class == 133 || lpMonster->Class == 134 )
		{
			lpUser->ach.counter[30]++;
			this->PlayerReward(aIndex, 30);
		}
	}
	//i34_BlueRabbitKiller
	if(this->m_AchInfo.at(33).Count > lpUser->ach.counter[33])
	{
		if(lpMonster->Class == 413)
		{
			lpUser->ach.counter[33]++;
			this->PlayerReward(aIndex, 33);
		}
	}
	//i35_HappyPouchesKiller
	if(this->m_AchInfo.at(34).Count > lpUser->ach.counter[34])
	{
		if(lpMonster->Class == 365)
		{
			lpUser->ach.counter[34]++;
			this->PlayerReward(aIndex, 34);
		}
	}
	//i36_PheonixKill
	if(this->m_AchInfo.at(35).Count > lpUser->ach.counter[35])
	{
		if(lpMonster->Class == 77)
		{
			lpUser->ach.counter[35]++;
			this->PlayerReward(aIndex, 35);
		}
	}
	//i37_KanturuMonsterKill
	if(this->m_AchInfo.at(36).Count > lpUser->ach.counter[36])
	{
		if(lpMonster->Map == MAP_KANTURU3)
		{
			lpUser->ach.counter[36]++;
			this->PlayerReward(aIndex, 36);
		}
	}
	//i42_KillCastleSiegeNPC
	if(this->m_AchInfo.at(41).Count > lpUser->ach.counter[41])
	{
		if(lpMonster->Class == 283 || lpMonster->Class == 277 || lpMonster->Class == 288 || lpMonster->Class == 222 || lpMonster->Class == 221)
		{
			lpUser->ach.counter[41]++;
			this->PlayerReward(aIndex, 41);
		}
	}
	//i44_KillErohim
	if(this->m_AchInfo.at(43).Count > lpUser->ach.counter[43])
	{
		if(lpMonster->Class == 295)
		{
			lpUser->ach.counter[43]++;
			this->PlayerReward(aIndex, 43);
		}
	}

	if(this->m_AchInfo.at(47).Count > lpUser->ach.counter[47])
	{
		if(lpMonster->Class == 589)
		{
			lpUser->ach.counter[47]++;
			this->PlayerReward(aIndex, 47);
		}
	}

	if(this->m_AchInfo.at(48).Count > lpUser->ach.counter[48])
	{
		if(lpMonster->Class == 598)
		{
			lpUser->ach.counter[48]++;
			this->PlayerReward(aIndex, 48);
		}
	}
}

void CAchievements::PlayerKill(int aIndex, int aTargetIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex) || !gObjIsConnected(aTargetIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];
	LPOBJ lpTarget = &gObj[aTargetIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i5_PlayerKill
	if(this->m_AchInfo.at(4).Count > lpUser->ach.counter[4])
	{
		lpUser->ach.counter[4]++;
		this->PlayerReward(aIndex, 4);
	}
	//i25_PhonomanKill
	if(this->m_AchInfo.at(24).Count > lpUser->ach.counter[24])
	{
		if(lpTarget->PKLevel >= 6)
		{
			lpUser->ach.counter[24]++;
			this->PlayerReward(aIndex, 24);
		}
	}
	//i28_KillInSelf_Defense
	if(this->m_AchInfo.at(27).Count > lpUser->ach.counter[27])
	{
		for(int n = 0; n < MAX_SELF_DEFENSE;n++)
		{
			if(lpUser->SelfDefense[n] >= 0)
			{
				if(lpUser->SelfDefense[n] == aTargetIndex)
				{
					lpUser->ach.counter[27]++;
					this->PlayerReward(aIndex, 27);
				}
			}
		}
	}
#if(GAMESERVER_TYPE==1)
	//i38_MovePlayerFromSwitch
	if(this->m_AchInfo.at(37).Count > lpUser->ach.counter[37])
	{
		int iCrownIndex1 = gCastleSiege.GetCrownSwitchUserIndex(217);
		int iCrownIndex2 = gCastleSiege.GetCrownSwitchUserIndex(218);

		if(gObjIsConnected(iCrownIndex1))
		{
			if(aTargetIndex == iCrownIndex1)
			{
				lpUser->ach.counter[37]++;
				this->PlayerReward(aIndex, 37);
			}
		}
		else if(gObjIsConnected(iCrownIndex2))
		{
			if(aTargetIndex == iCrownIndex2)
			{
				lpUser->ach.counter[37]++;
				this->PlayerReward(aIndex, 37);
			}
		}
	}	
	//i40_MovePlayerFromCrownkill
	if(this->m_AchInfo.at(39).Count > lpUser->ach.counter[39])
	{
		int iCrownIndex = gCastleSiege.GetCrownUserIndex();
		if(gObjIsConnected(iCrownIndex))
		{
			if(iCrownIndex == aTargetIndex)
			{
				lpUser->ach.counter[39]++;
				this->PlayerReward(aIndex, 39);
			}
		}
	}
	//i43_KillPlayerCastleSiege
	if(this->m_AchInfo.at(42).Count > lpUser->ach.counter[42])
	{
		if(lpUser->Map == MAP_CASTLE_SIEGE)
		{
			if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
			{
				lpUser->ach.counter[42]++;
				this->PlayerReward(aIndex, 42);
			}
		}
	}
#endif
}

void CAchievements::PickUp(int aIndex, int iItemNum, int iLevel, int iExl, int iAnc, int iDur)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i12_PickUpJewels
	if(this->m_AchInfo.at(11).Count > lpUser->ach.counter[11])
	{
		if( iItemNum == GET_ITEM(12, 15) ||
			iItemNum == GET_ITEM(14, 13) ||
			iItemNum == GET_ITEM(14, 14) ||
			iItemNum == GET_ITEM(14, 16) ||
			iItemNum == GET_ITEM(14, 22) ||
			iItemNum == GET_ITEM(14, 31) ||
			iItemNum == GET_ITEM(14, 42))
		{
			lpUser->ach.counter[11]++;
			this->PlayerReward(aIndex, 11);
		}
	}
	//i16_PickUpZen
	/*if(this->m_AchInfo.at(15).Count > lpUser->ach.counter[15])
	{
		if(iItemNum == GET_ITEM(14,15))
		{
			lpUser->ach.counter[15]++;
			this->PlayerReward(aIndex, 15);
		}
	}*/
	//i17_PickUpExellent
	if(this->m_AchInfo.at(16).Count > lpUser->ach.counter[16])
	{
		if(iExl > 0)
		{
			lpUser->ach.counter[16]++;
			this->PlayerReward(aIndex, 16);
		}
	}
	//i18_PickUpAncent
	if(this->m_AchInfo.at(17).Count > lpUser->ach.counter[17])
	{
		if(iAnc > 0)
		{
			lpUser->ach.counter[17]++;
			this->PlayerReward(aIndex, 17);
		}
	}
	//i48_PickUpSing
	//if(this->m_AchInfo.at(47).Count > lpUser->ach.counter[47])
	//{
	//	if(iItemNum == GET_ITEM(14,21) && iLevel == 0)
	//	{
	//		lpUser->ach.counter[47]++;
	//		this->PlayerReward(aIndex, 47);
	//	}
	//}
	////i49_PickUpLuckyCoins Sing of lord
	//if(this->m_AchInfo.at(48).Count > lpUser->ach.counter[48])
	//{
	//	//if(iItemNum == GET_ITEM(14,100))
	//	if(iItemNum == GET_ITEM(14,21) && iLevel == 3)
	//	{
	//		//lpUser->ach.mission.i49_PickUpLuckyCoins++;
	//		lpUser->ach.counter[48] += iDur;

	//		if(lpUser->ach.counter[48] > this->m_AchInfo.at(48).Count)
	//		{
	//			lpUser->ach.counter[48] = this->m_AchInfo.at(48).Count;
	//		}

	//		this->PlayerReward(aIndex, 48);
	//	}
	//}
}

void CAchievements::UsedJewel(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i13_UsedJewels
	if(this->m_AchInfo.at(12).Count > lpUser->ach.counter[12])
	{
		lpUser->ach.counter[12]++;
		this->PlayerReward(aIndex, 12);
	}
}

void CAchievements::TimeInGame(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i11_OnlineTime
	if(this->m_AchInfo.at(10).Count > lpUser->ach.counter[10])
	{
		lpUser->ach.InGameTime++;
		if(lpUser->ach.InGameTime >= 3600)
		{
			lpUser->ach.counter[10]++;
			this->PlayerReward(aIndex, 10);
			lpUser->ach.InGameTime = 0;
		}
	}
#if(GAMESERVER_TYPE==1)
	//i39_WithstandSwitch10min
	if(this->m_AchInfo.at(38).Count > lpUser->ach.counter[38])
	{
		int iCrownIndex1 = gCastleSiege.GetCrownSwitchUserIndex(217);
		int iCrownIndex2 = gCastleSiege.GetCrownSwitchUserIndex(218);

		if(gObjIsConnected(iCrownIndex1) || gObjIsConnected(iCrownIndex2))
		{
			if(aIndex == iCrownIndex1 || aIndex == iCrownIndex2)
			{
				lpUser->ach.CSSwitchTime++;
				if(lpUser->ach.CSSwitchTime >= 60)
				{
					lpUser->ach.counter[38]++;
					this->PlayerReward(aIndex, 38);
					lpUser->ach.CSSwitchTime = 0;
				}
			}
		}
	}
#endif
	//i50_OfflineAttackTime
	if(this->m_AchInfo.at(49).Count > lpUser->ach.counter[49])
	{
		if(lpUser->AttackCustom)
		{
			lpUser->ach.InOfflineAttackTime++;
			if(lpUser->ach.InOfflineAttackTime >= 3600)
			{
				lpUser->ach.counter[49]++;
				this->PlayerReward(aIndex, 49);
				lpUser->ach.InOfflineAttackTime = 0;
			}
		}
	}
}

void CAchievements::SuccessChaosMix(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i14_SuccessChaosMix
	if(this->m_AchInfo.at(13).Count > lpUser->ach.counter[13])
	{
		lpUser->ach.counter[13]++;
		this->PlayerReward(aIndex, 13);
	}
}

void CAchievements::FailedChaosMix(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i15_FailedChaosMix
	if(this->m_AchInfo.at(14).Count > lpUser->ach.counter[14])
	{
		lpUser->ach.counter[14]++;
		this->PlayerReward(aIndex, 14);
	}
}

void CAchievements::LevelUp(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i20_LevelUp
	if(this->m_AchInfo.at(19).Count > lpUser->ach.counter[19])
	{
		lpUser->ach.counter[19]++;
		this->PlayerReward(aIndex, 19);
	}
	//i47_PartyNoobLevelUp
	if(this->m_AchInfo.at(46).Count > lpUser->ach.counter[46])
	{
		if(lpUser->PartyNumber >= 0)
		{
			const int iPartyNumber = lpUser->PartyNumber;
			for(int i = 0; i < MAX_PARTY_USER; i++ )
			{
				const int aUserIndex = gParty.m_PartyInfo[iPartyNumber].Index[i];
				if (gObjIsConnected(aUserIndex))
				{
					const LPOBJ PartyUser = &gObj[aUserIndex];
					if(lpUser->Level > (PartyUser->Level+100))
					{
						lpUser->ach.counter[46]++;
						this->PlayerReward(aIndex, 46);
						break;
					}		
				}
			}
		}
	}
}

void CAchievements::ResetUp(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i21_ResetUp
	if(this->m_AchInfo.at(20).Count > lpUser->ach.counter[20])
	{
		lpUser->ach.counter[20]++;
		this->PlayerReward(aIndex, 20);
	}
}

void CAchievements::GrandResetUp(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i33_GrandResetUp
	if(this->m_AchInfo.at(32).Count > lpUser->ach.counter[32])
	{
		lpUser->ach.counter[32]++;
		this->PlayerReward(aIndex, 32);
	}
}

void CAchievements::MootopVoteUp(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i24_MootopVote
	if(this->m_AchInfo.at(23).Count > lpUser->ach.counter[23])
	{
		lpUser->ach.counter[23]++;
		this->PlayerReward(aIndex, 23);
	}
}

void CAchievements::TeleportInCC(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i9_TakePartInCC
	if(this->m_AchInfo.at(8).Count > lpUser->ach.counter[8])
	{
		lpUser->ach.counter[8]++;
		this->PlayerReward(aIndex, 8);
	}
}

void CAchievements::WinCastleSiegeNew(char* szGuildName)
{
	if(!this->bEnable)
	{
		//LogAdd(LOG_TEST,"WinCastleSiegeNew !this->bEnable");
		return;
	}

	GUILD_INFO_STRUCT * lpGuild = gGuildClass.SearchGuild(szGuildName);

	if ( lpGuild != NULL )
	{
		for ( int n=0;n<MAX_GUILD_USER;n++)
		{
			if ( lpGuild->Use[n] > 0 && lpGuild->Index[n] >= 0)
			{
				if (gObj[lpGuild->Index[n]].Connected == OBJECT_PLAYING && gObj[lpGuild->Index[n]].Type == OBJECT_USER)
				{
					//LogAdd(LOG_TEST,"WinCastleSiegeNew Name (%s) Get REWARD!",gObj[lpGuild->Index[n]].Name);
					gObj[lpGuild->Index[n]].ach.counter[40]++;
					this->PlayerReward(lpGuild->Index[n], 40);
				}
			}
		}

		/*CUnionInfo * pUnionInfo = gUnionManager.SearchUnion(lpGuild->GuildUnion);
		pUnionInfo->m_vtUnionMember*/

	}
	else
	{
		//LogAdd(LOG_TEST,"WinCastleSiegeNew lpGuild != NULL");
	}

	//LogAdd(LOG_TEST,"WinCastleSiegeNew DONE");
}

void CAchievements::WinCastleSiege(char* szGuildName)
{
	LogAdd(LOG_BLACK,"WinCastleSiege start (%s)",szGuildName);
	if(!this->bEnable)
	{
		LogAdd(LOG_BLACK,"WinCastleSiege !this->bEnable");
		return;
	}

	if(strlen(szGuildName) < 1)
	{
		LogAdd(LOG_BLACK,"WinCastleSiege strlen(szGuildName) < 1");
		return;
	}

	for(int aIndex = OBJECT_START_USER; aIndex < MAX_OBJECT; aIndex++)
	{
		//LogAdd(LOG_BLACK,"WinCastleSiege iteration: %n",aIndex);
		LPOBJ lpUser = &gObj[aIndex];

		if(gObjIsConnectedGP(aIndex) != 0)
		{
			//LogAdd(LOG_BLACK,"WinCastleSiege Name (%s)",lpUser->Name);
			if(!lpUser->ach.bLoaded)
			{
				LogAdd(LOG_BLACK,"WinCastleSiege Name (%s) !lpUser->ach.bLoaded",lpUser->Name);
				continue;
			}

			GUILD_INFO_STRUCT * lpGuildUser = lpUser->Guild;

			if(!lpGuildUser)
			{
				LogAdd(LOG_BLACK,"WinCastleSiege Name (%s) !lpGuildUser",lpUser->Name);
				continue;
			}

			if(!strcmp(lpGuildUser->Name, szGuildName))
			{
				LogAdd(LOG_BLACK,"WinCastleSiege Name (%s) Get REWARD!",lpUser->Name);
				//i41_WinSiege
				if(this->m_AchInfo.at(40).Count > lpUser->ach.counter[40])
				{
					lpUser->ach.counter[40]++;
					this->PlayerReward(aIndex, 40);
				}
				continue;
			}

			CUnionInfo * pUnionUser = gUnionManager.SearchUnion(lpGuildUser->GuildUnion);

			if(!pUnionUser)
			{
				continue;
			}

			if(!strcmp(pUnionUser->m_szMasterGuild, szGuildName))
			{
				//i41_WinSiege
				if(this->m_AchInfo.at(40).Count > lpUser->ach.counter[40])
				{
					lpUser->ach.counter[40]++;
					this->PlayerReward(aIndex, 40);
				}
			}
		}
		/*else
		{
			LogAdd(LOG_BLACK,"WinCastleSiege iteration: %n gObjIsConnectedGP == 0",aIndex);
		}*/
	}
	LogAdd(LOG_BLACK,"WinCastleSiege end");
}

void CAchievements::BuyDonateShopItem(int aIndex, int iCost)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i45_BuyDonateShopItem
	if(this->m_AchInfo.at(44).Count > lpUser->ach.counter[44])
	{
		lpUser->ach.counter[44] += iCost;
		if(lpUser->ach.counter[44] > this->m_AchInfo.at(44).Count)
		{
			lpUser->ach.counter[44] = this->m_AchInfo.at(44).Count;
			this->PlayerReward(aIndex, 44);
		}
	}
}

void CAchievements::GuildWarWin(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i46_GuildWarWin
	if(this->m_AchInfo.at(45).Count > lpUser->ach.counter[45])
	{
		lpUser->ach.counter[45]++;
		this->PlayerReward(aIndex, 45);
	}
}

void CAchievements::DuelWin(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i19_DuelWin
	if(this->m_AchInfo.at(18).Count > lpUser->ach.counter[18])
	{
		lpUser->ach.counter[18]++;
		this->PlayerReward(aIndex, 18);
	}
}

void CAchievements::CompleteBC(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i32_CompleteBC
	if(this->m_AchInfo.at(31).Count > lpUser->ach.counter[31])
	{
		lpUser->ach.counter[31]++;
		this->PlayerReward(aIndex, 31);
	}
}

void CAchievements::WinCC(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i10_WinningTheCC
	if(this->m_AchInfo.at(9).Count > lpUser->ach.counter[9])
	{
		lpUser->ach.counter[9]++;
		this->PlayerReward(aIndex, 9);
	}
}

void CAchievements::PickUpZen(int aIndex, int money)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	//i16_PickUpZen
	if(this->m_AchInfo.at(15).Count > lpUser->ach.counter[15])
	{
		lpUser->ach.counter[15] += money;
		this->PlayerReward(aIndex, 15);
	}
}

void CAchievements::CalculatorPower(int aIndex)
{
	if(!this->bEnable)
	{
		return;
	}

	if(!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return;
	}

	int iAchCount = 0;

	for	(int i=0; i<ACHIEVEMENTS_COUNT; i++)
	{
		if (lpUser->ach.counter[i] >= this->m_AchInfo.at(i).Count)
		{
			iAchCount++;
		}
	}

	int iAddDamage = 0;
	int iAddDefence = 0;
	int iAddLife = 0;
	int iAddExellentPercent = 0;
	int iAddCriticalPercent = 0;

	iAddDamage = this->m_DivDamage * iAchCount;
	iAddDefence = this->m_DivDefence * iAchCount;
	iAddLife =  this->m_DivLife * iAchCount;

	if(iAchCount > 0)
	{
		iAddExellentPercent = ( iAchCount * this->m_DivExellentPercent ) / 100;
		iAddCriticalPercent = ( iAchCount * this->m_DivCriticalPercent ) / 100;
	}

	if(iAddDamage > 0)
	{
		if(iAddDamage > this->iDamageMax)
		{
			iAddDamage = this->iDamageMax;
		}
		/*lpUser->m_AttackDamageMaxLeft += iAddDamage;
		lpUser->m_AttackDamageMinLeft += iAddDamage;
		lpUser->m_AttackDamageMaxRight += iAddDamage;
		lpUser->m_AttackDamageMinRight += iAddDamage;
		lpUser->m_MagicDamageMin += iAddDamage;
		lpUser->m_MagicDamageMax += iAddDamage;*/
		lpUser->PhysiDamageMinRight += iAddDamage;
		lpUser->PhysiDamageMaxRight += iAddDamage;
		lpUser->PhysiDamageMinLeft += iAddDamage;
		lpUser->PhysiDamageMaxLeft += iAddDamage;

		lpUser->MagicDamageMin += iAddDamage;
		lpUser->MagicDamageMax += iAddDamage;

		lpUser->CurseDamageMin += iAddDamage;
		lpUser->CurseDamageMax += iAddDamage;
	}

	if(iAddDefence > 0)
	{
		if(iAddDefence > this->iDefenceMax)
		{
			iAddDefence = this->iDefenceMax;
		}
		lpUser->Defense += iAddDefence;
		lpUser->MagicDefense += iAddDefence;
	}

	if(iAddLife > 0)
	{
		if(iAddLife > this->iLifeMax)
		{
			iAddLife = this->iLifeMax;
		}
		lpUser->AddLife += iAddLife;
	}

	if(iAddExellentPercent > 0)
	{
		if(iAddExellentPercent > this->iExellentDamageMax)
		{
			iAddExellentPercent = this->iExellentDamageMax;
		}
		lpUser->ExcellentDamageRate += iAddExellentPercent;
	}

	if(iAddCriticalPercent > 0)
	{
		if(iAddCriticalPercent > this->iCriticalDamageMax)
		{
			iAddCriticalPercent = this->iCriticalDamageMax;
		}
		lpUser->CriticalDamageRate += iAddCriticalPercent;
	}

	PMSG_GC_ACH_POWER pMsg;
	pMsg.h.set(0xFB, 0x0D, sizeof(pMsg));

	pMsg.iDamage = iAddDamage;
	pMsg.iDamageMax = this->iDamageMax;
	pMsg.iDefence = iAddDefence;
	pMsg.iDefenceMax = this->iDefenceMax;
	pMsg.iLife = iAddLife;
	pMsg.iLifeMax = this->iLifeMax;
	pMsg.iExellentDamage = iAddExellentPercent;
	pMsg.iExellentDamageMax = this->iExellentDamageMax;
	pMsg.iCriticalDamage = iAddCriticalPercent;
	pMsg.iCriticalDamageMax = this->iCriticalDamageMax;

	DataSend(aIndex, (LPBYTE)&pMsg, sizeof(pMsg));
}

bool CAchievements::GetItemDrop(int aIndex, int ItemNumber)
{
	if(!this->bEnable)
	{
		return true;
	}

	if(!gObjIsConnected(aIndex))
	{
		return false;
	}

	LPOBJ lpUser = &gObj[aIndex];

	if(!lpUser->ach.bLoaded)
	{
		return true;
	}

	//if(this->m_config.i48_PickUpRena > lpUser->ach.mission.i48_PickUpRena || this->m_config.i49_PickUpLuckyCoins > lpUser->ach.mission.i49_PickUpLuckyCoins)
	{
		if(ItemNumber == GET_ITEM(14,21))
		{
			return false;
		}
	}

	return true;
}