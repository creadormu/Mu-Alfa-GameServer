#include "stdafx.h"
#include "GameMain.h"
#include "BattleSoccerManager.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "ESProtocol.h"
#include "HackCheck.h"
#include "JSProtocol.h"
#include "MasterSkillTree.h"
#include "MonsterAI.h"
#include "MonsterManager.h"
#include "ObjectManager.h"
#include "Protect.h"
#include "QueueTimer.h"
#include "ServerInfo.h"
#include "SocketManagerUdp.h"
#include "User.h"
#include "Util.h"
#include "CustomAttack.h"
#include "OfflineMode.h"
#include "FakeOnline.h"
#include "Licencia.h"
#include "SkyEvent.h"
#include "SwampEvent.h"

CConnection gJoinServerConnection;
CConnection gDataServerConnection;

void GameMainInit(HWND hwnd) // OK
{
	// --- NUEVO ---
	srand((unsigned int)time(NULL)); // Inicializar semilla para rand()
	// --- FIN NUEVO ---

	if(CreateMutex(0,0,gServerInfo.m_ServerMutex) == 0 || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ErrorMessageBox("Could not open GameServer");
		return;
	}

	PROTECT_START

#if( LUIS_LICENCIA == 1)
		
	ReadyLicense(); //Licencia
			
#endif

	gSkyEvent.Init();

	gSwampEvent.Init();

	gObjInit();

	InitHackCheck();

	InitBattleSoccer();

	gGameServerLogOut = 0;

	gGameServerDisconnect = 0;

	gServerInfo.ReadInit();

	//gCommandManager.Init();

	gObjSetExperienceTable();

	gMonsterManager.SetMonsterData();

	#if(GAMESERVER_UPDATE>=401)

	gMasterSkillTree.SetMasterLevelExperienceTable();

	#endif

	gJoinServerConnection.Init(hwnd,JoinServerProtocolCore);

	gDataServerConnection.Init(hwnd,DataServerProtocolCore);

	PROTECT_FINAL
}

void ConnectServerInfoSend() // OK
{
	PROTECT_START

	SDHP_GAME_SERVER_LIVE_SEND pMsg;

	pMsg.header.set(0x01,sizeof(pMsg));

	pMsg.ServerCode = (WORD)gServerInfo.m_ServerCode;
if( Conectar == 1 )
{
	pMsg.UserTotal = (BYTE)((gObjTotalUser==0)?0:((gObjTotalUser*100) / 100));
}
else
{
	pMsg.UserTotal = (BYTE)((gObjTotalUser==0)?0:((gObjTotalUser*100)/ gServerInfo.m_ServerMaxUserNumber));
}

	pMsg.UserCount = (WORD)gObjTotalUser;

	pMsg.AccountCount = 0;

	pMsg.PCPointCount = 0;
if( Conectar == 1 )
{
	pMsg.MaxUserCount = (WORD) 100; //gServerInfo.m_ServerMaxUserNumber;
}
else
{
	pMsg.MaxUserCount = (WORD) gServerInfo.m_ServerMaxUserNumber;
}
	gSocketManagerUdp.DataSend((BYTE*)&pMsg,pMsg.header.size);

	PROTECT_FINAL
}

bool JoinServerConnect(DWORD wMsg) // OK
{
	if(gJoinServerConnection.Connect(gServerInfo.m_JoinServerAddress,(WORD)gServerInfo.m_JoinServerPort,wMsg) == 0)
	{
		return 0;
	}

	GJServerInfoSend();
	return 1;
}

bool DataServerConnect(DWORD wMsg) // OK
{
	if(gDataServerConnection.Connect(gServerInfo.m_DataServerAddress,(WORD)gServerInfo.m_DataServerPort,wMsg) == 0)
	{
		return 0;
	}

	GDServerInfoSend();
	return 1;
}

bool JoinServerReconnect(HWND hwnd,DWORD wMsg) // OK
{
	if(gJoinServerConnection.CheckState() == 0)
	{
		gJoinServerConnection.Init(hwnd,JoinServerProtocolCore);
		return JoinServerConnect(wMsg);
	}

	return 1;
}

bool DataServerReconnect(HWND hwnd,DWORD wMsg) // OK
{
	if(gDataServerConnection.CheckState() == 0)
	{
		gDataServerConnection.Init(hwnd,DataServerProtocolCore);
		return DataServerConnect(wMsg);
	}

	return 1;
}

void JoinServerMsgProc(WPARAM wParam,LPARAM lParam) // OK
{
	switch(LOWORD(lParam))
	{
		case FD_READ:
			gJoinServerConnection.DataRecv();
			break;
		case FD_WRITE:
			gJoinServerConnection.DataSendEx();
			break;
		case FD_CLOSE:
			gJoinServerConnection.Disconnect();
			gObjAllDisconnect();
			break;
	}
}

void DataServerMsgProc(WPARAM wParam,LPARAM lParam) // OK
{
	switch(LOWORD(lParam))
	{
		case FD_READ:
			gDataServerConnection.DataRecv();
			break;
		case FD_WRITE:
			gDataServerConnection.DataSendEx();
			break;
		case FD_CLOSE:
			gDataServerConnection.Disconnect();
			gObjAllDisconnect();
			break;
	}
}

void CALLBACK QueueTimerCallback(PVOID lpParameter,BOOLEAN TimerOrWaitFired) // OK
{
	PROTECT_START

	static CCriticalSection critical;

	critical.lock();

	switch(((int)lpParameter))
	{
		case QUEUE_TIMER_MONSTER:
			gObjectManager.ObjectMonsterAndMsgProc();
			break;
		case QUEUE_TIMER_MONSTER_MOVE:
			gObjectManager.ObjectMoveProc();
			break;
		case QUEUE_TIMER_MONSTER_AI:
			CMonsterAI::MonsterAIProc();
			break;
		case QUEUE_TIMER_MONSTER_AI_MOVE:
			CMonsterAI::MonsterMoveProc();
			break;
		case QUEUE_TIMER_EVENT:
			gObjEventRunProc();
			break;
		case QUEUE_TIMER_VIEWPORT:
			gObjViewportProc();

#if(OFFLINE_MODE==TRUE)
			for (int n = OBJ_STARTUSERINDEX; n < MAX_OBJECT; n++)
			{
			   if(gObj[n].Connected < OBJECT_LOGGED) // Ejemplo de chequeo si el slot está activo
				{
					continue;
				}

				g_OfflineMode.PickUP(n);
				g_OfflineMode.regresar(n);

#if USE_FAKE_ONLINE == TRUE
                s_FakeOnline.Attack(n); 
                s_FakeOnline.TuDongBuffSkill(n); 
                s_FakeOnline.SuDungMauMana(n); 
                s_FakeOnline.QuayLaiToaDoGoc(n); 
                s_FakeOnline.NhatItem(n); 
                s_FakeOnline.TuDongDanhSkill(n);
				// --- NUEVA LLAMADA ---
				s_FakeOnline.AttemptRandomBotComment(n);
				// --- FIN NUEVA LLAMADA ---
#endif


			}
#endif
			break;
		case QUEUE_TIMER_FIRST:
			gObjFirstProc();
			break;
		case QUEUE_TIMER_CLOSE:
			gObjCloseProc();
			break;
		case QUEUE_TIMER_MATH_AUTHENTICATOR:
			gObjMathAuthenticatorProc();
			break;
		case QUEUE_TIMER_ACCOUNT_LEVEL:
			gObjAccountLevelProc();
			break;
		case QUEUE_TIMER_PICK_COMMAND:
			gObjPickProc();
			break;
	}

	critical.unlock();

	PROTECT_FINAL
}
