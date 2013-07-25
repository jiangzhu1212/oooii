/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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

#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oStream.h>
#include <oPlatform/oWindow.h>
#include "resource.h"

void oWindowShow(threadsafe oWindow* _pWindow)
{
	oGUI_WINDOW_DESC* pDesc = nullptr;
	_pWindow->Map(&pDesc);
	pDesc->State = oGUI_WINDOW_RESTORED;
	_pWindow->Unmap();
}

class oWindowTestApp
{
public:
	oWindowTestApp();

	void Run()
	{
		// @oooii-tony: Why sleep the thread? Why not do what
		// all the cool kids do and block, using this thread to
		// process events. Consider changing the API:
		// WaitUntilClosed() -> RunEventLoop().
		if (TopLevelWindow)
			TopLevelWindow->WaitUntilClosed();
	}

private:
	oGDIScopedObject<HFONT> hFont;
	oRef<threadsafe oWindow> TopLevelWindow;
	oRef<threadsafe oStreamMonitor> StreamMonitor;

	bool EventHook(const oGUI_EVENT_DESC& _Event);
	void ActionHook(const oGUI_ACTION_DESC& _Action);

	void OnFileChange(oSTREAM_EVENT _Event, const oStd::uri_string& _ChangedURI);
};

oWindowTestApp::oWindowTestApp()
{
	oWINDOW_INIT init;

	// hide while everything is initialized
	init.WinDesc.hIcon = (oGUI_ICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, 0);
	init.WinDesc.State = oGUI_WINDOW_HIDDEN;
	init.WinDesc.Style = oGUI_WINDOW_FIXED;
	init.WinDesc.ClientSize = int2(256, 256);

	// Client area will mainly be a render target or some full-space occupying 
	// control, so prevent a lot of flashing by nooping this window's erase.
	init.WinDesc.DefaultEraseBackground = true;
	init.WinDesc.AllowAltEnter = false;
	init.WinDesc.EnableMainLoopEvent = false;
	init.WinDesc.ShowMenu = false;
	init.WinDesc.ShowStatusBar = false;
	init.WinDesc.Debug = false;
	init.WinDesc.AllowClientDragToMove = false;

	init.EventHook = oBIND(&oWindowTestApp::EventHook, this, oBIND1);
	init.ActionHook = oBIND(&oWindowTestApp::ActionHook, this, oBIND1);

	oVERIFY(oWindowCreate(init, &TopLevelWindow));

	TopLevelWindow->SetTitle("oWindowTestApp");

	oStd::uri_string dev_uri;
	oVERIFY(oSystemGetURI(dev_uri, oSYSPATH_DEV));
	
	{
		oSTREAM_MONITOR_DESC smd;
		smd.Monitor = dev_uri;
		oStd::sncatf(smd.Monitor, "Ouroboros/Source/oWindowTestApp/*.xml");
		smd.TraceEvents = false;
		smd.WatchSubtree = false;
		oVERIFY(oStreamMonitorCreate(smd, oBIND(&oWindowTestApp::OnFileChange, this, oBIND1, oBIND2), &StreamMonitor));
	}

	oWindowShow(TopLevelWindow);
}

bool oWindowTestApp::EventHook(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Event)
	{
		case oGUI_SIZED:
		{
			float2 Ratio = float2(_Event.ClientSize) / float2(_Event.ScreenSize);
			float R = max(Ratio);
			oGUI_FONT_DESC fd;
			fd.PointSize = oInt(round(R * 35.0f));
			hFont = oGDICreateFont(fd);
			break;
		}

		case oGUI_INPUT_DEVICE_CHANGED:
		{
			const oGUI_INPUT_DEVICE_EVENT_DESC& e = static_cast<const oGUI_INPUT_DEVICE_EVENT_DESC&>(_Event);
			oTRACE("device change: %s %s %s", oStd::as_string(e.Type), e.InstanceName.c_str(), oStd::as_string(e.Status));
			break;
		}

		case oGUI_TIMER:
		{
			const oGUI_TIMER_EVENT_DESC& e = static_cast<const oGUI_TIMER_EVENT_DESC&>(_Event);
			//clear any status text
			break;
		}

		default:
			break;
	}

	return true;
}

void oWindowTestApp::ActionHook(const oGUI_ACTION_DESC& _Action)
{
	switch (_Action.Action)
	{
		case oGUI_ACTION_CONTROL_ACTIVATED:
		{
			break;
		}

		case oGUI_ACTION_KEY_DOWN:
		case oGUI_ACTION_KEY_UP:
		{
			//oTRACE("%s: %s", oStd::as_string(_Action.Action), oStd::as_string(_Action.Key));
			break;
		}

		default:
			break;
	}
}

void oWindowTestApp::OnFileChange(oSTREAM_EVENT _Event, const oStd::uri_string& _ChangedURI)
{
	if (_Event == oSTREAM_ACCESSIBLE)
		oTRACE("accessible: %s", _ChangedURI.c_str());
}

oMAINA()
{
	oWindowTestApp App;
	App.Run();
	return 0;
}
