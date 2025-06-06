// MoveSummon.h: interface for the CMoveSummon class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct MOVE_PKFREE_INFO
{
	int Map;
	int X;
	int Y;
	int TX;
	int TY;
	int PKFree;
};

class cCPKFree
{
public:
	cCPKFree();
	virtual ~cCPKFree();
	void Load(char* path);
	bool CheckPKZona(int map,int x,int y);
	bool CheckPKFree(int map,int x,int y);
private:
	std::vector<MOVE_PKFREE_INFO> m_PKFreeInfo;
};

extern cCPKFree gPKFree;