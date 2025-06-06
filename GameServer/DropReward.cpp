#include "stdafx.h"
#include "DropReward.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"
#include "CashShop.h"
#include "ItemManager.h"
#include "Map.h"

cDropReward gDropReward;

cDropReward::cDropReward()
{
	this->m_CustomItemDropReward.clear();
}

cDropReward::~cDropReward()
{
}

void cDropReward::load(char * path)
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

	this->m_CustomItemDropReward.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(strcmp("end",lpMemScript->GetAsString()) == 0)
				{
					break;
				}
				if(section == 0)
				{
					DROP_REWARD_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetAsNumber(),lpMemScript->GetAsNumber()));

					info.DropZen = lpMemScript->GetAsNumber();

					info.DropRate = lpMemScript->GetAsNumber();

					info.Message = lpMemScript->GetAsNumber();

					info.Fireworks = lpMemScript->GetAsNumber();

					this->m_CustomItemDropInfo.insert(std::pair<int, DROP_REWARD_INFO>(info.ItemIndex, info));
				}
				else if(section == 1)
				{
					DROP_REWARD_STATS info;

					info.Index = lpMemScript->GetNumber();

					info.RuudMin = lpMemScript->GetAsNumber();

					info.RuudMax = lpMemScript->GetAsNumber();

					info.WCoinCMin = lpMemScript->GetAsNumber();

					info.WCoinCMax = lpMemScript->GetAsNumber();

					info.WCoinPMin = lpMemScript->GetAsNumber();

					info.WCoinPMax = lpMemScript->GetAsNumber();

					info.GoblinPMin = lpMemScript->GetAsNumber();

					info.GoblinPMax = lpMemScript->GetAsNumber();

					this->m_CustomItemDropReward.insert(std::pair<int, DROP_REWARD_STATS>(info.Index, info));

				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool cDropReward::DropRewardByItemIndex(int ItemIndex, LPOBJ lpObj)
{
	std::map<int, DROP_REWARD_INFO>::iterator it1 = this->m_CustomItemDropInfo.find(ItemIndex);
	std::map<int, DROP_REWARD_STATS>::iterator it2;
	DWORD CoinC;
	DWORD CoinP;
	DWORD GoblinP;
	DWORD Ruud;

	if(it1 != this->m_CustomItemDropInfo.end())
	{
		it2 = m_CustomItemDropReward.find(it1->second.Index);

		if(it2 != this->m_CustomItemDropReward.end())
		{
			if((GetLargeRand() % 100) >= it1->second.DropRate)
			{
				gMap[lpObj->Map].MoneyItemDrop(it1->second.DropZen,lpObj->X,lpObj->Y);

				return true;
			}

			CoinC = (it2->second.WCoinCMax > 0) ? it2->second.WCoinCMin + rand() % ((it2->second.WCoinCMax + 1) - it2->second.WCoinCMin) : 0;

			CoinP = (it2->second.WCoinPMax > 0) ? it2->second.WCoinPMin + rand() % ((it2->second.WCoinPMax + 1) - it2->second.WCoinPMin) : 0;

			GoblinP = (it2->second.GoblinPMax > 0) ? it2->second.GoblinPMin + rand() % ((it2->second.GoblinPMax + 1) - it2->second.GoblinPMin) : 0;

			Ruud = (it2->second.RuudMax > 0) ? it2->second.RuudMin + rand() % ((it2->second.RuudMax + 1) - it2->second.RuudMin) : 0;

			gCashShop.GDCashShopAddPointSaveSend(lpObj->Index, 0, (CoinC > 0) ? CoinC : 0, (CoinP > 0) ? CoinP : 0, (GoblinP > 0) ? GoblinP : 0, (Ruud > 0) ? Ruud : 0);
			
			gCashShop.CGCashShopPointRecv(lpObj->Index);

			if(CoinC > 0)
			{
				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(it1->second.Message), CoinC, "WCoinC");
			}
			
			if(CoinP > 0)
			{
				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(it1->second.Message), CoinP, "WCoinP");
			}
			
			if(GoblinP > 0)
			{
				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(it1->second.Message), GoblinP, "GoblinPoint");
			}

			if(Ruud > 0)
			{
				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(it1->second.Message), Ruud, "Ruud");
			}

			if(it1->second.Fireworks != 0)
			{
				GCFireworksSend(lpObj,lpObj->X,lpObj->Y);
			}
			return true;
		}
	}
	return false;
}