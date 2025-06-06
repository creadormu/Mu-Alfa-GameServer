#include "StdAfx.h"
#include "user.h"
#include "TuChan.h"
#include "GameMain.h"
#include "MemScript.h"
#include "Log.h"
#include "DSProtocol.h"
#include "Notice.h"
#include "ItemManager.h"
#include "Util.h"
#include "Message.h"
#include "CustomRankUser.h"
#include "ObjectManager.h"

cTitle gTitle;

cTitle::cTitle() // OK
{
	this->Init();
}

cTitle::~cTitle() // OK
{

}

void cTitle::Init() // OK
{
	this->m_TuChan.Enabled = 0;
	this->m_TuChan.gemcount = 0;
	this->m_TuChan.gemcount1 = 0;
	this->m_TuChan.gemcount2 = 0;
	this->m_TuChan.gemcount3 = 0;
	this->m_TuChan.rType = 0;
	this->m_TuChan.rIndex = 0;
	this->m_TuChan.rType1 = 0;
	this->m_TuChan.rIndex1 = 0;
	this->m_TuChan.rType2 = 0;
	this->m_TuChan.rIndex2 = 0;
	this->m_TuChan.rType3 = 0;
	this->m_TuChan.rIndex3 = 0;
	this->m_TuChan.rcrit = 0;
	this->m_TuChan.rdmg = 0;
	this->m_TuChan.rdouble = 0;
	this->m_TuChan.rexc = 0;
}

void cTitle::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if (strcmp("end", lpMemScript->GetString()) == 0)
			{
				break;
			}

			m_TuChan.Enabled = lpMemScript->GetNumber();

			m_TuChan.gemcount = lpMemScript->GetAsNumber();

			m_TuChan.gemcount1 = lpMemScript->GetAsNumber();

			m_TuChan.gemcount2 = lpMemScript->GetAsNumber();

			m_TuChan.gemcount3 = lpMemScript->GetAsNumber();

			m_TuChan.maxlv = lpMemScript->GetAsNumber();

			m_TuChan.rType = lpMemScript->GetAsNumber();

			m_TuChan.rIndex = lpMemScript->GetAsNumber();

			m_TuChan.rType1 = lpMemScript->GetAsNumber();

			m_TuChan.rIndex1 = lpMemScript->GetAsNumber();

			m_TuChan.rType2 = lpMemScript->GetAsNumber();

			m_TuChan.rIndex2 = lpMemScript->GetAsNumber();

			m_TuChan.rType3 = lpMemScript->GetAsNumber();

			m_TuChan.rIndex3 = lpMemScript->GetAsNumber();

			m_TuChan.rcrit = lpMemScript->GetAsNumber();

			m_TuChan.rexc = lpMemScript->GetAsNumber();

			m_TuChan.rdmg = lpMemScript->GetAsNumber();

			m_TuChan.rdouble = lpMemScript->GetAsNumber();
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void cTitle::gObjUserTitle(LPOBJ lpObj)
{
	if (!this->m_TuChan.Enabled) return;

	//LPOBJ lpObj = &gObj[lpObj->Index];

	if (lpObj->Lvtuchan > m_TuChan.maxlv)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You have attained the righteous result. Go down the mountain to save people");
		return;
	}
	gObjectManager.CharacterCalcAttribute(lpObj->Index);
	GCNewCharacterInfoSend(lpObj);
	GDCharacterInfoSaveSend(lpObj->Index);
	// ----
	int gemc = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(m_TuChan.rType, m_TuChan.rIndex), 0);
	int gemc1 = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(m_TuChan.rType1, m_TuChan.rIndex1), 0);
	int gemc2 = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(m_TuChan.rType2, m_TuChan.rIndex2), 0);
	int gemc3 = gItemManager.GetInventoryItemCount(lpObj, GET_ITEM(m_TuChan.rType3, m_TuChan.rIndex3), 0);

	if (lpObj->Lvtuchan <= 9)
	{
		if (gemc < this->m_TuChan.gemcount)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0,  "You lack %d Jade of Chaos for Cultivation",this->m_TuChan.gemcount);
			return;
		}

		//Tet

		gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(m_TuChan.rType, m_TuChan.rIndex), 0, this->m_TuChan.gemcount);
		// ---- 
		lpObj->rcrit += this->m_TuChan.rcrit;
		lpObj->rexc += this->m_TuChan.rexc;
		lpObj->rdmg	+=	this->m_TuChan.rdmg;
		lpObj->rdouble	+=	this->m_TuChan.rdouble;
		lpObj->Lvtuchan += 1;
		//++++++++++++++SendToUpDate++++++++++++++++++++//
		//gObjectManager.CharacterCalcAttribute(lpObj->Lvtuchan);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "Youth [%s] have successfully cultivated to %d", lpObj->Name, lpObj->Lvtuchan);
		gLog.Output(LOG_COMMAND,"[CommandTuChan][%s][%s] Tu chan Thanh cong = ItemType:[%d] ID:[%d] - OK",lpObj->Account,lpObj->Name,m_TuChan.rType, m_TuChan.rIndex);
		//++++++++++++++++++++++++++++++++++//
	}
	else if (lpObj->Lvtuchan >= 10 && lpObj->Lvtuchan <= 19)
	{
		if (gemc1 < this->m_TuChan.gemcount1)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You lack %d Jade of Wishing to Cultivate",this->m_TuChan.gemcount1);
			return;
		}
		//Tet	
		gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(m_TuChan.rType1, m_TuChan.rIndex1), 0, this->m_TuChan.gemcount1);
		lpObj->rcrit += this->m_TuChan.rcrit;
		lpObj->rexc += this->m_TuChan.rexc;
		lpObj->rdmg	+=	this->m_TuChan.rdmg;
		lpObj->rdouble	+=	this->m_TuChan.rdouble;
		lpObj->Lvtuchan += 1;

		//++++++++++++++SendToUpDate++++++++++++++++++++//
		//gObjectManager.CharacterCalcAttribute(lpObj->Lvtuchan);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "Youth [%s] have successfully cultivated to %d", lpObj->Name, lpObj->Lvtuchan);
		gLog.Output(LOG_COMMAND,"[CommandTuChan][%s][%s] Tu chan Thanh cong = ItemType:[%d] ID:[%d] - OK",lpObj->Account,lpObj->Name,m_TuChan.rType1, m_TuChan.rIndex1);
		//++++++++++++++++++++++++++++++++++//
	}
	else if (lpObj->Lvtuchan >= 20 && lpObj->Lvtuchan <= 29)
	{
		if (gemc2 < this->m_TuChan.gemcount2)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You lack %d Spiritual Gems to cultivate", this->m_TuChan.gemcount2);
			return;
		}
		gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(m_TuChan.rType2, m_TuChan.rIndex2), 0, this->m_TuChan.gemcount2);
		lpObj->rcrit += this->m_TuChan.rcrit;
		lpObj->rexc += this->m_TuChan.rexc;
		lpObj->rdmg	+=	this->m_TuChan.rdmg;
		lpObj->rdouble	+=	this->m_TuChan.rdouble;
		lpObj->Lvtuchan += 1;

		//++++++++++++++SendToUpDate++++++++++++++++++++//
		//gObjectManager.CharacterCalcAttribute(lpObj->Lvtuchan);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "Youth [%s] have successfully cultivated to %d", lpObj->Name, lpObj->Lvtuchan);
		gLog.Output(LOG_COMMAND,"[CommandTuChan][%s][%s] Tu chan Thanh cong = ItemType:[%d] ID:[%d] - OK",lpObj->Account,lpObj->Name,m_TuChan.rType2, m_TuChan.rIndex2);
		//++++++++++++++++++++++++++++++++++//
	}
	else if (lpObj->Lvtuchan >= 30 && lpObj->Lvtuchan <= 40)
	{
		if (gemc3 < this->m_TuChan.gemcount3)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You lack %d Creation Gems for Cultivation", this->m_TuChan.gemcount3);
			return;
		}
		gItemManager.DeleteInventoryItemCount(lpObj, GET_ITEM(m_TuChan.rType3, m_TuChan.rIndex3), 0, this->m_TuChan.gemcount3);
		lpObj->rcrit += this->m_TuChan.rcrit;
		lpObj->rexc += this->m_TuChan.rexc;
		lpObj->rdmg	+=	this->m_TuChan.rdmg;
		lpObj->rdouble	+=	this->m_TuChan.rdouble;
		lpObj->Lvtuchan += 1;

		//++++++++++++++SendToUpDate++++++++++++++++++++//
		//gObjectManager.CharacterCalcAttribute(lpObj->Lvtuchan);
		
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0,"Youth [%s] have successfully cultivated to %d", lpObj->Name, lpObj->Lvtuchan);
		if(lpObj->Lvtuchan == 40)
		{
			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "You have attained the righteous result. Go down the mountain to save people", lpObj->Name, lpObj->Lvtuchan);
		}
		gLog.Output(LOG_COMMAND,"[CommandTuChan][%s][%s] Tu chan Thanh cong = ItemType:[%d] ID:[%d] - OK",lpObj->Account,lpObj->Name,m_TuChan.rType3, m_TuChan.rIndex3);
		//++++++++++++++++++++++++++++++++++//
	}
	// ----
	GCNewCharacterInfoSend(lpObj);
	GDCharacterInfoSaveSend(lpObj->Index);
	GCFireworksSend(lpObj,lpObj->X,lpObj->Y);
	gCustomRankUser.CheckUpdate(lpObj);
	gCustomRankUser.GCReqRankLevelUser(lpObj->Index, lpObj->Index);
	//Sleep(10000)
}
void cTitle::Option(LPOBJ lpObj, bool flag)
{
	if (flag != 0)
	{
		return;
	}
	lpObj->CriticalDamageRate += lpObj->rcrit;
	lpObj->ExcellentDamageRate += lpObj->rexc;
	//dame
	lpObj->PhysiDamageMinLeft += lpObj->rdmg;
	lpObj->PhysiDamageMinRight += lpObj->rdmg;
	lpObj->PhysiDamageMaxLeft += lpObj->rdmg;
	lpObj->PhysiDamageMaxRight += lpObj->rdmg;
	//double
	lpObj->DoubleDamageRate += lpObj->rdouble;
}
