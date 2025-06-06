#pragma once

struct cItemLevel
{
	int index;
	int ItemIndex;
	int ItemLevel;
	char ItemName[32];
};

class cCItemLevel
{
public:
	cCItemLevel();
	~cCItemLevel();
	void Load(char* path);
	char * GetItemName(int ItemIndex, int Level);
public:
	std::map<int, cItemLevel> m_ItemData;
}; extern cCItemLevel gItemLevel;