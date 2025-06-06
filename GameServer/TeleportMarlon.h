#include "stdafx.h"
#define MAX_TELEPORT_MARLON 10

struct CUSTOM_TELEPORT_MARLON
{
	int  m_index;
	int  m_Mapa;
	int  m_CoordenadaX;
	int  m_CoordenadaY;
	int	 m_Dir;
};

class cCustomTeleportMarlon
{
public:
	int m_count;
	void Load(char* path);
	CUSTOM_TELEPORT_MARLON* GetTeleportMarlon(int index);
	CUSTOM_TELEPORT_MARLON m_Marlon[MAX_TELEPORT_MARLON];
private:

};
extern cCustomTeleportMarlon gCustomMarlon;