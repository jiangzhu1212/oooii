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

#include <oooii/oWindows.h> // Move all this code to somewhere that isn't windows so we don't have to include this.

#include <oooii/oKeyboard.h>
#include <oooii/oMouse.h>
#include <oooii/oWindow.h>

#include <oGfx/oGfx.h>

struct oApp
{
	oApp();
	void Run();

protected:
	void OnResize(oWindow::RECT_EVENT _Event, oWindow::STATE _State, oRECT _Rect);

	oRef<oWindow> Window;
	oRef<threadsafe oKeyboard> Keyboard;
	oRef<threadsafe oMouse> Mouse;
	oRef<threadsafe oGfxDevice> Device;
	oRef<threadsafe oWindow::Resizer> Resizer;
	oRef<oGfxRenderTarget2> RenderTarget;
};

oApp::oApp()
{
	{
		// @oooii-tony: If X is specified, but Y is intended to be default,
		// then bad things happen. We should find out why.
		oWindow::DESC d;
		d.ClientX = 1000;
		d.ClientY = 200;
		d.ClientWidth = 800;
		d.ClientHeight = 600;
		d.Style = oWindow::SIZEABLE;
		d.UseAntialiasing = true;
		oVERIFY(oWindow::Create(&d, nullptr, "oGfxTest", 0, &Window));
	}

	oVERIFY(oKeyboard::Create(Window->GetNativeHandle(), true, &Keyboard));

	{
		oMouse::DESC d;
		d.ShortCircuitEvents = false;
		oVERIFY(oMouse::Create(d, Window->GetNativeHandle(), &Mouse));
	}

	{
		oGfxDevice::DESC d;
		d.EnableDebugReporting = true;
		d.UseSoftwareEmulation = false;
		d.Version = 10.0f;
		oVERIFY(oGfxCreateDevice(d, &Device));
	}

	oVERIFY(Window->CreateResizer(oBIND(&oApp::OnResize, this, oBIND1, oBIND2, oBIND3), &Resizer));

	//oVERIFY(Device->CreateRenderTarget2("Output", Window, oSurface::R32_TYPELESS, &RenderTarget));

	Mouse->SetCursorState(oMouse::NORMAL);
}

void oApp::OnResize(oWindow::RECT_EVENT _Event, oWindow::STATE _State, oRECT _Rect)
{
	if (_State == oWindow::RESTORED || _State == oWindow::FULL_SCREEN)
	{
		if (_Event == oWindow::RESIZE_OCCURING)
		{
			RenderTarget = nullptr;
		}

		else if (_Event == oWindow::RECT_END)
		{
			oVERIFY(Device->CreateRenderTarget2("Output", Window, oSurface::R32_TYPELESS, &RenderTarget));
		}
	}
}

void oApp::Run()
{
	while (Window->IsOpen())
	{
		Window->Begin();
		Window->End();
	}
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	oApp app;
	app.Run();
	return 0;
}