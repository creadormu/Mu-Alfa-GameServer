#include "stdafx.h"
#include "Util.h"
#include "ItemManager.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "MemScript.h"
#include "RandomManager.h"
#include "CustomMix.h"
#include "Log.h"


CCustomMix gCustomMix;

void CCustomMix::Load(char* path){

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

	int count1 = 0;
	int count2 = 0;
	int count3 = 0;

	this->m_Data_Mix.clear();
	this->m_Data_Bag.clear();
	this->m_Data_Item.clear();

	for(int i=0;i<254;i++)
	{
		this->DataResult[i].MixNum = -1;
		this->DataResult[i].m_DataResult.clear();
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
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_MIX info;

					memset(&info,0,sizeof(info));

					info.m_Index = lpMemScript->GetNumber();

					info.m_MixMoney = lpMemScript->GetAsNumber();

					info.m_MixRate_AL0 = lpMemScript->GetAsNumber();

					info.m_MixRate_AL1 = lpMemScript->GetAsNumber();

					info.m_MixRate_AL2 = lpMemScript->GetAsNumber();

					info.m_MixRate_AL3 = lpMemScript->GetAsNumber();

					info.m_CountGroup = lpMemScript->GetAsNumber();

					info.m_CountItem = lpMemScript->GetAsNumber();

					info.m_OpcionExeRand = lpMemScript->GetAsNumber();

					this->m_Data_Mix.insert(std::pair<int,CUSTOM_MIX>(info.m_Index, info));
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_BAG_MIX info1;

					memset(&info1,0,sizeof(info1));
					
					info1.index = count1++;

					info1.m_Index = lpMemScript->GetNumber();

					info1.m_ItemMix = lpMemScript->GetAsNumber();

					info1.m_ItemLevel = lpMemScript->GetAsNumber();

					info1.m_Count = lpMemScript->GetAsNumber();

					this->m_Data_Bag.insert(std::pair<int,CUSTOM_BAG_MIX>(info1.index, info1));

					
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					CUSTOM_ITEM_MIX info2;

					memset(&info2,0,sizeof(info2));

					info2.index = count2++;

					info2.m_Index = lpMemScript->GetNumber();

					info2.m_Group = lpMemScript->GetAsNumber();
					
					info2.m_Count = lpMemScript->GetAsNumber();

					info2.m_ItemIndexMin = lpMemScript->GetAsNumber();

					info2.m_ItemIndexMax = lpMemScript->GetAsNumber();

					info2.m_ItemLevel = lpMemScript->GetAsNumber();

					info2.m_Skill = lpMemScript->GetAsNumber();

					info2.m_Luck = lpMemScript->GetAsNumber();

					info2.m_Opcion = lpMemScript->GetAsNumber();
					
					info2.m_Excelent = lpMemScript->GetAsNumber();

					info2.m_SetACC = lpMemScript->GetAsNumber();

					this->m_Data_Item.insert(std::pair<int, CUSTOM_ITEM_MIX>(info2.index, info2));
				}
				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_ITEM_MIX_RESULT info3;

					memset(&info3,0,sizeof(info3));

					info3.index = count3++;

					info3.m_Index = lpMemScript->GetNumber();

					info3.m_Group = lpMemScript->GetAsNumber();

					info3.m_ItemIndexMin = lpMemScript->GetAsNumber();

					info3.m_ItemIndexMax = lpMemScript->GetAsNumber();

					info3.m_ItemLevel = lpMemScript->GetAsNumber();

					info3.m_Skill = lpMemScript->GetAsNumber();

					info3.m_Luck = lpMemScript->GetAsNumber();

					info3.m_Opcion = lpMemScript->GetAsNumber();
					
					info3.m_Excelent = lpMemScript->GetAsNumber();

					info3.m_SetACC = lpMemScript->GetAsNumber();
					
					info3.Socket1 = lpMemScript->GetAsNumber();
					
					info3.Socket2 = lpMemScript->GetAsNumber();
					
					info3.Socket3 = lpMemScript->GetAsNumber();
					
					info3.Socket4 = lpMemScript->GetAsNumber();
					
					info3.Socket5 = lpMemScript->GetAsNumber();

					this->DataResult[info3.m_Index].m_DataResult.insert(std::pair<int, CUSTOM_ITEM_MIX_RESULT>(info3.m_Group, info3));

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

//---
CUSTOM_MIX * CCustomMix::GetCustomMix(int IndexMix){

	CUSTOM_MIX * value;

	std::map<int, CUSTOM_MIX>::iterator it = this->m_Data_Mix.find(IndexMix);

	if(it != this->m_Data_Mix.end())
	{
		value = &it->second;

		return value;
	}

	return 0;
}

bool CCustomMix::istItemBagMix(int IndexMix, CItem boxItem){
	
	for(std::map<int,CUSTOM_BAG_MIX>::iterator it = this->m_Data_Bag.begin();it != this->m_Data_Bag.end();it++)
	{
		if(it->second.m_Index == IndexMix && it->second.m_ItemMix == boxItem.m_Index
			&& it->second.m_ItemLevel == boxItem.m_Level)
		{
			return true;
		}
	}

	return false;
}
//--
bool CCustomMix::istItemMix(int IndexMix, int group, CItem boxItem){

	for(std::map<int, CUSTOM_ITEM_MIX>::iterator it = this->m_Data_Item.begin(); it != this->m_Data_Item.end(); it++)
	{
		if(it->second.m_Index == IndexMix && it->second.m_Group == group && boxItem.m_Index >= it->second.m_ItemIndexMin && boxItem.m_Index <= it->second.m_ItemIndexMax)
		{

			if(it->second.m_ItemLevel != -1 && it->second.m_SetACC != 0)
			{
				if(boxItem.m_Level < it->second.m_ItemLevel)
				{
					return false;
				}
			}

			if(it->second.m_Skill != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_Skill != boxItem.m_Option1)
				{
					return false;
				}
			}

			if(it->second.m_Luck != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_Luck != boxItem.m_Option2)
				{
					return false;
				}
			}
			if(it->second.m_Opcion != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_Opcion > boxItem.m_Option3)
				{
					return false;
				}
			}

			if(it->second.m_Excelent != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_Excelent != boxItem.m_NewOption)
				{
					return false;
				}
			}

			if(it->second.m_SetACC != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_SetACC != boxItem.m_SetOption)
				{
					return false;
				}
			}

			return true;
		}
	}

	return false;
}

int CCustomMix::GetCountItemBagMix(int IndexMix){

	int itemcount = 0;

	for(std::map<int,CUSTOM_BAG_MIX>::iterator it = this->m_Data_Bag.begin();it != this->m_Data_Bag.end();it++)
	{
		if( it->second.m_Index == IndexMix )
		{
			itemcount += it->second.m_Count;
		}
	}
	return itemcount;
}

int CCustomMix::GetCountItemMix(int IndexMix,int Group){

	int itemcount = 0;

	for(std::map<int,CUSTOM_ITEM_MIX>::iterator it = this->m_Data_Item.begin();it != this->m_Data_Item.end();it++)
	{
		if(it->second.m_Index == IndexMix && it->second.m_Group == Group)
		{
			itemcount += it->second.m_Count;
		}	
	}

	return itemcount;
}

CRandomManager CCustomMix::RandomManager(int IndexMix, int group)
{
	CRandomManager RandomManager;

	std::map<int, CUSTOM_ITEM_MIX_RESULT>::iterator it = this->DataResult[IndexMix].m_DataResult.find(group);

	if(it != this->DataResult[IndexMix].m_DataResult.end())
	{
		for(int n = it->second.m_ItemIndexMin; n <= it->second.m_ItemIndexMax ; n++)
		{

			RandomManager.AddElement(n, 1);
		}
	}
			
	return RandomManager;
}
//---
CUSTOM_ITEM_MIX_RESULT* CCustomMix::GetItemResult(int IndexMix, int group, int ItemIndex)
{
	std::map<int, CUSTOM_ITEM_MIX_RESULT>::iterator it = this->DataResult[IndexMix].m_DataResult.find(group);

	if(it != this->DataResult[IndexMix].m_DataResult.end())
	{
		if(ItemIndex >= it->second.m_ItemIndexMin && ItemIndex <= it->second.m_ItemIndexMax)
		{
			return &it->second;
		}
	}

	return 0;
}