#include "stdafx.h"
#include "ChangeClass.h"
#include "Notice.h"
#include "MasterSkillTree.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "DSProtocol.h"
#include "Util.h"
#include "CashShop.h"
#include "Message.h"
#include "ResetTable.h"
#include "PcPoint.h"
#include "ReadScript.h"
#include "Log.h"
#include "Quest.h"

cChangeClass gChangeClass;

cChangeClass::cChangeClass()
{
	this->Init();
}

cChangeClass::~cChangeClass()
{
	
}

void cChangeClass::Init()
{
	for (int i=0; i<MAX_ACCOUNT_LEVEL;i++)
	{
		this->m_Price[i] = 0;
		this->m_PriceType[i] = 0;
	}
}

void cChangeClass::Load(char* path)
{
	SMDToken Token;
	SMDFile = fopen(path, "r");
	// ----
	if( !SMDFile )
	{
		ErrorMessageBox("[ChangeClass.txt] file not found");
		return;
	}

	this->m_ChangeClassSwitch = GetPrivateProfileInt("Common", "ChangeClassActive", 0, path);
	this->m_ChangeClassEnable[0] = GetPrivateProfileInt("Common", "ChangeClassEnable_AL0", 0, path);
	this->m_ChangeClassEnable[1] = GetPrivateProfileInt("Common", "ChangeClassEnable_AL1", 0, path);
	this->m_ChangeClassEnable[2] = GetPrivateProfileInt("Common", "ChangeClassEnable_AL2", 0, path);
	this->m_ChangeClassEnable[3] = GetPrivateProfileInt("Common", "ChangeClassEnable_AL3", 0, path);
	this->m_ChangeClassRequireLevel[0] = GetPrivateProfileInt("Common", "ClassRequireLevel_AL0", 0, path);
	this->m_ChangeClassRequireLevel[1] = GetPrivateProfileInt("Common", "ClassRequireLevel_AL1", 0, path);
	this->m_ChangeClassRequireLevel[2] = GetPrivateProfileInt("Common", "ClassRequireLevel_AL2", 0, path);
	this->m_ChangeClassRequireLevel[3] = GetPrivateProfileInt("Common", "ClassRequireLevel_AL3", 0, path);
	this->m_ChangeClassRequireReset[0] = GetPrivateProfileInt("Common", "ClassRequireReset_AL0", 0, path);
	this->m_ChangeClassRequireReset[1] = GetPrivateProfileInt("Common", "ClassRequireReset_AL1", 0, path);
	this->m_ChangeClassRequireReset[2] = GetPrivateProfileInt("Common", "ClassRequireReset_AL2", 0, path);
	this->m_ChangeClassRequireReset[3] = GetPrivateProfileInt("Common", "ClassRequireReset_AL3", 0, path);
	this->m_PriceType[0] = GetPrivateProfileInt("Common", "ClassRequireType_AL0", 0, path);
	this->m_PriceType[1] = GetPrivateProfileInt("Common", "ClassRequireType_AL1", 0, path);
	this->m_PriceType[2] = GetPrivateProfileInt("Common", "ClassRequireType_AL2", 0, path);
	this->m_PriceType[3] = GetPrivateProfileInt("Common", "ClassRequireType_AL3", 0, path);
	this->m_Price[0] = GetPrivateProfileInt("Common", "ClassRequirePrice_AL0", 0, path);
	this->m_Price[1] = GetPrivateProfileInt("Common", "ClassRequirePrice_AL1", 0, path);
	this->m_Price[2] = GetPrivateProfileInt("Common", "ClassRequirePrice_AL2", 0, path);
	this->m_Price[3] = GetPrivateProfileInt("Common", "ClassRequirePrice_AL3", 0, path);
}

void cChangeClass::SendData(int aIndex)
{
	PMSG_CHANGECLASS_DATA pRequest;
	pRequest.Head.set(0xFB,0x08, sizeof(pRequest));

	pRequest.PriceType = this->m_PriceType[gObj[aIndex].AccountLevel];
	pRequest.Price = this->m_Price[gObj[aIndex].AccountLevel];

	DataSend(aIndex, (LPBYTE)&pRequest, sizeof(pRequest));
}

void cChangeClass::RecvChangeClass(CG_CHANGECLASS_RECV* Data, int aIndex)
{
	gCashShop.GDCashShopRecievePointSend(aIndex,(DWORD)&cChangeClass::ChangeClassCallback,(DWORD)Data->Type,0);
}

void cChangeClass::ChangeClassCallback(LPOBJ lpObj, int Class, DWORD null, DWORD WCoinC, DWORD WCoinP, DWORD GoblinPoint)
{
	if (gChangeClass.m_PriceType[lpObj->AccountLevel] == 0)
	{
		if (lpObj->Money < gChangeClass.m_Price[lpObj->AccountLevel])
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You do not have enough %s!","Zen");
			return;
		}
	}
	else if (gChangeClass.m_PriceType[lpObj->AccountLevel] == 1)
	{
		if (WCoinC < gChangeClass.m_Price[lpObj->AccountLevel])
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You do not have enough %s!","WCoinC");
			return;
		}
	}
	else if (gChangeClass.m_PriceType[lpObj->AccountLevel] == 2)
	{
		if (WCoinP < gChangeClass.m_Price[lpObj->AccountLevel])
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You do not have enough %s!","WCoinP");
			return;
		}
	}
	else if (gChangeClass.m_PriceType[lpObj->AccountLevel] == 3)
	{
		if (GoblinPoint < gChangeClass.m_Price[lpObj->AccountLevel])
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You do not have enough %s!","Goblin Point");
			return;
		}
	}
	else if (gChangeClass.m_PriceType[lpObj->AccountLevel] == 4)
	{
		if (lpObj->PCPoint < gChangeClass.m_Price[lpObj->AccountLevel])
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You do not have enough %s!","PCPoint");
			return;
		}
	}	
	if(lpObj->Interface.use != 0 || lpObj->Teleport != 0 || lpObj->DieRegen != 0 || lpObj->PShopOpen != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You can not use it now.");
		return;
	}
	if(gChangeClass.m_ChangeClassEnable[lpObj->AccountLevel] == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You do not have permission to use");
		return;
	}
	if(lpObj->Level < (gChangeClass.m_ChangeClassRequireLevel[lpObj->AccountLevel]))
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You need %d level to use!",gChangeClass.m_ChangeClassRequireLevel[lpObj->AccountLevel]);
		return;
	}
	if(lpObj->Reset < (gChangeClass.m_ChangeClassRequireReset[lpObj->AccountLevel]))
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You need Reset: %d to use!",gChangeClass.m_ChangeClassRequireReset[lpObj->AccountLevel]);
		return;
	}

	gChangeClass.ChangeClass(lpObj,Class);
}

void cChangeClass::ChangeClass(LPOBJ lpObj, int Class)
{
    if(gChangeClass.m_ChangeClassSwitch == 0)
    {
        return;
    }

	if (lpObj->Interface.type == INTERFACE_CHAOS_BOX || lpObj->Interface.type == INTERFACE_TRADE || lpObj->Interface.type == INTERFACE_PERSONAL_SHOP || lpObj->Interface.use != 0)
	{
		return;
	}

	int NextClass = 0;

	switch(Class)
	{
		case 0:
			if(lpObj->Class == CLASS_DW)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You must use a different class from yours!");
				return;
			}
			NextClass = CLASS_DW;
			break;
		case 16:
			if(lpObj->Class == CLASS_DK)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You must use a different class from yours!");
				return;
			}
			NextClass = CLASS_DK;
			break;
		case 32:
			if(lpObj->Class == CLASS_ELF)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You must use a different class from yours!");
				return;
			}
			NextClass = CLASS_FE;
			break;
		case 48:
			if(lpObj->Class == CLASS_MG)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You must use a different class from yours!");
				return;
			}
			NextClass = CLASS_MG;
			break;
		case 64:
			if(lpObj->Class == CLASS_DL)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You must use a different class from yours!");
				return;
			}
			NextClass = CLASS_DL;
			break;
		case 80:
			if(lpObj->Class == CLASS_SU)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You must use a different class from yours!");
				return;
			}
			NextClass = CLASS_SU;
			break;

		#if(GAMESERVER_UPDATE >= 601)
		case 96:
			if(lpObj->Class == CLASS_RF)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You must use a different class from yours!");
				return;
			}
			NextClass = CLASS_RF;
			break;
		#endif
	}

	for( int i = 0; i < 12; i++ )
	{
		if( lpObj->Inventory[i].IsItem() )
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You must remove all items!");
			return;
		}
	}

	lpObj->DBClass = Class;

	lpObj->Level		= 1;
	lpObj->Experience	= 0;
	lpObj->MasterLevel	= 0;
	lpObj->MasterExperience = 0;
	lpObj->MasterNextExperience = +1;
	lpObj->MasterPoint	= 0;

	gQuest.GCQuestInfoSend(lpObj->Index);
	memset(lpObj->Quest,0xFA,sizeof(lpObj->Quest));
	gQuest.GCQuestRewardSend(lpObj->Index,201,Class);
	
	lpObj->Strength		= gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Strength;
	lpObj->Dexterity	= gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Dexterity;
	lpObj->Energy		= gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Energy;
	lpObj->Vitality		= gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Vitality;
	lpObj->Leadership	= gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Leadership;
	lpObj->LevelUpPoint = gResetTable.GetResetPoint(lpObj);

	for(int n=0;n < MAX_SKILL_LIST;n++)
	{
		lpObj->Skill[n].Clear();
	}

	for(int n=0;n < MAX_MASTER_SKILL_LIST;n++)
	{
		lpObj->MasterSkill[n].Clear();
	}

	if( lpObj->PartyNumber >= 0 )
	{
		gParty.DelMember(lpObj->PartyNumber, lpObj->Index);
		lpObj->PartyTargetUser	= -1;
	}

	if (this->m_PriceType[lpObj->AccountLevel] == 0)
	{
		lpObj->Money -= this->m_Price[lpObj->AccountLevel];
		GCMoneySend(lpObj->Index, lpObj->Money);
	}
	else if (this->m_PriceType[lpObj->AccountLevel] == 1)
	{
		gCashShop.GDCashShopSubPointSaveSend(lpObj->Index,0,this->m_Price[lpObj->AccountLevel],0,0,0);
	}
	else if (this->m_PriceType[lpObj->AccountLevel] == 2)
	{
		gCashShop.GDCashShopSubPointSaveSend(lpObj->Index,0,0,this->m_Price[lpObj->AccountLevel],0,0);
	}
	else if (this->m_PriceType[lpObj->AccountLevel] == 3)
	{
		gCashShop.GDCashShopSubPointSaveSend(lpObj->Index,0,0,0,this->m_Price[lpObj->AccountLevel],0);
	}
	else if (this->m_PriceType[lpObj->AccountLevel] == 4)
	{
		lpObj->PCPoint -= this->m_Price[lpObj->AccountLevel];
		gPcPoint.GDPcPointPointSend(lpObj->Index);
		gCashShop.CGCashShopPointRecv(lpObj->Index);
	}

	GCNewCharacterInfoSend(lpObj);	
	gEffectManager.ClearAllEffect(lpObj);
	GDCharacterInfoSaveSend(lpObj->Index);
	gSkillManager.GCSkillListSend(lpObj,0);
	gMasterSkillTree.GCMasterInfoSend(lpObj);
	gObjectManager.CharacterCalcAttribute(lpObj->Index);
	gMasterSkillTree.GCMasterSkillListSend(lpObj->Index);
	gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
//	gObjMoveGate(lpObj->Index, gDefaultClassInfo.m_DefaultClassInfo[NextClass].StartGate);

	lpObj->CloseCount   = 1;
	lpObj->CloseType    = 1;

	gLog.Output(LOG_COMMAND,"[ChangeClass System][%s][%s] - (ClassNum: %d)",lpObj->Account,lpObj->Name,Class);
}
