#pragma once
#include "stdafx.h"
#define MAX_MONSTER_SKILL 1000

struct CUSTOM_MONSTER_SKILL
{
	int  m_MonsterClass;
	int  m_Skill1;
	int  m_Skill2;
};

class CCustomMonsterSkill
{
public:
	int m_count;

	void Load(char* path);
	CUSTOM_MONSTER_SKILL* GetSkillMonster(int MonsterClass);
	CUSTOM_MONSTER_SKILL m_Monster_Skill[MAX_MONSTER_SKILL];
private:

};
extern CCustomMonsterSkill gCustomMonsterSkill;