#include "stdafx.h"
#include "Log.h"
#include "GameMain.h"
#include "Notice.h"
#include "readscript.h"
#include "Monster.h"
#include "MapServerManager.h"
#include "DSProtocol.h"
#include "Event.h"
#include "ImperialGuardian.h"
#include "SwampEvent.h"
#include "Util.h"
#include "Protocol.h"
#include "YolaxD_Custom.h"
#include "Message.h"
#include "itembag.h"
#include "MemScript.h"
#include "ScheduleManager.h"
#include "ServerInfo.h"

cSwampEvent gSwampEvent;

cSwampEvent::~cSwampEvent() 
{

}

void cSwampEvent::Clean()
{
	#if(GAMESERVER_TYPE == 0)

	this->CurrentRoom = 0;
	this->CurrentGroup = 0;
	this->MobsStageGroupCount = 0;
	this->MobsKilled = 0;

	#endif
}

void cSwampEvent::Init() // OK
{
	this->m_iState = SWAMP_STATE_BLANK;
	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_TimeNotify = 0;
	this->Start = 0;

	if(this->Enabled == 0)
	{
		this->SetState(SWAMP_STATE_BLANK);
	}
	else{this->SetState(SWAMP_STATE_EMPTY);}
}

void cSwampEvent::MainProc()
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;
	
	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	this->m_RemainTime = (int)difftime(this->m_TargetTime,time(0));

	switch(this->m_iState)
	{
		case SWAMP_STATE_BLANK:
			this->ProcState_BLANK();
			break;
		case SWAMP_STATE_EMPTY:
			this->ProcState_EMPTY();
			break;
		case SWAMP_STATE_START:
			this->ProcState_START();
			break;
	}
}

void cSwampEvent::ProcState_BLANK() // OK
{

}

void cSwampEvent::ProcState_EMPTY() // OK
{
	if(this->Start == 0)
	{
		if(this->m_RemainTime > 0 && this->m_RemainTime <= 300 && this->m_TimeNotify == 0)
		{
			this->m_TimeNotify = 1;
		}

		if(this->m_RemainTime <= 0)
		{
			this->SetState(SWAMP_STATE_START);
		}
	}
}

void cSwampEvent::ProcState_START() // OK
{
	if(this->Start == 0)
	{
		if(this->m_RemainTime <= 0)
		{
			this->SetState(SWAMP_STATE_EMPTY);		
		}
	}
}

void cSwampEvent::SetState(int state) // OK
{
	this->m_iState = state;
	
	switch(this->m_iState)
	{
		case SWAMP_STATE_BLANK:
			this->SetState_BLANK();
			break;
		case SWAMP_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case SWAMP_STATE_START:
			this->SetState_START();
			break;
	}
}

void cSwampEvent::SetState_EMPTY() // OK
{
	if(this->Start == 1 && this->Enabled == 1)
	{
		return;
	}
	else
	{
		this->CheckSync();
	}

	LogAdd(LOG_BLACK,"[Swamp Event] SetState EMPTY");
}

void cSwampEvent::SetState_BLANK() // OK
{
	LogAdd(LOG_BLACK,"[Swamp Event] SetState BLANK");
}

void cSwampEvent::SetState_START()
{
	if(this->Enabled == 1)
	{
		this->StartEvent();
	}

	this->m_RemainTime = this->GroupTimer[255];

	this->m_TargetTime = (int)(time(0)+this->m_RemainTime);
}

void cSwampEvent::CheckSync() // OK
{
	if(this->SwampEventTime.empty() != 0)
	{
		this->SetState(SWAMP_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;
	
	for(std::vector<SWAMP_EVENT_TIME>::iterator it=this->SwampEventTime.begin();it != this->SwampEventTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
		
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(SWAMP_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));
	this->m_TargetTime = (int)ScheduleTime.GetTime();

	LogAdd(LOG_BLACK,"[Swamp Event] Sync Start Time. [%d] min remain",this->m_RemainTime/60);
}

void cSwampEvent::StartEvent()
{
	if((this->Start == 0) && (this->Enabled == 1))
		_beginthread( cSwampEvent__InsideTrigger, 0, NULL  );
}

void cSwampEvent::ReadMonsters(char * FilePath)
{
	SMDFile = fopen(FilePath, "r");
	
	if ( SMDFile == NULL )
	{
		ErrorMessageBox("Swamp Event data load error %s", FilePath);
		return;
	}

	for(int RoomID=0; RoomID < SWAMP_ROOMS; RoomID++)
	{
		pRoom[RoomID].Active = 0;
		pRoom[RoomID].BossID = 0;
		pRoom[RoomID].Groups = 0;
		memset(pRoom[RoomID].Territory, 0,sizeof(pRoom[RoomID].Territory));
	}

	EnterCriticalSection(&this->m_critEventData);
	this->m_swampMonsterInfo.clear();
	LeaveCriticalSection(&this->m_critEventData);

	int TotalMobCount = 0;
	int TotalMobLines = 0;

	this->CurrentRoom = 0;
	this->CurrentGroup = 0;
	this->MobsStageGroupCount = 0;
	this->MobsKilled = 0;

#if(GAMESERVER_TYPE == 0)

	if (gMapServerManager.CheckMapServer(MAP_SWAMP_OF_CALMNESS) == FALSE )
	{
		LogAdd(LOG_EVENT,"[Swamp Event] No need to load monster base file!");
	}

#endif

	int type = -1;
	SMDToken Token;

	while ( true )
	{
		Token = GetToken();
		if( Token == 2 )
		{
			break;
		}

		if( Token == NUMBER )
		{
			while(true)
			{
				type = TokenNumber;

				if( type == 0 )
				{
					while(true)
					{
						Token = GetToken();
					
						if( strcmp("end", TokenString) == 0 ) 
						{
							type++;
							break;
						}
					
						int RoomID = TokenNumber; 
						RoomID = RoomID - 1;

						if( RoomID >= SWAMP_ROOMS ) 
						{
							type++;
							break;
						}

						Token = GetToken();
						pRoom[RoomID].Groups = TokenNumber;

						Token = GetToken();
						pRoom[RoomID].BossID = TokenNumber;

						Token = GetToken();
						strcpy(pRoom[RoomID].Territory, TokenString);

						if (pRoom[RoomID].Groups > 0)
							pRoom[RoomID].Active = 1;
						else
							pRoom[RoomID].Active = 0;

						std::vector<SWAMP_EVENTMOB_DATA> vecMonsterInfo;
						this->m_swampMonsterInfo.insert( std::pair<int, std::vector<SWAMP_EVENTMOB_DATA> >(RoomID+1,vecMonsterInfo) );
						#if(GAMESERVER_TYPE == 0)
						LogAdd(LOG_EVENT,"[Swamp Section: 0] ROOM:%d [%s] has %d groups",
							RoomID+1, pRoom[RoomID].Territory, pRoom[RoomID].Groups
						);
						#endif
					}
				}
				else if(type == 1)
				{
					while(true)
					{
						Token = GetToken();
						if( strcmp("end", TokenString) == 0 ) 
						{
							type++;
							break;
						}

						SWAMP_EVENTMOB_DATA	m_MonsterInfo;

						m_MonsterInfo.Number = TokenNumber;

						Token = GetToken();
						m_MonsterInfo.XF = TokenNumber;

						Token = GetToken();
						m_MonsterInfo.YF = TokenNumber;

						Token = GetToken();
						m_MonsterInfo.XT = TokenNumber;

						Token = GetToken();
						m_MonsterInfo.YT = TokenNumber;

						Token = GetToken();
						m_MonsterInfo.Room = TokenNumber;

						Token = GetToken();
						m_MonsterInfo.Group = TokenNumber;

						Token = GetToken();
						m_MonsterInfo.Count = TokenNumber;

						TotalMobCount += m_MonsterInfo.Count;
						TotalMobLines += 1;

						EnterCriticalSection(&this->m_critEventData);

						std::map<int, std::vector<SWAMP_EVENTMOB_DATA> >::iterator it = this->m_swampMonsterInfo.find(m_MonsterInfo.Room);

						if( it != this->m_swampMonsterInfo.end() )
						{
							it->second.push_back(m_MonsterInfo);

							#if(GAMESERVER_TYPE == 0)
							LogAdd(LOG_EVENT,"[Swamp Section: 1] ROOM:%d[%d] Monster:%d Count:%d",m_MonsterInfo.Room, m_MonsterInfo.Group,
								m_MonsterInfo.Number, m_MonsterInfo.Count);
							#endif
						}
						LeaveCriticalSection(&this->m_critEventData);
					}
				}
				break;
			}
		}
	}
	fclose(SMDFile);
	#if(GAMESERVER_TYPE == 0)
	LogAdd(LOG_EVENT,"[Swamp Event] - %s file is Loaded Event Lines:%d Mobs:%d",FilePath,TotalMobLines,TotalMobCount);
	#endif
}

void cSwampEvent::LoadDataConfig(char * FilePath)
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,FilePath);
		return;
	}

	if(lpMemScript->SetBuffer(FilePath) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	for(int GroupID = 0; GroupID < SWAMP_MAXGROUPS; GroupID++)
	{
		this->GroupTimer[GroupID] = 0;
	}

	this->SwampEventTime.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}
		
			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					SWAMP_EVENT_TIME info;

					info.Year = lpMemScript->GetNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->SwampEventTime.push_back(info);		

				}
				else if (section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						
						break;
					}
					// - Section
				
					this->Enabled = lpMemScript->GetNumber();
	
					this->OnlyMastersEnter = lpMemScript->GetAsNumber();
	
					this->SwampMapEnterOnlyWhenStarted = lpMemScript->GetAsNumber();

					this->SwampIfWonKeepOpenMapMinutes = lpMemScript->GetAsNumber();

				}
				else if (section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
						
					this->SwampWarpGate = lpMemScript->GetNumber();
	
					this->SwampGateRangeStart = lpMemScript->GetAsNumber();
	
					this->SwampGateRangeEnd = lpMemScript->GetAsNumber();
					
				}
				else if (section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int GroupID = lpMemScript->GetNumber();
					GroupID = GroupID - 1;

					if( GroupID >= SWAMP_MAXGROUPS ) 
					{
						section++;
						break;
					}

					GroupTimer[GroupID] = lpMemScript->GetAsNumber();
					
					#if(GAMESERVER_TYPE == 0)
					LogAdd(LOG_EVENT,"[Swamp Event] GROUP:%d set to %d minutes",
						GroupID+1, GroupTimer[GroupID]);
					#endif
				}
				else
					break;
				}
			}
		}

	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
				
}

bool cSwampEvent::GateMove(int aIndex, int Gate)
{
	if(this->Enabled == 1)
	{
		if(Gate >= this->SwampGateRangeStart && Gate <= this->SwampGateRangeEnd)
		{
			if (this->OnlyMastersEnter == 1)
			{
				if(ObjYola.gObjIsNewClass(&gObj[aIndex]) == 0)
				{
					gNotice.NewNoticeSend(aIndex,0,0,0,0,0,"This map is only for Masters");
					return false;
				}
			}

			if (this->SwampWarpGate != Gate)
			{
				if(this->Start == 0)
				{
					if(this->SwampMapEnterOnlyWhenStarted == 1)
					{
						gNotice.NewNoticeSend(aIndex,0,0,0,0,0,"[SwampEvent] the event has not started");
						return false;
					}
				}
			}
		}
	}
	return true;
}

void cSwampEvent::MonsterDie(LPOBJ lpMon, int pIndex)
{
	#if(GAMESERVER_TYPE == 0)

	if(this->Enabled == 1)
	{
		if(lpMon->Map == MAP_SWAMP_OF_CALMNESS)
		{
			if( lpMon->Attribute == 63 )
			{
				if ( this->MobsStageGroupCount > this->MobsKilled)
				{
					EnterCriticalSection(&this->m_critEventData);
					this->MobsKilled++;
					LeaveCriticalSection(&this->m_critEventData);
				}

				lpMon->Attribute = 60;
			}
		}
	}

	#endif
}

void cSwampEvent::Timer(int Seconds, BYTE State)
{
	#if(GAMESERVER_TYPE == 0)

	PMSG_TIMER_IMPERIALGUARDIAN Time;
	
	ObjYola.PHeadSubSetB((LPBYTE)&Time, 0xF7, 0x04, sizeof(Time));

	int Remaining = 0;
	if (State == 0)
	{
		Remaining = SWAMP_STANBYSECONDS - Seconds;
	} else {
		Remaining = (this->GroupTimer[this->CurrentGroup-1] * 60) - Seconds;
	}

	int toTimer=0;
	if(Remaining == 34)
	{
		toTimer = (Remaining * 3.911) + 1;
	}else
	{
		toTimer = Remaining * 3.911;
	}
	Time.Time1 = LOBYTE(toTimer);
	Time.Time2 = HIBYTE(toTimer);
	Time.Time3 = 0;

	Time.MonstersLeft = this->MobsStageGroupCount - this->MobsKilled;
	Time.State = State;

	DataSendInside((BYTE*)&Time,sizeof(Time));

	#endif
}

void cSwampEvent::DataSendInside(LPBYTE lpMsg, int iMsgSize)
{
	#if(GAMESERVER_TYPE == 0)

	for ( int n = OBJECT_START_USER ; n < MAX_OBJECT ; n++)
	{
		if ( gObj[n].Connected == OBJECT_ONLINE )
		{
			if ( gObj[n].Type == OBJECT_USER && gObj[n].Map == MAP_SWAMP_OF_CALMNESS)
			{
				DataSend(n, (unsigned char*)lpMsg , iMsgSize );
			}
		}
	}

	#endif
}

void cSwampEvent::ClearAllMonsters()
{
	#if(GAMESERVER_TYPE == 0)

	if(this->Enabled == 1)
	{
		for ( int n = 0 ; n < OBJECT_START_USER ; n++)
		{
			if ( gObj[n].Map == MAP_SWAMP_OF_CALMNESS)
			{
				if( gObj[n].Attribute == 63 )
				{
					gObjDel(gObj[n].Index);
				}
			}
		}
	}

	#endif
}

void cSwampEvent::SetMonsters(int iAssultType, int iGroup)
{
	#if(GAMESERVER_TYPE == 0)

	if(this->Enabled == 0)
	{
		this->Start = 0;
		return;
	}

	if (gMapServerManager.CheckMapServer(MAP_SWAMP_OF_CALMNESS) == FALSE )
	{
		this->Enabled = 0;
		this->Start = 0;
		return;
	}

	this->MobsStageGroupCount = 0;
	this->MobsKilled = 0;

	std::map<int, std::vector<SWAMP_EVENTMOB_DATA> >::iterator it = this->m_swampMonsterInfo.find(iAssultType);

	if( it == this->m_swampMonsterInfo.end() ) 
		return;

	for( std::vector<SWAMP_EVENTMOB_DATA>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++ )
	{
		std::vector<SWAMP_EVENTMOB_DATA>::iterator pMonsterInfo = it2;

		if( pMonsterInfo->Group != iGroup )
			continue;

		LogAdd(LOG_BLACK,"[Swamp Event][AssultType:%d,Group:%d] - Adding Monsters ID: %d / QTY: %d",
			iAssultType,iGroup,pMonsterInfo->Number,pMonsterInfo->Count);

		for( int i = 0; i < pMonsterInfo->Count; i++ )
		{

			int cX = pMonsterInfo->XF;
			int cY = pMonsterInfo->YF;
			
			if( gObjGetRandomFreeLocation(MAP_SWAMP_OF_CALMNESS, &cX, &cY,(pMonsterInfo->XF-pMonsterInfo->YF),( pMonsterInfo->YT-pMonsterInfo->XT), 100) == TRUE )
			{
				cX = pMonsterInfo->XF;
				cY = pMonsterInfo->YF;

				int iMobIndex = gObjAddMonster(MAP_SWAMP_OF_CALMNESS);

				if(OBJECT_RANGE( iMobIndex >= 0 ))
				{

					gObj[iMobIndex].PosNum = -1;
					gObj[iMobIndex].X = cX;
					gObj[iMobIndex].Y = cY;
					gObj[iMobIndex].Map = MAP_SWAMP_OF_CALMNESS;
					gObj[iMobIndex].TX = gObj[iMobIndex].X;
					gObj[iMobIndex].TY = gObj[iMobIndex].Y;
					gObj[iMobIndex].OldX = gObj[iMobIndex].X;
					gObj[iMobIndex].OldY = gObj[iMobIndex].Y;
					gObj[iMobIndex].Dir = 1;
					gObj[iMobIndex].StartX = gObj[iMobIndex].X;
					gObj[iMobIndex].StartY = gObj[iMobIndex].Y;
				
					gObjSetMonster(iMobIndex, pMonsterInfo->Number);
					gObj[iMobIndex].MoveRange = 15;
					gObj[iMobIndex].ViewRange = 15;
					gObj[iMobIndex].Attribute = 63;
					gObj[iMobIndex].Dir = ( rand() % 8 );
					gObj[iMobIndex].DieRegen = 0;
					gObj[iMobIndex].RegenTime = 1;
					gObj[iMobIndex].MaxRegenTime = 1000;
					gObj[iMobIndex].LastCheckTick = GetTickCount();
					
					this->MobsStageGroupCount++;
					
					if (this->MobsStageGroupCount >= SWAMP_MOBSTOKILLCOUNT)
					{
						LogAdd(LOG_BLACK,"[Swamp Event][ERROR][AssultType:%d,Group:%d] Monster ammount is over",
							iAssultType,iGroup
						);
						return;
					}
				} else {
					LogAdd(LOG_BLACK,"[Swamp Event][ERROR][AssultType:%d,Group:%d] gObjAddMonster returned: %d",
							iAssultType,iGroup,
							iMobIndex
						);
				}
			} else {
				LogAdd(LOG_BLACK,"[Swamp Event][ERROR][AssultType:%d,Group:%d][%d] - Fail Getting Location for: %d [%d,%d,%d,%d]",
					iAssultType,iGroup,i,pMonsterInfo->Number,pMonsterInfo->XF, pMonsterInfo->YF,pMonsterInfo->XT, pMonsterInfo->YT);
			}
		}
	}

	#endif
}

void cSwampEvent::WarpOutside() 
{
	#if(GAMESERVER_TYPE == 0)

	for ( int n = OBJECT_START_USER ; n < MAX_OBJECT ; n++)
	{
		if ( gObj[n].Connected == OBJECT_ONLINE )
		{
			if ( gObj[n].Type  == OBJECT_USER )
			{
				if (gObj[n].Map == MAP_SWAMP_OF_CALMNESS)
				{
					BOOL ret = gObjMoveGate(gObj[n].Index, this->SwampWarpGate);

					if (ret = false)
					{
						CloseClient(n);
					}
				}
			}
		}
	}
	
	LogAdd(LOG_BLACK, "[Swamp Event] State -> FINISH: REMOVE_USERS");

	#endif
}

void cSwampEvent__InsideTrigger(void *  lpParam)
{
	int Seconds = 0;
	
	gSwampEvent.Start = 1;

	#if(GAMESERVER_TYPE == 0)

	for(int i = 6; i > 0; i--)
	{
		if (i == 6)
		{
			LogAdd(LOG_BLACK,"[SwampEvent] PREPARING EVENT");
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Medusas are preparing to attack");
		}
		else
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Started in %d Minutes",i);
			LogAdd(LOG_BLACK,"[SwampEvent] Started in %d Minutos",i);
		}
		Sleep(60000);
	}

	LogAdd(LOG_BLACK,"[SwampEvent] PREPARING NEW GROUP",1);
	
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Preparing for Next Round!",1);
	
	#else

	for(int i = 6; i > 0; i--)
	{
		if (i == 6)
		{
			LogAdd(LOG_BLACK,"[SwampEvent] PREPARING EVENT");
		}
		else
		{
			LogAdd(LOG_BLACK,"[SwampEvent] Started in %d Minutos",i);
		}
		Sleep(60000);
	}

	LogAdd(LOG_BLACK,"[SwampEvent] PREPARING NEW GROUP",1);

	#endif

	#if(GAMESERVER_TYPE == 0)

	BOOL Activate = FALSE;
	BYTE Overflow = 0;
	BYTE Success = 0;

	int roomCalculation = 0;
	roomCalculation = (rand()%(SWAMP_ROOMS*100))/100;
	gSwampEvent.CurrentRoom = roomCalculation;
	gSwampEvent.CurrentGroup = 1;
	gSwampEvent.Start = TRUE;

	while (Activate == FALSE && Overflow < 10)
	{
		gSwampEvent.CurrentRoom = (rand()%SWAMP_ROOMS) + 1;

		if (gSwampEvent.pRoom[gSwampEvent.CurrentRoom - 1].Active == 1)
			Activate = TRUE;

		Overflow++;
	}

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Medusas Attack: %s", gSwampEvent.pRoom[gSwampEvent.CurrentRoom - 1].Territory);
	
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"Find Medusa troops and destroy them!");
	
	gSwampEvent.SetMonsters(gSwampEvent.CurrentRoom,gSwampEvent.CurrentGroup);

	while(Activate)
	{
		if (gSwampEvent.Start == FALSE)
		{
			Activate = FALSE;
			Success = 0;
			break;
		}

		gSwampEvent.Timer(Seconds, 1);
		if(Seconds >= (gSwampEvent.GroupTimer[gSwampEvent.CurrentGroup-1] * 60))
		{
			LogAdd(LOG_BLACK, "[SwampEvent] State -> FAIL: TIME EXCEED");
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Protection failed",0);
			break;
		}
		
		if( gSwampEvent.MobsKilled >= gSwampEvent.MobsStageGroupCount )
		{
			LogAdd(LOG_BLACK, "[SwampEvent] Room:%d Group:%d Defeated -> SUCCESS", gSwampEvent.CurrentRoom, gSwampEvent.CurrentGroup);
			gSwampEvent.CurrentGroup++;

			if (gSwampEvent.pRoom[gSwampEvent.CurrentRoom-1].Groups >= gSwampEvent.CurrentGroup &&
				gSwampEvent.GroupTimer[gSwampEvent.CurrentGroup-1] > 0)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Preparing for Next Round!",0);
				Seconds = 0;

				while( true )
				{
					gSwampEvent.Timer(Seconds, 0);
					Seconds++;
					Sleep(1000);

					if (Seconds == SWAMP_STANBYSECONDS)
						break;
				}

				gSwampEvent.SetMonsters(gSwampEvent.CurrentRoom,gSwampEvent.CurrentGroup);
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] another army attacked the territory!",0);
				
			} else {
				Success = 1;
					
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Protection sucessfully",0);
				Sleep(5000);
				break;
			}
		}

		if (gSwampEvent.MobsStageGroupCount <= 0)
		{
			Success = 1;
			LogAdd(LOG_BLACK,"[SwampEvent] Room:%d Group:%d Defeated -> MOBCOUNT_ERROR -> SUCCESS", gSwampEvent.CurrentRoom, gSwampEvent.CurrentGroup);

		
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Protection sucessfully",0);
			Sleep(5000);
			break;
		}
		
		Seconds++;
		Sleep(1000);
	}

	LogAdd(LOG_BLACK,"[SwampEvent] State -> PRECLOSURE: Monster Clean");
	
	gSwampEvent.ClearAllMonsters();

	if (gSwampEvent.SwampMapEnterOnlyWhenStarted == 1)
	{
		if (Success == 1 && gSwampEvent.SwampIfWonKeepOpenMapMinutes > 0)
		{
			int secondsopen = 0;
			secondsopen = gSwampEvent.SwampIfWonKeepOpenMapMinutes*60*60;

			
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Map has been won and remain open for %d minutes",gSwampEvent.SwampIfWonKeepOpenMapMinutes);
			
			LogAdd(LOG_BLACK,"[SwampEvent] State -> PRECLOSURE: Keep Map Open");

			while(secondsopen > 0)
			{
				if (secondsopen == 300 || secondsopen == 240 || secondsopen == 180 || secondsopen == 120 || secondsopen == 60 )
				{
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SwampEvent] Map will close in %d minutes",secondsopen/60);
				}	

				secondsopen--;
				Sleep(1000);
			}
		}

		LogAdd(LOG_BLACK,"[SwampEvent] State -> PRECLOSURE: Prepare kick players");

		if (Activate == TRUE)
		{
			gNotice.GCNoticeSendToAll(1,0,0,0,0,0,"You will be warped outside in %d seconds", SWAMP_STANBYSECONDS);
			Sleep(SWAMP_STANBYSECONDS*1000);
		}
		gSwampEvent.WarpOutside();
	}

	LogAdd(LOG_BLACK,"[SwampEvent] State -> CLOSED");
	
	gSwampEvent.Clean();

	_endthread();

	#endif
	
	gSwampEvent.Start = 0;
}
