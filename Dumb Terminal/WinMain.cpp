/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: WinMain.cpp - An application that will allow the user to send and receive characters through
-- the serial port and display it on the screen.
--
-- PROGRAM: DataCommAssignment01
--
-- FUNCTIONS:
-- int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
--						WIN32 generated method. Default params.
-- DWORD WINAPI ConnectModeThread(LPVOID n)
--						Pointer required when creating a thread. Param unused.
-- LRESULT CALLBACK WndProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--						WIN32 generated method. Default params.
--
-- DATE: September 25th, 2015
--
-- REVISIONS: N/A
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "ConnectMode.cpp"

//CommConfig object that gets filled in when the user choses the settings.
COMMCONFIG commConfig = COMMCONFIG();

//Default settings. 2400bps, no parity, 8 databits and 1 leading bit.
string commSettings = "24, N, 8, 1";

//Comm channel name, defaulted to com1.
string commName = "com1";

//Reference to the ConnectMode object, which will handle reading/writing.
ConnectMode connectMode;

//Refernce to the windows handle.
HWND hwnd;

//Reference to the threads handle that takes care of reading. Essentially the tread that initiates
//and runs ConnectMode.
HANDLE hThread;

//WndProc prototype, 
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ConnectModeThread(LPVOID n);

#pragma warning (disable: 4096)
#pragma warning (disable: 4996)

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: September 25th, 2015
--
-- REVISIONS: N/A
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
--							WIN32 generated params.
--
-- RETURNS: int for whether the program fails or terminates successfully. If fail, the int will
--			be a non-zero value determining the error code.
--
-- NOTES:
-- Starting point of the application. Used to setup the window and then trigger the ConnectMode thread which will
-- establish the connection to the serial port and handle reading/writing to the serial port.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance,
 						  LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;

	Wcl.cbSize = sizeof (WNDCLASSEX);
	Wcl.style = 0;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	Wcl.hIconSm = NULL;
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	
	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = "Dumb Terminal";
	
	Wcl.lpszMenuName = TEXT("MYMENU");
	Wcl.cbClsExtra = 0;
	Wcl.cbWndExtra = 0; 
	
	if (!RegisterClassEx (&Wcl))
		return 0;

	hwnd = CreateWindow (Wcl.lpszClassName, Wcl.lpszClassName, WS_OVERLAPPEDWINDOW, 10, 10, 600, 400, NULL, NULL, hInst, NULL);
	ShowWindow (hwnd, nCmdShow);
	UpdateWindow (hwnd);

	connectMode = ConnectMode(hwnd);
	commConfig.dwSize = sizeof(commConfig);
	CommConfigDialog(commName.c_str(), hwnd, &commConfig);
	DWORD threadID;
	hThread = CreateThread(NULL, 0, &ConnectModeThread, 0, 0, &threadID);

	while (GetMessage (&Msg, NULL, 0, 0))
	{
   		TranslateMessage (&Msg);
		DispatchMessage (&Msg);
	}

	return Msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ConnectModeThread
--
-- DATE: September 25th, 2015
--
-- REVISIONS: N/A
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: DWORD WINAPI ConnectModeThread(LPVOID n)
--							void pointer required when creating a thread. Unused.
--
-- RETURNS: DWORD.
--
-- NOTES:
-- Function called when creating the ConnectMode thread. This method makes the connectMode object establish
-- its connection to the serial port and then send it into a loop to read from the port.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI ConnectModeThread(LPVOID n) {
	connectMode.ReaderLoop(commName, commSettings, commConfig);
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: September 25th, 2015
--
-- REVISIONS: N/A
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- INTERFACE: LRESULT CALLBACK WndProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--								WIN32 generated method and params.
--
-- RETURNS: LRESULT.
--
-- NOTES:
-- Function that is used to take the user input from typing, convert that char into a string and send that to
-- the connectMode object to handle sending the data.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
                          WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static unsigned k = 0;
	DWORD threadID;

	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDM_CON:
				TerminateThread(hThread, 0);
				hThread = CreateThread(NULL, 0, &ConnectModeThread, 0, 0, &threadID);
				break;
			case IDM_SET:
				TerminateThread(hThread, 0);
				commConfig.dwSize = sizeof(commConfig);
				CommConfigDialog(commName.c_str(), hwnd, &commConfig);
				hThread = CreateThread(NULL, 0, &ConnectModeThread, 0, 0, &threadID);
				break;
			case IDM_HELP:
				MessageBox(NULL, 
					"This application is used to send characters from one device to another via serial ports.\n\n"
					"To start, click Connection at the top left and pick settings.\n"
					"Select your bits per second, parity, data bits, start bits and any flow control.\n"
					"Make sure that the machine on the other end of the connection has the same settings as you."
					"To reconnect using the same settings as before, click Connection at the top left and sellect (Re)connect.", "Terminal Emulator Help", NULL);
				break;
		}
		break;
		case WM_CHAR:
			hdc = GetDC(hwnd);
			connectMode.SendString((char)wParam);
			ReleaseDC(hwnd, hdc);
			break;		
		break;
		
		default:
			return DefWindowProc (hwnd, Message, wParam, lParam);
	}
	return 0;
}