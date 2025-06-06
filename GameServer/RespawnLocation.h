#pragma once

#include "User.h"

struct RESPLOC_DATA
{
	WORD	Map;
	BYTE	MoveGate;
	int		Gate;
};

class cRespawnLocation
{
public:
	cRespawnLocation();
	virtual ~cRespawnLocation();
	void Load(char* path);
	bool IsMap(LPOBJ lpObj);
	bool GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level);
	std::vector<RESPLOC_DATA> m_RespLoc;
}; extern cRespawnLocation gRespawnLocation;