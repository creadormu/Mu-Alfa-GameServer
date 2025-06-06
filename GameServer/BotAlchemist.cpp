#include "stdafx.h"
#include "BotAlchemist.h"
#include "Log.h"
#include "GameMain.h"
#include "User.h"
#include "Protocol.h"
#include "Skill.h"
#include "DevilSquare.h"
#include "BloodCastle.h"
#include "ChaosCastle.h"
#include "util.h"
#include "readscript.h"
#include "SetItemOption.h"
#include "ObjectManager.h"
#include "Item.h"
#include "Monster.h"
#include "MuunSystem.h"
#include "Trade.h"
#include "DSProtocol.h"
#include "ServerInfo.h"
#include "Notice.h"
#include "Message.h"
#include "GensSystem.h"
#include "MemScript.h"

#if (BOT_ALCHEMIST == 1)

ObjBotAlchemist BotAlchemist;

void ObjBotAlchemist::Read(char * FilePath)
{
	if(this->Enabled == 1)
	{
		for(int botNum=0;botNum<MAX_BOTALCHEMIST;botNum++)
		{
			if(this->bot[botNum].Enabled == 1)
			{
				int bIndex = this->bot[botNum].index;
				if(gObjIsConnected(bIndex) == 1)
				{
					gObjDel(bIndex);
				}
			}
		}
	}

	this->Enabled = 0;

	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,FilePath);
		return;
	}

	if(lpMemScript->SetBuffer(FilePath) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

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
				if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					ErrorMessageBox("BotAlchemist error 1: BotAlchemistIndex: %d out of range!", BotNum);
					return;
				}

				this->bot[BotNum].Class = lpMemScript->GetAsNumber();
				this->bot[BotNum].Enabled = lpMemScript->GetAsNumber();
				this->bot[BotNum].ChangeColorName = lpMemScript->GetAsNumber();
				this->bot[BotNum].Rate = lpMemScript->GetAsNumber();
				strncpy_s(this->bot[BotNum].Name,lpMemScript->GetAsString(),sizeof(this->bot[BotNum].Name));			
				this->bot[BotNum].Map = lpMemScript->GetAsNumber();
				this->bot[BotNum].X = lpMemScript->GetAsNumber();
				this->bot[BotNum].Y = lpMemScript->GetAsNumber();
				this->bot[BotNum].Dir = lpMemScript->GetAsNumber();

				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					ErrorMessageBox("BotAlchemist error 2: BotAlchemistIndex: %d out of range!", BotNum);
					return;
				}

				this->bot[BotNum].OnlyVip = lpMemScript->GetAsNumber();
				this->bot[BotNum].ActiveGensFamily = lpMemScript->GetAsNumber();
				this->bot[BotNum].ContributionGens = lpMemScript->GetAsNumber();
				this->bot[BotNum].GensFamily = lpMemScript->GetAsNumber();
				this->bot[BotNum].Money = lpMemScript->GetAsNumber();
				this->bot[BotNum].PCPoint = lpMemScript->GetAsNumber();

				}

				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					LogAdd(LOG_RED,"BotAlchemist error 3: BotAlchemistIndex: %d doesnt exist", BotNum);
					return;
				}

				int Slot = lpMemScript->GetAsNumber();
				
				if(Slot < 0 || Slot > 8)
				{	
					ErrorMessageBox("BotAlchemist error: Min Slot 0 ; Max Slot 8");
					return;
				}

				int iType = lpMemScript->GetAsNumber();
				int iIndex = lpMemScript->GetAsNumber();

				this->bot[BotNum].body[Slot].num = GET_ITEM(iType,iIndex);
				this->bot[BotNum].body[Slot].level = lpMemScript->GetAsNumber();
				this->bot[BotNum].body[Slot].opt = lpMemScript->GetAsNumber();
				this->bot[BotNum].body[Slot].Enabled = 1;

				}

				else if(section == 4)
				{
					this->Enabled = 1;
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					LogAdd(LOG_RED,"BotAlchemist error 4: BotBufferIndex: %d doesnt exist", BotNum);
					return;
				}

				this->bot[BotNum].AllowLevel = lpMemScript->GetAsNumber();
				this->bot[BotNum].AllowOpt = lpMemScript->GetAsNumber();
				this->bot[BotNum].AllowLuck = lpMemScript->GetAsNumber();
				this->bot[BotNum].AllowSkill = lpMemScript->GetAsNumber();
				this->bot[BotNum].AllowExc = lpMemScript->GetAsNumber();
				this->bot[BotNum].AllowFFFF = lpMemScript->GetAsNumber();


				}

				else if(section == 5)
				{
					this->Enabled = 1;
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					LogAdd(LOG_RED,"BotAlchemist error 5: BotBufferIndex: %d doesnt exist", BotNum);
					return;
				}

				this->bot[BotNum].OnlySameType = lpMemScript->GetAsNumber();
				this->bot[BotNum].OnlyLowerIndex = lpMemScript->GetAsNumber();
				this->bot[BotNum].AcceptAncient = lpMemScript->GetAsNumber();
				this->bot[BotNum].MaxLevel = lpMemScript->GetAsNumber();
				this->bot[BotNum].MaxExc = lpMemScript->GetAsNumber();
				
				}
				else
				{
					break;
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

void ObjBotAlchemist::MakeBot()
{
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTALCHEMIST;botNum++)
		{
			if(this->bot[botNum].Enabled == true)
			{
				CItem item;

				int result = gObjAddSummon();

				if(result >= 0)
				{
					this->bot[botNum].index = result;
					
					gObj[result].PosNum = (WORD)-1;
					gObj[result].X = this->bot[botNum].X;
					gObj[result].Y = this->bot[botNum].Y;
					gObj[result].OldX = this->bot[botNum].X;
					gObj[result].OldY = this->bot[botNum].Y;
					gObj[result].TX = this->bot[botNum].X;
					gObj[result].TY = this->bot[botNum].Y;
					gObj[result].MTX = this->bot[botNum].X;
					gObj[result].MTY = this->bot[botNum].Y;
					gObj[result].Dir = this->bot[botNum].Dir;
					gObj[result].Map = this->bot[botNum].Map;
					gObj[result].Live = TRUE;
					gObj[result].PathCount = 0;
					gObj[result].IsBot = 2;
					gObj[result].BotPower = 0;
					gObjSetBots(result,this->bot[botNum].Class);				

					//gObj[result].ChangeUp = this->bot.Class & 0x07;
					gObj[result].ChangeUp = this->bot[botNum].Class;
					gObj[result].Class = this->bot[botNum].Class;
					gObj[result].Level = 400;
					gObj[result].Life = 1;
					gObj[result].MaxLife = 2;
					gObj[result].Mana = 1;
					gObj[result].MaxMana = 2;
					gObj[result].Experience = 0;
					gObj[result].DBClass = this->bot[botNum].Class;
					gObj[result].PKLevel = this->bot[botNum].ChangeColorName;
					gObj[result].GensFamily = this->bot[botNum].GensFamily;
					#if(GAMESERVER_UPDATE>=802)
					gObj[result].LoadMuunInventory = 1;
					#endif

					gObj[result].Inventory = new CItem[MAIN_INVENTORY_NORMAL_SIZE];					
					gObj[result].Inventory1 = new CItem[MAIN_INVENTORY_NORMAL_SIZE];
					gObj[result].InventoryMap1 = new BYTE[MAIN_INVENTORY_NORMAL_SIZE];
					gObj[result].InventoryMap = new BYTE[MAIN_INVENTORY_NORMAL_SIZE];
					
					#if(GAMESERVER_UPDATE>=802)
					gObj[result].MuunInventory = new CItem[MAIN_INVENTORY_NORMAL_SIZE+ (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE]; //+ (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE
					gObj[result].MuunInventoryMap = new BYTE[MAIN_INVENTORY_NORMAL_SIZE+ (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE]; // + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE
					#endif

					for (int i=0;i<MAIN_INVENTORY_NORMAL_SIZE;i++)
					{
						gObj[result].Inventory[i].Clear();
						gObj[result].Inventory1[i].Clear();
						#if(GAMESERVER_UPDATE>=802)
						gObj[result].MuunInventory[i].Clear();
						#endif

					}
			
					memset(&gObj[result].InventoryMap[0], (BYTE)-1, MAIN_INVENTORY_NORMAL_SIZE);
					memset(&gObj[result].InventoryMap1[0], (BYTE)-1, MAIN_INVENTORY_NORMAL_SIZE);
					#if(GAMESERVER_UPDATE>=802)
					memset(&gObj[result].MuunInventoryMap[0], (BYTE)-1, MUUN_INVENTORY_SIZE);
					#endif

					strncpy(gObj[result].Name,this->bot[botNum].Name,sizeof(gObj[result].Name));
					
					
					for(int i=0;i<9;i++)
					{
						if(this->bot[botNum].body[i].num >= 0 && this->bot[botNum].body[i].Enabled == true)
						{
							CItem item;
							item.m_Level = this->bot[botNum].body[i].level;
							item.m_Option1 = 0;
							item.m_Option2 = 1;
							item.m_Option3 = this->bot[botNum].body[i].opt;
							item.m_Durability = 255.0f;
							item.m_JewelOfHarmonyOption = 0;
							item.m_ItemOptionEx = 0;
							item.m_SocketOption[1] = 0;
							item.m_SocketOption[2] = 0;
							item.m_SocketOption[3] = 0;
							item.m_SocketOption[4] = 0;
							item.m_SocketOption[5] = 0;
							item.Convert(this->bot[botNum].body[i].num,item.m_Option1,item.m_Option2,item.m_Option3,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus);
							

							gObj[result].Inventory[i].m_Option1 = item.m_Option1;
							gObj[result].Inventory[i].m_Option2 = item.m_Option2;
							gObj[result].Inventory[i].m_Option3 = item.m_Option3;
							gObj[result].Inventory[i].m_JewelOfHarmonyOption = item.m_JewelOfHarmonyOption;
							gObj[result].Inventory[i].m_ItemOptionEx = item.m_ItemOptionEx;

							item.m_Number = 0;
							gObjInventoryInsertItemPos(gObj[result].Index,item,i,0);
							
						}
					}
					
					gObj[result].Inventory1 = gObj[result].Inventory;
					gObj[result].InventoryMap1 = gObj[result].InventoryMap;
					#if(GAMESERVER_UPDATE>=802)
					gObj[result].MuunInventory = gObj[result].MuunInventory;
					gObj[result].MuunInventoryMap = gObj[result].MuunInventoryMap;
					#endif

					gObjectManager.CharacterMakePreviewCharSet(result);

					gObj[result].AttackType = 0;
					gObj[result].BotSkillAttack = 0;					

					gObj[result].Attribute = 100;
					gObj[result].TargetNumber = (WORD)-1;
					gObj[result].ActionState.Emotion = 0;
					gObj[result].ActionState.Attack = 0;
					gObj[result].ActionState.EmotionCount = 0;
					gObj[result].PathCount = 0;
					gObj[result].BotPower = 0;
					gObj[result].BotDefense = this->bot[botNum].OnlyVip;
					gObj[result].BotLife = 1;
					gObj[result].BotMaxLife = 1;
					
					gObj[result].BotLvlUpDefense = 1;
					gObj[result].BotLvlUpPower = 1;
					gObj[result].BotLvlUpLife = 1;
					gObj[result].BotLvlUpMana = 1;
					gObj[result].BotLvlUpExp = 1;
					gObj[result].BotLvlUpMaxLevel = 1;

					gObj[result].MoveRange = 1;
					gObj[result].BotFollowMe = 0;
					gObj[result].NextExperience = gLevelExperience[gObj[result].Level];
					
					GCSummonLifeSend(gObj[result].SummonIndex,gObj[result].MaxLife,gObj[result].Life);

				}
			}
		}
	}
}

BYTE ObjBotAlchemist::gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck) //OK
{

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return -1;
	}

	LPOBJ lpObj;

	int useClass = 0;

	lpObj = &gObj[aIndex];

	if(lpObj->Inventory[pos].IsItem() == 1)
	{
		return -1;
	}

	if(item.IsItem() == 0)
	{
		return -1;
	}

	if(pos < 12)
	{
		if(lpObj->IsBot == 0)
		{
			useClass = item.IsClassBot((char)lpObj->Class,lpObj->ChangeUp);

			if(useClass == 0)
			{
				LogAdd(LOG_BLUE,"[Using Class Error] Error UseClass %d",useClass);
				return -1;
			}

		}
	}

	lpObj->Inventory[pos] = item;

	return pos;
}

BOOL ObjBotAlchemist::IsInTrade(int BotIndex)
{
	int bIndex = this->GetBotIndex(gObj[BotIndex].TargetNumber);
	if(bIndex != -1)
		return 1;
	else
		return 0;
		
}

int ObjBotAlchemist::GetBotIndex(int aIndex)
{
	for(int i=0;i<MAX_BOTALCHEMIST;i++)
	{
		if(this->bot[i].Enabled == true)
		{
			if(this->bot[i].index == aIndex)
			{
				return i;
			}
		}
	}
	return -1;
}

bool ObjBotAlchemist::AllowExc(BYTE BotNum, BYTE ExcOpt)
{				
	int dwExOpCount = gSetItemOption.GetSetItemMaxOptionCount(ExcOpt);

	if(dwExOpCount < this->bot[BotNum].MaxExc)
	{
		return true;
	}
	return false;
}

BYTE ObjBotAlchemist::Alchemy(int aIndex,int BotNum)
{
	int fitem=-1;
	int sitem=-1;
	int count=0;

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return 0;
	}

	if((gObj[aIndex].TradeMoney - this->bot[BotNum].Money) < 0)
	{
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : Insuficent Money! %d ",this->bot[BotNum].Name,this->bot[BotNum].Money);
		LogAdd(LOG_RED,"[%s] - (%s) Insuficent Money",this->bot[BotNum].Name,gObj[aIndex].Name);
		goto Cancel;		
	}

	if((gObj[aIndex].PCPoint - this->bot[BotNum].PCPoint) < 0)
	{
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : Insuficent PCPoints! %d ",this->bot[BotNum].Name,this->bot[BotNum].PCPoint);
		LogAdd(LOG_RED,"[%s] - (%s) Insuficent PCPoints",this->bot[BotNum].Name,gObj[aIndex].Name);
		goto Cancel;		
	}

	for(int n = 0; n < TRADE_SIZE; n++)
	{
		if(gObj[aIndex].Trade[n].IsItem() == 1)
		{
			if((n >= 0 && n<=3) || (n >= 8 && n<=11) || (n >= 16 && n<=19) || (n >= 24 && n<=27))
			{
				fitem = n;
			}else
			{
				sitem = n;
			}
			count++;
		}
	}
	if(count != 2)
	{
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : You must put only 2 items!",this->bot[BotNum].Name);
		LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Items Count Error (%d)",gObj[aIndex].Name,gObj[aIndex].Name,count);
		goto Cancel;
	}
	if((gObj[aIndex].Trade[fitem].m_Index >= 6144) || (gObj[aIndex].Trade[sitem].m_Index >= 6144))
	{
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : Only weapons and sets are allowed!!",this->bot[BotNum].Name);
		LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Items Error",gObj[aIndex].Account,gObj[aIndex].Name);
		goto Cancel;
	}
	if(fitem == -1 || sitem == -1)
	{
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : Items Position Error!",this->bot[BotNum].Name);
		LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Items Position Error",gObj[aIndex].Account,gObj[aIndex].Name);
		goto Cancel;
	}
	if(this->bot[BotNum].OnlySameType == 1)
	{
		int fType = gObj[aIndex].Trade[fitem].m_Index/512;
		int sType = gObj[aIndex].Trade[sitem].m_Index/512;
		if(fType != sType)
		{
			gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : You must put two items of same type!",this->bot[BotNum].Name);
			LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Item Types Error",gObj[aIndex].Account,gObj[aIndex].Name);
			goto Cancel;
		}
	}
	if(this->bot[BotNum].OnlyLowerIndex == 1)
	{
		int fLevel = ItemAttribute[gObj[aIndex].Trade[fitem].m_Index].Level;
		int sLevel = ItemAttribute[gObj[aIndex].Trade[sitem].m_Index].Level;

		if(fLevel > sLevel)
		{
			if(fLevel > (sLevel + 10))
			{
				gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : Only lower items than source works with my alchemy!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
				LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Item Destiny Lower Index Error",gObj[aIndex].Account,gObj[aIndex].Name);
				goto Cancel;
			}
		}
	}
	if(this->bot[BotNum].AcceptAncient == 0 && gObj[aIndex].Trade[fitem].m_SetOption > 0)
	{
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : I cant work with Ancient Items!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
		LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Ancient Item Error",gObj[aIndex].Account,gObj[aIndex].Name);
		goto Cancel;
	}
	if(this->bot[BotNum].AllowFFFF == 0)
	{
		if(gObj[aIndex].Trade[fitem].GetNumber() >= 0xFFFFFFF0 || gObj[aIndex].Trade[sitem].GetNumber() >= 0xFFFFFFF0)
		{
			gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : You cant transmutate those items!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
			LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) FFFF Item Error",gObj[aIndex].Account,gObj[aIndex].Name);
			goto Cancel;
		}
	}
	srand(static_cast<int>(time(NULL)));
	int random = rand()%100;
	bool failed = false;

	if(random > this->bot[BotNum].Rate )
	{
		failed = true;
	}
	if(this->bot[BotNum].AllowLuck == 1)
	{
		if(gObj[aIndex].Trade[fitem].m_Option2 == 0 && gObj[aIndex].Trade[sitem].m_Option2 == 1)
		{
			if(!failed)
				gObj[aIndex].Trade[fitem].m_Option2 = 1;
			gObj[aIndex].Trade[sitem].m_Option2 = 0;
		}
	}
	if(this->bot[BotNum].AllowLevel == 1)
	{
		int fLevel = gObj[aIndex].Trade[fitem].m_Level;
		if(fLevel < this->bot[BotNum].MaxLevel)
		{
			int sLevel = gObj[aIndex].Trade[sitem].m_Level;
			if ((fLevel + sLevel) > this->bot[BotNum].MaxLevel)
			{
				sLevel -= (this->bot[BotNum].MaxLevel - fLevel);
				if(!failed)
					fLevel = this->bot[BotNum].MaxLevel;
			}else
			{
				if(!failed)
					fLevel += sLevel;
				sLevel = 0;
			}
			gObj[aIndex].Trade[fitem].m_Level = fLevel;
			gObj[aIndex].Trade[sitem].m_Level = sLevel;
		}
	}
	if(this->bot[BotNum].AllowSkill == 1)
	{
		if(gObj[aIndex].Trade[fitem].m_Index < 3584)
		{
			if(gObj[aIndex].Trade[fitem].m_Option1 == 0 && gObj[aIndex].Trade[sitem].m_Option1 == 1)
			{
				if(!failed)
					gObj[aIndex].Trade[fitem].m_Option1 = 1;
				gObj[aIndex].Trade[sitem].m_Option1 = 0;
			}
		}
	}
	if(this->bot[BotNum].AllowOpt == 1)
	{
		int fOpt = gObj[aIndex].Trade[fitem].m_Option3;
		if(fOpt < 7)
		{
			int sOpt = gObj[aIndex].Trade[sitem].m_Option3;
			if ((fOpt + sOpt) > 7)
			{
				sOpt -= (7 - fOpt);
				if(!failed)
					fOpt = 7;
			}else
			{
				if(!failed)
					fOpt += sOpt;
				sOpt = 0;
			}
			gObj[aIndex].Trade[fitem].m_Option3 = fOpt;
			gObj[aIndex].Trade[sitem].m_Option3 = sOpt;
		}
	}
	if(this->bot[BotNum].AllowExc == 1)
	{
		if(gObj[aIndex].Trade[fitem].m_NewOption != 63)
		{
			if(gObj[aIndex].Trade[sitem].m_NewOption != 0)
			{		
				if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
				{
					BYTE fLife		= gObj[aIndex].Trade[fitem].m_NewOption & 0x20;//IsExtLifeAdd();
					BYTE fMana		= gObj[aIndex].Trade[fitem].m_NewOption & 0x10;//IsExtManaAdd();
					BYTE fDmg		= gObj[aIndex].Trade[fitem].m_NewOption & 8;//IsExtDamageMinus()
					BYTE fReflect	= gObj[aIndex].Trade[fitem].m_NewOption & 4;//IsExtDamageReflect()
					BYTE fDef		= gObj[aIndex].Trade[fitem].m_NewOption & 2;//IsExtDefenseSuccessfull()
					BYTE fMoney		= gObj[aIndex].Trade[fitem].m_NewOption & 1;//IsExtMonsterMoney()
					BYTE sLife		= gObj[aIndex].Trade[sitem].m_NewOption & 0x20;//IsExtLifeAdd()
					BYTE sMana		= gObj[aIndex].Trade[sitem].m_NewOption & 0x10;;//IsExtManaAdd()
					BYTE sDmg		= gObj[aIndex].Trade[sitem].m_NewOption & 8;//IsExtDamageMinus()
					BYTE sReflect	= gObj[aIndex].Trade[sitem].m_NewOption & 4;//IsExtDamageReflect()
					BYTE sDef		= gObj[aIndex].Trade[sitem].m_NewOption & 2;//IsExtDefenseSuccessfull()
					BYTE sMoney		= gObj[aIndex].Trade[sitem].m_NewOption & 1;//IsExtMonsterMoney()

					if(fLife == 0 && sLife != 0)
					{
						if(!failed)
							gObj[aIndex].Trade[fitem].m_NewOption |= 0x20;
						gObj[aIndex].Trade[sitem].m_NewOption &= ~0x20;
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fMana == 0 && sMana != 0)
						{
							if(!failed)
								gObj[aIndex].Trade[fitem].m_NewOption |= 0x10;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~0x10;
						}
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fDmg == 0 && sDmg != 0)
						{
							if(!failed)
								gObj[aIndex].Trade[fitem].m_NewOption |= 8;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~8;
						}
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fReflect == 0 && sReflect != 0)
						{
							if(!failed)
								gObj[aIndex].Trade[fitem].m_NewOption |= 4;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~4;
						}
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fDef == 0 && sDef != 0)
						{
							if(!failed)
								gObj[aIndex].Trade[fitem].m_NewOption |= 2;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~2;
						}
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fMoney == 0 && sMoney != 0)
						{
							if(!failed)
							{
								gObj[aIndex].Trade[fitem].m_NewOption |= 1;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~1;
							}
						}
					}
				}
			}
		}
	}
	int a = gItemManager.InventoryInsertItem(aIndex,gObj[aIndex].Trade[fitem]);
	int b = gItemManager.InventoryInsertItem(aIndex,gObj[aIndex].Trade[sitem]);

	if(a == 255 || b == 255)
	{
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : Insuficent space in inventory!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
		LogAdd(LOG_RED,"[BotAlchemist] Account: %s - Name: %s Doesnt have space on inventory",gObj[aIndex].Account,gObj[aIndex].Name);
		goto Cancel;
	}
	if(failed)
	{
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : Transmutation failed!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
		LogAdd(LOG_RED,"[BotAlchemist] - [Bot: %d] Name: %s Alchemy Failed",this->bot[BotNum].Name,gObj[aIndex].Name);
	}
	else
	{ 
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s : Transmutation success!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
		LogAdd(LOG_BOT,"[BotAlchemist] - [Bot: %s] Name: %s Alchemy Success",this->bot[BotNum].Name,gObj[aIndex].Name);
	}

	gObj[aIndex].Money -= this->bot[BotNum].Money;

	if(this->bot[BotNum].PCPoint > 0)
	{
		gObj[aIndex].PCPoint -= this->bot[BotNum].PCPoint;
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"-%d PCPoint: %d",this->bot[BotNum].PCPoint,lpObj->PCPoint); 
	}

	gObjInventoryCommit(aIndex);
	gObjectManager.CharacterMakePreviewCharSet(aIndex);
	GDCharacterInfoSaveSend(aIndex); 
	
	gItemManager.GCItemEquipmentSend(aIndex);
	
	lpObj->TargetNumber = -1;
	lpObj->Interface.use = 0;
	lpObj->Interface.state = 0;
	lpObj->TradeOk = 1;
	lpObj->TradeMoney = 1;
	GCMoneySend(aIndex,gObj[aIndex].Money);
	gTrade.GCTradeResultSend(aIndex,1);
	gItemManager.GCItemListSend(aIndex);

	for(int n = 0; n < TRADE_SIZE; n++)
	{
		gObj[aIndex].Trade[n].Clear();
	}
	gObjNotifyUpdateUnionV1(&gObj[aIndex]);
	gObjNotifyUpdateUnionV2(&gObj[aIndex]);
	return 1;
Cancel:
	gTrade.ResetTrade(aIndex);	
	gTrade.GCTradeResultSend(aIndex,3);
	
	return 0;
}

void ObjBotAlchemist::TradeOk(int aIndex)
{
	int MixNum=-1;
	int MixItem=-1;

	int number = this->GetBotIndex(gObj[aIndex].TargetNumber);

	if(number == -1)
	{	
		gObj[aIndex].Interface.state = 0;
		gObj[aIndex].Interface.type = 0;
		gObj[aIndex].TargetShopNumber = OBJECT_BOTS;
		gObj[aIndex].Transaction = 0;
		gObj[aIndex].TradeOkTime = GetTickCount();
		gObj[aIndex].TradeMoney = 1;
		gObj[aIndex].TradeOk = 1;

		return;
	}
	this->Alchemy(aIndex,number);
}

BOOL ObjBotAlchemist::TradeOpen(int index, int nindex)
{
	if(gObjIsConnectedGP(index) == 0)
	{
		return 0;
	}

	if(gObjIsConnectedGP(nindex) == 0)
	{

		return 0;
	}

	int number = this->GetBotIndex(nindex);

	if(number == -1)
		return 0;

	LPOBJ lpObj = &gObj[index];
	LPOBJ lpBot = &gObj[nindex];

	if(this->bot[number].OnlyVip != 0 && lpObj->AccountLevel == 0)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s : Im Only work for Account Vips!",gObj[nindex].Name);
		LogAdd(LOG_RED,"[%s] AccountLevel Free ( Account: %s Char: %s )",this->bot[number].Name,lpObj->Account,lpObj->Name);
		return 1;
	}

	if(this->bot[number].PCPoint > gObj[index].PCPoint)
	{
		gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,"%s : Insuficent PCPoints : %d",this->bot[number].Name,this->bot[number].PCPoint);
		LogAdd(LOG_RED,"[%s] Account: %s - Insuficent PCPoints : %d",this->bot[number].Name,gObj[index].Account,this->bot[number].PCPoint);
		return 1;		
	}
	if(this->bot[number].ActiveGensFamily == 1)
	{
		if(this->bot[number].GensFamily != GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_NONE )
		{
			gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,"%s : You Need be GensFamily Vanert",this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily != GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_NONE)
		{
			gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,"%s : You Need be GensFamily Duprian",this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily == GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_DUPRIAN)
		{
			gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,"%s : You Need be GensFamily Duprian",this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily == GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_VARNERT)
		{
			gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,"%s : You Need be GensFamily Vanert",this->bot[number].Name);
			return 1;
		}
		if(this->bot[number].ContributionGens > gObj[index].GensContribution)
		{
			gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,"%s : Insuficent GensContribution : %d",this->bot[number].Name,this->bot[number].ContributionGens);
			LogAdd(LOG_RED,"[%s] Account: %s - Insuficent GensContribution : %d",this->bot[number].Name,gObj[index].Account,this->bot[number].ContributionGens);
			return 1;
		}
	}

	if(this->Enabled == TRUE)
	{
		if ( lpObj->Interface.use > 0 )
		{
			return 0;
		}else
		{
			for(int n = 0; n < TRADE_SIZE; n++)
			{
				lpObj->Trade[n].Clear();
			}
			memset(lpObj->TradeMap, (BYTE)-1, TRADE_SIZE );
	
			gObjInventoryTransaction(lpObj->Index);
			gTrade.GCTradeResponseSendBOT(true, lpObj->Index, lpBot->Name, 400, 0);//GCTradeResponseSendBOT
			gTrade.GCTradeOkButtonSend(lpObj->Index, 1);
			lpObj->Interface.state = 1;
			lpObj->Interface.use = 1;
			lpObj->Interface.type = 1;
			lpObj->TradeMoney = 1;
			lpObj->TargetNumber = lpBot->Index;
			lpObj->Transaction = 1;
			
			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s : I'm Ready: %s",gObj[nindex].Name,gObj[index].Name);
			LogAdd(LOG_BOT,"[Alchemist] [BotName: %s] CharName: %s OPEN",gObj[nindex].Name,gObj[index].Name);
			
			if(this->bot[number].Money > 0)
			{
				gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s : I Need: %d Money!",gObj[nindex].Name,this->bot[number].Money);
			}

			if(this->bot[number].PCPoint > 0)
			{
				gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s : I Need: %d PCPoints!",gObj[nindex].Name,this->bot[number].PCPoint);
			}
			
		}
	}
	return 1;
}

void ObjBotAlchemist::TradeCancel(int aIndex)
{
	gTrade.ResetTrade(aIndex);	
	gTrade.GCTradeResultSend(aIndex,3);
}

#endif

