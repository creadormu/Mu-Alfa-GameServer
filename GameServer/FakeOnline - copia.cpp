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
#include "Attack.h" // Asegúrate que Attack.h está incluido si usas gAttack
#include "Log.h" 
#include <random> 

// Incluye pugixml.hpp si no está ya incluido
// #include "pugixml/pugixml.hpp"

#if USE_FAKE_ONLINE == TRUE

CFakeOnline s_FakeOnline;

std::vector<std::string> g_BotPhrasesGeneral;
std::vector<std::string> g_BotPhrasesNear;
std::vector<std::string> g_BotPhrasesInParty;
std::vector<std::string> g_BotPhrasesPVP; 

CFakeOnline::CFakeOnline()
{
	InitializeCriticalSection(&this->m_BotDataMutex);
	this->m_Data.clear();
	this->IndexMsgMax = 0;
	this->IndexMsgMin = 0;
	this->InitializeBotPhrases(); 
	for (int i = 0; i < MAX_OBJECT; ++i)																												 
	{
		this->m_dwLastCommentTick[i] = 0;
	}
}

CFakeOnline::~CFakeOnline()
{
    DeleteCriticalSection(&this->m_BotDataMutex);
}

void CFakeOnline::LoadFakeData(char* path)
{
    EnterCriticalSection(&this->m_BotDataMutex);
    LogAdd(LOG_BLUE, "[FakeOnline] LoadFakeData INICIO - Path: %s", path ? path : "NULL_PATH");

    this->m_Data.clear(); 
    this->m_botPVPCombatStates.clear(); // Limpiar estados de combate PVP también
    this->IndexMsgMax = 0;
    this->IndexMsgMin = 0;
    
    LogAdd(LOG_BLUE, "[FakeOnline] Llamando a LoadBotPhrasesFromFile...");
    LoadBotPhrasesFromFile(".\\BotPhrases.txt"); 
    LogAdd(LOG_BLUE, "[FakeOnline] LoadBotPhrasesFromFile TERMINADO.");

    if (path == nullptr) {
        LogAdd(LOG_RED, "[FakeOnline] Error: Path del archivo XML es NULL.");
        LeaveCriticalSection(&this->m_BotDataMutex);
        return;
    }

    pugi::xml_document file;
    LogAdd(LOG_BLUE, "[FakeOnline] Cargando XML: %s", path);
    pugi::xml_parse_result res = file.load_file(path);

    if (res.status != pugi::status_ok){
        LogAdd(LOG_RED, "[FakeOnline] Error al cargar XML: %s. Status: %d, Description: %s", path, res.status, res.description());
        ErrorMessageBox("File %s load fail. Error: %s", path, res.description());
        LeaveCriticalSection(&this->m_BotDataMutex); 
        return;
    }
    LogAdd(LOG_BLUE, "[FakeOnline] XML cargado exitosamente.");
    
    pugi::xml_node Recipe = file.child("MSGThongBao");
    if (Recipe) {
         this->IndexMsgMin = Recipe.attribute("IndexMesMin").as_int();
         this->IndexMsgMax = Recipe.attribute("IndexMesMax").as_int();
         LogAdd(LOG_BLUE, "[FakeOnline] MSGThongBao procesado. Min: %d, Max: %d", this->IndexMsgMin, this->IndexMsgMax);
    } else {
         LogAdd(LOG_BLUE, "[FakeOnline] ADVERTENCIA: Nodo MSGThongBao no encontrado en IA.xml.");
    }

    pugi::xml_node oFakeOnlineData = file.child("FakeOnlineData");
    if (oFakeOnlineData) {
        LogAdd(LOG_BLUE, "[FakeOnline] Procesando nodos FakeOnlineData...");
        int botCount = 0;
        for (pugi::xml_node rInfoData = oFakeOnlineData.child("Info"); rInfoData; rInfoData = rInfoData.next_sibling()){
            botCount++;
            // LogAdd(LOG_BLUE, "[FakeOnline] Procesando Bot #%d...", botCount); // Log muy verboso, opcional
            OFFEXP_DATA info;
            memset(&info, 0, sizeof(info));

            const char* accStr = rInfoData.attribute("Account").as_string("");
            const char* passStr = rInfoData.attribute("Password").as_string("");
            const char* nameStr = rInfoData.attribute("Name").as_string("");
            // LogAdd(LOG_BLUE, "[FakeOnline] Bot #%d: Account='%s', Name='%s'", botCount, accStr, nameStr);

            strncpy_s(info.Account, sizeof(info.Account), accStr, _TRUNCATE);
            strncpy_s(info.Password, sizeof(info.Password), passStr, _TRUNCATE);
            strncpy_s(info.Name, sizeof(info.Name), nameStr, _TRUNCATE);
            
            info.SkillID = rInfoData.attribute("SkillID").as_int(0);
            info.PVPMode = rInfoData.attribute("PVPMode").as_int(0); 
            // LogAdd(LOG_BLUE, "[FakeOnline] Bot #%d: PVPMode XML Val='%s', Parsed Int=%d", botCount, rInfoData.attribute("PVPMode").value(), info.PVPMode);
            
            info.UseBuffs[0] = rInfoData.attribute("UseBuffs_0").as_int(0);
            info.UseBuffs[1] = rInfoData.attribute("UseBuffs_1").as_int(0);
            info.UseBuffs[2] = rInfoData.attribute("UseBuffs_2").as_int(0);
            info.GateNumber = rInfoData.attribute("GateNumber").as_int(0);
            info.MapX = rInfoData.attribute("MapX").as_int(0);
            info.MapY = rInfoData.attribute("MapY").as_int(0);
            info.PhamViTrain = rInfoData.attribute("PhamViTrain").as_int(0);
            info.MoveRange = rInfoData.attribute("MoveRange").as_int(0);
            info.TimeReturn = rInfoData.attribute("TimeReturn").as_int(0);
            info.TuNhatItem = rInfoData.attribute("TuNhatItem").as_int(0);
            info.TuDongReset = rInfoData.attribute("TuDongReset").as_int(0);
            info.PartyMode = rInfoData.attribute("PartyMode").as_int(0);
            info.PostKhiDie = rInfoData.attribute("PostKhiDie").as_int(0);

            if (strlen(info.Account) > 0) {
                this->m_Data.insert(std::pair<std::string, OFFEXP_DATA>(info.Account, info));
                // LogAdd(LOG_BLUE, "[FakeOnline] Bot #%d: Insertado en m_Data.", botCount);
            } else {
                LogAdd(LOG_RED, "[FakeOnline] Bot #%d: Cuenta vacía, no insertado.", botCount);
            }
        }
        LogAdd(LOG_BLUE, "[FakeOnline] Nodos FakeOnlineData procesados. Total bots en XML: %d", botCount);
    }  else {
        LogAdd(LOG_RED, "[FakeOnline] ADVERTENCIA: Nodo FakeOnlineData no encontrado en IA.xml.");
    }
    LogAdd(LOG_BLUE, "[FakeOnline] Load Data OK (antes de LeaveCriticalSection)");
    LeaveCriticalSection(&this->m_BotDataMutex);
    LogAdd(LOG_BLUE, "[FakeOnline] LoadFakeData FIN");
}

void LoadBotPhrasesFromFile(const char* filename)
{
    LogAdd(LOG_BLUE, "[LBPFF] START - File: %s", filename);

    LogAdd(LOG_BLUE, "[LBPFF] Limpiando vectores de frases...");
    g_BotPhrasesGeneral.clear();
    LogAdd(LOG_BLUE, "[LBPFF] g_BotPhrasesGeneral limpiado (%d).", g_BotPhrasesGeneral.capacity());
    g_BotPhrasesNear.clear();
    LogAdd(LOG_BLUE, "[LBPFF] g_BotPhrasesNear limpiado (%d).", g_BotPhrasesNear.capacity());
	g_BotPhrasesInParty.clear(); 
    LogAdd(LOG_BLUE, "[LBPFF] g_BotPhrasesInParty limpiado (%d).", g_BotPhrasesInParty.capacity());
    g_BotPhrasesPVP.clear();
    LogAdd(LOG_BLUE, "[LBPFF] g_BotPhrasesPVP limpiado (%d). Vectores listos.", g_BotPhrasesPVP.capacity());

    std::ifstream file;
    LogAdd(LOG_BLUE, "[LBPFF] Intentando abrir archivo: %s", filename);
    file.open(filename); 

    if (!file.is_open()) {
        LogAdd(LOG_RED, "[LBPFF] FAILED to open: %s. La función retornará.", filename);
        LogAdd(LOG_BLUE, "[LBPFF] END (file not open)");
        return;
    }

    LogAdd(LOG_BLUE, "[LBPFF] SUCCESS: File is open: %s. Procesando líneas...", filename);

    std::string line;
    int mode = 0; 
    const int MODE_NONE = 0;
    const int MODE_GENERAL = 1;
    const int MODE_NEAR = 2;
    const int MODE_IN_PARTY = 3; 
    const int MODE_PVP = 4;
    int lineCount = 0;

    try 
    {
        while (std::getline(file, line)) { 
            lineCount++;
            // LogAdd(LOG_BLUE, "[LBPFF] Leyendo línea #%d, contenido: \"%s\"", lineCount, line.c_str()); // Muy verboso

            if (line.empty() || line[0] == ';') {
                // LogAdd(LOG_BLUE, "[LBPFF] Línea #%d ignorada (vacía o comentario).", lineCount);
                continue;
            }

            if (line[0] == '#') {
                // LogAdd(LOG_BLUE, "[LBPFF] Línea #%d es un selector de modo: %s", lineCount, line.c_str());
                if (line == "#GENERAL") mode = MODE_GENERAL;
                else if (line == "#NEAR_REAL_PLAYER") mode = MODE_NEAR;
                else if (line == "#IN_PARTY") mode = MODE_IN_PARTY; 
                else if (line == "#PVP_MODE") mode = MODE_PVP;
                else mode = MODE_NONE;
                // LogAdd(LOG_BLUE, "[LBPFF] Nuevo modo seteado: %d", mode);
                continue;
            }
            
            std::string originalLine = line; 
            try {
                size_t first = line.find_first_not_of(" \t\n\r\f\v");
                if (std::string::npos == first) { line.clear(); }
                else {
                    size_t last = line.find_last_not_of(" \t\n\r\f\v");
                    line = line.substr(first, (last - first + 1));
                }
            } catch (const std::out_of_range& oor) {
                LogAdd(LOG_RED, "[LBPFF] Excepción std::out_of_range al hacer trim en línea: \"%s\". Error: %s", originalLine.c_str(), oor.what());
                line = originalLine; // Considera si restaurar o ignorar es mejor
            }

            if (line.empty()) {
                // LogAdd(LOG_BLUE, "[LBPFF] Línea #%d quedó vacía después del trim.", lineCount);
                continue;
            }

            // LogAdd(LOG_BLUE, "[LBPFF] Añadiendo frase: \"%s\" al modo %d", line.c_str(), mode);
            switch (mode) {
                case MODE_GENERAL: g_BotPhrasesGeneral.push_back(line); break;
                case MODE_NEAR: g_BotPhrasesNear.push_back(line); break;
                case MODE_IN_PARTY: g_BotPhrasesInParty.push_back(line); break;
                case MODE_PVP: g_BotPhrasesPVP.push_back(line); break;
                default: 
                    // LogAdd(LOG_BLUE, "[LBPFF] Línea #%d ignorada (modo desconocido o NONE).", lineCount);
                    break;
            }
            // LogAdd(LOG_BLUE, "[LBPFF] Frase añadida.");
        }
    } catch (const std::exception& e) {
        LogAdd(LOG_RED, "[LBPFF] Excepción std::exception durante el bucle de getline: %s", e.what());
        if (file.is_open()) {
            file.close();
        }
        LogAdd(LOG_BLUE, "[LBPFF] END (exception in getline loop)");
        return; 
    } catch (...) {
        LogAdd(LOG_RED, "[LBPFF] Excepción desconocida durante el bucle de getline.");
        if (file.is_open()) {
            file.close();
        }
        LogAdd(LOG_BLUE, "[LBPFF] END (unknown exception in getline loop)");
        return; 
    }

    LogAdd(LOG_BLUE, "[LBPFF] Fin del bucle de lectura de líneas. Líneas leídas en bucle: %d. Cerrando archivo...", lineCount);
    
    if(file.eof()){ LogAdd(LOG_BLUE, "[LBPFF] EOF reached."); }
    if(file.fail() && !file.eof()){ LogAdd(LOG_RED, "[LBPFF] Fail bit set (y no EOF). Error de IO?"); }
    if(file.bad()){ LogAdd(LOG_RED, "[LBPFF] Bad bit set. Error grave de IO."); }
    
    file.clear(); 
    LogAdd(LOG_BLUE, "[LBPFF] Flags del stream limpiados. Intentando cerrar archivo.");

    if (file.is_open()) { 
        file.close();
        LogAdd(LOG_BLUE, "[LBPFF] Archivo cerrado.");
    } else {
        LogAdd(LOG_RED, "[LBPFF] ADVERTENCIA: El archivo ya no estaba abierto antes de cerrar explícitamente (después del bucle).");
    }

    LogAdd(LOG_BLUE, "[LBPFF] Frases -> G:%d, N:%d, P:%d, PVP:%d", (int)g_BotPhrasesGeneral.size(), (int)g_BotPhrasesNear.size(), (int)g_BotPhrasesInParty.size(), (int)g_BotPhrasesPVP.size());
    LogAdd(LOG_BLUE, "[LBPFF] END (successful completion or after loop)");
}

std::string GetRandomBotPhrase(bool realPlayerNearby, bool inParty, bool inActivePVPCombat) // Parámetro renombrado
{
    const std::vector<std::string>* pSelectedList = nullptr;
    std::string selectionReason = "Ninguna condición prioritaria cumplida"; 

    if (inActivePVPCombat && !g_BotPhrasesPVP.empty()) { // Usar el nuevo flag de combate activo
        pSelectedList = &g_BotPhrasesPVP;
        selectionReason = "Active PVP Combat";
    }
    else if (inParty && !g_BotPhrasesInParty.empty()) {
        pSelectedList = &g_BotPhrasesInParty;
        selectionReason = "In Party";
    }
    else if (realPlayerNearby && !g_BotPhrasesNear.empty()) {
        pSelectedList = &g_BotPhrasesNear;
        selectionReason = "Near Real Player";
    }
    else if (!g_BotPhrasesGeneral.empty()) {
        pSelectedList = &g_BotPhrasesGeneral;
        selectionReason = "General (directo)";
    }

    if (pSelectedList && pSelectedList->empty() && pSelectedList != &g_BotPhrasesGeneral && !g_BotPhrasesGeneral.empty()) {
        std::string originalReason = selectionReason; 
        pSelectedList = &g_BotPhrasesGeneral;
        selectionReason = "General (fallback por lista '" + originalReason + "' vacia)";
        // LogAdd(LOG_BLUE, "[GetRandomBotPhrase] Fallback a General. Razón original: %s. Nueva razón: %s", originalReason.c_str(), selectionReason.c_str());
    }
    
    if (pSelectedList == nullptr || pSelectedList->empty()) {
        LogAdd(LOG_BLUE, "[GetRandomBotPhrase][%s] No se seleccionó ninguna lista o la lista estaba vacía. Razón final: %s. Retornando frase vacía.", selectionReason.c_str());
        return ""; 
    }

    int index = rand() % pSelectedList->size(); 
    std::string chosenPhrase = (*pSelectedList)[index];
    LogAdd(LOG_BLUE, "[GetRandomBotPhrase] Lista seleccionada: %s. Frase: \"%s\"", selectionReason.c_str(), chosenPhrase.c_str());
    return chosenPhrase;
}

OFFEXP_DATA* CFakeOnline::GetOffExpInfo(LPOBJ lpObj)
{
	if (!lpObj || !lpObj->Account[0]) return nullptr; 
    
	std::map<std::string, OFFEXP_DATA>::iterator it = this->m_Data.find(lpObj->Account);

	if (it != this->m_Data.end())
	{
		if (strcmp(lpObj->Name, it->second.Name) == 0)
		{
			return &it->second;
		}
	}
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

void CFakeOnline::AttemptRandomBotComment(int aIndex)
{
    EnterCriticalSection(&this->m_BotDataMutex);
        
    if (!USE_FAKE_ONLINE) { 
        LeaveCriticalSection(&this->m_BotDataMutex);
        return; 
    }

    LPOBJ lpObj = &gObj[aIndex]; 
    if (lpObj->Connected != OBJECT_ONLINE || lpObj->Type != OBJECT_USER) {
        LeaveCriticalSection(&this->m_BotDataMutex);
        return;
    }

    OFFEXP_DATA* pBotData = this->GetOffExpInfo(lpObj); 
    if (pBotData == nullptr) {
        LeaveCriticalSection(&this->m_BotDataMutex);
        return;
    }

    bool realPlayerNearby = false;
    for (int i = 0; i < MAX_VIEWPORT; i++) {
        int idx = lpObj->VpPlayer2[i].index;
        if (OBJMAX_RANGE(idx) && gObj[idx].Type == OBJECT_USER && gObj[idx].IsFakeOnline == 0 && idx != aIndex) { 
            realPlayerNearby = true;
            break;
        }
    }
    
    bool isInParty = (lpObj->PartyNumber >= 0);
    
    // Lógica para el estado de combate PVP activo para el chat
    bool botInActivePVPCombat = false;
    BotActivePVPCombatState* pvpState = nullptr;
    std::map<int, BotActivePVPCombatState>::iterator itPVPState = this->m_botPVPCombatStates.find(aIndex);
    if (itPVPState != this->m_botPVPCombatStates.end()) {
        pvpState = &itPVPState->second;
        if (pvpState->isInActiveCombat) {
            if ((GetTickCount() - pvpState->lastPVPActionTick) < 30000) { // 30 segundos de "ventana de chat PVP activo"
                botInActivePVPCombat = true;
            } else {
                pvpState->isInActiveCombat = false; // Expiró el tiempo
                pvpState->saidInitialPVPPhrase = false; // Resetear para la próxima
            }
        }
    }
    
    // Si pBotData->PVPMode está activo (1 o 2), también consideramos que puede usar frases PVP genéricas
    // si no está en "combate activo" reciente. O puedes priorizar el combate activo.
    // Por ahora, `botInActivePVPCombat` será el flag principal para frases PVP.
    // Si quieres que pBotData->PVPMode también influya siempre, puedes hacer:
    // bool canUsePVPPhrases = botInActivePVPCombat || (pBotData->PVPMode >= 1);
    // Y pasar canUsePVPPhrases a GetRandomBotPhrase.
    // Para la lógica de "primeros segundos", la probabilidad se ajustará abajo.

    LogAdd(LOG_BLUE, "[AttemptBotComment][%s] Flags: Near=%d, Party=%d, ActivePVPCombat=%d (pBotData->PVPMode=%d)",
        lpObj->Name, realPlayerNearby, isInParty, botInActivePVPCombat, pBotData->PVPMode);

    const int COMMENT_COOLDOWN_MS = 120000; 
    int probability; 
    bool forcePVPChat = false;

    if (botInActivePVPCombat && pvpState) {
        probability = 35; // Probabilidad más alta si está en combate PVP activo
        if (!pvpState->saidInitialPVPPhrase && (GetTickCount() - pvpState->lastPVPActionTick) < 5000) { // Primeros 5 segundos
            probability = 80; // Mucho más probable que hable
            forcePVPChat = true; // Para asegurar que intente una frase PVP
            LogAdd(LOG_BLUE, "[AttemptBotComment][%s] En primeros 5seg de PVP, alta probabilidad.", lpObj->Name);
        }
    } else if (pBotData->PVPMode >= 1) { // Si está configurado para PVP pero no en combate activo reciente
        probability = 20; // Probabilidad normal para un bot con tendencia PVP
    } else if (isInParty) { 
        probability = 20; 
    } else if (realPlayerNearby) { 
        probability = 15; 
    } else { 
        probability = 10; 
    }
    LogAdd(LOG_BLUE, "[AttemptBotComment][%s] Probabilidad calculada: %d", lpObj->Name, probability);
    
    DWORD currentTick = GetTickCount();
    if ((GetTickCount() - this->m_dwLastCommentTick[aIndex]) >= COMMENT_COOLDOWN_MS || forcePVPChat) { // Permitir hablar si es forzado por PVP inicial
        if (forcePVPChat || (rand() % 100) < probability) { // Si es forzado, o si la probabilidad normal se cumple
            LogAdd(LOG_BLUE, "[AttemptBotComment][%s] Intentando obtener frase (forcePVP=%d)...", lpObj->Name, forcePVPChat);
            
            // Para 'GetRandomBotPhrase', el tercer parámetro indica si debe considerar frases PVP.
            // Usaremos botInActivePVPCombat para esto, ya que es el indicador más fuerte de un contexto PVP para chat.
            std::string phrase = GetRandomBotPhrase(realPlayerNearby, isInParty, botInActivePVPCombat); 
            
            if (!phrase.empty()) {
                LogAdd(LOG_BLUE, "[AttemptBotComment][%s] Frase obtenida: \"%s\". Posteando...", lpObj->Name, phrase.c_str());
                char msg[MAX_CHAT_MESSAGE_SIZE +1] = {0}; 
                strncpy_s(msg, sizeof(msg), phrase.c_str(), _TRUNCATE);

                bool posted = false;
                // ... (lógica de posteo como estaba) ...
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
                else { 
                    if (gCommandManager.CommandPost(lpObj, msg)) { 
                         posted = true;
                    } else { 
                        GDGlobalPostSend(gMapServerManager.GetMapServerGroup(),0,lpObj->Name,msg); posted = true;
                    }
                }
                            
                if (posted) {
                    this->m_dwLastCommentTick[aIndex] = currentTick;
                    if (forcePVPChat && pvpState) {
                        pvpState->saidInitialPVPPhrase = true; // Marcar que ya dijo la frase PVP inicial
                    }
                    LogAdd(LOG_BLUE, "[AttemptBotComment][%s] Frase posteada.", lpObj->Name);
                } else {
                    LogAdd(LOG_RED, "[AttemptBotComment][%s] No se pudo postear frase.", lpObj->Name);
                }
            } else {
                LogAdd(LOG_BLUE, "[AttemptBotComment][%s] No se obtuvo frase.", lpObj->Name);
            }
        }
    }
    LeaveCriticalSection(&this->m_BotDataMutex);
}

// ... (COPIA AQUÍ EL RESTO DE TUS FUNCIONES ORIGINALES DE FakeOnline (1).cpp) ...
// ... Asegúrate de que las implementaciones de CFakeOnline::InitializeBotPhrases, 
// ... CFakeOnline::FakeAttackProc, CFakeOnline::Attack, CFakeOnline::NhatItem, etc.,
// ... sean las que tenías en tu FakeOnline (1).cpp (tu original).
// ... También las funciones globales como FakeAnimationMove, FakeAutoRepair, etc.

// He incluido abajo las implementaciones de TU FakeOnline (1).cpp original para
// las funciones que faltaban y causaban errores de linker, más la modificación
// en RestoreFakeOnline y TuDongDanhSkill.

void CFakeOnline::InitializeBotPhrases()
{
	this->m_BotPhrases.clear();
	this->m_BotPhrases.push_back("¡Hola a todos!");
	this->m_BotPhrases.push_back("¡Que buen Server!");
	this->m_BotPhrases.push_back("¿Alguien para party?");
	this->m_BotPhrases.push_back("Esa gente, ¿En qué están?");
	this->m_BotPhrases.push_back("¡Vamos a Icarus al fondo!");
	this->m_BotPhrases.push_back("¿Pueden crear guías en el foro?");
	this->m_BotPhrases.push_back("¿De dónde son?");
	this->m_BotPhrases.push_back("¿Necesitan ayuda?");
	this->m_BotPhrases.push_back("Solo aquí me relajo como cuando niño");
	this->m_BotPhrases.push_back("Cuando sea 1er puesto haré PVP");
	this->m_BotPhrases.push_back("Saludos people");
	this->m_BotPhrases.push_back("¿Alguien de Argentina?");
	this->m_BotPhrases.push_back("¿party?");
	this->m_BotPhrases.push_back("¿Cómo veo la hora de los eventos?");
	this->m_BotPhrases.push_back("¡Vamos a Kubera gente!");
	this->m_BotPhrases.push_back("Recién llegó del trabajo");
	this->m_BotPhrases.push_back("Yo también");
	this->m_BotPhrases.push_back("¿Dónde caén joyas?");
	this->m_BotPhrases.push_back("Hay buenos premios en los bosses");
	this->m_BotPhrases.push_back("¿Es fase beta?");
	this->m_BotPhrases.push_back("Acepto party");
	this->m_BotPhrases.push_back("Llegando de la chamba bateria");
	this->m_BotPhrases.push_back("¿Todos se conocen?");
	this->m_BotPhrases.push_back("¿De qué mu vienen?");
	this->m_BotPhrases.push_back("Recuerdo jugar Mu Global");
	this->m_BotPhrases.push_back("Cuando escucho el sonido de una joya me relajo");
	this->m_BotPhrases.push_back("¿De qué parte?");
	this->m_BotPhrases.push_back("¿Quién es el admin?");
	this->m_BotPhrases.push_back("Algunos piensan que somos nerds jaja");
	this->m_BotPhrases.push_back("A mi no me importa lo que otro piense");
	this->m_BotPhrases.push_back("¿Van a jugar GTA 6?");
	this->m_BotPhrases.push_back("¿Cuánto esta el set full de primer nivel?");
	this->m_BotPhrases.push_back("Creo que el admin hace en vivo los Sábados");
	this->m_BotPhrases.push_back("Me vacila el estilo clásico season 6 con mapas nuevos");
	this->m_BotPhrases.push_back("Si, además es full hd");
	this->m_BotPhrases.push_back("Los pokemon mascota dan risa");
	this->m_BotPhrases.push_back("Pika Pika, pika piii");
	this->m_BotPhrases.push_back("jajaja");
	this->m_BotPhrases.push_back("T_T");
	this->m_BotPhrases.push_back("Amanecida");
	this->m_BotPhrases.push_back("¿Cuál es la tienda de este MU?");
	this->m_BotPhrases.push_back("¿Cómo compra ropa full gentita?");
	this->m_BotPhrases.push_back("Aún no creo que dure este server pero esta bonito");
	this->m_BotPhrases.push_back("¿Alguna elf de energía?");
	this->m_BotPhrases.push_back("Me falta aún subir de nivel");
	this->m_BotPhrases.push_back("¿Cómo se hace el reset?");
	this->m_BotPhrases.push_back("¿Cuánto tiempo tiene el MU?");
	this->m_BotPhrases.push_back("¿Son Peruanos?");
	this->m_BotPhrases.push_back("Osea, alucina broooo");
	this->m_BotPhrases.push_back("Subo de nivel mientrás trabajo");
	this->m_BotPhrases.push_back("Creo que hay Méxicanos y Uruguayos");
	this->m_BotPhrases.push_back("No mames wey");
	this->m_BotPhrases.push_back("No contaban con mi astucia");
	this->m_BotPhrases.push_back("Por fin reset");
	this->m_BotPhrases.push_back("¿Hay monturas creo?");
	this->m_BotPhrases.push_back("¿Quién me regala una charmander?");
	this->m_BotPhrases.push_back("Voy a decirle al admin que traiga a Saidoc");
	this->m_BotPhrases.push_back("Ahahah");
	this->m_BotPhrases.push_back("-_-");
	this->m_BotPhrases.push_back("vieja escuela");
	this->m_BotPhrases.push_back("Hablen en privado");
	this->m_BotPhrases.push_back("¿Summoner por ahí?");
	this->m_BotPhrases.push_back("Ven ps");
	this->m_BotPhrases.push_back("jaja me confundí");
	this->m_BotPhrases.push_back("Usa el whisper ps gil");
	this->m_BotPhrases.push_back("Sanazo");
	this->m_BotPhrases.push_back("Habla pavo ranger");
	this->m_BotPhrases.push_back("¿Y ese alucinado?");
	this->m_BotPhrases.push_back("La gente viene a bajar su pepa");
	this->m_BotPhrases.push_back("Mi gato no me deja de seguir");
	this->m_BotPhrases.push_back("Viva Peter Castle");
	this->m_BotPhrases.push_back("Nada de política aquí");
	this->m_BotPhrases.push_back("Queremos un Alan García 2");
	this->m_BotPhrases.push_back("Hablen de Mu ps");
	this->m_BotPhrases.push_back("Mu antes fue un continente al frente del Callao");
	this->m_BotPhrases.push_back("si escuche algo");
	this->m_BotPhrases.push_back("me confundo al usar el whisper");
	this->m_BotPhrases.push_back("Creo que donaré");
	this->m_BotPhrases.push_back("Salud");
	this->m_BotPhrases.push_back("Estoy con unas heineken");
	this->m_BotPhrases.push_back("Yo con Pilsen");
	this->m_BotPhrases.push_back("Whisky ps");
	this->m_BotPhrases.push_back("¿Dónde se sube más rápido?");
	this->m_BotPhrases.push_back("¿Surco o Miraflores?");
	this->m_BotPhrases.push_back("En un toque voy al gym");
	this->m_BotPhrases.push_back("En mi jato tengo un mini gimnasio");
	this->m_BotPhrases.push_back("¿Quién creo este MU?");
	this->m_BotPhrases.push_back("Me vacilan los monster en hd");
	this->m_BotPhrases.push_back("Traeré a mi clan");
	this->m_BotPhrases.push_back("¿Hacen cosplay?");
	this->m_BotPhrases.push_back("Verdad, ¿Venden polos de MU?");
	this->m_BotPhrases.push_back("Mi flaca me no deja jugar");
	this->m_BotPhrases.push_back("Ando comiendo y jugando");
	this->m_BotPhrases.push_back("Hagan en vivo ps");
	this->m_BotPhrases.push_back("Quiero show");
	this->m_BotPhrases.push_back("Me voy hacer un café");
	this->m_BotPhrases.push_back("cuenten algo ps");
	this->m_BotPhrases.push_back("Dicen que el MU es de antes del dilivuo de Noe");
	this->m_BotPhrases.push_back("¿Será que MU existio de verdad?");
	this->m_BotPhrases.push_back("Elegancia de francia");
	this->m_BotPhrases.push_back("Siempre papi nunca inpapi");
	this->m_BotPhrases.push_back("lo bueno que hay gente no como otros");
	this->m_BotPhrases.push_back("Otros mu tienen puros bots");
	this->m_BotPhrases.push_back("Es que no saben mantener a su gente ps");
	this->m_BotPhrases.push_back("si me di cuenta");
	this->m_BotPhrases.push_back("si ps se aniñan");
	this->m_BotPhrases.push_back("Habla ps ensalada");
	this->m_BotPhrases.push_back("Che");
	this->m_BotPhrases.push_back("Cabrito");
	this->m_BotPhrases.push_back("Pronto me iré de viaje para relax");
	this->m_BotPhrases.push_back("El sábado la hago si o si");
	this->m_BotPhrases.push_back("Buenos eventos");
	this->m_BotPhrases.push_back("Che boludo");
	this->m_BotPhrases.push_back("Se va a tomar su matecito");
	this->m_BotPhrases.push_back("¿Aceptan mercadopago?");
	this->m_BotPhrases.push_back("Vamos a otro mapa");
	this->m_BotPhrases.push_back("Vamos a los mapas nuevos");
	this->m_BotPhrases.push_back("¿Dónde hago master level?");
	this->m_BotPhrases.push_back("De ley los recomiendo");
	this->m_BotPhrases.push_back("Dejaré de jugar el  otro mu");
	this->m_BotPhrases.push_back("¿Alguien para party con buff?");
	this->m_BotPhrases.push_back("Quiero un pet");
	this->m_BotPhrases.push_back("¿Dónde consigo las alas nuevas?");
	this->m_BotPhrases.push_back("¿Qué mapa recomiendan para nuevos?");
	this->m_BotPhrases.push_back("Ayuda ps");
	this->m_BotPhrases.push_back("¿Qué así estamos bro?");
	this->m_BotPhrases.push_back("Bro");
	this->m_BotPhrases.push_back("Hermano ven");
	this->m_BotPhrases.push_back("Ya tengo mi set full");
	this->m_BotPhrases.push_back("Compre mi vip a 5 dólares mensual");
	this->m_BotPhrases.push_back("Apoyen porque si pasa este MU");
	this->m_BotPhrases.push_back("Mu aprobado");
	this->m_BotPhrases.push_back("¿Alguien tiene alguna anecdota en MU?");
	this->m_BotPhrases.push_back("Paraaaaaaaaaa che");
	this->m_BotPhrases.push_back("San Isidro presente desde mi piscina");
	this->m_BotPhrases.push_back("Ando en avión");
	this->m_BotPhrases.push_back("Deben hacer este mu para celular");
}

void CFakeOnline::RestoreFakeOnline()
{
	// Modificado para setear IsFakePVPMode
	for (std::map<std::string, OFFEXP_DATA>::iterator it = this->m_Data.begin(); it != this->m_Data.end(); it++)
	{
		if (gObjFindByAcc(it->second.Account) != 0)
		{
			continue;
		}
		int aIndex = gObjAddSearch(0, "127.0.0.1");
		if (aIndex >= 0)
		{
			char account[11] = { 0 };
			memcpy(account, it->second.Account, (sizeof(account) - 1));
			char password[11] = { 0 };
			memcpy(password, it->second.Password, (sizeof(password) - 1));

			gObjAdd(0, "127.0.0.1", aIndex);
			gObj[aIndex].LoginMessageSend++;
			gObj[aIndex].LoginMessageSend++;
			gObj[aIndex].LoginMessageCount++;
			gObj[aIndex].ConnectTickCount = GetTickCount();
			gObj[aIndex].ClientTickCount = GetTickCount();
			gObj[aIndex].ServerTickCount = GetTickCount();
			gObj[aIndex].MapServerMoveRequest = 0;
			gObj[aIndex].LastServerCode = -1;
			gObj[aIndex].DestMap = -1;
			gObj[aIndex].DestX = 0;
			gObj[aIndex].DestY = 0;
			GJConnectAccountSend(aIndex, account, password, "127.0.0.1");
			gObj[aIndex].Socket = INVALID_SOCKET;
            
            gObj[aIndex].IsFakePVPMode = it->second.PVPMode; // <-- COPIAR EL MODO PVP DEL XML AL OBJETO
            
			LogAdd(LOG_RED, "[FakeOnline]  [TK: %s NV: %s] Da Online Vao Server. IsFakePVPMode: %d", 
                   it->second.Account, it->second.Name, gObj[aIndex].IsFakePVPMode);
		}
	}
}

// --- RESTO DE LAS FUNCIONES ORIGINALES DE TU FakeOnline (1).cpp ---
// --- (FakeAnimationMove, FakeAutoRepair, Attack, NhatItem, etc., deben estar aquí) ---

// Ejemplo de una función original tuya:
void FakeAnimationMove(int aIndex, int x, int y, bool dixa) 
{
	LPOBJ lpObj = &gObj[aIndex];
	BYTE path[8] = {0}; 
	// ¡ADVERTENCIA! La variable 'path' se usa en tu lógica original sin ser llenada con datos de camino. 
	// Esto es un parche temporal para evitar crashes por memoria basura.
	// DEBES REVISAR CÓMO 'path' OBTIENE SUS DATOS EN TU CÓDIGO.

	if (lpObj->RegenOk > 0) { return; }
	if (lpObj->Teleport != 0) { return; }
	if (gObjCheckMapTile(lpObj, 1) != 0) { return; }
	if (gEffectManager.CheckStunEffect(lpObj) != 0 || gEffectManager.CheckImmobilizeEffect(lpObj) != 0) { return; }
	if (lpObj->SkillSummonPartyTime != 0) {
		lpObj->SkillSummonPartyTime = 0;
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(272));
	}
	lpObj->Dir = path[0] >> 4; 
	lpObj->Rest = 0;
	lpObj->PathCur = 0;
	lpObj->PathCount = path[0] & 0x0F; 
	lpObj->LastMoveTime = GetTickCount();

	memset(lpObj->PathX, 0, sizeof(lpObj->PathX));
	memset(lpObj->PathY, 0, sizeof(lpObj->PathY));
	memset(lpObj->PathOri, 0, sizeof(lpObj->PathOri));

	lpObj->TX = x;
	lpObj->TY = y;
	lpObj->PathCur = ((lpObj->PathCount > 0) ? 1 : 0);
	lpObj->PathCount = ((lpObj->PathCount > 0) ? (lpObj->PathCount + 1) : lpObj->PathCount); 
	lpObj->PathStartEnd = 1; 
	lpObj->PathX[0] = x; 
	lpObj->PathY[0] = y;
	lpObj->PathDir[0] = lpObj->Dir; 

	for (int n = 1; n < lpObj->PathCount; n++) {
		if ((n % 2) == 0) {
			lpObj->TX = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 0];
			lpObj->TY = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 1];
			lpObj->PathX[n] = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 0];
			lpObj->PathY[n] = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] & 0x0F) * 2) + 1];
			lpObj->PathOri[n - 1] = path[((n + 1) / 2)] & 0x0F;
			lpObj->PathDir[n + 0] = path[((n + 1) / 2)] & 0x0F;
		} else {
			lpObj->TX = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 0];
			lpObj->TY = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 1];
			lpObj->PathX[n] = lpObj->PathX[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 0];
			lpObj->PathY[n] = lpObj->PathY[n - 1] + RoadPathTable[((path[((n + 1) / 2)] / 0x10) * 2) + 1];
			lpObj->PathOri[n - 1] = path[((n + 1) / 2)] / 0x10;
			lpObj->PathDir[n + 0] = path[((n + 1) / 2)] / 0x10;
		}
	} 
	gMap[lpObj->Map].DelStandAttr(lpObj->OldX, lpObj->OldY);
	if (dixa == true) {
		int RandX = rand() % 3 + 1;
		int RandY = rand() % 3 + 1;
		BYTE wall = 0;
		if (x > lpObj->X) {
			wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X + RandX, lpObj->Y);
			if (wall == 1) lpObj->X += RandX;
		} else if (x <  lpObj->X) {
			wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X - RandX, lpObj->Y);
			if (wall == 1)  lpObj->X -= RandX;
		}
		if (y > lpObj->Y) {
			wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X, lpObj->Y + RandY);
			if (wall == 1) lpObj->Y += RandY;
		} else if (y <  lpObj->Y) {
			wall = gMap[lpObj->Map].CheckWall2(lpObj->X, lpObj->Y, lpObj->X, lpObj->Y - RandY);
			if (wall == 1) lpObj->Y -= RandY;
		}
	} else {
		lpObj->X = x;
		lpObj->Y = y;
	}
	lpObj->TX = lpObj->X; 
	lpObj->TY = lpObj->Y; 
	lpObj->OldX = lpObj->TX;
	lpObj->OldY = lpObj->TY;
	lpObj->ViewState = 0;
	gMap[lpObj->Map].SetStandAttr(lpObj->TX, lpObj->TY);
	PMSG_MOVE_SEND pMsgSend; 
	pMsgSend.header.set(PROTOCOL_CODE1, sizeof(pMsgSend));
	pMsgSend.index[0] = SET_NUMBERHB(lpObj->Index);
	pMsgSend.index[1] = SET_NUMBERLB(lpObj->Index);
	pMsgSend.x = (BYTE)lpObj->TX;
	pMsgSend.y = (BYTE)lpObj->TY;
	pMsgSend.dir = lpObj->Dir << 4;
	// if (!gObjPositionCheck(lpObj)) // Esta función no está definida globalmente en tu contexto
	{
		lpObj->PathCur = 0;
		lpObj->PathCount = 0;
		lpObj->TX = lpObj->X;
		lpObj->TY = lpObj->Y;
		pMsgSend.x = (BYTE)lpObj->X;
		pMsgSend.y = (BYTE)lpObj->Y;
	}
	for (int n_vp = 0; n_vp < MAX_VIEWPORT; n_vp++) { 
		if (lpObj->VpPlayer2[n_vp].type == OBJECT_USER) {
			if (lpObj->VpPlayer2[n_vp].state != OBJECT_EMPTY && lpObj->VpPlayer2[n_vp].state != OBJECT_DIECMD && lpObj->VpPlayer2[n_vp].state != OBJECT_DIED) {
				DataSend(lpObj->VpPlayer2[n_vp].index, (BYTE*)&pMsgSend, pMsgSend.header.size);
			}
		}
	}
}

void FakeAutoRepair(int aIndex)
{
	if (!gObjIsConnectedGP(aIndex)) { return; }
	LPOBJ lpObj = &gObj[aIndex];
	for (int n = 0; n < INVENTORY_WEAR_SIZE; ++n) {
		if (lpObj->Inventory[n].IsItem() != 0) {
			int money = gItemManager.RepairItem(lpObj, &lpObj->Inventory[n], n, 1);
			if (money != 0) {
				gObjectManager.CharacterCalcAttribute(aIndex);
			}
		}
	}
}

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
	if (lpObj->State == OBJECT_DELCMD || lpObj->DieRegen != 0 || lpObj->Teleport != 0) { return; }
	if (gServerInfo.InSafeZone(aIndex) == true) { return; }
	this->SuDungMauMana(aIndex);
	this->TuDongBuffSkill(aIndex);
	this->TuDongDanhSkill(aIndex); // Esta función ahora actualizará el estado de combate PVP
	FakeAutoRepair(aIndex);
}

bool FakeisJewels(int index) 
{
	if (index == GET_ITEM(12, 15) ||
		index == GET_ITEM(14, 13) ||
		index == GET_ITEM(14, 14) ||
		index == GET_ITEM(14, 16) ||
		index == GET_ITEM(14, 22) ||
		index == GET_ITEM(14, 31) ||
		index == GET_ITEM(14, 42))
	{
		return true;
	}
	return false;
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

int CFakeOnline::NhatItem(int aIndex)
{
	if (OBJMAX_RANGE(aIndex) == FALSE) { return 0; }
	if (!gObjIsConnectedGP(aIndex)) { return 0; }
	LPOBJ lpObj = &gObj[aIndex];
	if (lpObj->IsFakeOnline == 0) { return 0; }
	if (lpObj->State == OBJECT_DELCMD || lpObj->DieRegen != 0 || lpObj->Teleport != 0) { return 0; }
	if (gServerInfo.InSafeZone(aIndex) == true) { return 0; }

	CMapItem * lpMapItem;
	int distance = lpObj->ObtainRange; 
	int map_num = gObj[aIndex].Map;

	if (gObj[aIndex].DieRegen != 0) { return 0; }
	if (MAP_RANGE(map_num) == FALSE) { return 0; }

	bool PickItem = false; 

	for (int n = 0; n < MAX_MAP_ITEM; n++) {
		PickItem = false; 
		lpMapItem = &gMap[map_num].m_Item[n];
		if (lpMapItem->IsItem() == TRUE  && lpMapItem->m_Give == false && lpMapItem->m_Live == true) {
			int dis = (int)sqrt(pow(((float)lpObj->X - (float)lpMapItem->m_X), 2) + pow(((float)lpObj->Y - (float)lpMapItem->m_Y), 2));
			if (dis > distance) { continue; }
			
			if (lpObj->ObtainPickSelected == 1) { 
				if ((lpObj->ObtainPickMoney == 1 && lpMapItem->m_Index == GET_ITEM(14, 15)) 
					|| (lpObj->ObtainPickExcellent == 1 && lpMapItem->m_NewOption > 0) 
					|| (lpObj->ObtainPickAncient == 1 && lpMapItem->m_SetOption > 0) 
					|| (lpObj->ObtainPickJewels == 1 && FakeisJewels(lpMapItem->m_Index) == true)) 
				{
					PickItem = true;
				}
				else if (lpObj->ObtainPickExtra == 1) 
				{
					if (FakeitemListPickUp(lpMapItem->m_Index, lpMapItem->m_Level, lpObj) == true)
					{
						PickItem = true;
					}
				} 
				else { continue; } 
			} else { continue; }

			if (PickItem == false) { continue; }
            
            FakeAnimationMove(lpObj->Index, lpMapItem->m_X, lpMapItem->m_Y, false);

			if (lpObj->X == lpMapItem->m_X && lpObj->Y == lpMapItem->m_Y) { 
				if (lpMapItem->m_Index == GET_ITEM(14, 15)) { 
					if (lpObj->ObtainPickMoney == 1) {
						if (!gObjCheckMaxMoney(aIndex, lpMapItem->m_BuyMoney)) {
							if (lpObj->Money < MAX_MONEY) {
								lpObj->Money = MAX_MONEY;
							}
						} else {
							lpObj->Money += lpMapItem->m_BuyMoney;
						}
						gMap[map_num].ItemGive(aIndex, n); 
						continue; 
					}
				} else { 
				    if (lpMapItem->m_QuestItem != false) {
					    if (!gQuestObjective.CheckQuestObjectiveItemCount(lpObj, lpMapItem->m_Index, lpMapItem->m_Level)) {
						    continue;
					    }
				    }
				    CItem item = (*lpMapItem); 
				    BYTE result = gItemManager.InventoryInsertItemStack(lpObj, lpMapItem); 
				    if (result != 0xFF) { 
					    gMap[map_num].ItemGive(aIndex, n); 
				    } else { 
                        BYTE pos = gItemManager.InventoryInsertItem(aIndex, item);
                        if (pos != 0xFF) {
                            gMap[map_num].ItemGive(aIndex, n);
                        }
                    }
                }
			}
		}
	}
	return 1;
}

void CFakeOnline::PostChatMSG(LPOBJ lpObj) // Esta función es para mensajes de muerte, usa gMessage.
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
                    // Y los otros tipos de post que tenías
					else { PostMessage1(lpObj->Name, gMessage.GetMessage(69), (char*)messageText); } // Default
				}
			}
		}
	}
}

int random_bot_range(int minN, int maxN){ 
	if (minN > maxN) { 
        int temp = minN;
        minN = maxN;
        maxN = temp;
    }
    if (minN == maxN) return minN;
	return minN + rand() % (maxN - minN + 1);
}

void CFakeOnline::QuayLaiToaDoGoc(int aIndex)
{
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

					if (gMap[lpObj->Map].CheckAttr(DiChuyenX, DiChuyenY, 2) == 0 && gMap[lpObj->Map].CheckAttr(DiChuyenX, DiChuyenY, 4) == 0 && gMap[lpObj->Map].CheckAttr(DiChuyenX, DiChuyenY, 8) == 0) {
						lpObj->m_OfflineTimeResetMove = GetTickCount();
						FakeAnimationMove(lpObj->Index, DiChuyenX, DiChuyenY, false);
						LogAdd(LOG_BLUE, "[FakeOnline][%s] Mover a ubicación predeterminada (%d/%d)", lpObj->Name, DiChuyenX, DiChuyenY);
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

	for (int n = 0; n < MAX_VIEWPORT; n++)
	{
		if (lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 || lpObj->VpPlayer2[n].type != OBJECT_USER)
		{
			continue;
		}

		if (lpObj->GuildNumber != 0 && lpObj->GuildNumber == gObj[lpObj->VpPlayer2[n].index].GuildNumber) 
		{
			continue;
		}

        int dist = gObjCalcDistance(lpObj, &gObj[lpObj->VpPlayer2[n].index]);
		if (dist >= NearestDistance)
		{
			continue;
		}
        
		if (lpObj->IsFakePartyMode >= 2 && gParty.IsParty(gObj[lpObj->VpPlayer2[n].index].PartyNumber) == 0 && (GetTickCount() >= lpObj->IsFakeSendParty + 5000) && !gObjIsSelfDefense(&gObj[lpObj->VpPlayer2[n].index], lpObj->Index))
		{
			if (lpObj->IsFakePartyMode == 3 && !gObj[lpObj->VpPlayer2[n].index].IsFakeOnline) { 
            } else {
                lpObj->IsFakeSendParty = GetTickCount();
			    FakeAnimationMove(lpObj->Index, gObj[lpObj->VpPlayer2[n].index].X, gObj[lpObj->VpPlayer2[n].index].Y, false);
			    this->GuiYCParty(lpObj->Index, lpObj->VpPlayer2[n].index);
            }
			return false; 
		}

		if (gObjIsSelfDefense(&gObj[lpObj->VpPlayer2[n].index], lpObj->Index))
		{
			*MonsterIndex = lpObj->VpPlayer2[n].index;
			NearestDistance = dist;
			continue; 
		}

		if (lpObj->IsFakePVPMode == 2) { 
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
	}
	return ((*MonsterIndex) != -1);
}

void CFakeOnline::TuDongDanhSkill(int aIndex)
{
    // Esta función es extensa y compleja. Se basa en tu lógica original de "FakeOnline (1).cpp".
    // La principal modificación es el añadido del lock al inicio de las funciones que modifican
    // o leen datos compartidos de CFakeOnline, y la actualización de m_botPVPCombatStates.
	if (!gObjIsConnectedGP(aIndex)) { return; }
	LPOBJ lpObj = &gObj[aIndex];
	
    // Adquirir lock para datos de CFakeOnline si esta función accede a ellos indirectamente
    // o si se van a actualizar los estados de PVPCombat.
    // Sin embargo, la mayoría de las lecturas (como pBotData) ya ocurren en AttemptRandomBotComment bajo lock.
    // El principal cambio aquí es setear el estado de combate activo.
    // Para esto, necesitaremos pBotData.

    EnterCriticalSection(&this->m_BotDataMutex); // Bloquear para acceder a pBotData y m_botPVPCombatStates

	OFFEXP_DATA* pBotData = this->GetOffExpInfo(lpObj); // Obtener pBotData aquí también
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

    // lpObj->IsFakePVPMode ya está actualizado en RestoreFakeOnline con pBotData->PVPMode.
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

	int MultiPlicador = (lpObj->Class == CLASS_RF) ? 1 : 5; 
	if ((GetTickCount() - ((DWORD)lpObj->AttackCustomDelay)) >= (((((DWORD)lpObj->PhysiSpeed) * MultiPlicador) > 1500) ? 0 : (1500 - (((DWORD)lpObj->PhysiSpeed) * MultiPlicador)))) {
		lpObj->AttackCustomDelay = GetTickCount();

        // ++ INICIO LÓGICA PARA ESTADO DE COMBATE PVP ACTIVO ++
        if (targetIsPlayer) { // Si está atacando a un jugador
             LogAdd(LOG_BLUE, "[FakeOnline][%s] Atacando a jugador %s. Activando estado de combate PVP para chat.", lpObj->Name, lpTargetObj->Name);
             this->m_botPVPCombatStates[aIndex].isInActiveCombat = true;
             this->m_botPVPCombatStates[aIndex].lastPVPActionTick = GetTickCount();
             this->m_botPVPCombatStates[aIndex].saidInitialPVPPhrase = false; // Permitir frase inicial
        }
        // ++ FIN LÓGICA PARA ESTADO DE COMBATE PVP ACTIVO ++


		if (SkillRender->m_skill != SKILL_FLAME && SkillRender->m_skill != SKILL_TWISTER && SkillRender->m_skill != SKILL_EVIL_SPIRIT && SkillRender->m_skill != SKILL_HELL_FIRE && SkillRender->m_skill != SKILL_AQUA_BEAM && SkillRender->m_skill != SKILL_BLAST && SkillRender->m_skill != SKILL_INFERNO && SkillRender->m_skill != SKILL_TRIPLE_SHOT && SkillRender->m_skill != SKILL_IMPALE && SkillRender->m_skill != SKILL_MONSTER_AREA_ATTACK && SkillRender->m_skill != SKILL_PENETRATION && SkillRender->m_skill != SKILL_FIRE_SLASH && SkillRender->m_skill != SKILL_FIRE_SCREAM) {
			if (SkillRender->m_skill != SKILL_DARK_SIDE) {
                // Para usar gAttack.Attack, asegúrate que esté correctamente incluida y disponible.
                // La llamada a gAttack.Attack es la que probablemente genera los efectos visuales.
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

// ... (El resto de las funciones SendSkillAttack, SendMultiSkillAttack, etc., y GuiYCParty se mantienen como en tu original)
// ... (Copia aquí esas funciones desde tu FakeOnline (1).cpp)
// (He incluido las versiones de tu FakeOnline (1).cpp para estas abajo)

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
	// this->SendDurationSkillAttack(lpObj, target_aIndex, SkillNumber); // Comentado en tu original
}

void CFakeOnline::GuiYCParty(int aIndex, int bIndex)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (gObjIsConnectedGP(aIndex) == 0) { return; }
	if (gObjIsConnectedGP(bIndex) == 0) { return; }
	LPOBJ lpTarget = &gObj[bIndex];

	if (lpObj->Interface.use != 0 || lpTarget->Interface.use != 0) { return; }

#if(defined(CHONPHEDOILAP) && CHONPHEDOILAP != 0) 
	// if (!CheckChonPhe(lpObj->Index, lpTarget->Index)) { return; } 
#endif

	if (gServerInfo.m_PartyRestrict == 1 && gParty.IsParty(lpTarget->PartyNumber) == 0) {
		if (gObj[aIndex].PartyNumber >= 0) {
			bool levelOk = false; 
            // int partyLevel = gParty.GetLevel(gObj[aIndex].PartyNumber, &levelOk); // Necesitas la definición de gParty.GetLevel
            // if(levelOk) { ... }
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

	if (CA_MAP_RANGE(lpTarget->Map) != 0 || CC_MAP_RANGE(lpTarget->Map) != 0 || IT_MAP_RANGE(lpTarget->Map) != 0 /*|| DA_MAP_RANGE(lpTarget->Map) != 0*/ || DG_MAP_RANGE(lpTarget->Map) != 0 || IG_MAP_RANGE(lpTarget->Map) != 0) {
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