// $(header)

#include <oooii/oWindows.h>

#include <oooii/oMsgBox.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	oMsgBox::printf(oMsgBox::INFO, "oGfxText", "Hello World!");

	return 0;
}