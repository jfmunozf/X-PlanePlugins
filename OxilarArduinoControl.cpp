/* Oxilar Arduino Control
   ----------------------
*/


/*
*/

// SDK de X-Plane
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
// SDK Windows: WinAPI
#include <windows.h>

// Variable para almacenar el dataref al swiche de la batería
XPLMDataRef gDataRef = NULL;
// Handle del puerto serial
HANDLE hComm;
// Estructura para estabkecer parámetros del puerto serial
DCB dcb;
// Estructura para establecer parámetros de timeouts de lectura del pto serial
COMMTIMEOUTS cto;
// Para almacenar el valor retornado por el llamado a la función
// que establece estructura DCB
BOOL fSuccessDCB;
// Para almacenar el valor retornado por el llamado a la función
// que establece estructura CTO: COMMTIMEOUTS
BOOL fSuccessCTO;
// Para almacenar el valor retornado del llamado a la función ReadFile()
BOOL fReadFileSuccess;
// Buffer de lectura del puerto serial
char ReadBuffer[20] = { 0 };

// La función CALLBACK en donde se lee el puerto serial (USB) 
// que conecta al arduino
float ArduinoControl(float inElapsedSinceLastCall, float  inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

// Inicialización del plugin
PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	// Plugin Info
	strcpy(outName, "OxilarArduinoControl");
	strcpy(outSig, "Oxilar.Arduino.Control");
	strcpy(outDesc, "Plugin para controlar la bateria desde un swiche ON/OFF con un arduino");

	// Dataref del swiche de la batería
	gDataRef = XPLMFindDataRef("sim/cockpit/electrical/battery_on");

	if (gDataRef != NULL) {

		// Apertura del puerto serial COM3 que pone el arduino
		hComm = CreateFileA("\\\\.\\COM3",// Port name
			GENERIC_READ,                 // Read/Write
			0,                            // No Sharing
			NULL,                         // No Security
			OPEN_EXISTING,                // Open existing port only
			0,                            // Non Overlapped I/O
			NULL);                        // Null for Comm Devices

		if (hComm != INVALID_HANDLE_VALUE)
		{
			// Establecimiento de parámettros del puerto serial
			SecureZeroMemory(&dcb, sizeof(DCB));
			dcb.DCBlength = sizeof(DCB);
			dcb.BaudRate = CBR_115200;
			dcb.ByteSize = 8;
			dcb.Parity = NOPARITY;
			dcb.StopBits = ONESTOPBIT;
			fSuccessDCB = SetCommState(hComm, &dcb);

			// Establecimiento de parámetros de timeouts de lectura del 
			// puerto serial
			SecureZeroMemory(&cto, sizeof(COMMTIMEOUTS));
			cto.ReadIntervalTimeout = 1;
			cto.ReadTotalTimeoutConstant = 1;
			fSuccessCTO = SetCommTimeouts(hComm, &cto);			
		}
	}

	// Registro de la función CALLBACK para controlar el swiche de
	// la bateria desde el arduino
	XPLMRegisterFlightLoopCallback(ArduinoControl, 0.0, NULL);
	XPLMSetFlightLoopCallbackInterval(ArduinoControl, 0.01, 1, NULL);
	return 1;
}

PLUGIN_API void XPluginStop(void)
{
	// Cerrar el handle del puerto serial cuando el Plugin se detenga
	CloseHandle(hComm);

	// Eliminar el registro de la función CALLBACK ArduinoControl
	XPLMUnregisterFlightLoopCallback(ArduinoControl, NULL);
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginDisable(void) {}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void* inParam) {}

// Función CALLBACK que ejecuta X-Plane en cada ciclo de vuelo
float ArduinoControl(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon)
{
	// Se verifica que el establecimiento del puerto se haya completado
	// sastifactoariamente: DCB, COMMTIMEOUTS y dataref existente
	if (fSuccessDCB && fSuccessCTO && gDataRef != NULL)
	{
		DWORD NumBytesRead = 0;
		char* buttonVal = NULL;
		int value = -1;
		
		// Lectura del puerto serial
		fReadFileSuccess = ReadFile(hComm, ReadBuffer, sizeof(ReadBuffer) - 1, &NumBytesRead, NULL);

		// Obtener del buffer el caracter 17: 0 ó 1
		buttonVal = &ReadBuffer[17];

		// Convertir el caracter 17 a entero
		value = (int)*buttonVal - 48;

		// Establecer el valor del dataref de acuerdo con el valor del swiche del arduino: 0 ó 1
		XPLMSetDatai(gDataRef, value);

		// Limpiar buffer
		PurgeComm(hComm, PURGE_RXCLEAR);
	}
	return 0.1;
}
