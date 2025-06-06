#pragma once
#include "Protocol.h"

#define MAX_RUUDTOKENSELL_INFO		25
#define MAX_CLASSTOKENSELL_INFO		7

struct PMSG_BUY_ITEMRUUD_RECV
{
	PBMSG_HEAD header; // EE:30
	BYTE PAGE;
	BYTE SLOT;
};

struct PMSG_ITEMRUUD_LIST_SEND
{
	PBMSG_HEAD header; // C1:42
	BYTE result;
	BYTE count;
};

struct _tagRuudTokenSell
{
	BYTE byItemType;
	WORD wItemIndex;
	BYTE shPageShop;
	BYTE wSlotIndex;
	WORD dwRuudCount;
};

struct _RecvRuudTokenSell
{
	_tagRuudTokenSell JRShop[MAX_RUUDTOKENSELL_INFO];
};

class RuudTokenSellTemplate
{
public:
	RuudTokenSellTemplate();
	virtual ~RuudTokenSellTemplate();
	void Load_RuudTokenSell(char *pchFileName);
	void GCShopItemListSend(LPOBJ lpObj);
	void _RecvRuudBUY(PMSG_BUY_ITEMRUUD_RECV * lpMsg, int aIndex);
public:
	_RecvRuudTokenSell RuudTokenSell[ MAX_CLASSTOKENSELL_INFO ];
};

extern RuudTokenSellTemplate g_RuudTokenSellTemplate;