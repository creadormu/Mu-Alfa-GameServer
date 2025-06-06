#pragma once

#include "User.h"


#define BASIC_INV_HEIGHT			8
#define EXTENDED_INV_HEIGHT			4



class YolaCustom
{
public:

	 YolaCustom();
	~YolaCustom();

	//Packets
	void PHeadSetB(LPBYTE lpBuf, BYTE head, int size);
	void PHeadSubSetB(LPBYTE lpBuf, BYTE head, BYTE sub, int size);
	void PHeadSetBE( LPBYTE lpBuf, BYTE head,int size);
	void PHeadSetW(LPBYTE lpBuf, BYTE head,  int size);

	//Inventory
	BYTE gObjInventoryInsertItem(int aIndex, CItem item);
	void gObjInventoryItemSet(int aIndex, int itempos, BYTE set_byte);
	void gObjTempInventoryItemBoxSet(BYTE * TempMap, int itempos, int xl, int yl, BYTE set_byte);
	unsigned char gObjInventoryInsertItemPos(int aIndex, class CItem item, int pos, int RequestCheck);
	unsigned char gObjTempInventoryInsertItem(LPOBJ lpObj, class CItem item, unsigned char* TempMap);
	unsigned char gObjTempInventoryInsertItem(LPOBJ lpObj, int itemType, BYTE * TempMap);
	BOOL ExtentCheck(int x, int y, int w, int h);
	BYTE gObjInventoryRectCheck(int aIndex, int sx, int sy, int width, int height);
	BOOL IS_EXTENDED_INV(int aIndex, BYTE INVNUM);
	void gObjInventoryItemBoxSet(int aIndex, int itempos, int xl, int yl, BYTE set_byte);
	BYTE gObjTempInventoryRectCheck(BYTE * TempMap, int sx, int sy, int width, int height);
	BYTE gObjInventoryInsertItemBot(LPOBJ lpObj, int type, int index, int level, int iSerial, int iDur, int ItemSlot1, int ItemSlot2, int ItemSlot3, int ItemSlot4, int ItemSlot5);
	void GCInventoryItemOneSend(int aIndex, int pos);
	unsigned char gObjShopBuyInventoryInsertItem(int aIndex, class CItem item);
	BYTE gObjOnlyInventoryRectCheck(int aIndex, int sx, int sy, int width, int height);
	BYTE GETHEIGHT(int aIndex);
	//Trade
	void gObjTradeCancel(int aIndex);
	void gObjCharTradeClear(LPOBJ lpObj);

	//Items
	BOOL DeleteItemByLevel( int iIndex, DWORD itemId, BYTE ItemLevel);
	int  ItemGetNumberMake(int type, int index);
	int  ItemGetDurability(int index, int itemLevel, int ExcellentItem, int SetItem);
	int getNumberOfExcOptions(int checksum);
	int gObjCheckSerial0ItemList(class CItem* lpItem);
	BYTE BoxExcOptions(int maxOptions);

	//SkyEvent
	BOOL SetPosMonsterSkyEvent(int aIndex, int PosTableNum);
	int  gObjMonsterAdd(WORD Type, BYTE Map, BYTE X, BYTE Y);
	BOOL gObjIsNewClass(LPOBJ lpObj);

	//User
	int  gObjGetIndex(char* szId);

	//BotWarper
	BOOL CheckMainToMove(LPOBJ lpObj);
	BOOL CheckEquipmentToMove(LPOBJ lpObj, int iTargetMapNumber);
	BOOL CheckInterfaceToMove(LPOBJ lpObj);

	//Map
	BOOL gObjCheckTeleportAreaByMap(BYTE mapnumber, BYTE x, BYTE y);
	BOOL gObjItsInSafeZone(int PlayerID); //afk-system , botpet
	BOOL gObjMoveHackCheck(int aIndex);
	BOOL gObjGetRandomFreeArea(int iMapNumber, BYTE &cX, BYTE &cY, int iSX, int iSY, int iDX, int iDY, int iLoopCount);

	//Vars
	int g_RenaCount;	// GOLDEN ARCHER :D

};

extern YolaCustom ObjYola;

