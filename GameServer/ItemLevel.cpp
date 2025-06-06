#include "stdafx.h"
#include "ItemLevel.h"
#include "Util.h"
#include "MemScript.h"
#include "ItemManager.h"

cCItemLevel gItemLevel;

cCItemLevel:: cCItemLevel()
{
}

cCItemLevel::~cCItemLevel()
{
}

void cCItemLevel::Load(char* path) // OK
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

	this->m_ItemData.clear();

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
			
			cItemLevel info;

			memset(&info,0,sizeof(info));

			static int Index = 0;

			info.index = Index++;

			info.ItemIndex = ( lpMemScript->GetNumber() * 512 ) + lpMemScript->GetAsNumber();

			info.ItemLevel = lpMemScript->GetAsNumber();

			strcpy_s(info.ItemName, lpMemScript->GetAsString());

			this->m_ItemData.insert(std::pair<int, cItemLevel>(info.index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

char * cCItemLevel::GetItemName(int ItemIndex, int Level)
{
	for( std::map<int, cItemLevel>::iterator it =  this->m_ItemData.begin(); it !=  this->m_ItemData.end(); it++ )
	{
		if( it->second.ItemIndex == ItemIndex && it->second.ItemLevel == Level)
		{
			return it->second.ItemName;
		}
	}
	return gItemManager.GetItemName(ItemIndex);
}