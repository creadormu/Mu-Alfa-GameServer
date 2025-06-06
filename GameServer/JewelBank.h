#pragma once

#include "StdAfx.h"
#include "DSProtocol.h"

#define JEWELSBANK_DIR	"..\\Data\\Custom\\JewelsBank.ini"

struct PMSG_JEWELBANK
{
	PSBMSG_HEAD h;
	int Result;
};

struct PMSG_JEWELSBANK
{
#pragma pack(1)
	PSBMSG_HEAD h;
	int Zen;
	int Chaos;
	int Bless;
	int Soul;
	int Life;
	int CreateonBank;
	int GuardianBank;
	int HarmonyBank;
	int LowStoneBank;
	int HighStoneBank;
	int GemStoneBank;
#pragma pack()
};

class JewelsBank
{
public:
	void Load();
	void EnvioJewelsBank(int aIndex);

	//Send
	void sendzen(int aIndex, int Count);
	void sendchaosbank(int aIndex, int Count);
	void sendblessbank(int aIndex, int Count);
	void sendsoulbank(int aIndex, int Count);
	void sendlifebank(int aIndex, int Count);
	void sendcreateonbank(int aIndex, int Count);
	void sendGuardianBank(int aIndex, int Count);
	void sendHarmonyBank(int aIndex, int Count);
	void sendLowStoneBank(int aIndex, int Count);
	void sendHighStoneBank(int aIndex, int Count);
	void sendGemStoneBank(int aIndex, int Count);

	// Reck
	void recvzen(int aIndex, int Count);
	void recvchaosbank(int aIndex, int Count);
	void recvblessbank(int aIndex, int Count);
	void recvsoulbank(int aIndex, int Count);
	void recvlifebank(int aIndex, int Count);
	void recvcreateonbank(int aIndex, int Count);
	void recvGuardianBank(int aIndex, int Count);
	void recvHarmonyBank(int aIndex, int Count);
	void recvLowStoneBank(int aIndex, int Count);
	void recvHighStoneBank(int aIndex, int Count);
	void recvGemStoneBank(int aIndex, int Count);

	void Packet(int aIndex, PMSG_JEWELBANK* lpMsg);
private:
	bool Enable;
};
extern JewelsBank gJewelsBank;