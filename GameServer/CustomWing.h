// CustomWing.h: interface for the CCustomWing class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "User.h"

#define MAX_CUSTOM_WING 100

enum eOptionWings
{
	WINGS_ITEM_OPTION_ADD_PHYSI_DAMAGE = 80,
	WINGS_ITEM_OPTION_ADD_MAGIC_DAMAGE = 81,
	WINGS_ITEM_OPTION_ADD_DEFENSE_SUCCESS_RATE = 82,
	WINGS_ITEM_OPTION_ADD_DEFENSE = 83,
	WINGS_ITEM_OPTION_LUCK = 84,
	WINGS_ITEM_OPTION_ADD_HP_RECOVERY_RATE = 85,
	WINGS_ITEM_OPTION_ADD_HP_RECOVERY_RATE_ = 86,
	WINGS_ITEM_OPTION_MUL_HP = 87,
	WINGS_ITEM_OPTION_MUL_MP = 88,
	WINGS_ITEM_OPTION_ADD_DAMAGE_REDUCTION = 89,
	WINGS_ITEM_OPTION_ADD_DAMAGE_REFLECT = 90,
	WINGS_ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE = 91,
	WINGS_ITEM_OPTION_ADD_MONEY_AMOUNT_DROP_RATE = 92,
	WINGS_ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE = 93,
	WINGS_ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_LEVEL = 94,
	WINGS_ITEM_OPTION_MUL_PHYSI_DAMAGE = 95,
	WINGS_ITEM_OPTION_ADD_MAGIC_DAMAGE_BY_LEVEL = 96,
	WINGS_ITEM_OPTION_MUL_MAGIC_DAMAGE = 97,
	WINGS_ITEM_OPTION_ADD_SPEED_MAGIC = 98,
	WINGS_ITEM_OPTION_ADD_HUNT_HP = 99,
	WINGS_ITEM_OPTION_ADD_HUNT_MP = 100,
	WINGS_ITEM_OPTION_ADD_HP = 101,
	WINGS_ITEM_OPTION_ADD_MP = 102,
	WINGS_ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE = 103,
	WINGS_ITEM_OPTION_ADD_BP = 104,
	WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE = 106,
	WINGS_ITEM_OPTION_MUL_DEFENSE = 105,
	WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE = 107,
	WINGS_ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE_ = 108,
	WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE_ = 109,
	WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE_ = 110,
	WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_MP_RESTORE_RATE = 111,
	WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE__ = 112,
	WINGS_ITEM_OPTION_ADD_CURSE_DAMAGE = 113,
	WINGS_ITEM_OPTION_LUCK_ = 114,
	WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE1_ = 115,
	WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE2_ = 116,
	WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE3_ = 117,
	WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE4_ = 118,
};

struct CUSTOM_WING_INFO
{
	int Index;
	int ItemIndex;
	int DefenseConstA;
	int IncDamageConstA;
	int IncDamageConstB;
	int DecDamageConstA;
	int DecDamageConstB;
	int OptionIndex1;
	int OptionValue1;
	int OptionIndex2;
	int OptionValue2;
	int OptionIndex3;
	int OptionValue3;
	int NewOptionIndex1;
	int NewOptionValue1;
	int NewOptionIndex2;
	int NewOptionValue2;
	int NewOptionIndex3;
	int NewOptionValue3;
	int NewOptionIndex4;
	int NewOptionValue4;
	int ModelType;
	char ModelName[32];
};

class CCustomWing
{
public:
	CCustomWing();
	virtual ~CCustomWing();
	void Init();
	void Load(char* path);
	void SetInfo(CUSTOM_WING_INFO info);
	CUSTOM_WING_INFO* GetInfo(int index);
	CUSTOM_WING_INFO* GetInfoByItem(int ItemIndex);
	bool CheckCustomWing(int index);
	bool CheckCustomWingByItem(int ItemIndex);
	int GetCustomWingIndex(int ItemIndex);
	int GetCustomWingDefense(int ItemIndex, int ItemLevel);
	int GetCustomWingIncDamage(int ItemIndex, int ItemLevel);
	int GetCustomWingDecDamage(int ItemIndex, int ItemLevel);
	int GetCustomWingOptionIndex(int ItemIndex,int OptionNumber);
	int GetCustomWingOptionValue(int ItemIndex, int OptionNumber);
	int GetCustomWingNewOptionIndex(int ItemIndex, int OptionNumber);
	int GetCustomWingNewOptionValue(int ItemIndex, int OptionNumber);
	void CalcItemCustomOption(LPOBJ lpObj, bool flag);
	void OpcionAdd(int aIndex, int index, int value);
public:
	CUSTOM_WING_INFO m_CustomWingInfo[MAX_CUSTOM_WING];
};

extern CCustomWing gCustomWing;
