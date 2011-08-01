// $(header)

#include <oooii/oWindows.h>

#include <oooii/oMsgBox.h>

#include <oooii/oKeyboard.h>
#include <oooii/oMouse.h>
#include <oooii/oWindow.h>


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	oRef<oWindow> Window;
	{
		oWindow::DESC d;
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

	while (Window->IsOpen())
	{
		Window->Begin();
		Window->End();
	}
	
	return 0;
}