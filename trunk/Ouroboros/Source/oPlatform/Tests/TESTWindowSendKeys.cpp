/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oPlatform/oWindow.h>
#include <oPlatform/Windows/oWinKey.h>
#include <oConcurrency/event.h>

#ifdef CreateProcess
	#undef CreateProcess
#endif
static const char* TESTMessage = "Hello world";
struct PLATFORM_WindowSendKeysClient : public oSpecialTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oConcurrency::event WaitEvent;
		oWINDOW_INIT init;
		init.Title = "PLATFORM_WindowSendKeysClientWindow";
		init.Shape.State = oGUI_WINDOW_RESTORED;
		init.Shape.Style = oGUI_WINDOW_SIZABLE;
		init.Shape.ClientSize = int2(16,16);

		bool CapsLock = GetKeyState(VK_CAPITAL) == 0 ? false : true;
		bool Shift = false;
		ouro::sstring Result;

		init.ActionHook = [&](const oGUI_ACTION_DESC& _Action)
		{
			if(_Action.Action == oGUI_ACTION_KEY_DOWN )
			{
				oGUI_KEY Key = _Action.Key;
				if(oGUI_KEY_CAPSLOCK == _Action.Key)
				{
					CapsLock = !CapsLock;
				}
				else if(oGUI_KEY_LSHIFT == _Action.Key || oGUI_KEY_LSHIFT == _Action.Key)
				{
					Shift = true;
				}
				else
				{
					char ASCIIKey = (char)tolower((char)Key);
					bool Capped = Shift ^ CapsLock;
					if(Capped)
						ASCIIKey = (char)toupper(ASCIIKey);

					ouro::sncatf(Result.c_str(), "%c", ASCIIKey);
					if(Result.length() == strlen(TESTMessage))
						WaitEvent.set();
				}
			}
			if(_Action.Action == oGUI_ACTION_KEY_UP && (oGUI_KEY_LSHIFT == _Action.Key || oGUI_KEY_LSHIFT == _Action.Key))
			{
				Shift = false;
			}
		};

		ouro::intrusive_ptr<oWindow> TestWindow;
		oTESTB0( oWindowCreate(init, &TestWindow) );

		NotifyReady();
		WaitEvent.wait_for(oStd::chrono::milliseconds(5000));
		oTESTB(0 == strcmp(Result.c_str(), TESTMessage), "Didn't correct receive test message");
		return SUCCESS;
	}
};

struct PLATFORM_WindowSendKeys : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		ouro::process::id ID = ouro::process::get_id("notepad.exe");

		std::shared_ptr<ouro::process> Client;
		{
			int exitcode = 0;
			ouro::lstring msg;
			oTESTB(oSpecialTest::CreateProcess("PLATFORM_WindowSendKeysClient", &Client), "");
			oTESTB(oSpecialTest::Start(Client.get(), msg, msg.capacity(), &exitcode), "%s", msg);
		}

		HWND Hwnd;
		unsigned int ThreadID;
		oWinGetProcessTopWindowAndThread(Client->get_id(), &Hwnd, &ThreadID);
		oWinSendASCIIMessage(Hwnd, ThreadID, TESTMessage);

		oTESTB0(Client->wait_for(oSeconds(10)));

		int ClientExitCode;
		oTESTB(Client->exit_code(&ClientExitCode), "Could not get exit code for Client process");

  		return SUCCESS;
	}
};


oTEST_REGISTER(PLATFORM_WindowSendKeys);
oTEST_REGISTER(PLATFORM_WindowSendKeysClient);