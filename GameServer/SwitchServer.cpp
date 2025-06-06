#include "stdafx.h"
#include "Protocol.h"
#include "SwitchServer.h"
#include "JSProtocol.h"

gSwitchServer SwitchServer;

gSwitchServer::gSwitchServer()
{
}

gSwitchServer::~gSwitchServer()
{
}

void gSwitchServer::SwitchServerReq(ECCG_SWITCHSERVER_REQ* data, int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (gObjIsConnectedGP(aIndex) == 0){
		return;
	}
	GJMapServerMoveSend(lpObj->Index, data->subcode, 00, 125, 125);
}