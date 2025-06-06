#include "stdafx.h"
#include "RespawnLocation.h"
#include "Gate.h"
#include "Map.h"
#include "KanturuBattleUserMng.h"
#include "RaklionBattleUserMng.h"
#include "MemScript.h"
#include "Util.h"

cRespawnLocation gRespawnLocation;

cRespawnLocation::cRespawnLocation()
{
	this->m_RespLoc.clear();
}

cRespawnLocation::~cRespawnLocation()
{
	
}

void cRespawnLocation::Load(char* path)
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

	this->m_RespLoc.clear();

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

			RESPLOC_DATA info;

			info.Map = lpMemScript->GetNumber();

			info.MoveGate = lpMemScript->GetAsNumber();

			info.Gate = lpMemScript->GetAsNumber();

			this->m_RespLoc.push_back(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool cRespawnLocation::IsMap(LPOBJ lpObj)
{
	for(std::vector<RESPLOC_DATA>::iterator it=this->m_RespLoc.begin();it != this->m_RespLoc.end();it++)
	{
		if (it->Map == lpObj->Map)
		{
			return true;
		}
	}
	return false;
}

bool cRespawnLocation::GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level)
{
	bool result = 0;

	for(std::vector<RESPLOC_DATA>::iterator it=this->m_RespLoc.begin();it != this->m_RespLoc.end();it++)
	{
		if (lpObj->Map == it->Map)
		{
			if (it->MoveGate == 0)
			{
				return 0;
			}
			result = gGate.GetGate(it->Gate,gate,map,x,y,dir,level);

			if (lpObj->Map == MAP_KANTURU3)
			{
				gKanturuBattleUserMng.DelUserData(lpObj->Index);
			}

			if (lpObj->Map == MAP_RAKLION2)
			{
				gRaklionBattleUserMng.DelUserData(lpObj->Index);
			}
		}
	}
	return result;
}