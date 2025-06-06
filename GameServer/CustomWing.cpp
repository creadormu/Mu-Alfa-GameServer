// CustomWing.cpp: implementation of the CCustomWing class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomWing.h"
#include "MemScript.h"
#include "Util.h"
#include "User.h"
#include "ItemManager.h"
#include "Notice.h"

CCustomWing gCustomWing;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomWing::CCustomWing() // OK
{
	this->Init();
}

CCustomWing::~CCustomWing() // OK
{

}

void CCustomWing::Init() // OK
{
	for(int n=0;n < MAX_CUSTOM_WING;n++)
	{
		this->m_CustomWingInfo[n].Index = -1;
	}
}

void CCustomWing::Load(char* path) // OK
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

			CUSTOM_WING_INFO info;

			memset(&info,0,sizeof(info));

			info.Index = lpMemScript->GetNumber();

			info.ItemIndex = lpMemScript->GetAsNumber();

			info.DefenseConstA = lpMemScript->GetAsNumber();

			info.IncDamageConstA = lpMemScript->GetAsNumber();

			info.IncDamageConstB = lpMemScript->GetAsNumber();

			info.DecDamageConstA = lpMemScript->GetAsNumber();

			info.DecDamageConstB = lpMemScript->GetAsNumber();

			info.OptionIndex1 = lpMemScript->GetAsNumber();

			info.OptionValue1 = lpMemScript->GetAsNumber();

			info.OptionIndex2 = lpMemScript->GetAsNumber();

			info.OptionValue2 = lpMemScript->GetAsNumber();

			info.OptionIndex3 = lpMemScript->GetAsNumber();

			info.OptionValue3 = lpMemScript->GetAsNumber();

			info.NewOptionIndex1 = lpMemScript->GetAsNumber();

			info.NewOptionValue1 = lpMemScript->GetAsNumber();

			info.NewOptionIndex2 = lpMemScript->GetAsNumber();

			info.NewOptionValue2 = lpMemScript->GetAsNumber();

			info.NewOptionIndex3 = lpMemScript->GetAsNumber();

			info.NewOptionValue3 = lpMemScript->GetAsNumber();

			info.NewOptionIndex4 = lpMemScript->GetAsNumber();

			info.NewOptionValue4 = lpMemScript->GetAsNumber();

			info.ModelType = lpMemScript->GetAsNumber();

			strcpy_s(info.ModelName,lpMemScript->GetAsString());

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CCustomWing::SetInfo(CUSTOM_WING_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_WING)
	{
		return;
	}

	this->m_CustomWingInfo[info.Index] = info;
}

CUSTOM_WING_INFO* CCustomWing::GetInfo(int index) // OK
{
	if(index < 0 || index >= MAX_CUSTOM_WING)
	{
		return 0;
	}

	if(this->m_CustomWingInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_CustomWingInfo[index];
}

CUSTOM_WING_INFO* CCustomWing::GetInfoByItem(int ItemIndex) // OK
{
	for(int n=0;n < MAX_CUSTOM_WING;n++)
	{
		CUSTOM_WING_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->ItemIndex == ItemIndex)
		{
			return lpInfo;
		}
	}

	return 0;
}

bool CCustomWing::CheckCustomWing(int index) // OK
{
	if(this->GetInfo(index) != 0)
	{
		return 1;
	}

	return 0;
}

bool CCustomWing::CheckCustomWingByItem(int ItemIndex) // OK
{
	if(this->GetInfoByItem(ItemIndex) != 0)
	{
		return 1;
	}

	return 0;
}

int CCustomWing::GetCustomWingIndex(int ItemIndex) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	return lpInfo->Index;
}

int CCustomWing::GetCustomWingDefense(int ItemIndex,int ItemLevel) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->DefenseConstA*ItemLevel);
}

int CCustomWing::GetCustomWingIncDamage(int ItemIndex,int ItemLevel) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 100;
	}

	return (lpInfo->IncDamageConstA+(ItemLevel*lpInfo->IncDamageConstB));
}

int CCustomWing::GetCustomWingDecDamage(int ItemIndex,int ItemLevel) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 100;
	}

	return (lpInfo->DecDamageConstA-(ItemLevel*lpInfo->DecDamageConstB));
}

int CCustomWing::GetCustomWingOptionIndex(int ItemIndex,int OptionNumber) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	switch(OptionNumber)
	{
		case 0:
			return lpInfo->OptionIndex1;
		case 1:
			return lpInfo->OptionIndex2;
		case 2:
			return lpInfo->OptionIndex3;
	}

	return 0;
}

int CCustomWing::GetCustomWingOptionValue(int ItemIndex,int OptionNumber) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	switch(OptionNumber)
	{
		case 0:
			return lpInfo->OptionValue1;
		case 1:
			return lpInfo->OptionValue2;
		case 2:
			return lpInfo->OptionValue3;
	}

	return 0;
}

int CCustomWing::GetCustomWingNewOptionIndex(int ItemIndex,int OptionNumber) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	switch(OptionNumber)
	{
		case 0:
			return lpInfo->NewOptionIndex1;
		case 1:
			return lpInfo->NewOptionIndex2;
		case 2:
			return lpInfo->NewOptionIndex3;
		case 3:
			return lpInfo->NewOptionIndex4;
	}

	return 0;
}

int CCustomWing::GetCustomWingNewOptionValue(int ItemIndex,int OptionNumber) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	switch(OptionNumber)
	{
		case 0:
			return lpInfo->NewOptionValue1;
		case 1:
			return lpInfo->NewOptionValue2;
		case 2:
			return lpInfo->NewOptionValue3;
		case 3:
			return lpInfo->NewOptionValue4;
	}

	return 0;
}


void CCustomWing::OpcionAdd(int aIndex, int index, int value)
{

	if (!gObjIsConnectedGP(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	switch(index)
	{
	case WINGS_ITEM_OPTION_ADD_PHYSI_DAMAGE:
		lpObj->PhysiDamageMinRight += value;
		lpObj->PhysiDamageMaxRight += value;
		lpObj->PhysiDamageMinLeft += value;
		lpObj->PhysiDamageMaxLeft += value;
		break;
	case WINGS_ITEM_OPTION_ADD_MAGIC_DAMAGE:
		lpObj->MagicDamageMin += value;
		lpObj->MagicDamageMax += value;
		break;
	case WINGS_ITEM_OPTION_ADD_DEFENSE_SUCCESS_RATE:
		lpObj->DefenseSuccessRate += value;
		break;
	case WINGS_ITEM_OPTION_ADD_DEFENSE:
		lpObj->Defense += value;
		break;
	case WINGS_ITEM_OPTION_ADD_HP_RECOVERY_RATE:
	case WINGS_ITEM_OPTION_ADD_HP_RECOVERY_RATE_:
		lpObj->HPRecoveryRate += value;
		break;
	case WINGS_ITEM_OPTION_ADD_MONEY_AMOUNT_DROP_RATE:
		lpObj->MoneyAmountDropRate += value;
		break;
	case WINGS_ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE:
		lpObj->DefenseSuccessRate += (lpObj->DefenseSuccessRate*value)/100;
		break;
	case WINGS_ITEM_OPTION_ADD_DAMAGE_REFLECT:
		lpObj->DamageReflect += value;
		break;
	case WINGS_ITEM_OPTION_ADD_DAMAGE_REDUCTION:
		lpObj->DamageReduction[DAMAGE_REDUCTION_EXCELLENT_ITEM] += value;
		break;
	case WINGS_ITEM_OPTION_MUL_MP:
		lpObj->AddMana += (int)((lpObj->MaxMana*value)/100);
		break;
	case WINGS_ITEM_OPTION_MUL_HP:
		lpObj->AddLife += (int)((lpObj->MaxLife*value)/100);
		break;
	case WINGS_ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE:
		lpObj->ExcellentDamageRate += value;
		break;
	case WINGS_ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_LEVEL:
		lpObj->PhysiDamageMinRight += ((lpObj->Level+lpObj->MasterLevel)/value);
		lpObj->PhysiDamageMaxRight += ((lpObj->Level+lpObj->MasterLevel)/value);
		lpObj->PhysiDamageMinLeft += ((lpObj->Level+lpObj->MasterLevel)/value);
		lpObj->PhysiDamageMaxLeft += ((lpObj->Level+lpObj->MasterLevel)/value);
		break;
	case WINGS_ITEM_OPTION_MUL_PHYSI_DAMAGE:
		lpObj->PhysiDamageMinRight += (lpObj->PhysiDamageMinRight*value) / 100;
		lpObj->PhysiDamageMaxRight += (lpObj->PhysiDamageMaxRight*value) / 100;
		lpObj->PhysiDamageMinLeft += (lpObj->PhysiDamageMinLeft*value) / 100;
		lpObj->PhysiDamageMaxLeft += (lpObj->PhysiDamageMaxLeft*value) / 100;
		break;
	case WINGS_ITEM_OPTION_ADD_MAGIC_DAMAGE_BY_LEVEL:
		lpObj->MagicDamageMin += ((lpObj->Level+lpObj->MasterLevel) / value);
		lpObj->MagicDamageMax += ((lpObj->Level+lpObj->MasterLevel) / value);
		break;
	case WINGS_ITEM_OPTION_MUL_MAGIC_DAMAGE:
		lpObj->MagicDamageMin += (lpObj->MagicDamageMin * value) / 100;
		lpObj->MagicDamageMax += (lpObj->MagicDamageMax * value) / 100;
		break;
	case WINGS_ITEM_OPTION_ADD_SPEED_MAGIC:
		lpObj->MagicSpeed += value;
		break;
	case WINGS_ITEM_OPTION_ADD_HUNT_HP:
		lpObj->HuntHP += value;
		break;
	case WINGS_ITEM_OPTION_ADD_HUNT_MP:
		lpObj->HuntMP += value;
		break;
	case WINGS_ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE:
		lpObj->IgnoreDefenseRate += value;
		break;
	case WINGS_ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE_:
		lpObj->IgnoreDefenseRate += value;
		break;
	case WINGS_ITEM_OPTION_ADD_BP:
		lpObj->AddBP += value;
		break;
	case WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE:
		lpObj->FullDamageReflectRate += value;
		break;
	case WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE:
	case WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE_:
		lpObj->DefensiveFullHPRestoreRate += value;
		break;
	case WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_MP_RESTORE_RATE:
		lpObj->DefensiveFullMPRestoreRate += value;
		break;
	case WINGS_ITEM_OPTION_ADD_HP:
		lpObj->AddLife += value;
		break;
	case WINGS_ITEM_OPTION_ADD_MP:
		lpObj->AddMana += value;
		break;
	case WINGS_ITEM_OPTION_MUL_DEFENSE:
		lpObj->Defense += (lpObj->Defense * value)/100;
		break;
	default:
		break;
	}
}
void CCustomWing::CalcItemCustomOption(LPOBJ lpObj, bool flag) // OK
{
	if (flag != 0)
	{
		return;
	}

	for(int n=0; n < INVENTORY_WEAR_SIZE; n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0 && lpObj->Inventory[n].m_IsValidItem != 0 && lpObj->Inventory[n].m_Durability != 0)
		{

			if( this->CheckCustomWingByItem(lpObj->Inventory[n].m_Index) )
			{
				this->OpcionAdd(lpObj->Index, this->GetCustomWingNewOptionIndex(lpObj->Inventory[n].m_Index, 0), this->GetCustomWingNewOptionValue(lpObj->Inventory[n].m_Index, 0));
				this->OpcionAdd(lpObj->Index, this->GetCustomWingNewOptionIndex(lpObj->Inventory[n].m_Index, 1), this->GetCustomWingNewOptionValue(lpObj->Inventory[n].m_Index, 1));
				this->OpcionAdd(lpObj->Index, this->GetCustomWingNewOptionIndex(lpObj->Inventory[n].m_Index, 2), this->GetCustomWingNewOptionValue(lpObj->Inventory[n].m_Index, 2));
				this->OpcionAdd(lpObj->Index, this->GetCustomWingNewOptionIndex(lpObj->Inventory[n].m_Index, 3), 5);//this->GetCustomWingNewOptionValue(lpObj->Inventory[n].m_Index, 3));
			}
		}
	}
}