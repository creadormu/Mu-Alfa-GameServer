#include "stdafx.h"
#include "Util.h"
#include "User.h"
#include "RewardSystem.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "DSProtocol.h"
#include "SocketItemType.h"
#include "Notice.h"
#include "Message.h"

cRewardSystem gRewardSystem;

cRewardSystem::cRewardSystem() // OK
{
	for( int n = 0; n < 7 ; ++n )
	{
		this->m_RewardItemInfo[n].ItemStart.clear();
	}
}

cRewardSystem::~cRewardSystem() // OK
{

}

void cRewardSystem::Load(char* path) // OK
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

					REWARD_ITEM_INFO Item;

					int Class = lpMemScript->GetNumber();

					Item.Reset = lpMemScript->GetAsNumber();
					
					Item.MReset = lpMemScript->GetAsNumber();
					
					Item.UserType[ 0 ] = lpMemScript->GetAsNumber();
					
					Item.UserType[ 1 ] = lpMemScript->GetAsNumber();
					
					Item.UserType[ 2 ] = lpMemScript->GetAsNumber();
					
					Item.UserType[ 3 ] = lpMemScript->GetAsNumber();

					Item.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetAsNumber(), lpMemScript->GetAsNumber()));

					Item.ItemLevel = lpMemScript->GetAsNumber();

					Item.ItemLuck = lpMemScript->GetAsNumber();

					Item.ItemDurability = lpMemScript->GetAsNumber();

					Item.ItemSkill = lpMemScript->GetAsNumber();

					Item.ItemOption = lpMemScript->GetAsNumber();

					Item.ItemExcellent = lpMemScript->GetAsNumber();

					Item.ItemAncient = lpMemScript->GetAsNumber();

					Item.ItemJOH = lpMemScript->GetAsNumber();

					Item.ItemOpEx = lpMemScript->GetAsNumber();

					Item.ItemSocket1 = lpMemScript->GetAsNumber();

					Item.ItemSocket2 = lpMemScript->GetAsNumber();

					Item.ItemSocket3 = lpMemScript->GetAsNumber();

					Item.ItemSocket4 = lpMemScript->GetAsNumber();

					Item.ItemSocket5 = lpMemScript->GetAsNumber();

					if( Class >= 0 && Class <= 6 )
					{
						this->m_RewardItemInfo[ Class ].ItemStart.push_back( Item );
					}
					else
					{
						this->m_RewardItemInfo[ 0 ].ItemStart.push_back( Item );
						this->m_RewardItemInfo[ 1 ].ItemStart.push_back( Item );
						this->m_RewardItemInfo[ 2 ].ItemStart.push_back( Item );
						this->m_RewardItemInfo[ 3 ].ItemStart.push_back( Item );
						this->m_RewardItemInfo[ 4 ].ItemStart.push_back( Item );
						this->m_RewardItemInfo[ 5 ].ItemStart.push_back( Item );
						this->m_RewardItemInfo[ 6 ].ItemStart.push_back( Item );
					}
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

bool cRewardSystem::GetRewardByReset( LPOBJ lpObj )
{
	REWARD_INFO* lpInfo = &this->m_RewardItemInfo[ lpObj->Class ];

	for (std::vector<REWARD_ITEM_INFO>::iterator it = lpInfo->ItemStart.begin(); it != lpInfo->ItemStart.end(); it++)
	{
		if( it->Reset != -1 )
		{
			if ( lpObj->Reset != it->Reset )
			{
				continue;
			}
		}

		if( it->MReset != -1 )
		{
			if ( lpObj->MasterReset != it->MReset )
			{
				continue;
			}
		}

		for ( int n = 0 ; n < it->UserType[ lpObj->AccountLevel ]; ++n )
		{
			BYTE ItemSocketOption[MAX_SOCKET_OPTION] = { 0xFF,0xFF,0xFF,0xFF,0xFF };

			if (gSocketItemType.CheckSocketItemType(it->ItemIndex) == 1)
			{
				int qtd = gSocketItemType.GetSocketItemMaxSocket(it->ItemIndex);

				ItemSocketOption[0] = (BYTE)((qtd > 0) ? ((it->ItemSocket1 != 255) ? it->ItemSocket1 : 255) : 255);
				ItemSocketOption[1] = (BYTE)((qtd > 1) ? ((it->ItemSocket2 != 255) ? it->ItemSocket2 : 255) : 255);
				ItemSocketOption[2] = (BYTE)((qtd > 2) ? ((it->ItemSocket3 != 255) ? it->ItemSocket3 : 255) : 255);
				ItemSocketOption[3] = (BYTE)((qtd > 3) ? ((it->ItemSocket4 != 255) ? it->ItemSocket4 : 255) : 255);
				ItemSocketOption[4] = (BYTE)((qtd > 4) ? ((it->ItemSocket5 != 255) ? it->ItemSocket5 : 255) : 255);
			}
			GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, it->ItemIndex, it->ItemLevel, 0, it->ItemSkill, it->ItemLuck, it->ItemOption, -1, it->ItemExcellent, it->ItemAncient, it->ItemJOH, it->ItemOpEx, ItemSocketOption, 0xFF, 0);
		}
	}

	return 1;
}