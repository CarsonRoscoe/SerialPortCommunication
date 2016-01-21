#include "InputSender.cpp"
#include "Headers.h"

using namespace std;

/*------------------------------------------------------------------------------------------------------------------
-- CLASS NAME: ConnectMode
--
-- DESCRIPTION: Object used to establish a connection with the serial port, read from the serial port and
-- write to it when invoked.
--
-- METHODs:
-- ConnectMode(HWND hw)
--				Handle to the window, used when drawing characters to the screen.
-- void EstablishConnection()
-- void ReaderLoop(string commname, string commsettings, COMMCONFIG commconfig)
--				String for the serial port comm name. Most likely "com1", the default.
--				CommSettings string for the serial port default values. Defaulted to 2400bps, no parity, 8 databits, 1 start bit.
--				CommConfig object data was put into if the user specifically changed the values. If this is set, nullafies commSettings' use.
-- void SendString(char message)
--				Char received from WndProc to be sent to InputSender for writing to serial port.
-- void drawChar(char c)
--				Char read from serial port to be drawn onto the screen.
-- 
-- DATE: September 25th, 2015
--
-- REVISIONS: N/A
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES: ConnectMode is an object used to establish a connection with the serial port, read from it, write to
-- it and draws read characters to the window as they come in.
----------------------------------------------------------------------------------------------------------------------*/
class ConnectMode
{
	//Handle to the serial port connection.
	HANDLE handle = NULL;

	//Amount of characters that can be received at a time.
	DWORD bufferSizeReceive;

	//Amount of characters that can be sent at a time.
	DWORD bufferSizeTransmit;

	//Handle to the window
	HWND hwnd;

	//x position offset to draw the text onto the screen at.
	int x;

	//y position offset to draw the text onto the screen at.
	int y;

	//InputSender object which will handle writing to the serial port once the connectMode passes
	//it the message.
	InputSender sender = InputSender();

	//The message to write to the serial port.
	string message = "";

	//Config file for comm, created before the ConnectMode object is created.
	COMMCONFIG commConfig;

	//String for the comm name, e.g. "com1"
	string commName;

	//String for the comm settings in string format. This determines the BPS, parity bits, etc.
	string commSettings;

	//Bool over whether the program will loop indefinitely reading.
	bool doReaderLoop = true;

public:
	//Default constructor.
	ConnectMode() {}

	/*------------------------------------------------------------------------------------------------------------------
	-- METHOD: ConnectMode Constructor
	--
	-- DATE: September 25th, 2015
	--
	-- REVISIONS: N/A
	--
	-- DESIGNER: Carson Roscoe
	--
	-- PROGRAMMER: Carson Roscoe
	--
	-- INTERFACE: ConnectMode(HWND hw)
	--				Handle to the window to draw to.
	--
	-- NOTES:
	-- This constructor is used to instantiate the ConnectMode object and pass in the handle to the applications UI Window.
	-- It sets the offset x and y to be 1 each and sets the charsCounted to zero. This will put the drawing location of
	-- the first characters at the top left.
	----------------------------------------------------------------------------------------------------------------------*/
	ConnectMode(HWND hw) {
		hwnd = hw;
		x = 1;
		y = 1;
	}

	//Destructor, shuts off infinite loop before deleting itself.
	~ConnectMode() {
		doReaderLoop = false;
	}
	
	/*------------------------------------------------------------------------------------------------------------------
	-- METHOD: EstablishConnection
	--
	-- DATE: September 25th, 2015
	--
	-- REVISIONS: N/A
	--
	-- DESIGNER: Carson Roscoe
	--
	-- PROGRAMMER: Carson Roscoe
	--
	-- INTERFACE: void EstablishConnection()
	--
	-- RETURNS: void.
	--
	-- NOTES:
	-- This method creates the connection to the serial port with the settings set by the menu. If no settings have
	-- been set, this method is still called, just with default parameters.
	----------------------------------------------------------------------------------------------------------------------*/
	void EstablishConnection() {
		LPCSTR port = commName.c_str();
		DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
		DWORD dwShareMode = 0;
		LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
		DWORD dwCreationDistribution = OPEN_EXISTING;
		DWORD dwFlagsAndAttributes = FILE_FLAG_OVERLAPPED;
		HANDLE hTemplateFile = NULL;
		LPCSTR settings = commSettings.c_str();
		DCB dcb = DCB();
		COMMPROP commprop = COMMPROP();
		bufferSizeReceive = 1;
		bufferSizeTransmit = 1;

		//Creates the file connection to the serial port and returns the handle to it.
		if (handle == NULL) {
			handle = CreateFile(port, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);

			if (handle == INVALID_HANDLE_VALUE) {
				OutputDebugStringA("CreateFile handle was invalid.");
				DWORD error = GetLastError();
				return;
			}

			SetupComm(handle, bufferSizeReceive, bufferSizeTransmit);
		}

		if (!GetCommProperties(handle, &commprop)){
			OutputDebugStringA("GetCommProperties did not succeed.");
			DWORD error = GetLastError();
			return;
		}

		if (!BuildCommDCB(settings, &dcb)) {
			OutputDebugStringA("BuildCommDCB did not succeed.");
			DWORD error = GetLastError();
			return;
		}

		SetCommConfig(handle, &commConfig, commConfig.dwSize);
	}

	/*------------------------------------------------------------------------------------------------------------------
	-- METHOD: ReaderLoop
	--
	-- DATE: September 25th, 2015
	--
	-- REVISIONS:
	--		October 6th, 2015: Had a bug where sometimes it was "skipping" characters being read in. Rewrote it and now it works.
	--
	-- DESIGNER: Carson Roscoe
	--
	-- PROGRAMMER: Carson Roscoe
	--
	-- INTERFACE: void ReaderLoop(string commname, string commsettings, COMMCONFIG commconfig)
	--				String for the serial port comm name. Most likely "com1", the default.
	--				CommSettings string for the serial port default values. Defaulted to 2400bps, no parity, 8 databits, 1 start bit.
	--				CommConfig object data was put into if the user specifically changed the values. If this is set, nullafies commSettings' use.
	--
	-- RETURNS: void.
	--
	-- NOTES:
	-- Calls the EstablishConnection method to establish a connection, and then goes into an infite loop
	-- reading from the serial port. Once it receives information it hands it char by char to the drawChar
	-- method to draw it to the screen.
	----------------------------------------------------------------------------------------------------------------------*/
	void ReaderLoop(string commname, string commsettings, COMMCONFIG commconfig) {
		x = 1;
		y = 1;
		commConfig = commconfig;
		commName = commname;
		commSettings = commsettings;
		bool read = false;
		EstablishConnection();
		unsigned char buffer[1] = { 0 };
		OVERLAPPED osReader = { 0 };
		osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (osReader.hEvent == NULL) {
			OutputDebugStringA("osReader's hEvent failed to be created.");
			return;
		}
		while (doReaderLoop) {
			if (read) {
				if (WaitForSingleObject(osReader.hEvent, 250) == WAIT_OBJECT_0) {
					drawChar(buffer[0]);
					read = false;
				}
			} else {
				if (!ReadFile(handle, buffer, bufferSizeReceive, 0, &osReader)) {
					if (GetLastError() == ERROR_IO_PENDING) {
						read = !read;
					}
				}
			}
		}
	}

	/*------------------------------------------------------------------------------------------------------------------
	-- METHOD: SendString
	--
	-- DATE: September 25th, 2015
	--
	-- REVISIONS: N/A
	--
	-- DESIGNER: Carson Roscoe
	--
	-- PROGRAMMER: Carson Roscoe
	--
	-- INTERFACE: void SendString(char message)
	--					Character to be written to the serial port. Called by WndProc, sends to InputSender.
	--
	-- RETURNS: void.
	--
	-- NOTES:
	-- Hands a string being passed in as a parameter to the InputSender object for writing the string to the serial
	-- port.
	----------------------------------------------------------------------------------------------------------------------*/
	void SendString(char message) {
		sender.SendInput(message, handle);
	}

	/*------------------------------------------------------------------------------------------------------------------
	-- METHOD: drawChar
	--
	-- DATE: September 25th, 2015
	--
	-- REVISIONS: N/A
	--
	-- DESIGNER: Carson Roscoe
	--
	-- PROGRAMMER: Carson Roscoe
	--
	-- INTERFACE: void drawChar(char c)
	--					Char to be drawn to the screen, read from the serial port.
	--
	-- RETURNS: void.
	--
	-- NOTES:
	-- Takes in a char to be draw to the screen and then draws it to the appropriate position. It keeps track of all
	-- chars drawn on the screen in a string so that, when the user backspaces, we can get the exact width of whichever
	-- character we are deleting.
	----------------------------------------------------------------------------------------------------------------------*/
	void drawChar(char c) {
		HDC hdc = GetDC(hwnd); // get device context
		SIZE size;

		if (c == 8) { //If the char is the backspace character.
			string str(4, ' ');
			string oldChar(1, message.length() > 0 ? message[message.length() - 1] : ' ');
			GetTextExtentPoint32(hdc, (LPCSTR)oldChar.c_str(), oldChar.length(), &size);
			if (x - size.cx <= 0) {
				x = 0;
			}
			else {
				x -= size.cx;
			}
			TextOut(hdc, 1 + x, 1 + y, (LPCSTR)str.c_str(), str.length());
			if (message.length() > 0)
				message.pop_back();
			ReleaseDC(hwnd, hdc); // Release device context
			return;
		}
		else {
			message += c;
		}
		
		string str(1, c);
		GetTextExtentPoint32(hdc, (LPCSTR)str.c_str(), 1, &size);
		TextOut(hdc, 1 + x, 1 + y, (LPCSTR)str.c_str(), 1);
		x += size.cx;
		if (c == '\r') { //if the char is a new line character.
			y += size.cy;
			x = 1;
		}/*
		RECT winRect = RECT();
		GetWindowRect(hwnd, &winRect);
		if (x >= (winRect.right - winRect.left)) {
			x = 1;
			y += size.cy;
		}*/
		ReleaseDC(hwnd, hdc); // Release device context
	}
};
