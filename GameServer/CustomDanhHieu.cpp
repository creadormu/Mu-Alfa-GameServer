#include "StdAfx.h"
#include "user.h"
#include "CustomDanhHieu.h"
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

DanhHieu gDanhHieu;
// -------------------------------------------------------------------------------
DanhHieu::DanhHieu() // OK
{
	this->Init();
}
// -------------------------------------------------------------------------------
DanhHieu::~DanhHieu() // OK
{
}
// -------------------------------------------------------------------------------
void DanhHieu::Init()
{
	for (int n = 0; n < 20; n++)
	{
		this->M_OPTION_DANH_HIEU[n].Index = 0;
	}
}
// -------------------------------------------------------------------------------
void DanhHieu::Load(char* path) // OK
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

					DANH_HIEU_OPTION_DATA info;

					memset(&info, 0, sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.DANH_HIEU_SAT_THUONG = lpMemScript->GetAsNumber();

					info.DANH_HIEU_SAT_THUONG_X2 = lpMemScript->GetAsNumber();

					info.DANH_HIEU_PHONG_THU = lpMemScript->GetAsNumber();

					info.DANH_HIEU_THE_LUC = lpMemScript->GetAsNumber();

					info.DANH_HIEU_SD = lpMemScript->GetAsNumber();

					this->SET_DANH_HIEU_INFO(info);

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
void DanhHieu::SET_DANH_HIEU_INFO(DANH_HIEU_OPTION_DATA info) // OK
{
	if (info.Index < 0 || info.Index >= 20)
	{
		return;
	}

	this->M_OPTION_DANH_HIEU[info.Index] = info;
}
// -------------------------------------------------------------------------------
void DanhHieu::DANH_HIEU_OPTION(LPOBJ lpObj, bool flag)
{
	if (flag != 0)
	{
		return;
	}
	// Add Damage		
	lpObj->PhysiDamageMinLeft += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_SAT_THUONG;
	lpObj->PhysiDamageMinRight += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_SAT_THUONG;
	lpObj->PhysiDamageMaxLeft += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_SAT_THUONG;
	lpObj->PhysiDamageMaxRight += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_SAT_THUONG;
	// Add Double Damage
	lpObj->DoubleDamageRate += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_SAT_THUONG_X2;
	// Add Defense
	lpObj->Defense += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_PHONG_THU;
	lpObj->MagicDefense += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_PHONG_THU;
	// Add HP
	lpObj->AddLife += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_THE_LUC;
	// Add SD
	lpObj->AddShield += this->M_OPTION_DANH_HIEU[lpObj->rDanhHieu].DANH_HIEU_SD;

}
// -------------------------------------------------------------------------------
void DanhHieu::BUY_DANH_HIEU(LPOBJ lpObj, BUY_DANH_HIEU_REQ *aRecv)
{
	if (aRecv->Number == 1)
	{
		if (lpObj->Coin1 <  gServerInfo.m_WCOINC_DANHHIEU)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You need %d) WcoinC To update [Title]", gServerInfo.m_WCOINC_DANHHIEU);
			return;
		}
		if (lpObj->rDanhHieu >  15)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "You Have Reached Maximum Level [Title]!");
			return;
		}
		gCashShop.GDCashShopSubPointSaveSend(lpObj->Index, 0, gServerInfo.m_WCOINC_DANHHIEU, 0, 0, 0);
		gCashShop.CGCashShopPointRecv(lpObj->Index);
		lpObj->rDanhHieu += 1; // Thay Đổi
		GCFireworksSend(lpObj, lpObj->X, lpObj->Y);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[%s] Upgraded [Title] success!", lpObj->Name);
		gCustomRankUser.GCReqRankLevelUser(lpObj->Index, lpObj->Index);
		GDCharacterInfoSaveSend(lpObj->Index);
		gObjectManager.CharacterCalcAttribute(lpObj->Index);
	}
	//-----------------------------------------------
}

