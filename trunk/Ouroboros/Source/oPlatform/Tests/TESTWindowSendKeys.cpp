/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include <oPlatform/oTest.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oPlatform/oWindow.h>

static const char* TESTMessage = "Hello world";
struct TESTWindowSendKeysClient : public oSpecialTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oEvent WaitEvent;
		oWINDOW_INIT init;
		init.WindowThreadDebugName = "TESTWindowSendKeysClient";
		init.WindowTitle = "TESTWindowSendKeysClientWindow";

		init.WinDesc.Style = oGUI_WINDOW_SIZEABLE;
		init.WinDesc.ClientSize = int2(16,16);
		init.WinDesc.Debug = true;

		bool CapsLock = GetKeyState(VK_CAPITAL) == 0 ? false : true;
		bool Shift = false;
		oStringS Result;

		init.ActionHook = [&](const oGUI_ACTION_DESC& _Action)
		{
			if(_Action.Action == oGUI_ACTION_KEY_DOWN )
			{
				oKEYBOARD_KEY Key = _Action.Key;
				if(oKB_Caps_Lock == _Action.Key)
				{
					CapsLock = !CapsLock;
				}
				else if(oKB_Shift_L == _Action.Key || oKB_Shift_L == _Action.Key)
				{
					Shift = true;
				}
				else
				{
					char ASCIIKey = (char)tolower((char)Key);
					bool Capped = Shift ^ CapsLock;
					if(Capped)
						ASCIIKey = (char)toupper(ASCIIKey);

					oStrAppendf(Result.c_str(), "%c", ASCIIKey);
					if(Result.length() == strlen(TESTMessage))
						WaitEvent.Set();
				}
			}
			if(_Action.Action == oGUI_ACTION_KEY_UP && (oKB_Shift_L == _Action.Key || oKB_Shift_L == _Action.Key))
			{
				Shift = false;
			}
		};

		oRef<threadsafe oWindow> TestWindow;
		oTESTB0( oWindowCreate(init, &TestWindow) );

		NotifyReady();
		WaitEvent.Wait(5000);
		oTESTB(0 == oStrcmp(Result.c_str(), TESTMessage), "Didn't correct receive test message");
		return SUCCESS;
	}
};

struct TESTWindowSendKeys : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		unsigned int ID = oProcessGetID("notepad.exe");

		oRef<threadsafe oProcess> Client;
		{
			int exitcode = 0;
			oStringL msg;
			oTESTB(oSpecialTest::CreateProcess("TESTWindowSendKeysClient", &Client), "");
			oTESTB(oSpecialTest::Start(Client, msg, msg.capacity(), &exitcode), "%s", msg);
		}

		HWND Hwnd;
		unsigned int ThreadID;
		oWinGetProcessTopWindowAndThread(Client->GetProcessID(), &Hwnd, &ThreadID);
		oWinSendASCIIMessage(Hwnd, ThreadID, TESTMessage);

		oTESTB0(Client->Wait());

		int ClientExitCode;
		oTESTB(Client->GetExitCode(&ClientExitCode), "Could not get exit code for Client process");

  		return SUCCESS;
	}
};


oTEST_REGISTER(TESTWindowSendKeys);
oTEST_REGISTER(TESTWindowSendKeysClient);