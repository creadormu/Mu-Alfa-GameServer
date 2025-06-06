#pragma once

#include "Item.h"
#include "User.h"
#include "Protocol.h"

#define MAX_LUCKYWHEEL_ITEM 12

struct LUCKYWHEEL_INFO
{
	int Index;
	int ItemType;
	int ItemIndex;
	int Level;
	int Luck;
	int Skill;
	int Option;
	int Exc;
};

struct ITEM_WIN_SEND
{
	PSBMSG_HEAD header;
	int	number;
};

class CLuckyWheel
{
public:
	CLuckyWheel();
	virtual ~CLuckyWheel();
	void Init();
	void Load(char* path);
	void SetInfo(LUCKYWHEEL_INFO info);
	void Start(LPOBJ lpUser);
public:
	LUCKYWHEEL_INFO m_LuckyWheelInfo[MAX_LUCKYWHEEL_ITEM];
};

extern CLuckyWheel gLuckyWheel;
