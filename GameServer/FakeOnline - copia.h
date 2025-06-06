#pragma once

#include "User.h" // Asumo que User.h o stdafx.h incluye <windows.h> para CRITICAL_SECTION
#include <fstream>
#include <vector>
#include <string>
#include <map> 

#if USE_FAKE_ONLINE == TRUE

struct OFFEXP_DATA
{
	char Account[11];
	char Password[11];
	char Name[11];
	WORD SkillID;
	WORD UseBuffs[3];
	int TuNhatItem;
	int TuDongReset;
	int GateNumber;
	int MapX;
	int MapY;
	int PhamViTrain;
	int TimeReturn;
	int PostKhiDie;
	int PVPMode; // Leído del IA.xml
	int PartyMode;
	int MoveRange;
};

// ++ NUEVA ESTRUCTURA PARA ESTADO DE COMBATE PVP ACTIVO PARA CHAT ++
struct BotActivePVPCombatState
{
	bool isInActiveCombat;
	DWORD lastPVPActionTick;      // Timestamp del último evento de combate PVP relevante
	bool saidInitialPVPPhrase; // Para controlar la frase "al inicio" del combate

	BotActivePVPCombatState() : isInActiveCombat(false), lastPVPActionTick(0), saidInitialPVPPhrase(false) {}
};
// ++ FIN NUEVA ESTRUCTURA ++

class CFakeOnline
{
public:
	CFakeOnline();
	~CFakeOnline(); 

	void RestoreFakeOnline();
	void AttemptRandomBotComment(int aIndex);
	OFFEXP_DATA* GetOffExpInfo(LPOBJ lpObj);
	OFFEXP_DATA* GetOffExpInfoByAccount(LPOBJ lpObj);
	void LoadFakeData(char* path);

	void FakeAttackProc(LPOBJ lpObj);
	void Attack(int UserIndex);
	void OnAttackAlreadyConnected(LPOBJ lpObj);
	int NhatItem(int aIndex);
	void QuayLaiToaDoGoc(int aIndex);
	void PostChatMSG(LPOBJ lpObj); 
	int	IndexMsgMin; 
	int	IndexMsgMax; 
	void SuDungMauMana(int aIndex);
	void TuDongBuffSkill(int aIndex);
	void TuDongDanhSkill(int aIndex); // Se modificará para actualizar el estado de combate PVP
	bool GetTargetPlayer(LPOBJ lpObj, int SkillNumber, int* MonsterIndex); 
	bool GetTargetMonster(LPOBJ lpObj, int SkillNumber, int* MonsterIndex);
	void SendSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber);
	void SendMultiSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber);
	void SendDurationSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber);
	void SendRFSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber);
	void GuiYCParty(int aIndex, int bIndex);
	void InitializeBotPhrases(); 
	
public:
	std::map<std::string, OFFEXP_DATA> m_Data;
	int AccountsRestored;
	DWORD TimeFakeLogIn;
    std::map<int, BotActivePVPCombatState> m_botPVPCombatStates; // ++ NUEVO MAPA PARA ESTADOS DE COMBATE PVP ++


private:
	std::vector<std::string> m_BotPhrases; 
	DWORD m_dwLastCommentTick[MAX_OBJECT];
    CRITICAL_SECTION m_BotDataMutex; 
};
   
// Funciones globales y variables externas
void LoadBotPhrasesFromFile(const char* filename); 
std::string GetRandomBotPhrase(bool realPlayerNearby, bool inParty, bool inActivePVPCombat); // Parámetro cambiado

extern std::vector<std::string> g_BotPhrasesGeneral;
extern std::vector<std::string> g_BotPhrasesNear;
extern std::vector<std::string> g_BotPhrasesInParty; 
extern std::vector<std::string> g_BotPhrasesPVP; 
extern CFakeOnline s_FakeOnline;

#endif // USE_FAKE_ONLINE == TRUE