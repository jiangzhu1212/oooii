// $(header)

#include <oooii/oWindows.h> // Move all this code to somewhere that isn't windows so we don't have to include this.

#include <oooii/oKeyboard.h>
#include <oooii/oMouse.h>
#include <oooii/oWindow.h>

#include <oGfx/oGfx.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	oRef<oWindow> Window;
	{
		// @oooii-tony: If X is specified, but Y is intended to be default,
		// then bad things happen. We should find out why.
		oWindow::DESC d;
		d.ClientX = 1000;
		d.ClientY = 200;
		d.ClientWidth = 800;
		d.ClientHeight = 600;
		d.Style = oWindow::FIXED;
		d.UseAntialiasing = true;
		oVERIFY(oWindow::Create(&d, nullptr, "oGfxTest", 0, &Window));
	}

	oRef<threadsafe oKeyboard> Keyboard;
	oVERIFY(oKeyboard::Create(Window->GetNativeHandle(), true, &Keyboard));

	oRef<threadsafe oMouse> Mouse;
	{
		oMouse::DESC d;
		d.ShortCircuitEvents = false;
		oVERIFY(oMouse::Create(d, Window->GetNativeHandle(), &Mouse));
	}

	Mouse->SetCursorState(oMouse::NORMAL);

	oRef<threadsafe oGfxDevice> GfxDevice;
	{
		oGfxDevice::DESC d;
		d.EnableDebugReporting = true;
		d.UseSoftwareEmulation = false;
		d.Version = 10.0f;
		oVERIFY(oGfxCreateDevice(d, &GfxDevice));
	}

	oRef<oGfxRenderTarget2> RenderTarget;
	oVERIFY(GfxDevice->CreateRenderTarget2("Output", Window, oSurface::R32_TYPELESS, &RenderTarget));

	while (Window->IsOpen())
	{
		Window->Begin();
		Window->End();
	}
	
	return 0;
}