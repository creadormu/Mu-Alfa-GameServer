// CustomPet.cpp: implementation of the CLuckyWheel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuckyWheel.h"
#include "MemScript.h"
#include "DSProtocol.h"
#include "Util.h"
#include "ItemManager.h"
#include "CashShop.h"
#include "User.h"
#include "Notice.h"
#include "Message.h"
#include "ServerInfo.h"

CLuckyWheel gLuckyWheel;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLuckyWheel::CLuckyWheel() // OK
{
	this->Init();
}

CLuckyWheel::~CLuckyWheel() // OK
{

}

void CLuckyWheel::Init() // OK
{
	for(int n=0;n < MAX_LUCKYWHEEL_ITEM;n++)
	{
		this->m_LuckyWheelInfo[n].Index = -1;
	}
}

void CLuckyWheel::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			LUCKYWHEEL_INFO info;

			memset(&info,0,sizeof(info));

			info.Index = lpMemScript->GetNumber();

			info.ItemType = lpMemScript->GetAsNumber();

			info.ItemIndex = lpMemScript->GetAsNumber();

			info.Level = lpMemScript->GetAsNumber();

			info.Skill = lpMemScript->GetAsNumber();

			info.Luck = lpMemScript->GetAsNumber();

			info.Option = lpMemScript->GetAsNumber();

			info.Exc = lpMemScript->GetAsNumber();

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CLuckyWheel::SetInfo(LUCKYWHEEL_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_LUCKYWHEEL_ITEM)
	{
		return;
	}

	this->m_LuckyWheelInfo[info.Index] = info;
}

void CLuckyWheel::Start(LPOBJ lpUser)
{
	if(lpUser->Coin1 < gServerInfo.m_LuckyWheelWCoinC || lpUser->Coin2 < gServerInfo.m_LuckyWheelWCoinP || lpUser->Coin3 < gServerInfo.m_LuckyWheelGPCoin)
	{
		gNotice.GCNoticeSend(lpUser->Index,1,0,0,0,0,0,"[LuckyWheel] You need [%d WCoinC] [%d WCoinP] [%d GPCoin]",gServerInfo.m_LuckyWheelWCoinC,gServerInfo.m_LuckyWheelWCoinP,gServerInfo.m_LuckyWheelGPCoin);
		ITEM_WIN_SEND pMsg;
		pMsg.header.set(0xF5,0x23,sizeof(pMsg));
		pMsg.number = 13;
		DataSend(lpUser->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gServerInfo.m_LuckyWheelWCoinC > 0)
	{
		gCashShop.GDCashShopSubPointSaveSend(lpUser->Index,0,gServerInfo.m_LuckyWheelWCoinC,0,0,0); //
	}
	if(gServerInfo.m_LuckyWheelWCoinP > 0)
	{
		gCashShop.GDCashShopSubPointSaveSend(lpUser->Index,0,0,gServerInfo.m_LuckyWheelWCoinP,0,0); //
	}
	if(gServerInfo.m_LuckyWheelGPCoin> 0)
	{
		gCashShop.GDCashShopSubPointSaveSend(lpUser->Index,0,0,0,gServerInfo.m_LuckyWheelGPCoin,0); //
	}
	
	Sleep(3000);
	srand(time((NULL)));//srand((unsigned int)time((NULL)));
	int number = rand() % 12;
	GDCreateItemSend(lpUser->Index,0xEB,0,0,GET_ITEM(m_LuckyWheelInfo[number].ItemType,m_LuckyWheelInfo[number].ItemIndex),m_LuckyWheelInfo[number].Level,0,m_LuckyWheelInfo[number].Skill,m_LuckyWheelInfo[number].Luck,m_LuckyWheelInfo[number].Option,-1,m_LuckyWheelInfo[number].Exc,0,0,0,0,0xFF,0);
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[%s] ¡Date la vuelta para tener suerte, consigue una recompensa!",lpUser->Name);
	
	ITEM_WIN_SEND pMsg;
	pMsg.header.set(0xF5,0x23,sizeof(pMsg));
	pMsg.number = number;
	DataSend(lpUser->Index,(BYTE*)&pMsg,pMsg.header.size);
}