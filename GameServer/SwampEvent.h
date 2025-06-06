
#if _MSC_VER > 1000
#pragma once
#endif 

#include "user.h"

#define SWAMP_MOBSTOKILLCOUNT		255
#define SWAMP_MAXGROUPS				255
#define SWAMP_STANBYSECONDS			10
#define SWAMP_ROOMS					4

struct PMSG_TIMER_IMPERIALGUARDIAN
{
	PBMSG_HEAD h;	// C1:F7:04
	BYTE subcode;	// 3
	BYTE State;	// 4 //
	BYTE unk1;	// 5
	BYTE unk2;	// 5
	BYTE unk3;	// 5
	BYTE unk4;	// 5
	BYTE Time1;	// 5
	BYTE Time2;	// 5
	BYTE Time3;	// 5
	BYTE MonstersLeft;
};

struct SwampRoomSettings
{
	int Groups;
	int BossID;
	char Territory[32];
	BYTE Active;
};

enum State
{	
	SWAMP_STATE_BLANK = 0,
	SWAMP_STATE_EMPTY = 1,
	SWAMP_STATE_START = 2,
};

struct SWAMP_EVENT_TIME
{
    int Year;
	int Month;
	int Day;
	int DayOfWeek;
   	int Hour;
	int Minute;
	int Second;

};

struct SWAMP_EVENTMOB_DATA
{
	short Number;
	BYTE Room;
	BYTE Group;
	int XF;
	int YF;
	int XT;
	int YT;
	BYTE Count;
};

class cSwampEvent
{
public:
	cSwampEvent()
	{
		InitializeCriticalSection(&this->m_critEventData);
	};
	virtual ~cSwampEvent();
	void Clean();
	void Init();
	void MainProc();
	void ProcState_BLANK();
	void ProcState_EMPTY();
	void ProcState_START();
	void SetState(int state);
	void SetState_BLANK();
	void SetState_EMPTY();
	void SetState_START();
	void CheckSync();
	void StartEvent();
	void ReadMonsters(char * FilePath);
	void LoadDataConfig(char * FilePath);
	bool GateMove(int aIndex, int Gate);
	void MonsterDie(LPOBJ lpMon, int pIndex);
	void Timer(int Seconds, BYTE State);
	void DataSendInside(LPBYTE pMsg, int size);
	void ClearAllMonsters();
	void SetMonsters(int iAssultType, int iGroup);
	void WarpOutside();	
	
	BOOL Enabled;
	BOOL Start;
	BYTE OnlyMastersEnter;
	BYTE SwampMapEnterOnlyWhenStarted;
	short SwampIfWonKeepOpenMapMinutes;

	BYTE CurrentRoom;
	BYTE CurrentGroup;
	BYTE MobsStageGroupCount;
	BYTE MobsKilled;

	short SwampWarpGate;
	short SwampGateRangeStart;
	short SwampGateRangeEnd;

	BYTE GroupTimer[SWAMP_MAXGROUPS];
	SwampRoomSettings pRoom[SWAMP_ROOMS];

	std::vector<SWAMP_EVENTMOB_DATA> m_SwampEventNewMonster;
public:
	int m_iState;
	int m_RemainTime;
	int m_TargetTime;
	int m_TickCount;
	int m_TimeNotify;
	
	std::map<int, std::vector<SWAMP_EVENTMOB_DATA> > m_swampMonsterInfo;
	
	std::vector<SWAMP_EVENT_TIME> SwampEventTime;
	CRITICAL_SECTION m_critEventData;
};

void cSwampEvent__InsideTrigger(void * lpParam);

extern cSwampEvent gSwampEvent;

