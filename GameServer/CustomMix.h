// ---
#include "stdafx.h"
#include "RandomManager.h"
// ---
#define MAX_MIX 5000
// ---
struct CUSTOM_MIX
{
	int  m_Index;
	int  m_MixMoney;
	int  m_MixRate_AL0;
	int  m_MixRate_AL1;
	int  m_MixRate_AL2;
	int  m_MixRate_AL3;
	int  m_CountGroup;
	int  m_CountItem;
	int m_OpcionExeRand;
//Index		MixMoney	MixRate_AL0		MixRate_AL1		MixRate_AL2		MixRate_AL3		CountGroup	CountItem
};

struct CUSTOM_BAG_MIX
{
	int index;
	int  m_Index;
	int  m_ItemMix;
	int  m_ItemLevel;
	int  m_Count;
//Index	ItemBag	level	Count
};

struct CUSTOM_ITEM_MIX
{
	int index;
	int  m_Index;
	int  m_Group;
	int  m_Count;
	int  m_ItemIndexMin;
	int  m_ItemIndexMax;
	int  m_ItemLevel;
	int  m_Skill;
	int  m_Luck;
	int  m_Opcion;
	int  m_Excelent;
	int  m_SetACC;
//Index		Group		Count	ItemIndex	ItemIndex	MinItemLevel	Skill	Luck	MinAdicional	Exe		ItemSetOption
};
// ---
struct CUSTOM_ITEM_MIX_RESULT
{
	int index;
	int  m_Index;
	int  m_Group;
	int  m_ItemIndexMin;
	int  m_ItemIndexMax;
	int  m_ItemLevel;
	int  m_Skill;
	int  m_Luck;
	int  m_Opcion;
	int  m_Excelent;
	int  m_SetACC;
	int  Socket1;
	int  Socket2;
	int  Socket3;
	int  Socket4;
	int  Socket5;
//Index		Group	ItemIndexMin	ItemIndexMax	ItemLevel	Skill	Luck	MinAdicional	Exe		ItemSetOption	ItemClass
};

struct DataResult
{
	int MixNum;
	std::map<int, CUSTOM_ITEM_MIX_RESULT> m_DataResult;
};

class CCustomMix
{
public:
	int m_count1;
	int m_count2;
	int m_count3;
	int m_count4;

	void Load(char* path);
	int GetCountItemMix(int IndexMix,int counGroup);
	CRandomManager RandomManager(int IndexMix,int counGroup);
	CUSTOM_MIX* GetCustomMix(int IndexMix);
	CUSTOM_ITEM_MIX_RESULT* GetItemResult(int IndexMix,int group,int ItemIndex);
	int GetCountItemBagMix(int IndexMix);
	bool istItemMix(int IndexMix, int group, CItem boxItem);
	bool istItemBagMix(int IndexMix, CItem boxItem);
public:
	std::map<int, CUSTOM_MIX> m_Data_Mix;
	std::map<int, CUSTOM_BAG_MIX> m_Data_Bag;
	std::map<int, CUSTOM_ITEM_MIX> m_Data_Item;
	DataResult DataResult[254];
};

extern CCustomMix gCustomMix;
// ---