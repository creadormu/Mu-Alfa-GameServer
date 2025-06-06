#include "stdafx.h"
#include "Util.h"
#include "Message.h"
#include "Path.h"
#include "MemScript.h"
#include "TeleportMarlon.h"

cCustomTeleportMarlon gCustomMarlon;

void cCustomTeleportMarlon::Load(char* path){

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

	this->m_count = 1;

	for(int n=0;n < MAX_TELEPORT_MARLON;n++)
	{
		this->m_Marlon[this->m_count].m_index = -1;
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

					this->m_Marlon[this->m_count].m_index = m_count;

					this->m_Marlon[this->m_count].m_Mapa = lpMemScript->GetNumber();

					this->m_Marlon[this->m_count].m_CoordenadaX = lpMemScript->GetAsNumber();

					this->m_Marlon[this->m_count].m_CoordenadaY = lpMemScript->GetAsNumber();

					this->m_Marlon[this->m_count].m_Dir = lpMemScript->GetAsNumber();

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

CUSTOM_TELEPORT_MARLON* cCustomTeleportMarlon::GetTeleportMarlon(int index){

	for(int n=0;n < MAX_TELEPORT_MARLON;n++)
	{
		if(this->m_Marlon[n].m_index == index)
		{
			return &this->m_Marlon[n];
		}
	}
	return 0;
}