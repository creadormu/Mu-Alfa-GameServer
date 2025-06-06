#include "stdafx.h"
#include "ChangeReset.h"
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

cChangeReset gChangeReset;

cChangeReset::cChangeReset()
{
	this->Init();
}

cChangeReset::~cChangeReset()
{
	
}

void cChangeReset::Init()
{

}

void cChangeReset::RecvChangeReset(CG_CHANGE_RESET_RECV* Data, int aIndex)
{
	gCashShop.GDCashShopRecievePointSend(aIndex,(DWORD)&cChangeReset::ChangeClassCallback,(DWORD)Data->Type,0);
}

void cChangeReset::ChangeClassCallback(LPOBJ lpObj, int Class, DWORD null, DWORD WCoinC, DWORD WCoinP, DWORD GoblinPoint)
{
	if(lpObj->Interface.use != 0 || lpObj->Teleport != 0 || lpObj->DieRegen != 0 || lpObj->PShopOpen != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You can not use it now.");
		return;
	}

	gChangeReset.ChangeReset(lpObj,Class);
}

void cChangeReset::ChangeReset(LPOBJ lpObj, int Class)
{
	if (lpObj->Interface.type == INTERFACE_CHAOS_BOX || lpObj->Interface.type == INTERFACE_TRADE || lpObj->Interface.type == INTERFACE_PERSONAL_SHOP || lpObj->Interface.use != 0)
	{
		return;
	}

	switch(Class)
	{
		case CHANGE_RESET_5:
			if(lpObj->Reset < 5)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(66),5);
				return;
			}
			lpObj->Reset -= 5;
			GDSetCoinSend(lpObj->Index,1000, 0, 0,"Change Reset 5");
			break;
		case CHANGE_RESET_10:
			if(lpObj->Reset < 10)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(66),10);
				return;
			}
			lpObj->Reset -= 10;
			GDSetCoinSend(lpObj->Index,2000, 0, 0,"Change Reset 10");
			break;
		case CHANGE_RESET_15:
			if(lpObj->Reset < 15)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(66),15);
				return;
			}
			lpObj->Reset -= 15;
			GDSetCoinSend(lpObj->Index,3000, 0, 0,"Change Reset 15");
			break;
		case CHANGE_RESET_20:
			if(lpObj->Reset < 20)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(66),20);
				return;
			}
			lpObj->Reset -= 20;
			GDSetCoinSend(lpObj->Index,4000, 0, 0,"Change Reset 20");
			break;
		case CHANGE_RESET_25:
			if(lpObj->Reset < 25)
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(66),25);
				return;
			}
			lpObj->Reset -= 25;
			GDSetCoinSend(lpObj->Index,5000, 0, 0,"Change Reset 25");
			break;
	}

	GDResetInfoSaveSend(lpObj->Index,0,0,0);
	GCNewCharacterInfoSend(lpObj);	
	GDCharacterInfoSaveSend(lpObj->Index);
	gSkillManager.GCSkillListSend(lpObj,0);
	gMasterSkillTree.GCMasterInfoSend(lpObj);
	gObjectManager.CharacterCalcAttribute(lpObj->Index);
	gMasterSkillTree.GCMasterSkillListSend(lpObj->Index);
	gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
}
