#pragma once

#include "User.h"
#include "Protocol.h"

struct PMSG_WAREHOUSEINTERFACE_RECV
{
	PSBMSG_HEAD header;
	int Number;
};



class cWarehousePanel
{
public:
	cWarehousePanel();
	virtual ~cWarehousePanel();
	void CGWarehouseOpen(PMSG_WAREHOUSEINTERFACE_RECV* lpMsg, int aIndex);

}; extern cWarehousePanel gWarehousePanel;


