// $(header)

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