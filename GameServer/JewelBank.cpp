#include "StdAfx.h"
#include "JewelBank.h"
#include "user.h"
#include "Message.h"
#include "DSProtocol.h"
#include "Notice.h"
#include "ItemManager.h"
#include "Util.h"
#include "readscript.h"

JewelsBank gJewelsBank;

void JewelsBank::EnvioJewelsBank(int aIndex)
{

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	// ---
	LPOBJ lpObj = &gObj[aIndex];
	// ---
	PMSG_JEWELSBANK pMsg = { 0 };
	// ---
	pMsg.h.set(0xF5, 0xE5, sizeof(pMsg));//0xFB
	// ---
	pMsg.Zen = lpObj->Zen;
	pMsg.Chaos = lpObj->ChaosBank;
	pMsg.Bless = lpObj->BlessBank;
	pMsg.Soul = lpObj->SoulBank;
	pMsg.Life = lpObj->LifeBank;
	pMsg.CreateonBank = lpObj->CreateonBank;
	pMsg.GuardianBank = lpObj->GuardianBank;
	pMsg.HarmonyBank = lpObj->HarmonyBank;
	pMsg.LowStoneBank = lpObj->LowStoneBank;
	pMsg.HighStoneBank = lpObj->HighStoneBank;
	pMsg.GemStoneBank = lpObj->GemStoneBank;
	// ---
	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
}

int gObjGetItemCountInInventory(int aIndex, int ItemID, int ItemLevel)
{
	if (aIndex >= 15000)
		return 0;

	LPOBJ lpObj = &gObj[aIndex];

	int Count = 0;
	for (int i = INVENTORY_WEAR_SIZE; i < INVENTORY_SIZE; i++)
	{
		if (lpObj->Inventory[i].m_Index == ItemID && lpObj->Inventory[i].m_Level == ItemLevel)
			Count++;
	}
	return Count;
}

int gObjDeleteItemsCount(int aIndex, short Type, short Level, int dCount)
{
	if (aIndex >= 15000)
		return 0;

	LPOBJ lpObj = &gObj[aIndex];

	int count = 0;
	for (int i = INVENTORY_WEAR_SIZE; i < INVENTORY_SIZE; i++)
	{
		if (lpObj->Inventory[i].m_Index == Type && lpObj->Inventory[i].m_Level == Level)
		{
			gItemManager.InventoryDelItem(lpObj->Index, i);
			gItemManager.GCItemDeleteSend(lpObj->Index, i, 1);
			count++;
			if (dCount == count)
				break;
		}
	}
	return count;
}

int ExCheckInventoryEmptySpace(int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];
	int iEmptyCount = 0;
	for (int x = 0; x<64; x++)
	{
		if (lpObj->InventoryMap[x] == 0xFF)
		{
			iEmptyCount++;
		}
	}

	for (int x = 64; x<96; x++)
	{
		if (lpObj->InventoryMap[x] == 0xFF)
		{
			iEmptyCount++;
		}
	}

	for (int x = 96; x<128; x++)
	{
		if (lpObj->InventoryMap[x] == 0xFF)
		{
			iEmptyCount++;
		}
	}

	return iEmptyCount;
}

void JewelsBank::Load()
{
	SMDToken Token;
	SMDFile = fopen(JEWELSBANK_DIR, "r");
	// ----
	if( !SMDFile )
	{
		ErrorMessageBox("[JewelsBank.ini] %s file not found", JEWELSBANK_DIR);
		return;
	}

	this->Enable = GetPrivateProfileInt("JewelsBank", "Enable", 0, JEWELSBANK_DIR);
}

//-----------------------------------------------------
void JewelsBank::sendzen(int aIndex, int Count)
{
	LPOBJ lpUser = &gObj[aIndex];
	unsigned int M = 1000000 * Count;
	if (M > lpUser->Money) return;
	lpUser->Money -= M;
	lpUser->Zen += Count;
	GCMoneySend(aIndex, lpUser->Money);
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
}

void JewelsBank::sendchaosbank(int aIndex, int Count)
{
	if (!this->Enable) return;
	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(12, 15), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d Chaos", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(12, 15), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->ChaosBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d Chaos", Count);
}

void JewelsBank::sendblessbank(int aIndex, int Count)
{
	if (!this->Enable) return;
	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 13), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d Bless", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 13), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->BlessBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d Bless", Count);
}

void JewelsBank::sendsoulbank(int aIndex, int Count)
{
	if (!this->Enable) return;
	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 14), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d Soul", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 14), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->SoulBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d Soul", Count);
}

void JewelsBank::sendlifebank(int aIndex, int Count)
{
	if (!this->Enable) return;

	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 16), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d Life", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 16), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->LifeBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d Life", Count);
}

void JewelsBank::sendcreateonbank(int aIndex, int Count)
{
	if (!this->Enable) return;

	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 22), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d Createon", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 22), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->CreateonBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d Createon", Count);
}

void JewelsBank::sendGuardianBank(int aIndex, int Count)
{
	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 31), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d Guardian", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 31), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->GuardianBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d Guardian", Count);
}

void JewelsBank::sendHarmonyBank(int aIndex, int Count)
{
	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 42), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d Harmony", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 42), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->HarmonyBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d Harmony", Count);
}

void JewelsBank::sendLowStoneBank(int aIndex, int Count)
{
	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 43), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d LowStone", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 43), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->LowStoneBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d LowStone", Count);
}

void JewelsBank::sendHighStoneBank(int aIndex, int Count)
{
	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 44), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d HighStone", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 44), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->HighStoneBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d HighStone", Count);
}

void JewelsBank::sendGemStoneBank(int aIndex, int Count)
{
	if (gObjGetItemCountInInventory(aIndex, GET_ITEM(14, 41), 0) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Need %d GemStoneBank", Count);
		return;
	}
	gObjDeleteItemsCount(aIndex, GET_ITEM(14, 41), 0, Count);
	LPOBJ lpObj = &gObj[aIndex];
	lpObj->GemStoneBank += Count;
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Send %d GemStoneBank", Count);
}

//-----------------------------------------------------
void JewelsBank::recvzen(int aIndex, int Count)
{
	LPOBJ lpUser = &gObj[aIndex];
	int M = 1000000;
	unsigned long long totalzen = M + lpUser->Money;
	if (totalzen > 2000000000) return;
	if (Count > lpUser->Zen) return;
	lpUser->Money += M;
	lpUser->Zen -= Count;
	GCMoneySend(aIndex, lpUser->Money);
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
}

void JewelsBank::recvchaosbank(int aIndex, int Count)
{
	if (!this->Enable) return;
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->ChaosBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d Chaos", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->ChaosBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(12, 15), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d Chaos", Count);
}

void JewelsBank::recvblessbank(int aIndex, int Count)
{
	if (!this->Enable) return;
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->BlessBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d Bless", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->BlessBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 13), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d Bless", Count);
}

void JewelsBank::recvsoulbank(int aIndex, int Count)
{
	if (!this->Enable) return;
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->SoulBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d Soul", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->SoulBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 14), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d Soul", Count);
}

void JewelsBank::recvlifebank(int aIndex, int Count)
{
	if (!this->Enable) return;

	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->LifeBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d Life", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->LifeBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 16), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d Life", Count);
}

void JewelsBank::recvcreateonbank(int aIndex, int Count)
{
	if (!this->Enable) return;

	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->CreateonBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d Createon", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->CreateonBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 22), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d Createon", Count);
}

void JewelsBank::recvGuardianBank(int aIndex, int Count)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->GuardianBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d Guardian", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->GuardianBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 31), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d Guardian", Count);
}

void JewelsBank::recvHarmonyBank(int aIndex, int Count)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->HarmonyBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d Harmony", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->HarmonyBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 42), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d Harmony", Count);
}

void JewelsBank::recvLowStoneBank(int aIndex, int Count)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->LowStoneBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d LowStone", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->LowStoneBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 43), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d LowStone", Count);
}

void JewelsBank::recvHighStoneBank(int aIndex, int Count)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->HighStoneBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d HighStone", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->HighStoneBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 44), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d HighStone", Count);
}

void JewelsBank::recvGemStoneBank(int aIndex, int Count)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->GemStoneBank < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] No %d GemStone", Count);
		return;
	}
	if (ExCheckInventoryEmptySpace(aIndex) < Count)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Inventory %d", Count);
		return;
	}
	lpObj->GemStoneBank -= Count;
	for (int i = 0; i<Count; i++)
	{
		GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, GET_ITEM(14, 41), 0, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
	}
	EnvioJewelsBank(aIndex);
	GDCharacterInfoSaveSend(aIndex);
	gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[JewelsBank] Recv %d GemStone", Count);
}

//------------------------------------------------------------------
void JewelsBank::Packet(int aIndex, PMSG_JEWELBANK* lpMsg)
{
	if (!this->Enable)
	{
		return;
	}

	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	switch (lpMsg->Result)
	{
	case 0:
		this->sendzen(aIndex, 1);
		break;
	case 1:
		this->sendchaosbank(aIndex, 1);
		break;
	case 2:
		this->sendblessbank(aIndex, 1);
		break;
	case 3:
		this->sendsoulbank(aIndex, 1);
		break;
	case 4:
		this->sendlifebank(aIndex, 1);
		break;
	case 5:
		this->sendcreateonbank(aIndex, 1);
		break;
	case 6:
		this->sendGuardianBank(aIndex, 1);
		break;
	case 7:
		this->sendHarmonyBank(aIndex, 1);
		break;
	case 8:
		this->sendLowStoneBank(aIndex, 1);
		break;
	case 9:
		this->sendHighStoneBank(aIndex, 1);
		break;
	case 10:
		this->sendGemStoneBank(aIndex, 1);
		break;

	case 11:
		this->recvzen(aIndex, 1);
		break;
	case 12:
		this->recvchaosbank(aIndex, 1);
		break;
	case 13:
		this->recvblessbank(aIndex, 1);
		break;
	case 14:
		this->recvsoulbank(aIndex, 1);
		break;
	case 15:
		this->recvlifebank(aIndex, 1);
		break;
	case 16:
		this->recvcreateonbank(aIndex, 1);
		break;
	case 17:
		this->recvGuardianBank(aIndex, 1);
		break;
	case 18:
		this->recvHarmonyBank(aIndex, 1);
		break;
	case 19:
		this->recvLowStoneBank(aIndex, 1);
		break;
	case 20:
		this->recvHighStoneBank(aIndex, 1);
		break;
	case 21:
		this->recvGemStoneBank(aIndex, 1);
		break;
	}
}
