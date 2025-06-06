// MonsterSetBase.h: interface for the CMonsterSetBase class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

/*//-- Max Monster 10000
#define MAX_MSB_MONSTER 12000*/
/*//-- Max Monster 30000
#define MAX_MSB_MONSTER 32500*/
//-- Max Monster 12000
#define MAX_MSB_MONSTER 14000

#define OBJ_MAXMONSTER	(OBJ_STARTUSERINDEX-100)	//MC	//11500 //5800

struct MONSTER_SET_BASE_INFO
{
	int Type;
	int MonsterClass;
	int Map;
	int Dis;
	int X;
	int Y;
	int Dir;
	int TX;
	int TY;
	int Value;
};

class CMonsterSetBase
{
public:
	CMonsterSetBase();
	virtual ~CMonsterSetBase();
	void Load(char* path);
	void SetInfo(MONSTER_SET_BASE_INFO info);
	bool GetPosition(int index,short map,short* ox,short* oy);
	bool GetBoxPosition(int map,int x,int y,int tx,int ty,short* ox,short* oy);
	void SetBoxPosition(int index,int map,int x,int y,int tx,int ty);
public:
	MONSTER_SET_BASE_INFO m_MonsterSetBaseInfo[MAX_MSB_MONSTER];
	MONSTER_SET_BASE_INFO m_Mp[OBJ_MAXMONSTER];//MC
	int m_count;
	int m_Count;
};

extern CMonsterSetBase gMonsterSetBase;
