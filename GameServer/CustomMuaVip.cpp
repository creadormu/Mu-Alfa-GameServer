#include "StdAfx.h"
#include "user.h"
#include "CustomMuaVip.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Path.h"
#include "MemScript.h"
#include "Util.h"
#include "CashShop.h"
#include "Message.h"
#include "Notice.h"
#include "CustomRankUser.h"
#include "ObjectManager.h"
#include "ServerInfo.h"

MuaVip gMuaVip;
// -------------------------------------------------------------------------------
MuaVip::MuaVip() // OK
{
	this->Init();
}
// -------------------------------------------------------------------------------
MuaVip::~MuaVip() // OK
{
}
// -------------------------------------------------------------------------------
void MuaVip::Init()
{
	for (int n = 0; n < 5; n++)
	{
		this->M_OPTION_MUA_VIP[n].Index = 0;
	}
}
// -------------------------------------------------------------------------------
void MuaVip::Load(char* path) // OK
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

			int section = lpMemScript->GetNumber();

			while (true)
			{
				if (section == 0)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					MUA_VIP_OPTION_DATA info;

					memset(&info, 0, sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.MUA_VIP_SAT_THUONG = lpMemScript->GetAsNumber();

					info.MUA_VIP_SAT_THUONG_X2 = lpMemScript->GetAsNumber();

					info.MUA_VIP_PHONG_THU = lpMemScript->GetAsNumber();

					info.MUA_VIP_THE_LUC = lpMemScript->GetAsNumber();

					info.MUA_VIP_SD = lpMemScript->GetAsNumber();

					this->SET_MUA_VIP_INFO(info);

				}
				else

				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}
// -------------------------------------------------------------------------------
void MuaVip::SET_MUA_VIP_INFO(MUA_VIP_OPTION_DATA info) // OK
{
	if (info.Index < 0 || info.Index >= 5)
	{
		return;
	}

	this->M_OPTION_MUA_VIP[info.Index] = info;
}
// -------------------------------------------------------------------------------
void MuaVip::MUA_VIP_OPTION(LPOBJ lpObj, bool flag)
{
	if (flag != 0)
	{
		return;
	}
	// Add Damage		
	lpObj->PhysiDamageMinLeft += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_SAT_THUONG;
	lpObj->PhysiDamageMinRight += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_SAT_THUONG;
	lpObj->PhysiDamageMaxLeft += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_SAT_THUONG;
	lpObj->PhysiDamageMaxRight += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_SAT_THUONG;
	// Add Double Damage
	lpObj->DoubleDamageRate += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_SAT_THUONG_X2;
	// Add Defense
	lpObj->Defense += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_PHONG_THU;
	lpObj->MagicDefense += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_PHONG_THU;
	// Add HP
	lpObj->AddLife += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_THE_LUC;
	// Add SD
	lpObj->AddShield += this->M_OPTION_MUA_VIP[lpObj->rMuaVip].MUA_VIP_SD;

}
// -------------------------------------------------------------------------------
void MuaVip::BUY_MUA_VIP(LPOBJ lpObj, BUY_MUA_VIP_REQ *aRecv)
{
	if (aRecv->Number == 1)
	{
		if (lpObj->Coin1 <  gServerInfo.m_WCOINC_MUAVIP)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0,  "You need (%d) WcoinC To update [Mua Vip]", gServerInfo.m_WCOINC_MUAVIP);
			return;
		}
		if (lpObj->rMuaVip >  3)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You've Reached Maximum Level [Mua Vip]");
			return;
		}
		gCashShop.GDCashShopSubPointSaveSend(lpObj->Index, 0, gServerInfo.m_WCOINC_MUAVIP, 0, 0, 0);
		gCashShop.CGCashShopPointRecv(lpObj->Index);
		lpObj->rMuaVip += 1; // Thay Đổi
		GCFireworksSend(lpObj, lpObj->X, lpObj->Y);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[%s] Upgraded [Mua Víp] success!", lpObj->Name);
		gCustomRankUser.GCReqRankLevelUser(lpObj->Index, lpObj->Index);
		GDCharacterInfoSaveSend(lpObj->Index);
		gObjectManager.CharacterCalcAttribute(lpObj->Index);
	}
	//-----------------------------------------------
}
