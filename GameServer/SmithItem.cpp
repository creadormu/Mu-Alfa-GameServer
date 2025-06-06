#include "stdafx.h"
#include "User.h"
#include "Log.h"
#include "readscript.h"
#include "SmithItem.h"
#include "CashShop.h"
#include "Notice.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "Util.h"
#include "Message.h"

SCFVipShop SVShop;

void SCFVipShop::Init(char * Ini, char * FilePath)
{
	SMDToken Token;
	SMDFile = fopen(Ini, "r");
	// ----
	if( !SMDFile )
	{
		ErrorMessageBox("[SmithItem.ini] %s file not found", Ini);
		return;
	}

	SMDFile = fopen(FilePath, "r");

	if( !SMDFile )
	{
		ErrorMessageBox("[SmithItem.txt] %s file not found", FilePath);
		return;
	}

	this->Enabled		= GetPrivateProfileInt("Common", "Enabled",0, Ini) ;

	this->pLevel		= GetPrivateProfileInt("Cost", "ItemLevel",0, Ini) ;
	this->pOpt			= GetPrivateProfileInt("Cost", "ItemOption",0, Ini) ;
	this->pLuck			= GetPrivateProfileInt("Cost", "ItemLuck",0, Ini) ;
	this->pSkill		= GetPrivateProfileInt("Cost", "ItemSkill",0, Ini) ;
	this->pExc			= GetPrivateProfileInt("Cost", "ItemExcOption",0, Ini) ;
	this->pExc1			= GetPrivateProfileInt("Cost", "ItemExcOption1",0, Ini) ;
	this->pExc2			= GetPrivateProfileInt("Cost", "ItemExcOption2",0, Ini) ;
	this->pExc3			= GetPrivateProfileInt("Cost", "ItemExcOption3",0, Ini) ;
	this->pExc4			= GetPrivateProfileInt("Cost", "ItemExcOption4",0, Ini) ;
	this->pExc5			= GetPrivateProfileInt("Cost", "ItemExcOption5",0, Ini) ;

	this->MaxLevel		= GetPrivateProfileInt("Max", "ItemMaxLevel",0, Ini) ;
	this->MaxOpt		= GetPrivateProfileInt("Max", "ItemMaxOption",0, Ini) ;
	this->MaxExc		= GetPrivateProfileInt("Max", "ItemMaxExcOption",0, Ini) ;
	this->MaxLuck		= GetPrivateProfileInt("Max", "ItemMaxLuck",0, Ini) ;
	this->MaxSkill		= GetPrivateProfileInt("Max", "ItemMaxSkill",0, Ini) ;

	this->TypeCount = 0;

	for(int i=0;i<(MAX_TYPE_ITEMS*MAX_SUBTYPE_ITEMS);i++)
	{
		this->item[i].Enabled = false;
	}

	this->Read(FilePath);
}

void SCFVipShop::SendInfo(int aIndex)
{
	if(this->Enabled == 1)
	{
		LPOBJ lpObj = &gObj[aIndex];
		SDHP_SENDSVSHOP pInfo;
		pInfo.h.set(0xFB,0x19,sizeof(pInfo));
		pInfo.MaxLevel=this->MaxLevel;
		pInfo.MaxOpt=this->MaxOpt;
		pInfo.MaxExc=this->MaxExc;
		pInfo.MaxSkill=this->MaxSkill;
		pInfo.MaxLuck=this->MaxLuck;
		pInfo.pLevel=this->pLevel;
		pInfo.pOpt=this->pOpt;
		pInfo.pSkill=this->pSkill;
		pInfo.pLuck=this->pLuck;
		pInfo.pExc=this->pExc;
		pInfo.pExc1=this->pExc1;
		pInfo.pExc2=this->pExc2;
		pInfo.pExc3=this->pExc3;
		pInfo.pExc4=this->pExc4;
		pInfo.pExc5=this->pExc5;
		pInfo.TotalPrice=this->TotalPrice;
		DataSend(aIndex,(BYTE*)&pInfo,pInfo.h.size);
	}
}

void SCFVipShop::RecvBuy(int aIndex,SDHP_BUYSVSHOP * lpMsg)
{	
	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	int id=GET_ITEM(lpMsg->ItemType,lpMsg->ItemIndex);
	if(this->item[id].Enabled == true)
	{
		if(lpMsg->mLevel > this->MaxLevel)
		{
			//Hack attempt
			gLog.Output(LOG_SMITHITEM,"[SmithItem] Attempted Character-Buy item with Hack in LEVEL [%s][%s]",gObj[aIndex].Account,gObj[aIndex].Name);
			return;
		}
		if(lpMsg->mOpt > this->MaxOpt)
		{
			//Hack attempt
			gLog.Output(LOG_SMITHITEM,"[SmithItem] Attempted Character-Buy item with Hack in OPT [%s][%s]",gObj[aIndex].Account,gObj[aIndex].Name);
			return;
		}
		if(lpMsg->mLuck > this->MaxLuck)
		{
			gLog.Output(LOG_SMITHITEM,"[SmithItem] Attempted Character-Buy item with Hack in LUCK [%s][%s]",gObj[aIndex].Account,gObj[aIndex].Name);
			return;
		}
		if(lpMsg->mSkill > this->MaxSkill)
		{
			//Hack attempt
			gLog.Output(LOG_SMITHITEM,"[SmithItem] Attempted Character-Buy item with Hack in SKILL [%s][%s]",gObj[aIndex].Account,gObj[aIndex].Name);
			return;
		}
		if(lpMsg->mExc > this->MaxExc)
		{
			//Hack attempt
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"Your can chose only %d Excellent Option",this->MaxExc);
			gLog.Output(LOG_SMITHITEM,"[SmithItem] Attempted Character-Buy item with Hack in EXCOPT [%s][%s]",gObj[aIndex].Account,gObj[aIndex].Name);
			return;
		}
		if(gItemManager.CheckItemInventorySpace(lpObj,lpMsg->ItemIndex) == 0)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"Your inventory is full");
			return;
		}

		int Val = 0;
		switch(lpMsg->Days)
		{
			case 0:
			{
				Val = this->item[id].Price_1Day;
			}break;
			case 1:
			{
				Val = this->item[id].Price_7Days;
			}break;
			case 2:
			{
				Val = this->item[id].Price_30Days;
			}break;
		}
		int Total = Val;

		if(Val == 0)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You cant buy items with that number of days!");
			return;
		}

		if(lpMsg->ExcOption == 32)
		{
			Total += this->pExc;
		}
		if(lpMsg->ExcOption1 == 16)
		{
			Total += this->pExc1;
		}
		if(lpMsg->ExcOption2 == 8)
		{
			Total += this->pExc2;
		}
		if(lpMsg->ExcOption3 == 4)
		{
			Total += this->pExc3;
		}
		if(lpMsg->ExcOption4 == 2)
		{
			Total += this->pExc4;
		}
		if(lpMsg->ExcOption5 == 1)
		{
			Total += this->pExc5;
		}
		Total += (lpMsg->mLevel * this->pLevel);
		Total += (lpMsg->mLuck * this->pLuck);
		Total += (lpMsg->mOpt * this->pOpt);
		Total += (lpMsg->mSkill * this->pSkill);	
		BYTE EXCARC = lpMsg->ExcOption + lpMsg->ExcOption1 + lpMsg->ExcOption2 + lpMsg->ExcOption3 + lpMsg->ExcOption4 + lpMsg->ExcOption5;
		TotalPrice = Total;
		if(lpObj->Coin1 >= Total)
		{	
				int Days = 1;
				switch(lpMsg->Days)
				{
					case 0:
					{
						Days = 1;
					}break;
					case 1:
					{
						Days = 7;
					}break;
					case 2:
					{
						Days = 30;
					}break;
				}
				time_t t = time(NULL);
				localtime(&t);
				DWORD iTime = (DWORD)t + Days * 86400;
				GDCreateItemSend(lpObj->Index,0xEB,0,0,id,lpMsg->mLevel,0,lpMsg->mSkill,lpMsg->mLuck,lpMsg->mOpt,-1,EXCARC,0,0,0,0,0xFF,iTime);
				gCashShop.GDCashShopSubPointSaveSend(lpObj->Index,0,Total,0,0,0);
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"Item Buy Success!");
				gLog.Output(LOG_SMITHITEM,"[SmithItem](%s)(%s) Buy Item:%d Price:%d Total:%d [Level:%d Option:%d Luck:%d Skill:%d ExcOption:%d]",
				gObj[aIndex].Account,gObj[aIndex].Name,id,Val,Total,lpMsg->mLevel,lpMsg->mOpt,lpMsg->mLuck,lpMsg->mSkill,lpMsg->mExc);
				}
			else
				{
					gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You need more WCoins");
				}
		}
	else
		{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,  "Item Dissabled!");
		}
}

void SCFVipShop::Read(char * FilePath)
{
	if(this->Enabled == 1)
	{
		int Token;

		SMDFile = fopen(FilePath, "r");

		if ( SMDFile == NULL )
		{
			return;
		}

		while ( true )
		{
			int iType = GetToken();
			
			if ( iType == 1 )
			{
				while(true)
				{
					Token = GetToken();
					if ( stricmp("end", TokenString) == 0 )
					{
						break;
					}

					int type = TokenNumber;
					
					Token = GetToken();
					int Index = TokenNumber;
					
					if(type >= 12)
					{
						MessageBox(0,"SMITHITEM MAX TYPE = 11","Error!",0);
						exit(1);
					}
					if(type >= 512)
					{
						MessageBox(0,"SMITHITEM MAX INDEX = 512","Error!",0);
						exit(1);
					}
					
					Token = GetToken();
					int Price_1Day = TokenNumber;
					Token = GetToken();
					int Price_7Days = TokenNumber;
					Token = GetToken();
					int Price_30Days = TokenNumber;

					int ID = GET_ITEM(type,Index);
					this->item[ID].Enabled = true;
					this->item[ID].Price_1Day = Price_1Day;
					this->item[ID].Price_7Days = Price_7Days;
					this->item[ID].Price_30Days = Price_30Days;
				}
			}
			break;
		}			
		fclose(SMDFile);
	}
}