// CustomStartItem.h: interface for the CustomStartItem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_CLASS 7


struct CUSTOM_START_ITEM_INFO
{
	int ItemIndex;
	int ItemLevel;
	int ItemDurability;
	int ItemSkill;
	int ItemLuck;
	int ItemOption;
	int ItemExcellent;
	int ItemAncient;
	int ItemJOH;
	int ItemOpEx;
	int ItemSocket1;
	int ItemSocket2;
	int ItemSocket3;
	int ItemSocket4;
	int ItemSocket5;
	int	ItemTime;
};

struct CustomStartItem_INFO
{
	int Class;
	std::vector<CUSTOM_START_ITEM_INFO> ItemStart;
};

struct CUSTOM_START_BUFF_INFO
{
	int Type;
	int Effect;
	int Power1;
	int Power2;
	int Time;
};

struct CustomStartBuff_INFO
{
	int Class;
	std::vector<CUSTOM_START_BUFF_INFO> BuffStart;
};

class CCustomStartItem
{

public:
	CCustomStartItem();
	virtual ~CCustomStartItem();
	void Init();
	void Load(char* path);
	void SetInfo(CustomStartItem_INFO info);
	void SetBuffs(CustomStartBuff_INFO buffs);
	bool CheckSkillEffect(int m_index);
	void GiftItem(LPOBJ lpObj);
public:
	CustomStartItem_INFO m_CustomStartItemInfo[MAX_CLASS];
	CustomStartBuff_INFO m_CustomStartBuffInfo[MAX_CLASS];
};

extern CCustomStartItem gCustomStartItem;
