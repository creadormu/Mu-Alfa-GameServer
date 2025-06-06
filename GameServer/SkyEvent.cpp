#include "stdafx.h"
#include "SkyEvent.h"
#include "Util.h"
#include "GameMain.h"
#include "Notice.h"
#include "Message.h"
#include "readscript.h"
#include "Monster.h"
#include "MonsterSetBase.h"
#include "MapServerManager.h"
#include "DSProtocol.h"
#include "Event.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "YolaxD_Custom.h"
#include "MemScript.h"
#include "ScheduleManager.h"
#include "ServerInfo.h"
#include "SocketItemType.h"

cSkyEvent gSkyEvent;

BOOL SKYEVENT_MAP_RANGE(int Map, BYTE X, BYTE Y)	//OK
{
	if (Map != 10)
	{
		return FALSE;
	}else
	{
		if(X >= 130)
			return TRUE;
	}
	return FALSE;
}

cSkyEvent::cSkyEvent()
{
	this->m_State = SKY_STATE_BLANK;
	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
}

cSkyEvent::~cSkyEvent()
{

}

void cSkyEvent::Init()
{
	if(this->Enabled == 0)
	{
		this->SetState(SKY_STATE_BLANK);
	}
	else
	{		
		this->SetState(SKY_STATE_EMPTY);		
	}

	this->MonsterLoaded = 0;
}

void cSkyEvent::LoadData(char * FilePath)
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
		
	for(int iLevel; iLevel < MAX_SKY_EVENT_LEVEL; iLevel++)
	{
		for(int Stage = 0; Stage < MAX_SKY_EVENT_STAGES; Stage++)
		{
			iLevel = 0;
			this->Level_Min[iLevel] = 0;
			this->Level_Max[iLevel]	= 0;
			this->ExtraExp[iLevel][Stage] = 0;
			this->ItemWinType[iLevel] = 0;
			this->ItemWinIndex[iLevel] = 0;
			this->ItemWinLevel[iLevel] = 0;
			this->ItemWinDur[iLevel] = 0;
			this->ItemWinLuck[iLevel] = 0;						
			this->ItemWinSkill[iLevel] = 0;						
			this->ItemWinOpt[iLevel] = 0;					
			this->ItemWinExc[iLevel] = 0;					
			this->WCoinCWin[iLevel] = 0;						
			this->WCoinPWin[iLevel] = 0;						
			this->GoblinPointWin[iLevel] = 0;	
		}
	}

	this->m_SkyEventTime.clear();

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
						
					SKY_EVENT_START_TIME info;

					info.Year = lpMemScript->GetNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();
					
					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();
					
					this->m_SkyEventTime.push_back(info);
				}
				else if (section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->Enabled = lpMemScript->GetNumber();

					this->StageMinutes[0] = lpMemScript->GetAsNumber();	

					this->StageMinutes[1] = lpMemScript->GetAsNumber();	

					this->StageMinutes[2] = lpMemScript->GetAsNumber();	

					this->Start = 0;
				}
				else if (section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					
					int	iLevel = lpMemScript->GetNumber();

					this->Level_Min[iLevel] = lpMemScript->GetAsNumber();

					this->Level_Max[iLevel]	= lpMemScript->GetAsNumber();
					
					this->ExtraExp[iLevel][0] = lpMemScript->GetAsNumber();

					this->ExtraExp[iLevel][1] = lpMemScript->GetAsNumber();

					this->ExtraExp[iLevel][2] = lpMemScript->GetAsNumber();

					this->ItemWinType[iLevel]	= lpMemScript->GetAsNumber();
						
					this->ItemWinIndex[iLevel]	= lpMemScript->GetAsNumber();
						
					this->ItemWinLevel[iLevel]	= lpMemScript->GetAsNumber();
						
					this->ItemWinDur[iLevel]	= lpMemScript->GetAsNumber();
						
					this->ItemWinLuck[iLevel]	= lpMemScript->GetAsNumber();
						
					this->ItemWinSkill[iLevel]	= lpMemScript->GetAsNumber();
						
					this->ItemWinOpt[iLevel]	= lpMemScript->GetAsNumber();
						
					this->ItemWinExc[iLevel]	= lpMemScript->GetAsNumber();
						
					this->WCoinCWin[iLevel]		= lpMemScript->GetAsNumber();
						
					this->WCoinPWin[iLevel]		= lpMemScript->GetAsNumber();
						
					this->GoblinPointWin[iLevel]	= lpMemScript->GetAsNumber();					
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

void cSkyEvent::MainProc()
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	this->m_RemainTime = (int)difftime(this->m_TargetTime,time(0));

	switch(this->m_State)
	{
		case SKY_STATE_BLANK:
			this->ProcState_BLANK();
			break;
		case SKY_STATE_EMPTY:
			this->ProcState_EMPTY();
			break;
		case SKY_STATE_START:
			this->ProcState_START();
			break;
	}
}

void cSkyEvent::ProcState_BLANK()
{
	
}

void cSkyEvent::ProcState_EMPTY()
{
	if(this->Start == 0)
	{
		if(this->m_RemainTime <= 0)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SkyEvent] Event Started");

			this->SetState(SKY_STATE_START);
		}
	}
}

void cSkyEvent::ProcState_START()
{	
	if(this->Start == 0)
	{
		if(this->m_RemainTime <= 0)
		{
			this->SetState(SKY_STATE_EMPTY);		
		}
	}
}

void cSkyEvent::SetState(int state)
{
	this->m_State = state;

	switch(this->m_State)
	{
		case SKY_STATE_BLANK:
			this->SetState_BLANK();
			break;
		case SKY_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case SKY_STATE_START:
			this->SetState_START();
			break;
	}
}

void cSkyEvent::SetState_BLANK()
{
	LogAdd(LOG_BLACK,"[SkyEvent] SetState_BLANK");
}

void cSkyEvent::SetState_EMPTY()
{
	if(this->Start == 1 )
	{
		return;
	}
	else
	{
		this->CheckSync();
	}
	
	LogAdd(LOG_BLACK,"[SkyEvent] SetState_EMPTY");
}

void cSkyEvent::SetState_START()
{
	if(this->Enabled == 1)
	{
		this->StartEvent();
	}

	this->m_RemainTime = this->StageMinutes[3];

	this->m_TargetTime = (int)(time(0)+this->m_RemainTime);
}

void cSkyEvent::CheckSync()
{
	if(this->m_SkyEventTime.empty() != 0)
	{
		this->SetState(SKY_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<SKY_EVENT_START_TIME>::iterator it=this->m_SkyEventTime.begin();it != this->m_SkyEventTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(SKY_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	this->m_TargetTime = (int)ScheduleTime.GetTime();

	LogAdd(LOG_BLACK,"[SkyEvent] Sync Start Time. [%d] min remain",this->m_RemainTime/60);

}

int cSkyEvent::GetWinner(BYTE iLevel)	//OK
{
	int MaxKillMob = 0;
	
	int WinUser = -1;
	
	for(int iUser = 0; iUser < MAX_SKY_EVENT_SUB_LEVEL; iUser++)
	{
		if(this->m_UserData[iLevel][iUser].m_Index > -1)
		{
			int PlayerKillMobs = 0;
			for(int iStage = 0;iStage < MAX_SKY_EVENT_STAGES; iStage++)
			{
				PlayerKillMobs += (this->m_UserData[iLevel][iUser].KillCount[iStage] * (iStage+1));
			}

			if(PlayerKillMobs > MaxKillMob)
				MaxKillMob = PlayerKillMobs;
				WinUser = iUser;
		}			
	}
	return WinUser;
}

int cSkyEvent::GetPlayerSubIndex(LPOBJ lpObj, BYTE iLevel) //OK
{
	for(int iUser = 0; iUser < MAX_SKY_EVENT_SUB_LEVEL; iUser++)
	{
		int m_Index = this->m_UserData[iLevel][iUser].m_Index;
		
		if(m_Index == lpObj->Index)
		{
			return iUser;
		}
	}
	return -1;
}

int cSkyEvent::GetLevel(LPOBJ lpObj) //OK
{
	int Err = -1;
	
	if((this->Enabled == 1) && (this->Start == 1))
	{
		if((lpObj->Map == MAP_ICARUS) && (lpObj->X >= 130))
		{
			int X = lpObj->X;
			int Y = lpObj->Y;


			if (X >= 130 && X <= 166)
			{
				if (Y >= 188 && Y <= 228) //Lvl 1
				{
					return 0;
				}
				else if (Y >= 128 && Y <= 170) //Lvl 3
				{
					return 2;
				}
			}
			else if (X >= 193 && X <= 228)
			{
				if (Y >= 188 && Y <= 228) //Lvl 2
				{
					return 1;
				}
				else if (Y >= 128 && Y <= 170) //Lvl 4
				{
					return 3;
				}
				else if (Y >= 68 && Y <= 111) //Lvl 5
				{
					return 4;
				}
			}
		}
	}
	return Err;
}

int cSkyEvent::MonsterDie(LPOBJ lpObj)	//OK
{
	int iLevel = this->GetLevel(lpObj);
	
	if(iLevel > -1)
	{
		int iUser = this->GetPlayerSubIndex(lpObj,iLevel);
		
		if(iUser > -1)
		{
			if(this->CurrentStage > 0)
			{
				this->m_UserData[iLevel][iUser].KillCount[(this->CurrentStage-1)]++;
				
				return (this->ExtraExp[iLevel][(this->CurrentStage-1)]);
			}
		}
		else
		{
			LogAdd(LOG_EVENT,"error-L3 : [Sky Event] (%d) doesn't match the user (%d) %s",
				iLevel+1, lpObj->Index,lpObj->Name);
		}
	}
	return 1;
}

void cSkyEvent::DropWinnerItem(int iLevel,int iUser,BYTE SocketBonus,LPBYTE SocketOptions)	
{
	int iIndex = this->m_UserData[iLevel][iUser].m_Index;
	
	if ( OBJMAX_RANGE(iIndex) == FALSE )
	{
		return;
	}
	
	int iType = ObjYola.ItemGetNumberMake(this->ItemWinType[iLevel], this->ItemWinIndex[iLevel]);

	GDCreateItemSend(gObj[iIndex].Index,0xEB,gObj[iIndex].X,gObj[iIndex].Y,GET_ITEM(this->ItemWinType[iLevel],
		this->ItemWinIndex[iLevel]),this->ItemWinLevel[iLevel],0,this->ItemWinSkill[iLevel],this->ItemWinLuck[iLevel],
		this->ItemWinOpt[iLevel],
		-1,this->ItemWinExc[iLevel],0,0,SocketBonus,SocketOptions,0xFE,0);
	
	int Coin1 = this->WCoinCWin[iLevel];

	int Coin2 = this->WCoinPWin[iLevel];

	int Coin3 = this->GoblinPointWin[iLevel];

	if (Coin1 > 0 || Coin2 > 0 || Coin3 > 0)
	{	
		GDSetCoinSend(gObj[iIndex].Index, Coin1, Coin2, Coin3, "Lottery");
	}


	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"Character: %s Winner",&gObj[iIndex].Name);
	
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"SkyEvent Level[%d]",iLevel+1);
}

void cSkyEvent::MsgStringSend(LPSTR  szMsg, BYTE type) //OK
{
	for ( int n = OBJECT_START_USER ; n < MAX_OBJECT ; n++)
	{
		if ( gObj[n].Connected == OBJECT_ONLINE )
		{
			if (( gObj[n].Type  == OBJECT_USER ) && ( gObj[n].Map == MAP_ICARUS) && ( gObj[n].X >= 130))
			{
				gNotice.GCNoticeSend(gObj[n].Index,1,0,0,0,0,0,szMsg);
			}
		}
	}
}

void cSkyEvent::ClearAllMonsters()	//OK
{		
	for(int iStage = 0; iStage < MAX_SKY_EVENT_STAGES; iStage++)
	{
		for(int iLevel = 0; iLevel < MAX_SKY_EVENT_LEVEL; iLevel++)
		{
			for(int MobCount=0; MobCount < this->MonsterStageCount[iLevel][iStage] ;MobCount++)
			{
				if (this->m_MonsterData[iLevel][iStage][MobCount].ID >= 0)
				{
					int aIndex = this->m_MonsterData[iLevel][iStage][MobCount].ID;
					
					if (gObj[aIndex].Class == this->m_MonsterData[iLevel][iStage][MobCount].Number &&
						SKYEVENT_MAP_RANGE(gObj[aIndex].Map,gObj[aIndex].X,gObj[aIndex].Y))
					{
						gObjDel(aIndex);
					}

					this->m_MonsterData[iLevel][iStage][MobCount].ID = -1;
				}
			}
		}
	}
}

void cSkyEvent::ClearMonsters(BYTE iLevel,BYTE iStage)	//OK
{
	if(this->Enabled == 1 && this->Start == 1)
	{
		for(int MobCount=0; MobCount < this->MonsterStageCount[iLevel][iStage] ;MobCount++)
		{
			if (this->m_MonsterData[iLevel][iStage][MobCount].ID >= 0)
			{
				int aIndex = this->m_MonsterData[iLevel][iStage][MobCount].ID;
				
				if (gObj[aIndex].Class == this->m_MonsterData[iLevel][iStage][MobCount].Number &&
					SKYEVENT_MAP_RANGE(gObj[aIndex].Map,gObj[aIndex].X,gObj[aIndex].Y))
				{
					gObjDel(aIndex);
				}

				this->m_MonsterData[iLevel][iStage][MobCount].ID = -1;
			}
		}
	}
}

void cSkyEvent::SetMonsters(BYTE iLevel,BYTE iStage)	//OK
{
	if(this->Enabled == 1 && this->Start == 1)
	{
		for(int MobCount=0; MobCount < this->MonsterStageCount[iLevel][iStage] ;MobCount++)
		{			
			int MobID = gObjAddMonster(gMonsterSetBase.m_Mp[this->m_MonsterData[iLevel][iStage][MobCount].Pos].Map);
			if(MobID >= 0)
			{
				this->m_MonsterData[iLevel][iStage][MobCount].ID = MobID;
				ObjYola.SetPosMonsterSkyEvent(MobID, this->m_MonsterData[iLevel][iStage][MobCount].Pos);
				gObjSetMonster(MobID, gMonsterSetBase.m_Mp[this->m_MonsterData[iLevel][iStage][MobCount].Pos].Type);
			}
		}
		
	}
}


void cSkyEvent::ReadMonsters(char * FilePath)	//OK
{
	MONSTER_SET_BASE_INFO info;

	if(this->Enabled == 1)
	{
		int Token;
		int iLevel;
		int MobNum;
		int X;
		int Y;
		int Count;

	if (FilePath == NULL || !strcmp(FilePath, ""))
	{
		ErrorMessageBox("[SkyEvent] file load error - File Name Error");
		return;
	}

	SMDFile = fopen(FilePath, "r");

	if (!SMDFile)
	{
		ErrorMessageBox("[SkyEvent] file load error - fopen In: %s",FilePath);
		return;
	}

		for(int Stages = 0; Stages < MAX_SKY_EVENT_STAGES; Stages++)
		{
			for(int Level = 0; Level < MAX_SKY_EVENT_LEVEL; Level++)
			{
				this->MonsterStageCount[Level][Stages] = 0;
			}
		}

		for(int iL = 0; iL < MAX_SKY_EVENT_LEVEL; iL++)
		{
			for (int iS = 0; iS < MAX_SKY_EVENT_STAGES; iS++)
			{
				for (int iM = 0; iM <MAX_SKY_EVENT_STAGE_MOB_COUNT; iM++)
				{
					this->m_MonsterData[iL][iS][iM].ID = -1;
					this->m_MonsterData[iL][iS][iM].Pos = -1;
					this->m_MonsterData[iL][iS][iM].Number = -1;
				}
			}
		}

		while ( true )
		{
			
			for(int iStages = 1; iStages < (MAX_SKY_EVENT_STAGES+1); iStages++)
			{
				int iType = GetToken();
				while(true)
				{
					Token = GetToken();
					if ( strcmp("end", TokenString) == 0 )
					{
						break;
					}
					iLevel = TokenNumber;

					Token = GetToken();
					MobNum = TokenNumber;

					Token = GetToken();
					X = TokenNumber;

					Token = GetToken();
					Y = TokenNumber;

					Token = GetToken();
					Count = TokenNumber;

					for(int i=0;i<Count;i++)
					{
						int MobCount = this->MonsterStageCount[(iLevel-1)][(iStages-1)];

						if (iLevel <= MAX_SKY_EVENT_LEVEL &&
							iStages <= MAX_SKY_EVENT_STAGES &&
							MobCount < MAX_SKY_EVENT_STAGE_MOB_COUNT)
						{
							int MobPos = ObjYola.gObjMonsterAdd(MobNum,10,X,Y); //(info.MonsterClass,10,X,Y);
							
							if(MobPos >= 0)
							{
								this->m_MonsterData[(iLevel-1)][(iStages-1)][MobCount].ID = -1;
								this->m_MonsterData[(iLevel-1)][(iStages-1)][MobCount].Pos = MobPos;
								this->m_MonsterData[(iLevel-1)][(iStages-1)][MobCount].Number = MobNum; //info.MonsterClass;
								this->MonsterStageCount[(iLevel-1)][(iStages-1)]++;

							} else {
								LogAdd(LOG_EVENT,"[SkyEvent] - Cant add monster to Lvl:%d, Stage:%d, Count:%d [ID:%d:,X:%d,Y:%d] Pos:%d",
									iLevel,iStages,MobCount,MobNum,X,Y,MobPos);
							}
						} else {
							LogAdd(LOG_EVENT,"[SkyEvent][Overflow] - Cant add monster to Lvl:%d, Stage:%d, Count:%d [ID:%d:,X:%d,Y:%d]",
								iLevel,iStages,MobCount,MobNum,X,Y);
						}
					}

					//LogAdd(LOG_RED, "RequestParty");
				}
			}
			break;
		}
			
		fclose(SMDFile);
		LogAdd(LOG_EVENT,"[SkyEvent] - %s file is Loaded",FilePath);
		
	}
}

void cSkyEvent::StartEvent()	//OK
{
	if((this->Start == 0) && (this->Enabled == 1))
	_beginthread( cSkyEvent__InsideTrigger, 0, NULL  );
}

void cSkyEvent__InsideTrigger(void * lpParam)	//OK
{
	gSkyEvent.Start = 1;
	
	gSkyEvent.NPCEnabled = TRUE;
	
	char sBuf[1024] = {0};
	
	gSkyEvent.CurrentStage = 0;
	
	BYTE ActiveLevelsSum = 0;

	for(int i = 5; i > 0; i--) //5
	{
		LogAdd(LOG_EVENT, "[Sky Event] will close after %d minute(s)", i);
		
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[Sky Event] will close after %d minute(s)", i);
		
		Sleep(60000);
	}

	gSkyEvent.NPCEnabled = FALSE;
	
	LogAdd(LOG_EVENT,"[Sky Event] Event entrance is closed!");
	
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[SkyEvent] Event entrance is Closed");
	
	for(int iStages = 0; iStages < MAX_SKY_EVENT_STAGES; iStages++)
	{
		gSkyEvent.CurrentStage++;
		
		ActiveLevelsSum = 0;
		
		for(int iLevel = 0; iLevel < MAX_SKY_EVENT_LEVEL; iLevel++)
		{
			int ActiveUsers = gSkyEvent.CheckUsersAlive(iLevel);
			
			if(ActiveUsers > 0)
			{
				if(iLevel > 0 && iStages > 0)
					gSkyEvent.ClearMonsters((iLevel-1),(iStages-1));	

				gSkyEvent.SetMonsters(iLevel,iStages);
				ActiveLevelsSum++;
			}
			else
			{			
				if(iLevel > 0 && iStages > 0)
					gSkyEvent.ClearMonsters((iLevel-1),(iStages-1));	
				gSkyEvent.ClearLevelData(iLevel);
			}
			
			gSkyEvent.SendUserExpInfo(iLevel);
		}

		gSkyEvent.MsgStringSend("[SkyEvent] Stage Experience has changed!",0x01);
		
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[Sky] Stage %d Time Duration: %d Minute(s)",iStages+1,gSkyEvent.StageMinutes[iStages]);

		if(ActiveLevelsSum > 0)
			Sleep(gSkyEvent.StageMinutes[iStages] * 60000);
		else
			break;
	}

	gSkyEvent.ClearAllMonsters();
	
	for(int iLevel = 0; iLevel < MAX_SKY_EVENT_LEVEL; iLevel++)
	{
		int ActiveUsers = gSkyEvent.CheckUsersAlive(iLevel);
		
		if(ActiveUsers > 0)
		{
			int WinUser = gSkyEvent.GetWinner(iLevel);

			if(WinUser > -1)
				gSkyEvent.DropWinnerItem(iLevel,WinUser,0,0);		
		}
	}

	LogAdd(LOG_EVENT,"[Sky Event] Event End");

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[Sky Event] Event End");

	for(int i = 2; i > 0; i--)
	{
		wsprintf(sBuf, "%d Seconds to warp to Devias", i*10);
		gSkyEvent.MsgStringSend(sBuf,0x01);
		Sleep(10000);
	}
	
	gSkyEvent.ExitPlayers();

	for(int iLevel = 0; iLevel < MAX_SKY_EVENT_LEVEL; iLevel++)
	{
		gSkyEvent.ClearLevelData(iLevel);
	}

	gSkyEvent.Start = 0;

	_endthread();
}

void cSkyEvent::ExitPlayers()	//OK
{
	for ( int n = OBJECT_START_USER ; n < MAX_OBJECT ; n++)
	{
		if ( gObj[n].Connected == OBJECT_ONLINE )
		{
			if (( gObj[n].Type  == OBJECT_USER ) && ( gObj[n].Map == MAP_ICARUS) && ( gObj[n].X >= 130))
			{
				this->Teleport(&gObj[n],255);
			}
		}
	}
}

void cSkyEvent::SendUserExpInfo(BYTE iLevel)	//OK
{
	for(int iUser = 0; iUser < MAX_SKY_EVENT_SUB_LEVEL; iUser++)
	{
		int Stage = MAX_SKY_EVENT_STAGES;

		int m_Index = this->m_UserData[iLevel][iUser].m_Index;
		
		if(m_Index != -1)
		{
			if ( gObj[m_Index].Connected >= OBJECT_CONNECTED )
			{
				if (this->m_UserData[iLevel][iUser].DBNumber == gObj[m_Index].DBNumber)
				{
					if((gObj[this->m_UserData[iLevel][iUser].m_Index].Map == MAP_ICARUS) && (gObj[this->m_UserData[iLevel][iUser].m_Index].X >= 130)) 
					{
						if(this->CurrentStage == 1)
						{
							gNotice.NewNoticeSend(this->m_UserData[iLevel][iUser].m_Index,0,0,0,0,0,"[SkyEvent] - ExtraExp: %d",this->ExtraExp[iLevel][0]);
						}
						if(this->CurrentStage == 2)
						{
							gNotice.NewNoticeSend(this->m_UserData[iLevel][iUser].m_Index,0,0,0,0,0,"[SkyEvent] - ExtraExp: %d",this->ExtraExp[iLevel][1]);
						}
						if(this->CurrentStage == 3)
						{
							gNotice.NewNoticeSend(this->m_UserData[iLevel][iUser].m_Index,0,0,0,0,0,"[SkyEvent] - ExtraExp: %d",this->ExtraExp[iLevel][2]);
						}
						
					}
				}
			}
		}
	}
}

int cSkyEvent::CheckUsersAlive(BYTE iLevel)	//OK
{
	int UserCount = 0;
	
	for(int iUser = 0; iUser < MAX_SKY_EVENT_SUB_LEVEL; iUser++)
	{
		int m_Index = this->m_UserData[iLevel][iUser].m_Index;
		
		if(m_Index != -1)
		{
			if ( gObj[m_Index].Connected >= OBJECT_CONNECTED )
			{
				if (this->m_UserData[iLevel][iUser].DBNumber != gObj[m_Index].DBNumber)
				{
					this->RemoveUser(iLevel,iUser);
				}else
				{
					if((gObj[this->m_UserData[iLevel][iUser].m_Index].Map == MAP_ICARUS) && (gObj[this->m_UserData[iLevel][iUser].m_Index].X >= 130)) 
					{
						UserCount++;
					}
					else 
					{
						this->RemoveUser(iLevel,iUser);
					}
				}
			}
			else
			{
				this->RemoveUser(iLevel,iUser);
			}
		}
	}
	return UserCount;
}

void cSkyEvent::RemoveUser(BYTE iLevel,int iUser)	//OK
{
	this->m_UserData[iLevel][iUser].m_Index = -1;
	
	this->m_UserData[iLevel][iUser].DBNumber = -1;
	
	for(int iStages = 0; iStages < MAX_SKY_EVENT_STAGES; iStages++)
	{
		this->m_UserData[iLevel][iUser].KillCount[iStages] = 0;		
	}
}

void cSkyEvent::ClearLevelData(BYTE iLevel)	//OK
{
	this->PlayerCount[iLevel] = 0;

	for(int iUser = 0; iUser < MAX_SKY_EVENT_SUB_LEVEL; iUser++)
	{
		this->m_UserData[iLevel][iUser].m_Index = -1;	
		
		this->m_UserData[iLevel][iUser].DBNumber = -1;			
		
		for(int iStages = 0; iStages < MAX_SKY_EVENT_STAGES; iStages++)
		{
			this->m_UserData[iLevel][iUser].KillCount[iStages] = 0;		
		}
	}
}

void cSkyEvent::Teleport(LPOBJ lpObj, BYTE iLevel)	//OK
{
	int rNum = rand()%5;
	switch(iLevel)
	{
		case 0:
		{
			gObjTeleport(lpObj->Index,10,148 + rNum,209 + rNum);	
		}break;

		case 1:
		{
			gObjTeleport(lpObj->Index,10,210 + rNum,209 + rNum);	
		}break;

		case 2:
		{
			gObjTeleport(lpObj->Index,10,147 + rNum,148 + rNum);	
		}break;
		
		case 3:
		{
			gObjTeleport(lpObj->Index,10,211 + rNum,148 + rNum);	
		}break;
		
		case 4:
		{
			gObjTeleport(lpObj->Index,10,211 + rNum,89 + rNum);	
		}break;
		case 255:
		{			
			gObjMoveGate(lpObj->Index, 22);
		}break;
	}
}

int cSkyEvent::CheckEnterLevel(int iIndex)
{

	if ( OBJMAX_RANGE(iIndex) == FALSE )
	{
		return -1;
	}

	if ( gObj[iIndex].Type != OBJECT_USER || gObj[iIndex].Connected <= OBJECT_LOGGED )
	{
		return -1;
	}

	int pLevel = gObj[iIndex].Level;

	for(int iLevel = 0; iLevel < MAX_SKY_EVENT_LEVEL; iLevel++)
	{
		if(pLevel >= Level_Min[iLevel] && pLevel <= Level_Max[iLevel])
			return iLevel;
	
		LogAdd(LOG_BLACK,"[SkyEvent] Level %d",iLevel+1);
	}
	return -1;
}

BOOL cSkyEvent::NpcTalk(LPOBJ lpNpc, LPOBJ lpObj)
{
	int iIndex = lpObj->Index;

	if((this->Enabled == 1) && (this->Start == 1) && (this->NPCEnabled == 1))
	{
		int iLevel = this->CheckEnterLevel(iIndex);
		
		if(iLevel > -1)
		{
			if(this->PlayerCount[iLevel] < MAX_SKY_EVENT_SUB_LEVEL)
			{
				if(ObjYola.DeleteItemByLevel(iIndex,0x1C33,iLevel+1) == 1)
				{
					lpObj->Interface.use = 0;
					
					this->AddUser(iLevel,iIndex,lpObj);
					
					this->Teleport(&gObj[iIndex],iLevel);
					
					return TRUE;
				}
				else
				{
					gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"You need %s + %d for enter into Sky Event Level %d!", ItemAttribute[0x1C33].Name, iLevel+1, iLevel+1);
				}
			}
			else
			{
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,"[Sky Event][%d] Room Full!", iLevel+1);

			}
		}
	}
	return FALSE;
}

void cSkyEvent::AddUser(BYTE iLevel,int iIndex,LPOBJ lpObj)	//OK
{
	int iUser = this->PlayerCount[iLevel];
	
	this->m_UserData[iLevel][iUser].m_Index = iIndex;	
	
	this->m_UserData[iLevel][iUser].DBNumber = gObj[iIndex].DBNumber;	
	
	for(int iStages = 0; iStages < MAX_SKY_EVENT_STAGES; iStages++)
	{
		this->m_UserData[iLevel][iUser].KillCount[iStages] = 0;		
	}
	
	this->PlayerCount[iLevel]++;
	
	gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"[SkyEvent] TotalUser %d",this->PlayerCount[iLevel]);		
	
	LogAdd(LOG_BLACK,"[SkyEvent] [Level %d] - TotalUser %d",iLevel+1,this->PlayerCount[iLevel]);
}
