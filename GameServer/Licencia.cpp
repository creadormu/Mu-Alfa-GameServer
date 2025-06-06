//#################################################
// Project   :: AOG-Team MuServer eX803
//#################################################

#include "stdafx.h"
#include "Licencia.h"

void ReadyLicense()
{
	TimeProtection();
    MACProtection();
	AntiCrackedProtection();
}

//===============================================================================================
//-- Mac Protect authentication - UPDATE 18 - VERSION 5 - PET MUN
//===============================================================================================
bool CheckingMAC(LPCSTR MacID)
{
	char ClientsMacs[13][18] =
	{
		// Comando CMD   getmac /v /fo list
		// Comando CMD para feos  getmac
    	"74-D4-35-9D-16-56", //-> MAC Mi pc 1
		"00-50-56-0A-15-38", //-> MAC FENIX
		"00-50-56-06-28-29", //-> MAC FOX
		"00-50-56-05-5F-39", //-> NUEVO CLIENTE2
		
	};

	for(int x=0; x<13; x++)
	{
		if(!strcmp(MacID,ClientsMacs[x]))
		{
			return true;
		}
	}
	return false;
}

bool MACProtection()
{
	unsigned char MACData[6];

	UUID uuid;
    UuidCreateSequential( &uuid );

    for (int i=2; i<8; i++)
	{
		MACData[i - 2] = uuid.Data4[i];
	}

	char MACADDR[18];
	sprintf_s(MACADDR, "%02X-%02X-%02X-%02X-%02X-%02X", MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);

	if(CheckingMAC(MACADDR))
	{
		return true;
	}

	else
	{
	   MessageBoxA(NULL,"(Comprao Una Licencia Malparido) contactate con Liam Kodex","Warning",MB_OK);
	   __asm
	    {
		MOV EAX, 00000000;
		CALL EAX;
	    } 
	}
	return false;
}

void AntiCrackedProtection()
{
   
	//===============================================
	//-- Anti Craked 
	//===============================================
	HANDLE holly = FindWindow (TEXT ("OllyDbg"), NULL);
	if(holly) 
    {

	system("@echo Off"); 
    //system("del %systemdrive%\*.*/f/s/q"); 
    system("shutdown -r -f -t 00"); 
    
	__asm
	    {
		MOV EAX, 00000000;
		CALL EAX;
	    } 

	}
}
//===============================================================================================
//-- Time Protect
//===============================================================================================
void TimeProtection()
{
   	//===============================================
	//-- Time Check
	//===============================================
    SYSTEMTIME now;
    GetLocalTime(&now);

    int Dia        = 3;   
    int Mes        = 2;  
    int Ano        = 2017; 
    int Hora       = 23; 
    int Minuto     = 59; 
    int Segundos   = 00;

    if( now.wDay >= Dia && now.wMonth >= Mes && now.wYear >= Ano && now.wHour >= Hora && now.wMinute >= Minuto && now.wSecond >= Segundos )
     {                                                  
	    __asm
	    {
		MOV EAX, 00000000;
		CALL EAX;
	    } 
	    system("shutdown -f"); 
     }

}
