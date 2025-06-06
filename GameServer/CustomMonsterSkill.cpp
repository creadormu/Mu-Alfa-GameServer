#include "stdafx.h"
#include "Util.h"
#include "ItemManager.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "MemScript.h"
#include "CustomMonsterSkill.h"

CCustomMonsterSkill gCustomMonsterSkill;

void CCustomMonsterSkill::Load(char* path){

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

	this->m_count = 0;

	for(int n=0;n < MAX_MONSTER_SKILL;n++)
	{
		this->m_Monster_Skill[n];
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

					this->m_Monster_Skill[this->m_count].m_MonsterClass = lpMemScript->GetNumber();

					this->m_Monster_Skill[this->m_count].m_Skill1 = lpMemScript->GetAsNumber();

					this->m_Monster_Skill[this->m_count].m_Skill2 = lpMemScript->GetAsNumber();

					this->m_count++;
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

CUSTOM_MONSTER_SKILL* CCustomMonsterSkill::GetSkillMonster(int MonsterClass){

	for(int n=0;n < MAX_MONSTER_SKILL;n++)
	{
		if(this->m_Monster_Skill[n].m_MonsterClass == MonsterClass)
		{
			return &this->m_Monster_Skill[n];
		}
	}
	return 0;
}