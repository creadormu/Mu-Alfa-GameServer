#include "stdafx.h"
#include "MultiWare.h"
#include "CommandManager.h"
#include "ServerInfo.h"
#include "Notice.h"
#include "Util.h"
#include "Message.h"
#include "Log.h"
#include "Map.h"
#include "Warehouse.h"
#include "ServerInfo.h"

cWarehousePanel gWarehousePanel;

cWarehousePanel::cWarehousePanel()
{

}

cWarehousePanel::~cWarehousePanel()
{
	
}

void cWarehousePanel::CGWarehouseOpen(PMSG_WAREHOUSEINTERFACE_RECV* lpMsg, int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gServerInfo.m_CommandWareNumber[lpObj->AccountLevel] == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(85));
		return;
	}

	if(lpObj->Interface.use != 0 || lpObj->State == OBJECT_DELCMD || lpObj->DieRegen != 0 || lpObj->Teleport != 0 || lpObj->LoadWarehouse != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(86));
		return;
	}

	int number = lpMsg->Number;

	if(number < 0 || number >= gServerInfo.m_CommandWareNumber[lpObj->AccountLevel])
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(87));
		return;
	}

	lpObj->WarehouseMoney = 0;

	memset(lpObj->WarehouseMap,0xFF,WAREHOUSE_SIZE);

	for(int n=0;n < WAREHOUSE_SIZE;n++)
	{
		lpObj->Warehouse[n].Clear();
	}

	lpObj->Interface.use = 0;

	lpObj->Interface.type = INTERFACE_NONE;

	lpObj->Interface.state = 0;

	lpObj->LoadWarehouse = 0;

	lpObj->WarehouseNumber = number;

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_WAREHOUSE;
	lpObj->Interface.state = 0;

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(88),number);

	gWarehouse.GDWarehouseItemSend(lpObj->Index,lpObj->Account);

	LogAdd(LOG_GREEN,"[PanelWare][%s][%s] - (Number: %d)", lpObj->Account,lpObj->Name,lpMsg->Number);
}