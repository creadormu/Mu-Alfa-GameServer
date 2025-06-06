#pragma once

#include "Protocol.h"

struct PMSG_RECV_PARTYSEARCH_ADD
{
	PSBMSG_HEAD header;
	bool SystemActive;
	bool OnlyGuild;
	bool OneClass;
	bool DarkWizard;
	bool DarkKnight;
	bool Elf;
	bool MagicGladiator;
	bool DarkLord;
	bool Summoner;
	bool RageFighter;
	int Level;
};

struct PMSG_SEND_PARTYLIST
{
	PSWMSG_HEAD header;
	int Count;
};

struct PMSG_PARTYLIST
{
	char Name[11];
	int Map;
	int X;
	int Y;
	int Level;
	bool DarkWizard;
	bool DarkKnight;
	bool Elf;
	bool MagicGladiator;
	bool DarkLord;
	bool Summoner;
	bool RageFighter;
	bool OnlyGuild;
	bool IsSameGuild;
	int Count;
};

struct PMSG_PARTYSETTINGS_SEND
{
	PSBMSG_HEAD header;
	bool SystemActive;
	bool OnlyGuild;
	bool OneClass;
	bool DarkWizard;
	bool DarkKnight;
	bool Elf;
	bool MagicGladiator;
	bool DarkLord;
	bool Summoner;
	bool RageFighter;
	int Level;
};

struct PMSG_PARTY_REQ_REQ
{
	PSBMSG_HEAD header;
	char Name[11];
};

struct REGISTERED_INFO
{
	char Name[11];
	int aIndex;
	bool OnlyGuild;
	bool OneClass;
	bool DarkWizard;
	bool DarkKnight;
	bool Elf;
	bool MagicGladiator;
	bool DarkLord;
	bool Summoner;
	bool RageFighter;
	int Level;
	BYTE IsOnline;
};

class cPartySearch
{
public:
	cPartySearch();
	~cPartySearch();
	void Load(char* filename);
	void PartySearchAddToList(PMSG_RECV_PARTYSEARCH_ADD* lpMsg, int aIndex);
	void PartySearchDelFromList(char* Name);
	void GCPartyListSend(int aIndex);
	bool Dialog(LPOBJ lpObj, LPOBJ lpNpc);
	void RequestParty(PMSG_PARTY_REQ_REQ* lpMsg, int aIndex);
	void SendPartySettings(LPOBJ lpObj);
	// ----
	int NpcID;
	int NpcMap;
	int NpcX;
	int NpcY;
	int DelAfterRelog;
	// ----
	std::map<std::string,REGISTERED_INFO> m_Registered;
}; extern cPartySearch gPartySearch;