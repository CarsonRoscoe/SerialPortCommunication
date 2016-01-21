#include "Headers.h"

/*------------------------------------------------------------------------------------------------------------------
-- CLASS NAME: InputSender
--
-- DESCRIPTION: Object used to write data to the serial port.
--
-- METHODs:
-- void SendInput(char message, HANDLE handle)
--					char received from WndProc to be sent to the serial port.
--					handle to the serial port.
--
-- DATE: September 25th, 2015
--
-- REVISIONS: N/A
--
-- DESIGNER: Carson Roscoe
--
-- PROGRAMMER: Carson Roscoe
--
-- NOTES: ConnectMode is the only object that holds the InputSender object. This object relies on ConnectMode to
-- hand it the string message and the handle of the serial port connection.
--
----------------------------------------------------------------------------------------------------------------------*/
class InputSender
{
public:
	//Default Constructor
	InputSender() {}
	//Default Destructor
	~InputSender() {}

	/*------------------------------------------------------------------------------------------------------------------
	-- METHOD: SendInput
	--
	-- DATE: September 25th, 2015
	--
	-- REVISIONS: N/A
	--
	-- DESIGNER: Carson Roscoe
	--
	-- PROGRAMMER: Carson Roscoe
	--
	-- INTERFACE: void SendInput(char message, HANDLE handle)
	--								char received from WndProc to be sent to the serial port.
	--								handle to the serial port.
	--
	-- RETURNS: void.
	--
	-- NOTES:
	-- Takes in the string to send and a handle to the serial port connection to write to.
	-- It takes these and fills a buffer with the message characters and then writes the message.
	----------------------------------------------------------------------------------------------------------------------*/
	void SendInput(char message, HANDLE handle) {
		unsigned char buffer[1] = { 0 };
		buffer[0] = message;
		//for (size_t i = 0; i < message.length(); i++)
		//	buffer[i] = message[i];
		//OVERLAPPED overlapped = OVERLAPPED();
		BOOL success = WriteFile(handle, buffer, sizeof(char), 0, &OVERLAPPED());
	}
};
