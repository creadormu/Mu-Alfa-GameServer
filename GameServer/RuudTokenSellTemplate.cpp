#include "stdafx.h"
#include "RuudTokenSellTemplate.h"
#include "MemScript.h"
#include "Path.h"
#include "Util.h"
#include "Message.h"
#include "Notice.h"
#include "ItemManager.h"
#include "DSProtocol.h"
#include "CashShop.h"

RuudTokenSellTemplate g_RuudTokenSellTemplate;

RuudTokenSellTemplate::RuudTokenSellTemplate()
{
	for (int i = 0; i < MAX_CLASSTOKENSELL_INFO; i++)
	{
		for (int n = 0; n < MAX_RUUDTOKENSELL_INFO; n++)
		{
			RuudTokenSell[ i ].JRShop[ n ].byItemType = -1;
			RuudTokenSell[ i ].JRShop[ n ].wItemIndex = -1;
			RuudTokenSell[ i ].JRShop[ n ].shPageShop = -1;
			RuudTokenSell[ i ].JRShop[ n ].wSlotIndex = -1;
			RuudTokenSell[ i ].JRShop[ n ].dwRuudCount = -1;
		}
	}
}

RuudTokenSellTemplate::~RuudTokenSellTemplate()
{
}

void RuudTokenSellTemplate::Load_RuudTokenSell(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox("[RuudTokenSellTemplate] Info file Load Fail [%s]", path);
		delete lpMemScript;
		return;
	}

	int Conut = 0;

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

			_tagRuudTokenSell info;

			info.byItemType = lpMemScript->GetNumber();

			info.wItemIndex = lpMemScript->GetAsNumber();

			int Class = lpMemScript->GetAsNumber();

			info.shPageShop = lpMemScript->GetAsNumber();

			info.wSlotIndex = lpMemScript->GetAsNumber();

			info.dwRuudCount = lpMemScript->GetAsNumber();

			if( info.shPageShop > 5 || info.wSlotIndex > 5 )
			{
				continue;
			}

			if( Class >= 0 && Class < 7 )
			{
				this->RuudTokenSell[ Class ].JRShop[ ((info.shPageShop - 1) * 5) + info.wSlotIndex - 1 ] = info;
			}
			else
			{
				this->RuudTokenSell[ 0 ].JRShop[ ((info.shPageShop - 1) * 5) + info.wSlotIndex - 1 ] = info;
				this->RuudTokenSell[ 1 ].JRShop[ ((info.shPageShop - 1) * 5) + info.wSlotIndex - 1 ] = info;
				this->RuudTokenSell[ 2 ].JRShop[ ((info.shPageShop - 1) * 5) + info.wSlotIndex - 1 ] = info;
				this->RuudTokenSell[ 3 ].JRShop[ ((info.shPageShop - 1) * 5) + info.wSlotIndex - 1 ] = info;
				this->RuudTokenSell[ 4 ].JRShop[ ((info.shPageShop - 1) * 5) + info.wSlotIndex - 1 ] = info;
				this->RuudTokenSell[ 5 ].JRShop[ ((info.shPageShop - 1) * 5) + info.wSlotIndex - 1 ] = info;
				this->RuudTokenSell[ 6 ].JRShop[ ((info.shPageShop - 1) * 5) + info.wSlotIndex - 1 ] = info;
			}
			Conut++;
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void RuudTokenSellTemplate::GCShopItemListSend(LPOBJ lpObj)
{
	if(gObjIsConnected(lpObj->Index) == false)
	{
		return;
	}

	BYTE send[1024];

	PMSG_ITEMRUUD_LIST_SEND pMsg;

	pMsg.header.set(0xEA, 0);

	int size = sizeof(pMsg);

	pMsg.result = 1;

	pMsg.count = 0;

	_tagRuudTokenSell info;

	for ( int k = 0 ; k < MAX_RUUDTOKENSELL_INFO ; k++ )
	{
		
		info.byItemType = this->RuudTokenSell[lpObj->Class].JRShop[ k ].byItemType;

		info.wItemIndex = this->RuudTokenSell[lpObj->Class].JRShop[ k ].wItemIndex;

		info.shPageShop = this->RuudTokenSell[lpObj->Class].JRShop[ k ].shPageShop;

		info.wSlotIndex = this->RuudTokenSell[lpObj->Class].JRShop[ k ].wSlotIndex;

		info.dwRuudCount = this->RuudTokenSell[lpObj->Class].JRShop[ k ].dwRuudCount;

		memcpy(&send[size],&info,sizeof(info));

		size += sizeof(info);
		
		pMsg.count++;
	}

	pMsg.header.size = size;

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(lpObj->Index, send, size);
}

void RuudTokenSellTemplate::_RecvRuudBUY(PMSG_BUY_ITEMRUUD_RECV * lpMsg, int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];
	int ItemID;

	for ( int k = 0 ; k < MAX_RUUDTOKENSELL_INFO ; k++ )
	{
		if( this->RuudTokenSell[lpObj->Class].JRShop[ k ].shPageShop == lpMsg->PAGE
			&& this->RuudTokenSell[lpObj->Class].JRShop[ k ].wSlotIndex == lpMsg->SLOT )
		{
			if ( this->RuudTokenSell[lpObj->Class].JRShop[ k ].dwRuudCount > lpObj->Ruud )
			{
				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(834));
				return;
			}
			else
			{
				ItemID = GET_ITEM(this->RuudTokenSell[lpObj->Class].JRShop[ k ].byItemType, this->RuudTokenSell[lpObj->Class].JRShop[ k ].wItemIndex);

				ITEM_INFO ItemInfo;
				if( gItemManager.GetInfo(ItemID, &ItemInfo) == 0 )
				{
					return;
				}

				int crear = 0;

				int MaxY = ( gItemManager.GetInventoryMaxValue( lpObj )-INVENTORY_WEAR_SIZE ) / 8;

				for( int y = 0 ; y < MaxY; y++ )
				{
					for( int x = 0 ; x < 8; x++ )
					{
						if( lpObj->InventoryMap[((y*8)+x)] == 0xFF )
						{
							BYTE slot = gItemManager.InventoryRectCheck(lpObj->Index, x, y, ItemInfo.Width, ItemInfo.Height);

							if(slot != 0xFF)
							{
								if( crear == 0 )
								{
									BYTE ItemSocketOption[MAX_SOCKET_OPTION] = { 0xFF,0xFF,0xFF,0xFF,0xFF };

								GDCreateItemSend(lpObj->Index,
									0xEB,
									0,
									0,
									ItemID,
									(BYTE)0,
									0,
									0,
									0,
									0,
									lpObj->Index,
									0,
									0,
									0,
									0,
									ItemSocketOption,
									0,
									0);
									//--
									crear = 1;

									gCashShop.GDCashShopSubPointSaveSend(lpObj->Index,0,0,0,0,this->RuudTokenSell[lpObj->Class].JRShop[ k ].dwRuudCount);
									//-- Ruud Update
									gCashShop.CGCashShopPointRecv(lpObj->Index);
								}
							}
						}
					}
				}
			
				if ( crear == 0 )
				{
					gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(824));
					return;
				}
			}
		}
	}
}