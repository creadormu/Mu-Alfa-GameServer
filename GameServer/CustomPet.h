// CustomPet.h: interface for the CCustomPet class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "Item.h"
#include "User.h"

#define MAX_CUSTOM_PET 512 // 500

struct CUSTOM_PET_INFO
{
	int Index;
	int ItemIndex;
	int IncDamageRate;
	int IncLife;
	int IncMana;
	int IncAttackSpeed;
	int IncDoubleDamageRate;
	int IncTripleDamageRate;
	int IncExperience;
	int IncResistDoubleDamage;
	int IncResistIgnoreDefense;
	int IncResistIgnoreSD;
	int IncResistCriticalDamage;
	int IncResisteExcellentDamage;
	int IncBlockStuck;
	int IncReflectRate;
	int Recovery;
};

class CCustomPet
{
public:
	CCustomPet();
	virtual ~CCustomPet();
	void Load(char* path);
	bool CheckCustomPetByItem(int ItemIndex);
	bool PetIsInmortal(int ItemIndex);
	int GetCustomPetDamageRate(int ItemIndex);
	void CalcCustomPetOption(LPOBJ lpObj, bool flag);
public:
	std::map<int, CUSTOM_PET_INFO> m_CustomPetInfo;
};

extern CCustomPet gCustomPet;