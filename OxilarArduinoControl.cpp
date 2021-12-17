/* Oxilar Arduino Control
   ----------------------
*/


/*

*/

#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

XPLMDataRef gDataRef = NULL;
HANDLE hComm;
DCB dcb;
COMMTIMEOUTS cto;
BOOL fSuccess;
BOOL fReadFileSuccess;
char ReadBuffer[20] = { 0 };


float ArduinoControl(float inElapsedSinceLastCall, float  inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc)
{
	// Plugin Info
	strcpy(outName, "OxilarArduinoControl");
	strcpy(outSig,  "Oxilar.Arduino.Control");
	strcpy(outDesc, "This example illustrates creating and sending a custom command to X-Plane using a generic trigger.");
		
	gDataRef = XPLMFindDataRef("sim/cockpit/electrical/battery_on");
	
	if (gDataRef != NULL) {

		hComm = CreateFileA("\\\\.\\COM3",// Port name
			GENERIC_READ | GENERIC_WRITE, // Read/Write
			0,                            // No Sharing
			NULL,                         // No Security
			OPEN_EXISTING,                // Open existing port only
			0,                            // Non Overlapped I/O
			NULL);                        // Null for Comm Devices

		if (hComm != INVALID_HANDLE_VALUE)
		{
			SecureZeroMemory(&dcb, sizeof(DCB));
			dcb.DCBlength = sizeof(DCB);
			dcb.BaudRate = CBR_115200;
			dcb.ByteSize = 8;
			dcb.Parity = NOPARITY;
			dcb.StopBits = ONESTOPBIT;

			SecureZeroMemory(&cto, sizeof(COMMTIMEOUTS));			
			cto.ReadIntervalTimeout = 1;
			cto.ReadTotalTimeoutConstant = 1;

			fSuccess = SetCommTimeouts(hComm, &cto);
			
			fSuccess = SetCommState(hComm, &dcb);
		}
	}

	XPLMRegisterFlightLoopCallback(ArduinoControl, 0.0, NULL);
	XPLMSetFlightLoopCallbackInterval(ArduinoControl, 0.01, 1, NULL);	
	return 1;
}

PLUGIN_API void  XPluginStop(void)
{
	CloseHandle(hComm);	
	XPLMUnregisterFlightLoopCallback(ArduinoControl, NULL);
}


PLUGIN_API void XPluginDisable(void) {}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void * inParam) {}


float ArduinoControl(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon)
{
	if (fSuccess && gDataRef != NULL)
	{
		DWORD NumBytesRead = 0;
		char* buttonVal = NULL;
		int value = -1;
		
		fReadFileSuccess = ReadFile(hComm, ReadBuffer, sizeof(ReadBuffer) - 1, &NumBytesRead, NULL);		
		buttonVal = &ReadBuffer[17];
		value = (int)*buttonVal - 48;			
		XPLMSetDatai(gDataRef, value);		
		PurgeComm(hComm, PURGE_RXCLEAR);
	}
	return 0.1;
}
