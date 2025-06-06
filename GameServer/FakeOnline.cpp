#include "stdafx.h" 
#include "FakeOnline.h"
#include "ItemManager.h"
#include "Map.h"
#include "MasterSkillTree.h"
#include "Notice.h"
#include "SkillManager.h"
#include "SocketManager.h"
#include "Viewport.h"
#include "Util.h"
#include "MemScript.h" 
#include "Path.h"
#include "Party.h"
#include "EffectManager.h"
#include "MapManager.h" 
#include "Message.h"
#include "Monster.h"
#include "DSProtocol.h" 
#include "Protocol.h"   
#include "Quest.h"
#include "QuestObjective.h"
#include <list>
#include <string>
#include "JSProtocol.h"
#include "ObjectManager.h"
#include "OfflineMode.h" 
#include "Move.h" 
#include "CommandManager.h"
#include "Gate.h"
#include "ItemLevel.h"
#include "ServerInfo.h" 
#include "MapServerManager.h"
#include "CustomAttack.h" 
#include "Attack.h" 
#include "Log.h" 
#include <random> 
#include <string> 
#include "DefaultClassInfo.h" 
#include <cmath> 
#include "pugixml.hpp" 

#if USE_FAKE_ONLINE == TRUE // INICIO DEL BLOQUE CONDICIONAL

CFakeOnline s_FakeOnline;

// --- Definici�n de variables globales ---
std::vector<std::string> g_BotPhrasesGeneral;
std::vector<std::string> g_BotPhrasesNear;
std::vector<std::string> g_BotPhrasesInParty;
std::vector<std::string> g_BotPhrasesPVP; 
std::map<int, std::vector<std::string>> g_BotPhrasesMapSpecific; 
std::map<int, std::vector<std::string>> g_BotPhrasesClassSpecific;
int g_ProbGeneral = 10, g_ProbNearRealPlayer = 15, g_ProbInParty = 12, g_ProbPVP = 20, g_ProbMapSpecificBase = 10, g_ProbClassSpecificBase = 11; 
// --- Fin de variables globales ---

// --- Funciones de Ayuda (static para evitar conflictos) ---
static bool FakeisJewels(int index) 
{
	if (index == GET_ITEM(12, 15) || index == GET_ITEM(14, 13) || index == GET_ITEM(14, 14) || 
		index == GET_ITEM(14, 16) || index == GET_ITEM(14, 22) || index == GET_ITEM(14, 31) || 
		index == GET_ITEM(14, 42) || index == GET_ITEM(14, 41) || index == GET_ITEM(14, 43) || 
		index == GET_ITEM(12, 30) || index == GET_ITEM(12, 31) || index == GET_ITEM(12, 42) || 
		index == GET_ITEM(12, 43) || index == GET_ITEM(12, 44)   
        ) 
	{
		return true;
	}
	return false;
}

static int random_bot_range(int minN, int maxN)
{ 
	if (minN > maxN) { 
        int temp = minN;
        minN = maxN;
        maxN = temp;
    }
    if (minN == maxN) return minN;
	return minN + rand() % (maxN - minN + 1);
}
// --- Fin Funciones de Ayuda ---

// --- Prototipos de funciones globales ---
void FakeAutoRepair(int aIndex); 
void FakeAnimationMove(int aIndex, int x, int y, bool dixa);
// --- Fin Prototipos ---


CFakeOnline::CFakeOnline()
{
	InitializeCriticalSection(&this->m_BotDataMutex);
	this->m_Data.clear();
	this->IndexMsgMax = 0;
	this->IndexMsgMin = 0;
	for (int i = 0; i < MAX_OBJECT; ++i)																												 
	{
		this->m_dwLastCommentTick[i] = 0;
		this->m_dwLastPlayerNearbyCommentTick[i] = 0; 
	}
}

CFakeOnline::~CFakeOnline()
{
    DeleteCriticalSection(&this->m_BotDataMutex);
}

OFFEXP_DATA* CFakeOnline::GetOffExpInfo(LPOBJ lpObj)
{
	if (!lpObj || !lpObj->Account[0]) return nullptr; 
    EnterCriticalSection(&this->m_BotDataMutex); 
	std::map<std::string, OFFEXP_DATA>::iterator it = this->m_Data.find(lpObj->Account);
	if (it != this->m_Data.end())
	{
		if (strcmp(lpObj->Name, it->second.Name) == 0)
		{
            LeaveCriticalSection(&this->m_BotDataMutex); 
			return &it->second;
		}
	}
    LeaveCriticalSection(&this->m_BotDataMutex); 
	return nullptr;
}

OFFEXP_DATA* CFakeOnline::GetOffExpInfoByAccount(LPOBJ lpObj)
{
    EnterCriticalSection(&this->m_BotDataMutex);
    OFFEXP_DATA* result = nullptr; 
    if (lpObj && lpObj->Account[0]) { 
        std::map<std::string, OFFEXP_DATA>::iterator it = this->m_Data.find(lpObj->Account);
        if (it != this->m_Data.end()) {
            result = &it->second;
        }
    }
    LeaveCriticalSection(&this->m_BotDataMutex);
    return result;
}

void CFakeOnline::LoadFakeData(char* path)
{
    EnterCriticalSection(&this->m_BotDataMutex);
    this->m_Data.clear(); 
    this->m_botPVPCombatStates.clear(); 
    this->IndexMsgMax = 0; this->IndexMsgMin = 0;
    LoadBotPhrasesFromFile(".\\BotPhrases.txt"); 
    if (!path) { LeaveCriticalSection(&this->m_BotDataMutex); return; }
    pugi::xml_document file;
    if (file.load_file(path).status != pugi::status_ok){ ErrorMessageBox("XML Load Fail: %s", path); LeaveCriticalSection(&this->m_BotDataMutex); return; }
    pugi::xml_node Recipe = file.child("MSGThongBao");
    if (Recipe) { this->IndexMsgMin = Recipe.attribute("IndexMesMin").as_int(); this->IndexMsgMax = Recipe.attribute("IndexMesMax").as_int(); }
    pugi::xml_node oFakeOnlineData = file.child("FakeOnlineData");
    if (oFakeOnlineData) {
        for (pugi::xml_node rInfoData = oFakeOnlineData.child("Info"); rInfoData; rInfoData = rInfoData.next_sibling()){
            OFFEXP_DATA info; memset(&info, 0, sizeof(info));
            strncpy_s(info.Account, rInfoData.attribute("Account").as_string(""), _TRUNCATE);
            strncpy_s(info.Password, rInfoData.attribute("Password").as_string(""), _TRUNCATE);
            strncpy_s(info.Name, rInfoData.attribute("Name").as_string(""), _TRUNCATE);
            info.SkillID = rInfoData.attribute("SkillID").as_int(0); info.PVPMode = rInfoData.attribute("PVPMode").as_int(0); 
            info.UseBuffs[0] = rInfoData.attribute("UseBuffs_0").as_int(0); info.UseBuffs[1] = rInfoData.attribute("UseBuffs_1").as_int(0); info.UseBuffs[2] = rInfoData.attribute("UseBuffs_2").as_int(0);
            info.GateNumber = rInfoData.attribute("GateNumber").as_int(0); info.MapX = rInfoData.attribute("MapX").as_int(0); info.MapY = rInfoData.attribute("MapY").as_int(0);
            info.PhamViTrain = rInfoData.attribute("PhamViTrain").as_int(0); info.MoveRange = rInfoData.attribute("MoveRange").as_int(0); info.TimeReturn = rInfoData.attribute("TimeReturn").as_int(0);
            info.TuNhatItem = rInfoData.attribute("TuNhatItem").as_int(0); info.TuDongReset = rInfoData.attribute("TuDongReset").as_int(0);
            info.PartyMode = rInfoData.attribute("PartyMode").as_int(0); info.PostKhiDie = rInfoData.attribute("PostKhiDie").as_int(0);
            if (strlen(info.Account) > 0) { this->m_Data.insert(std::pair<std::string, OFFEXP_DATA>(info.Account, info));}
        }
    }
    LeaveCriticalSection(&this->m_BotDataMutex);
}

void LoadBotPhrasesFromFile(const char* filename)
{
    g_BotPhrasesGeneral.clear(); g_BotPhrasesNear.clear(); g_BotPhrasesInParty.clear(); 
    g_BotPhrasesPVP.clear(); g_BotPhrasesMapSpecific.clear(); g_BotPhrasesClassSpecific.clear(); 
    std::ifstream file(filename); 
    if (!file.is_open()) { return; }
    std::string line; int mode = 0; 
    const int MODE_NONE = 0, MODE_GENERAL = 1, MODE_NEAR = 2, MODE_IN_PARTY = 3, MODE_PVP = 4, MODE_MAP_SPECIFIC = 5, MODE_CLASS_SPECIFIC = 6; 
    int currentMapIndexForPhrases = -1, currentDBClassForPhrases = -1; 
    try {
        while (std::getline(file, line)) { 
            if (line.empty() || line[0] == ';') continue;
            if (line[0] == '#') {
                std::string selector_full = line; std::string selector_category = line; size_t commaPos = line.find(',');
                if (commaPos != std::string::npos) selector_category = line.substr(0, commaPos); 
                selector_category.erase(0, selector_category.find_first_not_of(" \t")); selector_category.erase(selector_category.find_last_not_of(" \t") + 1);
                if (selector_category == "#GENERAL") { mode = MODE_GENERAL; if (commaPos != std::string::npos) { try { g_ProbGeneral = std::stoi(line.substr(commaPos + 1)); } catch (const std::exception&) {}}}
                else if (selector_category == "#NEAR_REAL_PLAYER") { mode = MODE_NEAR; if (commaPos != std::string::npos) { try { g_ProbNearRealPlayer = std::stoi(line.substr(commaPos + 1)); } catch (const std::exception&) {}}}
                else if (selector_category == "#IN_PARTY") { mode = MODE_IN_PARTY; if (commaPos != std::string::npos) { try { g_ProbInParty = std::stoi(line.substr(commaPos + 1)); } catch (const std::exception&) {}}}
                else if (selector_category == "#PVP_MODE") { mode = MODE_PVP; if (commaPos != std::string::npos) { try { g_ProbPVP = std::stoi(line.substr(commaPos + 1)); } catch (const std::exception&) {}}}
                else if (selector_category == "#MAP_BASE_PROB") { if (commaPos != std::string::npos) { try { g_ProbMapSpecificBase = std::stoi(line.substr(commaPos + 1)); } catch (const std::exception&) {}} mode = MODE_NONE; }
                else if (selector_category == "#CLASS_BASE_PROB") { if (commaPos != std::string::npos) { try { g_ProbClassSpecificBase = std::stoi(line.substr(commaPos + 1)); } catch (const std::exception&) {}} mode = MODE_NONE;}
                else if (selector_category.rfind("#MAP_", 0) == 0) { try { currentMapIndexForPhrases = std::stoi(selector_category.substr(5)); mode = MODE_MAP_SPECIFIC; } catch (const std::exception&) { mode = MODE_NONE;}}
                else if (selector_category.rfind("#CLASS_", 0) == 0) { try { currentDBClassForPhrases = std::stoi(selector_category.substr(7)); mode = MODE_CLASS_SPECIFIC; } catch (const std::exception&) { mode = MODE_NONE;}}
                else mode = MODE_NONE;
                continue;
            }
            std::string originalLine = line; try { size_t first = line.find_first_not_of(" \t\n\r\f\v"); if (std::string::npos == first) line.clear(); else { size_t last = line.find_last_not_of(" \t\n\r\f\v"); line = line.substr(first, (last - first + 1));}} catch (const std::out_of_range&) { line = originalLine; }
            if (line.empty()) continue;
            switch (mode) {
                case MODE_GENERAL: g_BotPhrasesGeneral.push_back(line); break; case MODE_NEAR: g_BotPhrasesNear.push_back(line); break;
                case MODE_IN_PARTY: g_BotPhrasesInParty.push_back(line); break; case MODE_PVP: g_BotPhrasesPVP.push_back(line); break;
                case MODE_MAP_SPECIFIC: if (currentMapIndexForPhrases != -1) g_BotPhrasesMapSpecific[currentMapIndexForPhrases].push_back(line); break;
                case MODE_CLASS_SPECIFIC: if (currentDBClassForPhrases != -1) g_BotPhrasesClassSpecific[currentDBClassForPhrases].push_back(line); break;
            }
        }
    } catch (...) {}
    if (file.is_open()) file.close();
}

std::string GetRandomBotPhrase(int botDBClass, int currentMap, bool realPlayerNearby, bool inParty, bool inActivePVPCombat)
{
    const std::vector<std::string>* pSelectedList = nullptr;
    if (inActivePVPCombat && !g_BotPhrasesPVP.empty()) { pSelectedList = &g_BotPhrasesPVP; }
    else if (inParty && !g_BotPhrasesInParty.empty()) { pSelectedList = &g_BotPhrasesInParty; }
    else if (realPlayerNearby && !g_BotPhrasesNear.empty()) { pSelectedList = &g_BotPhrasesNear; }
    else {
        auto itClass = g_BotPhrasesClassSpecific.find(botDBClass);
        if (itClass != g_BotPhrasesClassSpecific.end() && !itClass->second.empty()) { pSelectedList = &itClass->second;}
        else {
            auto itMap = g_BotPhrasesMapSpecific.find(currentMap);
            if (itMap != g_BotPhrasesMapSpecific.end() && !itMap->second.empty()) { pSelectedList = &itMap->second; }
        }
    }
    if (pSelectedList == nullptr || pSelectedList->empty()) { if (!g_BotPhrasesGeneral.empty()) { pSelectedList = &g_BotPhrasesGeneral; }}
    if (pSelectedList == nullptr || pSelectedList->empty()) return ""; 
    return (*pSelectedList)[rand() % pSelectedList->size()];
}

void CFakeOnline::AttemptRandomBotComment(int aIndex)
{
    EnterCriticalSection(&this->m_BotDataMutex);
    if (!USE_FAKE_ONLINE) { LeaveCriticalSection(&this->m_BotDataMutex); return; }
    LPOBJ lpObj = &gObj[aIndex]; 
    if (lpObj->Connected != OBJECT_ONLINE || lpObj->Type != OBJECT_USER) { LeaveCriticalSection(&this->m_BotDataMutex); return; }
    OFFEXP_DATA* pBotData = this->GetOffExpInfo(lpObj); 
    if (pBotData == nullptr) { LeaveCriticalSection(&this->m_BotDataMutex); return; }
    bool forcePVPChat = false; bool posted = false; bool realPlayerNearby = false; char nearbyPlayerName[11] = {0}; 
    for (int i = 0; i < MAX_VIEWPORT; i++) {
        if (lpObj->VpPlayer2[i].type == OBJECT_USER) { 
            int targetIndex = lpObj->VpPlayer2[i].index;
            if (OBJMAX_RANGE(targetIndex) && targetIndex != aIndex) { 
                LPOBJ lpTarget = &gObj[targetIndex];
                if (lpTarget->Connected == OBJECT_ONLINE && lpTarget->IsFakeOnline == 0) { 
                    realPlayerNearby = true; strncpy_s(nearbyPlayerName, sizeof(nearbyPlayerName), lpTarget->Name, _TRUNCATE); break; 
                }
            }
        }
    }
    bool isInParty = (lpObj->PartyNumber >= 0); bool botInActivePVPCombat = false; BotActivePVPCombatState* pvpState = nullptr;
    std::map<int, BotActivePVPCombatState>::iterator itPVPState = this->m_botPVPCombatStates.find(aIndex);
    if (itPVPState != this->m_botPVPCombatStates.end()) {
        pvpState = &itPVPState->second;
        if (pvpState->isInActiveCombat) {
            if ((GetTickCount() - pvpState->lastPVPActionTick) < 30000) botInActivePVPCombat = true;
            else { pvpState->isInActiveCombat = false; pvpState->saidInitialPVPPhrase = false; }
        }
    }
    const int GENERAL_COMMENT_COOLDOWN_MS = 480000; const int PLAYER_NEARBY_COMMENT_COOLDOWN_MS = 20000; 
    int probability = 0; bool canCommentNow = false; DWORD currentTick = GetTickCount(); std::string commentContextLog = "N/A";
    if (botInActivePVPCombat && pvpState && !pvpState->saidInitialPVPPhrase && (currentTick - pvpState->lastPVPActionTick) < 5000) {
        canCommentNow = true; forcePVPChat = true; probability = 80; commentContextLog = "Burst PVP Inicial";
    } else if (realPlayerNearby && nearbyPlayerName[0] != '\0') {
        if ((currentTick - this->m_dwLastPlayerNearbyCommentTick[aIndex]) >= PLAYER_NEARBY_COMMENT_COOLDOWN_MS) {
            canCommentNow = true; probability = g_ProbNearRealPlayer; commentContextLog = "Jugador Cercano";
        }
    }
    if (!canCommentNow) {
        if ((currentTick - this->m_dwLastCommentTick[aIndex]) >= GENERAL_COMMENT_COOLDOWN_MS) {
            canCommentNow = true;
            if (botInActivePVPCombat) { probability = g_ProbPVP; commentContextLog = "ActivePVPCombat (General CD)"; }
            else if (pBotData->PVPMode >= 1) { probability = g_ProbPVP; commentContextLog = "PVPMode General (General CD)"; }
            else if (isInParty) { probability = g_ProbInParty; commentContextLog = "InParty (General CD)"; }
            else { 
                auto itClassCheck = g_BotPhrasesClassSpecific.find(lpObj->DBClass);
                if (itClassCheck != g_BotPhrasesClassSpecific.end() && !itClassCheck->second.empty()){ probability = g_ProbClassSpecificBase; commentContextLog = "ClassSpecific (General CD)"; }
                else {
                    auto itMapCheck = g_BotPhrasesMapSpecific.find(lpObj->Map);
                    if (itMapCheck != g_BotPhrasesMapSpecific.end() && !itMapCheck->second.empty()){ probability = g_ProbMapSpecificBase; commentContextLog = "MapSpecific (General CD)"; }
                    else { probability = g_ProbGeneral; commentContextLog = "General (General CD)"; }
                }
            }
        }
    }
    if (canCommentNow) {
        if (forcePVPChat || (rand() % 100) < probability) { 
            std::string phrase = GetRandomBotPhrase(lpObj->DBClass, lpObj->Map, realPlayerNearby, isInParty, botInActivePVPCombat); 
            if (!phrase.empty()) {
                std::string processedPhrase = phrase; const std::string placeholder = "{player_name}";
                if (realPlayerNearby && nearbyPlayerName[0] != '\0') { 
                    size_t placeholderPos = processedPhrase.find(placeholder);
                    if (placeholderPos != std::string::npos) {
                        processedPhrase.replace(placeholderPos, placeholder.length(), nearbyPlayerName);
                    }
                }
                char msg[MAX_CHAT_MESSAGE_SIZE +1] = {0}; strncpy_s(msg, sizeof(msg), processedPhrase.c_str(), _TRUNCATE);
                if(gServerInfo.m_CommandPostType == 0) { PostMessage1(lpObj->Name,gMessage.GetMessage(69),msg); posted = true; }
                else if(gServerInfo.m_CommandPostType == 1) { PostMessage2(lpObj->Name,gMessage.GetMessage(69),msg); posted = true; }
                else if(gServerInfo.m_CommandPostType == 2) { PostMessage3(lpObj->Name,gMessage.GetMessage(69),msg); posted = true; }
                else if(gServerInfo.m_CommandPostType == 3) { PostMessage4(lpObj->Name,gMessage.GetMessage(69),msg); posted = true; }
                else if(gServerInfo.m_CommandPostType == 4) { PostMessage5(lpObj->Name,gMessage.GetMessage(69),msg); posted = true; } 
                else if(gServerInfo.m_CommandPostType == 5) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(),0,lpObj->Name,msg); posted = true; }
                else if(gServerInfo.m_CommandPostType == 6) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(),1,lpObj->Name,msg); posted = true; }
                else if(gServerInfo.m_CommandPostType == 7) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(),2,lpObj->Name,msg); posted = true; }
                else if(gServerInfo.m_CommandPostType == 8) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(),3,lpObj->Name,msg); posted = true; }
                else if(gServerInfo.m_CommandPostType == 9) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(),4,lpObj->Name,msg); posted = true; }
                else { if (gCommandManager.CommandPost(lpObj, msg)) { posted = true;} else { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(),0,lpObj->Name,msg); posted = true;} }
                if (posted) {
                    if (commentContextLog == "Jugador Cercano") { this->m_dwLastPlayerNearbyCommentTick[aIndex] = currentTick; }
                    this->m_dwLastCommentTick[aIndex] = currentTick; 
                    if (forcePVPChat && pvpState) { pvpState->saidInitialPVPPhrase = true; }
                }
            } 
        }
    }
    LeaveCriticalSection(&this->m_BotDataMutex);
}

void CFakeOnline::RestoreFakeOnline()
{
	for (std::map<std::string, OFFEXP_DATA>::iterator it = this->m_Data.begin(); it != this->m_Data.end(); it++)
	{
		if (gObjFindByAcc(it->second.Account) != 0) continue;
		int aIndex = gObjAddSearch(0, "127.0.0.1");
		if (aIndex >= 0)
		{
			char account[11] = { 0 }; memcpy(account, it->second.Account, (sizeof(account) - 1));
			char password[11] = { 0 }; memcpy(password, it->second.Password, (sizeof(password) - 1));
			gObjAdd(0, "127.0.0.1", aIndex); 
			LPOBJ lpObj = &gObj[aIndex]; 
			lpObj->LoginMessageSend++; lpObj->LoginMessageSend++; lpObj->LoginMessageCount++;
			lpObj->ConnectTickCount = GetTickCount(); lpObj->ClientTickCount = GetTickCount(); lpObj->ServerTickCount = GetTickCount();
			lpObj->MapServerMoveRequest = 0; lpObj->LastServerCode = -1; lpObj->DestMap = -1; lpObj->DestX = 0; lpObj->DestY = 0;
			GJConnectAccountSend(aIndex, account, password, "127.0.0.1");
			lpObj->Socket = INVALID_SOCKET;
            lpObj->IsFakeOnline = 1; lpObj->AttackCustom = 0; lpObj->m_OfflineMode = 0; 
            lpObj->AttackCustomDelay = GetTickCount(); 
            lpObj->FakeBotPartyInviteCooldownTick = 0; 
            lpObj->IsFakeTimeLag = 0; lpObj->m_OfflineMoveDelay = 0; 
            lpObj->IsFakePVPMode = it->second.PVPMode; 
            gObjectManager.CharacterCalcAttribute(aIndex); 
            LogAdd(LOG_RED, "[FakeOnline]  [TK: %s NV: %s][Cls:%d] Da Online Vao Server. PVPMode:%d. PhysiSpeed:%d. DBClass:%d", 
                   it->second.Account, it->second.Name, lpObj->Class, lpObj->IsFakePVPMode, lpObj->PhysiSpeed, lpObj->DBClass);
		}
	}
}



void FakeAnimationMove(int aIndex, int x, int y, bool dixa) 
{
	LPOBJ lpObj = &gObj[aIndex];
	BYTE path[8] = {0}; 
	if (lpObj->RegenOk > 0) { return; } if (lpObj->Teleport != 0) { return; } if (gObjCheckMapTile(lpObj, 1) != 0) { return; }
	if (gEffectManager.CheckStunEffect(lpObj) != 0 || gEffectManager.CheckImmobilizeEffect(lpObj) != 0) { return; }
	if (lpObj->SkillSummonPartyTime != 0) { lpObj->SkillSummonPartyTime = 0; gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(272));}
	lpObj->Dir = path[0] >> 4; lpObj->Rest = 0; lpObj->PathCur = 0; lpObj->PathCount = path[0] & 0x0F; lpObj->LastMoveTime = GetTickCount();
	memset(lpObj->PathX, 0, sizeof(lpObj->PathX)); memset(lpObj->PathY, 0, sizeof(lpObj->PathY)); memset(lpObj->PathOri, 0, sizeof(lpObj->PathOri));
	lpObj->TX = x; lpObj->TY = y; lpObj->PathCur = ((lpObj->PathCount > 0) ? 1 : 0); lpObj->PathCount = ((lpObj->PathCount > 0) ? (lpObj->PathCount + 1) : lpObj->PathCount); 
	lpObj->PathStartEnd = 1; lpObj->PathX[0] = x; lpObj->PathY[0] = y; lpObj->PathDir[0] = lpObj->Dir; 
	for (int n = 1; n < lpObj->PathCount; n++) {
		if ((n % 2) == 0) {
			lpObj->TX = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 0];
			lpObj->TY = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 1];
			lpObj->PathX[n] = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 0];
			lpObj->PathY[n] = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 1];
			lpObj->PathOri[n - 1] = path[((n + 1) / 2)] & 0x0F; lpObj->PathDir[n + 0] = path[((n + 1) / 2)] & 0x0F;
		} else {
			lpObj->TX = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 0];
			lpObj->TY = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 1];
			lpObj->PathX[n] = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 0];
			lpObj->PathY[n] = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 1];
			lpObj->PathOri[n - 1] = path[((n + 1) / 2)] / 0x10; lpObj->PathDir[n + 0] = path[((n + 1) / 2)] / 0x10;
		}
	} 
	gMap[lpObj->Map].DelStandAttr(lpObj->OldX, lpObj->OldY);
	if (dixa == true) {
		int RandX = rand() % 3 + 1; int RandY = rand() % 3 + 1; BYTE wall = 0;
		if (x > lpObj->X) { wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X + RandX, lpObj->Y); if (wall == 1) lpObj->X += RandX;
		} else if (x <  lpObj->X) { wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X - RandX, lpObj->Y); if (wall == 1)  lpObj->X -= RandX; }
		if (y > lpObj->Y) { wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X, lpObj->Y + RandY); if (wall == 1) lpObj->Y += RandY;
		} else if (y <  lpObj->Y) { wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X, lpObj->Y - RandY); if (wall == 1) lpObj->Y -= RandY; }
	} else { lpObj->X = x; lpObj->Y = y; }
	lpObj->TX = lpObj->X; lpObj->TY = lpObj->Y; lpObj->OldX = lpObj->TX; lpObj->OldY = lpObj->TY; lpObj->ViewState = 0;
	gMap[lpObj->Map].SetStandAttr(lpObj->TX, lpObj->TY); PMSG_MOVE_SEND pMsgSend; 
	pMsgSend.header.set(PROTOCOL_CODE1, sizeof(pMsgSend)); pMsgSend.index[0] = SET_NUMBERHB(lpObj->Index); pMsgSend.index[1] = SET_NUMBERLB(lpObj->Index);
	pMsgSend.x = (BYTE)lpObj->TX; pMsgSend.y = (BYTE)lpObj->TY; pMsgSend.dir = lpObj->Dir << 4;
	{ lpObj->PathCur = 0; lpObj->PathCount = 0; lpObj->TX = lpObj->X; lpObj->TY = lpObj->Y; pMsgSend.x = (BYTE)lpObj->X; pMsgSend.y = (BYTE)lpObj->Y; }
	for (int n_vp = 0; n_vp < MAX_VIEWPORT; n_vp++) { 
		if (lpObj->VpPlayer2[n_vp].type == OBJECT_USER) {
			if (lpObj->VpPlayer2[n_vp].state != OBJECT_EMPTY && lpObj->VpPlayer2[n_vp].state != OBJECT_DIECMD && lpObj->VpPlayer2[n_vp].state != OBJECT_DIED) {
				DataSend(lpObj->VpPlayer2[n_vp].index, (BYTE*)&pMsgSend, pMsgSend.header.size);
			}
		}
	}
}

//void CFakeOnline::CheckAutoReset(LPOBJ lpObj)
//{
//    // ... (Tu c�digo de CheckAutoReset, asegur�ndote que el nivel de reset sea 400 o la variable correcta de gServerInfo)
//    // ... (Y que las llamadas a gDefaultClassInfo usen .m_DefaultClassInfo[DBClass].Stat)
//    // ... (Y que las funciones GC...Send sean las correctas o est�n comentadas si no existen)
//}
void FakeAutoRepair(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex)) { return; } 
    LPOBJ lpObj = &gObj[aIndex];
	for (int n = 0; n < INVENTORY_WEAR_SIZE; ++n) {
		if (lpObj->Inventory[n].IsItem() != 0) {
			gItemManager.RepairItem(lpObj, &lpObj->Inventory[n], n, 1); 
		}
	}
}

// --- COPIA AQU� LAS DEFINICIONES COMPLETAS DE LAS SIGUIENTES FUNCIONES MIEMBRO DE CFakeOnline ---
// --- DESDE TU �LTIMO ARCHIVO FakeOnline.cpp FUNCIONAL:
// void CFakeOnline::QuayLaiToaDoGoc(int aIndex) { /*...*/ }
// void CFakeOnline::SuDungMauMana(int aIndex) { /*...*/ }
// void CFakeOnline::TuDongBuffSkill(int aIndex) { /*...*/ }
// void CFakeOnline::TuDongDanhSkill(int aIndex) { /*...*/ } // Incluye los logs de velocidad de ataque
// bool CFakeOnline::GetTargetMonster(LPOBJ lpObj, int SkillNumber, int* MonsterIndex) { /*...*/ }
// bool CFakeOnline::GetTargetPlayer(LPOBJ lpObj, int SkillNumber, int* MonsterIndex) { /*...*/ }
// void CFakeOnline::SendSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber) { /*...*/ }
// void CFakeOnline::SendMultiSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber) { /*...*/ }
// void CFakeOnline::SendDurationSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber) { /*...*/ }
// void CFakeOnline::SendRFSkillAttack(LPOBJ lpObj, int aIndex, int SkillNumber) { /*...*/ }
// void CFakeOnline::GuiYCParty(int aIndex, int bIndex) { /*...*/ }

// --- Y LA FUNCI�N GLOBAL FakeAnimationMove SI NO EST� YA DEFINIDA ARRIBA ---
// void FakeAnimationMove(int aIndex, int x, int y, bool dixa) { /*...*/ }

void CFakeOnline::FakeAttackProc(LPOBJ lpObj)
{
	if (lpObj->IsFakeOnline != 0) {
		lpObj->CheckSumTime = GetTickCount();
		lpObj->ConnectTickCount = GetTickCount();
	}
}

void CFakeOnline::OnAttackAlreadyConnected(LPOBJ lpObj)
{
	if (lpObj->IsFakeOnline != 0) {
		lpObj->IsFakeOnline = 0;
		gObjDel(lpObj->Index);
	}
}

void CFakeOnline::Attack(int aIndex)
{
	if (OBJMAX_RANGE(aIndex) == FALSE) { return; }
	if (!gObjIsConnectedGP(aIndex)) { return; }
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->IsFakeOnline == 0 || !lpObj->IsFakeRegen) { return; } 
	if (lpObj->State == OBJECT_DELCMD || lpObj->DieRegen != 0 || lpObj->Teleport != 0 || lpObj->RegenOk > 0 ) { return; } 
	if (gServerInfo.InSafeZone(aIndex) == true) { return; }

    OFFEXP_DATA* pBotData = this->GetOffExpInfo(lpObj);
    if (pBotData != nullptr && pBotData->TuNhatItem == 1) {
        if (this->NhatItem(aIndex) == 2) { 
             return; 
        }
    }

	this->SuDungMauMana(aIndex);
	this->TuDongBuffSkill(aIndex);
	this->TuDongDanhSkill(aIndex); 
	FakeAutoRepair(aIndex); 
}


bool FakeitemListPickUp(int Index, int Level, LPOBJ lpObj) 
{
	for (int i = 0; i < lpObj->ObtainPickExtraCount; i++) 
	{
		if (strstr(gItemLevel.GetItemName(Index, Level), lpObj->ObtainPickItemList[i]) != NULL) 
		{
			return true;
		}
	}
	return false;
}

// En FakeOnline.cpp

// ... (includes y otras funciones como estaban) ...
// ... (FakeisJewels, constructor, destructor, GetOffExpInfo, GetOffExpInfoByAccount, LoadFakeData, LoadBotPhrasesFromFile, GetRandomBotPhrase, AttemptRandomBotComment, RestoreFakeOnline como estaban) ...
// ... (Aseg�rate que CheckAutoReset NO est� siendo llamada desde Attack por ahora)


// En FakeOnline.cpp

// ... (includes y otras funciones como estaban) ...
// ... (FakeisJewels, constructor, destructor, GetOffExpInfo, GetOffExpInfoByAccount, LoadFakeData, LoadBotPhrasesFromFile, GetRandomBotPhrase, AttemptRandomBotComment, RestoreFakeOnline como estaban) ...
// ... (Aseg�rate que CheckAutoReset NO est� siendo llamada desde Attack por ahora)
// ... (includes y definiciones globales como estaban) ...
// ... (FakeisJewels, CFakeOnline constructor/destructor, GetOffExpInfo, GetOffExpInfoByAccount, LoadFakeData, LoadBotPhrasesFromFile, GetRandomBotPhrase, AttemptRandomBotComment, RestoreFakeOnline como estaban) ...
// ... (Aseg�rate que la definici�n de CheckAutoReset est� aqu�, pero su llamada en Attack estar� comentada)

// Funci�n de ayuda para mover una casilla hacia un objetivo
// Devuelve true si se intent� mover, false si ya est� en el objetivo o no se puede mover
bool MoveBotOneStepTowards(LPOBJ lpObj, int targetX, int targetY)
{
    if (lpObj->X == targetX && lpObj->Y == targetY)
    {
        return false; // Ya est� en el destino
    }

    int offsetX = 0;
    int offsetY = 0;

    if (lpObj->X < targetX) offsetX = 1;
    else if (lpObj->X > targetX) offsetX = -1;

    if (lpObj->Y < targetY) offsetY = 1;
    else if (lpObj->Y > targetY) offsetY = -1;

    int nextX = lpObj->X + offsetX;
    int nextY = lpObj->Y + offsetY;

    // Comprobar si la siguiente casilla es v�lida y caminable (ATTR_WALL = 1)
    // Tu gMap[map_num].CheckAttr puede tener diferentes flags. ATTR_WALL suele ser 1.
    // Revisa c�mo verificas si una casilla es caminable en tu c�digo.
    // Si gMap[map_num].CheckAttr(nextX, nextY, 1) es true si hay pared, entonces la condici�n es == 0.
    // Si gMap[map_num].m_MapAttr[nextY * gMap[map_num].m_width + nextX] & 1 es pared...
    // Por ahora, asumir� que un valor de atributo bajo (ej. 0) es caminable.
    // ��DEBES AJUSTAR ESTA VERIFICACI�N DE PARED A TU C�DIGO!!
    BYTE attr = gMap[lpObj->Map].GetAttr(nextX, nextY); 
    if ((attr & 1) == 0 && (attr & 4) == 0 && (attr & 8) == 0) // Ejemplo: No es pared, no es zona segura (si aplica), no es agua (si aplica)
    {
        // Usar FakeAnimationMove para el movimiento de un solo paso podr�a ser excesivo
        // o podr�as tener una funci�n m�s simple para mover un paso.
        // Por ahora, usaremos FakeAnimationMove para consistencia con tu c�digo.
        LogAdd(LOG_BLUE, "[MoveBotOneStepTowards][%s] Moviendo de %d,%d hacia %d,%d (target %d,%d)", 
               lpObj->Name, lpObj->X, lpObj->Y, nextX, nextY, targetX, targetY);
        FakeAnimationMove(lpObj->Index, nextX, nextY, false);
        return true; // Se intent� mover
    }
    LogAdd(LOG_BLUE, "[MoveBotOneStepTowards][%s] No se pudo mover a %d,%d (attr: %d)", lpObj->Name, nextX, nextY, attr);
    return false; // No se pudo mover (obst�culo)
}


int CFakeOnline::NhatItem(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex)) { return 0; }
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->IsFakeOnline == 0) { return 0; }

	OFFEXP_DATA* pBotData = this->GetOffExpInfo(lpObj);
	if (pBotData == nullptr || pBotData->TuNhatItem == 0) { return 0; }

	if (lpObj->DieRegen != 0 || lpObj->Teleport != 0 || lpObj->State == OBJECT_DELCMD || lpObj->RegenOk > 0) { return 0; }
	if (gServerInfo.InSafeZone(aIndex) == true) { return 0; }

	CMapItem* lpMapItem;
	int distanceToPickup = 3; 
	int map_num = lpObj->Map;
    bool attemptedMoveThisCycle = false; 
    int dis = 0;

	if (MAP_RANGE(map_num) == FALSE) { return 0; }

	for (int n = 0; n < MAX_MAP_ITEM; n++) {
		lpMapItem = &gMap[map_num].m_Item[n];
		if (lpMapItem->IsItem() == TRUE  && lpMapItem->m_Give == false && lpMapItem->m_Live == true) {
			dis = (int)sqrt(pow(((float)lpObj->X - (float)lpMapItem->m_X), 2) + pow(((float)lpObj->Y - (float)lpMapItem->m_Y), 2)); 
			if (dis > distanceToPickup) continue;

			bool bShouldPickThisItem = false;
			if (lpMapItem->m_Index == GET_ITEM(14, 15)) { bShouldPickThisItem = true;} 
			else if (FakeisJewels(lpMapItem->m_Index)) { bShouldPickThisItem = true;}
			
			if (!bShouldPickThisItem) continue; 
            
            if (lpObj->X == lpMapItem->m_X && lpObj->Y == lpMapItem->m_Y) { 
				if (lpMapItem->m_Index == GET_ITEM(14, 15)) { 
					if (!gObjCheckMaxMoney(aIndex, lpMapItem->m_BuyMoney)) { if (lpObj->Money < MAX_MONEY) lpObj->Money = MAX_MONEY; } else lpObj->Money += lpMapItem->m_BuyMoney;
					gMap[map_num].ItemGive(aIndex, n); 
                    GCMoneySend(aIndex, lpObj->Money); 
					LogAdd(LOG_EVENT, "[FakeOnline][%s] RECOGI� Zen: %d", lpObj->Name, lpMapItem->m_BuyMoney);
					return 1; 
				} else { 
				    if (lpMapItem->m_QuestItem != false) continue;
				    CItem itemForInfo;
                    itemForInfo.Convert(lpMapItem->m_Index, lpMapItem->m_Option1, lpMapItem->m_Option2, lpMapItem->m_Option3, lpMapItem->m_NewOption, lpMapItem->m_SetOption, lpMapItem->m_JewelOfHarmonyOption, lpMapItem->m_ItemOptionEx, lpMapItem->m_SocketOption, lpMapItem->m_SocketOptionBonus);
                    itemForInfo.m_Durability = lpMapItem->m_Durability;
                    itemForInfo.m_Level = lpMapItem->m_Level;
                    
				    BYTE resultStack = gItemManager.InventoryInsertItemStack(lpObj, lpMapItem); 
				    if (resultStack != 0xFF) { 
					    gMap[map_num].ItemGive(aIndex, n); 
						LogAdd(LOG_EVENT, "[FakeOnline][%s] RECOGI� (stack) item: %s en slot %d", lpObj->Name, itemForInfo.GetName(), resultStack);
                        gItemManager.GCItemModifySend(aIndex, resultStack); 
                        return 1; 
				    } else { 
                        BYTE posNoStack = gItemManager.InventoryInsertItem(aIndex, itemForInfo); 
                        if (posNoStack != 0xFF) {
                            gMap[map_num].ItemGive(aIndex, n); 
							LogAdd(LOG_EVENT, "[FakeOnline][%s] RECOGI� (no-stack) item: %s en slot %d", lpObj->Name, itemForInfo.GetName(), posNoStack);
                            gItemManager.GCItemModifySend(aIndex, posNoStack);
                            return 1; 
                        } else {
							LogAdd(LOG_ORANGE, "[FakeOnline][%s] Inventario lleno, no pudo recoger: %s", lpObj->Name, itemForInfo.GetName());
                            return 0; 
						}
                    }
                }
			} 
            else if (dis > 0 && !attemptedMoveThisCycle) 
            {
                LogAdd(LOG_BLUE, "[NhatItem][%s] �tem '%s' cerca (Dist: %d). Moviendo a %d,%d.", 
                       lpObj->Name, lpMapItem->GetName(), dis, lpMapItem->m_X, lpMapItem->m_Y);
                FakeAnimationMove(lpObj->Index, lpMapItem->m_X, lpMapItem->m_Y, false);
                attemptedMoveThisCycle = true; 
                return 2; // Indicar a Attack() que se priorice el movimiento
            }
		} 
	} 
	return 0; 
}
void CFakeOnline::PostChatMSG(LPOBJ lpObj) 
{
	OFFEXP_DATA *info = this->GetOffExpInfo(lpObj); 
	if (info != 0 && lpObj->Socket == INVALID_SOCKET) { 
		if (info->PostKhiDie == 1) {
			Sleep(100); 
			if (this->IndexMsgMin >= 0 && this->IndexMsgMax >= 0 && this->IndexMsgMin <= this->IndexMsgMax && (this->IndexMsgMax - this->IndexMsgMin + 1) > 0) { 
				int messageId = rand() % (this->IndexMsgMax - this->IndexMsgMin + 1) + this->IndexMsgMin;
				const char* messageText = gMessage.GetMessage(messageId);
				if (messageText && strlen(messageText) > 0) { 
					if (gServerInfo.m_CommandPostType == 0) { PostMessage1(lpObj->Name, gMessage.GetMessage(69), (char*)messageText); }
					else if (gServerInfo.m_CommandPostType == 1) { PostMessage2(lpObj->Name, gMessage.GetMessage(69), (char*)messageText); }
					else if (gServerInfo.m_CommandPostType == 2) { PostMessage3(lpObj->Name, gMessage.GetMessage(69), (char*)messageText); }
					else if (gServerInfo.m_CommandPostType == 3) { PostMessage4(lpObj->Name, gMessage.GetMessage(69), (char*)messageText); }
                    else if (gServerInfo.m_CommandPostType == 4) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(), 0, lpObj->Name, (char*)messageText); }
					else if (gServerInfo.m_CommandPostType == 5) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(), 1, lpObj->Name, (char*)messageText); }
					else if (gServerInfo.m_CommandPostType == 6) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(), 2, lpObj->Name, (char*)messageText); }
					else if (gServerInfo.m_CommandPostType == 7) { GDGlobalPostSend(gMapServerManager.GetMapServerGroup(), 3, lpObj->Name, (char*)messageText); }
					else { PostMessage1(lpObj->Name, gMessage.GetMessage(69), (char*)messageText); } 
				}
			}
		}
	}
}

void CFakeOnline::QuayLaiToaDoGoc(int aIndex) {
	if (OBJMAX_RANGE(aIndex) == FALSE) { return; }
	if (!gObjIsConnectedGP(aIndex)) { return; }
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->IsFakeOnline == 0) { return; }

	OFFEXP_DATA *info = this->GetOffExpInfo(lpObj); 
	if (info != 0 && lpObj->Socket == INVALID_SOCKET) {
		if (lpObj->State == OBJECT_DELCMD || lpObj->DieRegen != 0 || lpObj->Teleport != 0) { return; }
		int PhamViDiTrain = (int)sqrt(pow(((float)lpObj->X - (float)info->MapX), 2) + pow(((float)lpObj->Y - (float)info->MapY), 2));

		if ((GetTickCount() >= lpObj->IsFakeTimeLag + 30000) && (GetTickCount() >= lpObj->AttackCustomDelay + 30000) && lpObj->IsFakeRegen && (GetTickCount() >= lpObj->m_OfflineMoveDelay + 30000)) {
			lpObj->IsFakeRegen = false;
			lpObj->IsFakeTimeLag = GetTickCount();
			lpObj->m_OfflineMoveDelay = GetTickCount();
			lpObj->AttackCustomDelay = GetTickCount();
			PhamViDiTrain = (lpObj->IsFakeMoveRange + 10); 
			LogAdd(LOG_BLUE, "[FakeOnline][%s] Fix Lag Reset Move", lpObj->Name);
		}

		if (gGate.MapIsInGate(lpObj, info->GateNumber) == 0 || (PhamViDiTrain >= 100 && !lpObj->IsFakeRegen)) {
			gObjMoveGate(lpObj->Index, info->GateNumber);
			LogAdd(LOG_BLUE, "[FakeOnline][%s] Move Gate", lpObj->Name);
			return;
		}
		if (GetTickCount() >= lpObj->m_OfflineTimeResetMove + 2000) {
			if ((PhamViDiTrain >= (lpObj->IsFakeMoveRange + 5) && !lpObj->IsFakeRegen) || gServerInfo.InSafeZone(lpObj->Index) == true) {
				int DiChuyenX = lpObj->X;
				int DiChuyenY = lpObj->Y;
				for (int n = 0; n < 16; n++) { 
					if (lpObj->X > info->MapX) { DiChuyenX -= random_bot_range(1, 3); } 
					else if (lpObj->X < info->MapX){ DiChuyenX += random_bot_range(1, 3); }
					else { DiChuyenX = info->MapX; }

					if (lpObj->Y > info->MapY) { DiChuyenY -= random_bot_range(1, 3); }
					else if (lpObj->Y < info->MapY) { DiChuyenY += random_bot_range(1, 3); }
					else { DiChuyenY = info->MapY; }

					if (DiChuyenX == info->MapX && DiChuyenY == info->MapY) { lpObj->IsFakeRegen = true; }

					BYTE attr = gMap[lpObj->Map].GetAttr(DiChuyenX, DiChuyenY);
					if ((attr & 1) == 0 && (attr & 4) == 0 && (attr & 8) == 0) { 
						lpObj->m_OfflineTimeResetMove = GetTickCount();
						FakeAnimationMove(lpObj->Index, DiChuyenX, DiChuyenY, false);
						LogAdd(LOG_BLUE, "[FakeOnline][%s] Mover a ubicaci�n predeterminada (%d/%d)", lpObj->Name, DiChuyenX, DiChuyenY);
						return;
					}
				}
				return; 
			} else if (!lpObj->IsFakeRegen) {
				lpObj->m_OfflineTimeResetMove = GetTickCount();
				lpObj->IsFakeRegen = true;
			}
		}

		if (lpObj->IsFakeMoveRange != 0) {
			if (GetTickCount() >= lpObj->m_OfflineTimeResetMove + 2000 && lpObj->IsFakeRegen) {
				int MoveRangeVal = 3; 
				int maxmoverange = MoveRangeVal * 2 + 1;
				int searchc = 10;
				BYTE tpx = lpObj->X; 
				BYTE tpy = lpObj->Y;

				while (searchc-- != 0) {
					int randXOffset = (GetLargeRand() % maxmoverange) - MoveRangeVal; 
					int randYOffset = (GetLargeRand() % maxmoverange) - MoveRangeVal;
					tpx = lpObj->X + randXOffset;
					tpy = lpObj->Y + randYOffset;
					
					BYTE attr = gMap[lpObj->Map].GetAttr(tpx, tpy);
					if ((attr & 1) != 1 && (attr & 2) != 2 && (attr & 4) != 4 && (attr & 8) != 8 && GetTickCount() >= lpObj->m_OfflineMoveDelay + 2000) {
						LogAdd(LOG_BLUE, "[FakeOnline] Rango de movimiento (%d,%d)", tpx, tpy);
						lpObj->m_OfflineMoveDelay = GetTickCount();
						FakeAnimationMove(lpObj->Index, tpx, tpy, false);
						return;
					}
				}
			}
		}
		
		if (lpObj->DistanceReturnOn != 0) { 
			if (GetTickCount() >= lpObj->m_OfflineTimeResetMove + 1000 + ((lpObj->DistanceMin * 60) * 1000)) {
				if (lpObj->m_OfflineCoordX != lpObj->X && lpObj->m_OfflineCoordY != lpObj->Y) {
					LogAdd(LOG_BLUE, "[FakeOnline] Volver a Coordenadas de esquina (%d,%d)", lpObj->m_OfflineCoordX, lpObj->m_OfflineCoordY);
					FakeAnimationMove(lpObj->Index, lpObj->m_OfflineCoordX, lpObj->m_OfflineCoordY, false);
					return;
				}
				lpObj->m_OfflineTimeResetMove = GetTickCount();
			}
		}
	}
}

void CFakeOnline::SuDungMauMana(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex)) { return; }
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->RecoveryPotionOn != 0) { 
		if (lpObj->Life > 0 && lpObj->Life < ((lpObj->MaxLife * lpObj->RecoveryPotionPercent) / 100)) {
			PMSG_ITEM_USE_RECV pMsg;
			pMsg.header.set(0x26, sizeof(pMsg));
			pMsg.SourceSlot = 0xFF;
			pMsg.SourceSlot = ((pMsg.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 3), -1) : pMsg.SourceSlot);
			pMsg.SourceSlot = ((pMsg.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 2), -1) : pMsg.SourceSlot);
			pMsg.SourceSlot = ((pMsg.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 1), -1) : pMsg.SourceSlot);
			pMsg.TargetSlot = 0xFF;
			pMsg.type = 0;
			if (INVENTORY_FULL_RANGE(pMsg.SourceSlot) != 0) {
				gItemManager.CGItemUseRecv(&pMsg, lpObj->Index);
			}
		}
	}

	if (lpObj->RecoveryHealOn != 0) { 
		CSkill* RenderSkillHealing = gSkillManager.GetSkill(lpObj, SKILL_HEAL);
		if (RenderSkillHealing != 0) {
			if (lpObj->Life < ((lpObj->MaxLife * lpObj->RecoveryHealPercent) / 100)) {
				if (gEffectManager.CheckEffect(lpObj, gSkillManager.GetSkillEffect(RenderSkillHealing->m_index)) == 0) {
					gSkillManager.UseAttackSkill(lpObj->Index, lpObj->Index, RenderSkillHealing);
				}
			}
		}
	}
}

void CFakeOnline::TuDongBuffSkill(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex)) { return; }
	LPOBJ lpObj = &gObj[aIndex];
	LPOBJ lpTarget;

	if (gServerInfo.InSafeZone(aIndex) == true) { return; }

	if (lpObj->BuffOn != 0) { 
		CSkill* RenderBuff;
		for (int n = 0; n < 3; n++) { 
			if (lpObj->BuffSkill[n] > 0) {
				RenderBuff = gSkillManager.GetSkill(lpObj, lpObj->BuffSkill[n]);
				if (RenderBuff != 0) {
					if (gEffectManager.CheckEffect(lpObj, gSkillManager.GetSkillEffect(RenderBuff->m_index)) == 0) {
						gSkillManager.UseAttackSkill(lpObj->Index, lpObj->Index, RenderBuff);
					}
				}
			}
		}
	}

	if (lpObj->PartyModeOn != 0 && lpObj->PartyNumber >= 0) { 
		if (lpObj->PartyModeHealOn != 0 && lpObj->Class == CLASS_FE) {
			CSkill* RenderPartyHealing;
			for (int i = 0; i < MAX_PARTY_USER; i++) {
				if (OBJECT_RANGE(gParty.m_PartyInfo[lpObj->PartyNumber].Index[i]) != 0 && gObjCalcDistance(lpObj, &gObj[gParty.m_PartyInfo[lpObj->PartyNumber].Index[i]]) < MAX_PARTY_DISTANCE) {
					RenderPartyHealing = gSkillManager.GetSkill(lpObj, SKILL_HEAL);
					if (RenderPartyHealing != 0) {
						lpTarget = &gObj[gParty.m_PartyInfo[lpObj->PartyNumber].Index[i]];
						if (lpTarget->Index == lpObj->Index) { continue; }
						if (lpTarget->Life < ((lpTarget->MaxLife * lpObj->PartyModeHealPercent) / 100)) { 
							if (gEffectManager.CheckEffect(lpTarget, gSkillManager.GetSkillEffect(RenderPartyHealing->m_index)) == 0) {
								gSkillManager.UseAttackSkill(lpObj->Index, lpTarget->Index, RenderPartyHealing);
							}
						}
					}
				}
			}
		}
		if (lpObj->PartyModeBuffOn != 0 && lpObj->PartyNumber >= 0) { 
			CSkill* RenderPartyBuff;
			for (int i = 0; i < MAX_PARTY_USER; i++) {
				if (OBJECT_RANGE(gParty.m_PartyInfo[lpObj->PartyNumber].Index[i]) != 0 && gObjCalcDistance(lpObj, &gObj[gParty.m_PartyInfo[lpObj->PartyNumber].Index[i]]) < MAX_PARTY_DISTANCE) {
					for (int n = 0; n < 3; n++) {
						if (lpObj->BuffSkill[n] > 0) { 
							RenderPartyBuff = gSkillManager.GetSkill(lpObj, lpObj->BuffSkill[n]);
							if (RenderPartyBuff != 0) {
								lpTarget = &gObj[gParty.m_PartyInfo[lpObj->PartyNumber].Index[i]];
								if (gEffectManager.CheckEffect(lpTarget, gSkillManager.GetSkillEffect(RenderPartyBuff->m_index)) == 0) {
									gSkillManager.UseAttackSkill(lpObj->Index, gParty.m_PartyInfo[lpObj->PartyNumber].Index[i], RenderPartyBuff);
								}
							}
						}
					}
				}
			}
		}
	}
}

bool CFakeOnline::GetTargetMonster(LPOBJ lpObj, int SkillNumber, int* MonsterIndex)
{
	int NearestDistance = 100; 
    *MonsterIndex = -1;

	for (int n = 0; n < MAX_VIEWPORT; n++)
	{
		if (lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 || lpObj->VpPlayer2[n].type != OBJECT_MONSTER)
		{
			continue;
		}

		if (gSkillManager.CheckSkillTarget(lpObj, lpObj->VpPlayer2[n].index, -1, lpObj->VpPlayer2[n].type) == 0)
		{
			continue;
		}
        
        int dist = gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]);
		if (dist >= NearestDistance) 
		{
			continue;
		}

		if (gSkillManager.CheckSkillRange(SkillNumber, lpObj->X, lpObj->Y, gObj[lpObj->VpPlayer2[n].index].X, gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			*MonsterIndex = lpObj->VpPlayer2[n].index;
			NearestDistance = dist; 
		}
		else if (gSkillManager.CheckSkillRadio(SkillNumber, lpObj->X, lpObj->Y, gObj[lpObj->VpPlayer2[n].index].X, gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			*MonsterIndex = lpObj->VpPlayer2[n].index;
			NearestDistance = dist;
		}
	}
	return ((*MonsterIndex) != -1); 
}

bool CFakeOnline::GetTargetPlayer(LPOBJ lpObj, int SkillNumber, int* MonsterIndex)
{
    int NearestDistance = 100;
    *MonsterIndex = -1; 

    DWORD currentTick = GetTickCount();
    DWORD partySendCooldownDuration = 50000; // Mantener alto para pruebas, luego ajustar a 5000 o configurable

    for (int n = 0; n < MAX_VIEWPORT; n++)
    {
        if (lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 || lpObj->VpPlayer2[n].type != OBJECT_USER)
        {
            continue;
        }
        
        LPOBJ lpTargetVp = &gObj[lpObj->VpPlayer2[n].index];

        if (lpObj->Index == lpTargetVp->Index) 
        {
            continue;
        }
        
        int dist = gObjCalcDistance(lpObj, lpTargetVp);
        
        bool isPartyCooldownOver = (currentTick >= (lpObj->FakeBotPartyInviteCooldownTick + partySendCooldownDuration));

        if (lpObj->IsFakePartyMode >= 2) { 
            LogAdd(LOG_GREEN, "[PartyCooldown][%s] Eval. target %s. Tick:%u, BotLastPartyTick:%u, CoolVal:%u. CondPass:%s",
                lpObj->Name, lpTargetVp->Name, currentTick, lpObj->FakeBotPartyInviteCooldownTick, 
                partySendCooldownDuration, isPartyCooldownOver ? "PASS" : "WAIT");
        }
        
        if (lpObj->IsFakePartyMode >= 2 &&                                       
            gParty.IsParty(lpTargetVp->PartyNumber) == 0 &&                     
            isPartyCooldownOver && 
            !gObjIsSelfDefense(lpTargetVp, lpObj->Index))                       
        {
            LogAdd(LOG_BLUE, "[GetTargetPlayer][%s] Considera invitar a party a %s. PartyMode=%d.", 
                lpObj->Name, lpTargetVp->Name, lpObj->IsFakePartyMode);

            bool allowInviteToThisTarget = true;
            if (lpObj->IsFakePartyMode == 3 && lpTargetVp->IsFakeOnline == 0) { 
                LogAdd(LOG_BLUE, "[GetTargetPlayer][%s] PartyMode=3, target %s es JUGADOR REAL. No se env�a invitaci�n.", lpObj->Name, lpTargetVp->Name);
                allowInviteToThisTarget = false;
            }

            if (allowInviteToThisTarget) {
                bool canSendInviteConditionsMet = true;

                if (gParty.IsParty(lpObj->PartyNumber)) {
                    if (!gParty.IsLeader(lpObj->PartyNumber, lpObj->Index)) {
                        LogAdd(LOG_BLUE, "[GetTargetPlayer][%s] Est� en party pero NO ES L�DER. No puede invitar.", lpObj->Name);
                        canSendInviteConditionsMet = false;
                    } else {
                        int memberCount = 0;
                        for (int k = 0; k < MAX_PARTY_USER; ++k) { 
                            if (gParty.m_PartyInfo[lpObj->PartyNumber].Index[k] >= 0 && gObjIsConnected(gParty.m_PartyInfo[lpObj->PartyNumber].Index[k])) {
                                memberCount++;
                            }
                        }
                        if (memberCount >= MAX_PARTY_USER) {
                            LogAdd(LOG_BLUE, "[GetTargetPlayer][%s] Es l�der, pero su party est� LLENA (%d/%d). No puede invitar.", lpObj->Name, memberCount, MAX_PARTY_USER);
                            canSendInviteConditionsMet = false;
                        } else {
                             LogAdd(LOG_BLUE, "[GetTargetPlayer][%s] Es l�der, party tiene %d/%d miembros. Puede invitar.", lpObj->Name, memberCount, MAX_PARTY_USER);
                        }
                    }
                } else {
                    LogAdd(LOG_BLUE, "[GetTargetPlayer][%s] No est� en party. Puede formar una nueva al invitar.", lpObj->Name);
                }

                if (canSendInviteConditionsMet) {
                    LogAdd(LOG_GREEN, "[GetTargetPlayer][%s] Condiciones cumplidas. ENVIANDO solicitud de party a %s.", lpObj->Name, lpTargetVp->Name);
                    lpObj->FakeBotPartyInviteCooldownTick = currentTick; 
                    FakeAnimationMove(lpObj->Index, lpTargetVp->X, lpTargetVp->Y, false); 
                    this->GuiYCParty(lpObj->Index, lpTargetVp->Index); 
                    return false; 
                }
            }
        }

        if (dist < NearestDistance) { 
            if (gObjIsSelfDefense(lpTargetVp, lpObj->Index))
            {
                *MonsterIndex = lpTargetVp->Index;
                NearestDistance = dist;
                LogAdd(LOG_BLUE, "[GetTargetPlayer][%s] Target %s es self-defense. Seleccionado para atacar.", lpObj->Name, lpTargetVp->Name);
            }
            else if (lpObj->IsFakePVPMode == 2) 
            { 
                if (gSkillManager.CheckSkillRange(SkillNumber, lpObj->X, lpObj->Y, lpTargetVp->X, lpTargetVp->Y) != 0 ||
                    gSkillManager.CheckSkillRadio(SkillNumber, lpObj->X, lpObj->Y, lpTargetVp->X, lpTargetVp->Y) != 0)
                {
                    *MonsterIndex = lpTargetVp->Index;
                    NearestDistance = dist;
                    LogAdd(LOG_BLUE, "[GetTargetPlayer][%s] Target %s en rango (PVPMode 2). Seleccionado para atacar.", lpObj->Name, lpTargetVp->Name);
                }
            }
        }
	} 

	return ((*MonsterIndex) != -1); 
}

void CFakeOnline::TuDongDanhSkill(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex)) { return; }
	LPOBJ lpObj = &gObj[aIndex];
	
    EnterCriticalSection(&this->m_BotDataMutex); 

	OFFEXP_DATA* pBotData = this->GetOffExpInfo(lpObj); 
    if(!pBotData) {
        LeaveCriticalSection(&this->m_BotDataMutex);
        return;
    }

    int caminar = 0; 
	int distance = (lpObj->HuntingRange > 6) ? 6 : lpObj->HuntingRange; 

	CSkill* SkillRender;
	SkillRender = (lpObj->Life < ((lpObj->MaxLife * lpObj->RecoveryDrainPercent) / 100) && lpObj->RecoveryDrainOn != 0) 
	              ? gSkillManager.GetSkill(lpObj, SKILL_DRAIN_LIFE) 
	              : gSkillManager.GetSkill(lpObj, lpObj->SkillBasicID); 

	if (SkillRender == 0) {
        LeaveCriticalSection(&this->m_BotDataMutex);
        return; 
    }

	int targetIndex = -1; 
	bool targetIsPlayer = false;

	if (lpObj->IsFakePVPMode >= 1) { 
        if (this->GetTargetPlayer(lpObj, SkillRender->m_index, &targetIndex)) {
            targetIsPlayer = true;
        }
    }

    if (targetIndex == -1) {
        if (!this->GetTargetMonster(lpObj, SkillRender->m_index, &targetIndex)) {
            LeaveCriticalSection(&this->m_BotDataMutex);
            return; 
        }
        targetIsPlayer = false;
    }
    
	if (OBJMAX_RANGE(targetIndex) == FALSE) { 
        LeaveCriticalSection(&this->m_BotDataMutex);
        return; 
    }
	LPOBJ lpTargetObj = &gObj[targetIndex];

	if (lpTargetObj->Live == 0 || lpTargetObj->State == OBJECT_EMPTY || lpTargetObj->RegenType != 0) {
        LeaveCriticalSection(&this->m_BotDataMutex);
        return; 
    }
	if (gServerInfo.InSafeZone(targetIndex) == true) {
        LeaveCriticalSection(&this->m_BotDataMutex);
        return; 
    }

	int dis = gObjCalcDistance(lpObj, lpTargetObj);

	if (dis > distance) { 
        if (targetIsPlayer && lpObj->IsFakePVPMode == 2) { 
             FakeAnimationMove(lpObj->Index, lpTargetObj->X, lpTargetObj->Y, false);
        }
        LeaveCriticalSection(&this->m_BotDataMutex);
		return; 
	} else { 
		caminar = 1; 
		if (gSkillManager.CheckSkillRange(SkillRender->m_index, lpObj->X, lpObj->Y, lpTargetObj->X, lpTargetObj->Y) != 0) {
			caminar = 0;
		} else if (gSkillManager.CheckSkillRadio(SkillRender->m_index, lpObj->X, lpObj->Y, lpTargetObj->X, lpTargetObj->Y) != 0) {
			caminar = 0;
		}

		if (caminar == 1) {
			FakeAnimationMove(lpObj->Index, lpTargetObj->X, lpTargetObj->Y, false);
            LeaveCriticalSection(&this->m_BotDataMutex);
            return; 
		}
	}

	if (lpObj->Mana < gSkillManager.GetSkillMana(SkillRender->m_index)) {
		PMSG_ITEM_USE_RECV pMsgMP;
		pMsgMP.header.set(0x26, sizeof(pMsgMP));
		pMsgMP.SourceSlot = 0xFF;
		pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 6), -1) : pMsgMP.SourceSlot);
		pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 5), -1) : pMsgMP.SourceSlot);
		pMsgMP.SourceSlot = ((pMsgMP.SourceSlot == 0xFF) ? gItemManager.GetInventoryItemSlot(lpObj, GET_ITEM(14, 4), -1) : pMsgMP.SourceSlot);
		pMsgMP.TargetSlot = 0xFF;
		pMsgMP.type = 0;
		if (INVENTORY_FULL_RANGE(pMsgMP.SourceSlot) != 0) {
			gItemManager.CGItemUseRecv(&pMsgMP, lpObj->Index);
		}
        LeaveCriticalSection(&this->m_BotDataMutex);
		return; 
	}

    DWORD current_tick_ds = GetTickCount();
    DWORD last_attack_tick_ds = (DWORD)lpObj->AttackCustomDelay;
    DWORD phys_speed_ds = (DWORD)lpObj->PhysiSpeed; 
    DWORD magic_speed_ds = (DWORD)lpObj->MagicSpeed; 
    int current_multiplicador_ds = (lpObj->Class == CLASS_RF) ? 1 : 5;

    DWORD required_delay_ds = ( ((phys_speed_ds * current_multiplicador_ds) > 1500) ? 0 : (1500 - (phys_speed_ds * current_multiplicador_ds)) );
    DWORD elapsed_time_ds = current_tick_ds - last_attack_tick_ds;

    LogAdd(LOG_GREEN, "[TuDongDanhSkill][%s][Cls:%d] AtkCalc: CTick=%u, LastAtk=%u, Elapsed=%u | ReqDelay=%u (PhysSpd=%u,MagicSpd=%u,Multi=%d)",
        lpObj->Name, lpObj->Class, 
        current_tick_ds, last_attack_tick_ds, elapsed_time_ds,
        required_delay_ds, phys_speed_ds, magic_speed_ds, current_multiplicador_ds);

	if (elapsed_time_ds >= required_delay_ds) {
        LogAdd(LOG_GREEN, "[TuDongDanhSkill][%s] ATACANDO. Elapsed %u >= ReqDelay %u", 
            lpObj->Name, elapsed_time_ds, required_delay_ds);
		
        lpObj->AttackCustomDelay = current_tick_ds; 

        if (targetIsPlayer) { 
             LogAdd(LOG_BLUE, "[FakeOnline][%s] Atacando a jugador %s. Activando estado de combate PVP para chat.", lpObj->Name, lpTargetObj->Name);
             this->m_botPVPCombatStates[aIndex].isInActiveCombat = true;
             this->m_botPVPCombatStates[aIndex].lastPVPActionTick = GetTickCount();
             this->m_botPVPCombatStates[aIndex].saidInitialPVPPhrase = false; 
        }

		if (SkillRender->m_skill != SKILL_FLAME && SkillRender->m_skill != SKILL_TWISTER && SkillRender->m_skill != SKILL_EVIL_SPIRIT && SkillRender->m_skill != SKILL_HELL_FIRE && SkillRender->m_skill != SKILL_AQUA_BEAM && SkillRender->m_skill != SKILL_BLAST && SkillRender->m_skill != SKILL_INFERNO && SkillRender->m_skill != SKILL_TRIPLE_SHOT && SkillRender->m_skill != SKILL_IMPALE && SkillRender->m_skill != SKILL_MONSTER_AREA_ATTACK && SkillRender->m_skill != SKILL_PENETRATION && SkillRender->m_skill != SKILL_FIRE_SLASH && SkillRender->m_skill != SKILL_FIRE_SCREAM) {
			if (SkillRender->m_skill != SKILL_DARK_SIDE) {
                gAttack.Attack(lpObj, lpTargetObj, SkillRender, TRUE, 1, 1, TRUE, 1); 
			} else { 
				this->SendRFSkillAttack(lpObj, targetIndex, SkillRender->m_index);
			}
		} else { 
			this->SendMultiSkillAttack(lpObj, targetIndex, SkillRender->m_index); 
		}
	}
    LeaveCriticalSection(&this->m_BotDataMutex);
}


void CFakeOnline::SendSkillAttack(LPOBJ lpObj, int target_aIndex, int SkillNumber)
{
	PMSG_SKILL_ATTACK_RECV pMsg;
	pMsg.header.set(0x19, sizeof(pMsg));
#if(GAMESERVER_UPDATE>=701)
	pMsg.skillH = SET_NUMBERHB(SkillNumber);
	pMsg.skillL = SET_NUMBERLB(SkillNumber);
	pMsg.indexH = SET_NUMBERHB(target_aIndex);
	pMsg.indexL = SET_NUMBERLB(target_aIndex);
#else
	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);
	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);
	pMsg.index[0] = SET_NUMBERHB(target_aIndex);
	pMsg.index[1] = SET_NUMBERLB(target_aIndex);
#endif
	pMsg.dis = 0;
	lpObj->IsFakeTimeLag = GetTickCount(); 
	gSkillManager.CGSkillAttackRecv(&pMsg, lpObj->Index);
}

void CFakeOnline::SendMultiSkillAttack(LPOBJ lpObj, int main_target_aIndex, int SkillNumber)
{
	this->SendDurationSkillAttack(lpObj, main_target_aIndex, SkillNumber);

	BYTE send_buff[256]; 
	PMSG_MULTI_SKILL_ATTACK_RECV pMsg;
	pMsg.header.set(PROTOCOL_CODE4, sizeof(pMsg)); 
	int size = sizeof(pMsg);

#if(GAMESERVER_UPDATE>=701)
	pMsg.skillH = SET_NUMBERHB(SkillNumber);
	pMsg.skillL = SET_NUMBERLB(SkillNumber);
#else
	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);
	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);
#endif
	pMsg.x = (BYTE)lpObj->X;
	pMsg.y = (BYTE)lpObj->Y;
	pMsg.serial = 0; 
	pMsg.count = 0;

	PMSG_MULTI_SKILL_ATTACK info;
	for (int n = 0; n < MAX_VIEWPORT; n++) {
		if (lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0) { continue; }
		
		int current_target_idx = lpObj->VpPlayer2[n].index;
        
		if (gSkillManager.CheckSkillTarget(lpObj, current_target_idx, main_target_aIndex, lpObj->VpPlayer2[n].type) == 0) { continue; } 
        
		if (gSkillManager.CheckSkillRadio(SkillNumber, lpObj->X, lpObj->Y, gObj[current_target_idx].X, gObj[current_target_idx].Y) == 0) { continue; }

#if(GAMESERVER_UPDATE>=701)
		info.indexH = SET_NUMBERHB(current_target_idx);
		info.indexL = SET_NUMBERLB(current_target_idx);
#else
		info.index[0] = SET_NUMBERHB(current_target_idx);
		info.index[1] = SET_NUMBERLB(current_target_idx);
#endif
		info.MagicKey = 0; 
		memcpy(&send_buff[size], &info, sizeof(info));
		size += sizeof(info);
        pMsg.count++; 
		if (CHECK_SKILL_ATTACK_COUNT(pMsg.count) == 0) { break; } 
	}

    if (pMsg.count > 0) { 
	    pMsg.header.size = size;
	    memcpy(send_buff, &pMsg, sizeof(pMsg));
	    lpObj->IsFakeTimeLag = GetTickCount(); 
	    gSkillManager.CGMultiSkillAttackRecv((PMSG_MULTI_SKILL_ATTACK_RECV*)send_buff, lpObj->Index, 0); 
    }
}

void CFakeOnline::SendDurationSkillAttack(LPOBJ lpObj, int target_aIndex, int SkillNumber)
{
	PMSG_DURATION_SKILL_ATTACK_RECV pMsg;
	pMsg.header.set(0x1E, sizeof(pMsg));
#if(GAMESERVER_UPDATE>=701)
	pMsg.skillH = SET_NUMBERHB(SkillNumber);
	pMsg.skillL = SET_NUMBERLB(SkillNumber);
#else
	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);
	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);
#endif
	pMsg.x = (BYTE)gObj[target_aIndex].X;
	pMsg.y = (BYTE)gObj[target_aIndex].Y;
	pMsg.dir = (gSkillManager.GetSkillAngle(gObj[target_aIndex].X, gObj[target_aIndex].Y, lpObj->X, lpObj->Y) * 255) / 360;
	pMsg.dis = 0; 
	pMsg.angle = (gSkillManager.GetSkillAngle(lpObj->X, lpObj->Y, gObj[target_aIndex].X, gObj[target_aIndex].Y) * 255) / 360;
#if(GAMESERVER_UPDATE>=803)
	pMsg.indexH = SET_NUMBERHB(target_aIndex);
	pMsg.indexL = SET_NUMBERLB(target_aIndex);
#else
	pMsg.index[0] = SET_NUMBERHB(target_aIndex);
	pMsg.index[1] = SET_NUMBERLB(target_aIndex);
#endif
	pMsg.MagicKey = 0; 
	lpObj->IsFakeTimeLag = GetTickCount(); 
	gSkillManager.CGDurationSkillAttackRecv(&pMsg, lpObj->Index);
}

void CFakeOnline::SendRFSkillAttack(LPOBJ lpObj, int target_aIndex, int SkillNumber)
{
	PMSG_SKILL_DARK_SIDE_RECV MsgDS; 
	MsgDS.skill[0] = SET_NUMBERHB(SkillNumber);
	MsgDS.skill[1] = SET_NUMBERLB(SkillNumber);
	MsgDS.index[0] = SET_NUMBERHB(target_aIndex);
	MsgDS.index[1] = SET_NUMBERLB(target_aIndex);
	gSkillManager.CGSkillDarkSideRecv(&MsgDS, lpObj->Index);

	PMSG_RAGE_FIGHTER_SKILL_ATTACK_RECV pMsg;
	pMsg.header.set(0x19, sizeof(pMsg)); 
#if(GAMESERVER_UPDATE>=701)
	pMsg.skillH = SET_NUMBERHB(SkillNumber);
	pMsg.skillL = SET_NUMBERLB(SkillNumber);
	pMsg.indexH = SET_NUMBERHB(target_aIndex);
	pMsg.indexL = SET_NUMBERLB(target_aIndex);
#else
	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);
	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);
	pMsg.index[0] = SET_NUMBERHB(target_aIndex);
	pMsg.index[1] = SET_NUMBERLB(target_aIndex);
#endif
	pMsg.dis = 0;
	gSkillManager.CGRageFighterSkillAttackRecv(&pMsg, lpObj->Index);
	lpObj->IsFakeTimeLag = GetTickCount(); 
}

void CFakeOnline::GuiYCParty(int aIndex, int bIndex)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (gObjIsConnectedGP(aIndex) == 0) { return; }
	if (gObjIsConnectedGP(bIndex) == 0) { return; }
	LPOBJ lpTarget = &gObj[bIndex];

	if (lpObj->Interface.use != 0 || lpTarget->Interface.use != 0) { return; }

#if(defined(CHONPHEDOILAP) && CHONPHEDOILAP != 0) 
#endif

	if (gServerInfo.m_PartyRestrict == 1 && gParty.IsParty(lpTarget->PartyNumber) == 0) {
		if (gObj[aIndex].PartyNumber >= 0) {
			// bool levelOk = false; // Variable no usada
		} else {
			short sMaxMinLevel[2];
			if (gObj[aIndex].Level > gObj[bIndex].Level) {
				sMaxMinLevel[1] = gObj[aIndex].Level;
				sMaxMinLevel[0] = gObj[bIndex].Level;
			} else {
				sMaxMinLevel[1] = gObj[bIndex].Level;
				sMaxMinLevel[0] = gObj[aIndex].Level;
			}
			if ((sMaxMinLevel[1] - sMaxMinLevel[0]) > gServerInfo.m_DifferenceMaxLevelParty) {
				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GetMessage(861), gServerInfo.m_DifferenceMaxLevelParty);
				return;
			}
		}
	}

	if (CA_MAP_RANGE(lpTarget->Map) != 0 || CC_MAP_RANGE(lpTarget->Map) != 0 || IT_MAP_RANGE(lpTarget->Map) != 0 || DG_MAP_RANGE(lpTarget->Map) != 0 || IG_MAP_RANGE(lpTarget->Map) != 0) {
		gParty.GCPartyResultSend(aIndex, 0); return;
	}

	if (OBJECT_RANGE(lpObj->PartyTargetUser) != 0 || OBJECT_RANGE(lpTarget->PartyTargetUser) != 0) {
		gParty.GCPartyResultSend(aIndex, 0); return;
	}

	if (gServerInfo.m_GensSystemPartyLock != 0 && lpObj->GensFamily != 0 && lpTarget->GensFamily != 0 && lpObj->GensFamily != lpTarget->GensFamily) {
		gParty.GCPartyResultSend(aIndex, 6); return;
	}

	if (gParty.AutoAcceptPartyRequest(lpObj, lpTarget) != 0) { return; }

	if (gParty.IsParty(lpObj->PartyNumber) != 0 && gParty.IsLeader(lpObj->PartyNumber, aIndex) == 0) {
		gParty.GCPartyResultSend(aIndex, 0); return;
	}

	if ((lpTarget->Option & 1) == 0) { 
		gParty.GCPartyResultSend(aIndex, 1); return;
	}

	if (gParty.IsParty(lpTarget->PartyNumber) != 0) { 
		gParty.GCPartyResultSend(aIndex, 4); return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_PARTY;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = GetTickCount();
	lpObj->TargetNumber = bIndex;
	lpObj->PartyTargetUser = bIndex;

	lpTarget->Interface.use = 1;
	lpTarget->Interface.type = INTERFACE_PARTY;
	lpTarget->Interface.state = 0;
	lpTarget->InterfaceTime = GetTickCount();
	lpTarget->TargetNumber = aIndex;
    lpTarget->PartyTargetUser = aIndex; 

	PMSG_PARTY_REQUEST_SEND pMsg;
	pMsg.header.set(0x40, sizeof(pMsg));
	pMsg.index[0] = SET_NUMBERHB(aIndex);
	pMsg.index[1] = SET_NUMBERLB(aIndex);
	DataSend(bIndex, (BYTE*)&pMsg, pMsg.header.size);
}

#endif // USE_FAKE_ONLINE == TRUE