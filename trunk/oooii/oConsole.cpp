// $(header)
#include <oooii/oConsole.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oSingleton.h>
#include <oooii/oStddef.h>
#include <oooii/oMutex.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>

struct oConsoleContext : public oProcessSingleton<oConsoleContext>
{
	struct Run { Run() { oConsoleContext::Singleton(); } };

	oConsoleContext()
	{
		extern void SetConsoleOOOiiIcon();
		SetConsoleOOOiiIcon();
	}

	oMutex ConsoleLock;
};

static oConsoleContext::Run sInstantiateConsoleContext; // @oooii-tony: safe static, just meant to make sure singleton is instantiated

static void GetColor(WORD _wAttributes, oColor* _pForeground, oColor* _pBackground)
{
	{
		float r = 0.0f, g = 0.0f, b = 0.0f;
		bool intense = !!(_wAttributes & FOREGROUND_INTENSITY);
		if (_wAttributes & FOREGROUND_RED) r = intense ? 1.0f : 0.5f;
		if (_wAttributes & FOREGROUND_GREEN) g = intense ? 1.0f : 0.5f;
		if (_wAttributes & FOREGROUND_BLUE) b = intense ? 1.0f : 0.5f;
		*_pForeground = oComposeColor(r, g, b, 1.0f);
	}
		
	{
		float r = 0.0f, g = 0.0f, b = 0.0f;
		bool intense = !!(_wAttributes & BACKGROUND_INTENSITY);
		if (_wAttributes & BACKGROUND_RED) r = intense ? 1.0f : 0.5f;
		if (_wAttributes & BACKGROUND_GREEN) g = intense ? 1.0f : 0.5f;
		if (_wAttributes & BACKGROUND_BLUE) b = intense ? 1.0f : 0.5f;
		*_pBackground = oComposeColor(r, g, b, 1.0f);
	}
}

static WORD FindNearestColor(oColor _Color, bool _Foreground)
{
	if (!_Color) return 0xffff;
	WORD flags = 0;

	float h,s,v;
	oDecomposeToHSV(_Color, &h, &s, &v);
	if (v > 0.5f) flags |= (_Foreground ? FOREGROUND_INTENSITY : BACKGROUND_INTENSITY);
	float r,g,b,a;
	oDecomposeColor(_Color, &r, &g, &b, &a);
	if (r > 0.5f) flags |= _Foreground ? FOREGROUND_RED : BACKGROUND_RED;
	if (g > 0.5f) flags |= _Foreground ? FOREGROUND_GREEN : BACKGROUND_GREEN;
	if (b > 0.5f) flags |= _Foreground ? FOREGROUND_BLUE : BACKGROUND_BLUE;
	return flags;
}

#define FOREGROUND_MASK (FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)
#define BACKGROUND_MASK (BACKGROUND_INTENSITY|BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE)

// returns prior wAttributes
static WORD SetConsoleColor(HANDLE _hStream, oColor _Foreground, oColor _Background)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(_hStream, &csbi);
	WORD wOriginalAttributes = csbi.wAttributes;
	WORD wAttributes = csbi.wAttributes & ~(FOREGROUND_MASK|BACKGROUND_MASK);
	wAttributes |= (_Foreground ? FindNearestColor(_Foreground, true) : csbi.wAttributes & FOREGROUND_MASK);
	wAttributes |= (_Background ? FindNearestColor(_Background, false) : csbi.wAttributes & BACKGROUND_MASK);
	SetConsoleTextAttribute(_hStream, wAttributes);
	return wOriginalAttributes;
}

static bool FindParentProcessID(DWORD _ProcessID, DWORD _ParentProcessID, const char* _ProcessExePath, unsigned int _SearchProcessID, unsigned int* _pOutSearchParentProcessID)
{
	if (_SearchProcessID == _ProcessID)
	{
		*_pOutSearchParentProcessID = _ParentProcessID;
		return false;
	}

	return true;
}

unsigned int oProcessGetParentID(unsigned int _ProcessID)
{
	unsigned int ppid = 0;
	oProcessEnum(oBIND(FindParentProcessID, oBIND1, oBIND2, oBIND3, _ProcessID, &ppid));
	return ppid;
}

void* oConsole::GetNativeHandle()
{
	return GetConsoleWindow();
}

void oConsole::GetPixelWidthHeight(unsigned int* _pPixelWidth, unsigned int* _pPixelHeight)
{
	RECT r;
	GetWindowRect(GetConsoleWindow(), &r);
	*_pPixelWidth = r.right - r.left;
	*_pPixelHeight = r.bottom - r.top;
}

void oConsole::GetDesc(DESC* _pDesc)
{
	oMutex::ScopedLock lock(oConsoleContext::Singleton()->ConsoleLock);
	CONSOLE_SCREEN_BUFFER_INFO info;
	oASSERT(GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE, "");

	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
	{
		if (GetLastError() == ERROR_INVALID_HANDLE && oProcessGetParentID(oProcessGetCurrentID()))
		{
			oSetLastError(EPERM, "Failed to access console because this is a child process.");
			return;
		}
	}

	_pDesc->BufferWidth = info.dwSize.X;
	_pDesc->BufferHeight = info.dwSize.Y;
	_pDesc->Left = info.srWindow.Left;
	_pDesc->Top = info.srWindow.Top;
	_pDesc->Width = info.srWindow.Right - info.srWindow.Left;
	_pDesc->Height = info.srWindow.Bottom - info.srWindow.Top;
	GetColor(info.wAttributes, &_pDesc->Foreground, &_pDesc->Background);
	_pDesc->Show = !!IsWindowVisible(GetConsoleWindow());
}

void oConsole::SetDesc(const DESC* _pDesc)
{
	oMutex::ScopedLock lock(oConsoleContext::Singleton()->ConsoleLock);
	DESC desc;
	GetDesc(&desc);
	#define DEF(x) if (_pDesc->x != DEFAULT) desc.x = _pDesc->x
	DEF(BufferWidth);
	DEF(BufferHeight);
	DEF(Left);
	DEF(Top);
	DEF(Width);
	DEF(Height);
	desc.Foreground = _pDesc->Foreground;
	desc.Background = _pDesc->Background;
	#undef DEF

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD c;
	c.X = (SHORT)desc.BufferWidth;
	c.Y = (SHORT)desc.BufferHeight;
	if (!SetConsoleScreenBufferSize(hConsole, c))
	{
		if (GetLastError() == ERROR_INVALID_HANDLE && oProcessGetParentID(oProcessGetCurrentID()))
		{
			oSetLastError(EPERM, "Failed to access console because this is a child process.");
			return;
		}
	}

	// @oooii-tony: I couldn't get this to behave properly.
	SMALL_RECT r;
	r.Left = 0;
	r.Top = 0;
	r.Right = (SHORT)(r.Left + desc.Width);
	r.Bottom = (SHORT)(r.Top + desc.Height);
	oVB(SetConsoleWindowInfo(hConsole, TRUE, &r));
	UINT show = desc.Show ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
	oVB(SetWindowPos(GetConsoleWindow(), HWND_TOP, desc.Left, desc.Top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|show));
	SetConsoleColor(hConsole, _pDesc->Foreground, _pDesc->Background);
}

void oConsole::SetTitle(const char* _Title)
{
	oVB(SetConsoleTitle(_Title));
}

void oConsole::GetTitle(char* _strDestination, size_t _SizeofStrDestination)
{
	oVB(GetConsoleTitle(_strDestination, static_cast<DWORD>(_SizeofStrDestination)));
}

void oConsole::SetCursorPosition(unsigned short _X, unsigned short _Y)
{
	COORD c;
	if (_X == DEFAULT || _Y == DEFAULT)
	{
		unsigned short curX, curY;
		GetCursorPosition(&curX, &curY);
		if (_X == DEFAULT) _X = curX;
		if (_Y == DEFAULT) _Y = curY;
	}

	c.X = _X; c.Y = _Y;
	oVB(SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c));
}

void oConsole::GetCursorPosition(unsigned short* _pX, unsigned short* _pY)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	*_pX = csbi.dwCursorPosition.X;
	*_pY = csbi.dwCursorPosition.Y;
}

bool oConsole::HasFocus()
{
	return oHasFocus(GetConsoleWindow());
}

int oConsole::vfprintf(FILE* _pStream, oColor _Foreground, oColor _Background, const char* _Format, va_list _Args)
{
	HANDLE hConsole = 0;
	WORD wOriginalAttributes = 0;

	oMutex::ScopedLock lock(oConsoleContext::Singleton()->ConsoleLock);

	if (_pStream == stdout || _pStream == stderr)
	{
		hConsole = GetStdHandle(_pStream == stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
		wOriginalAttributes = SetConsoleColor(hConsole, _Foreground, _Background);
	}

	int n = ::vfprintf(_pStream, _Format, _Args);

	if (hConsole)
		SetConsoleTextAttribute(hConsole, wOriginalAttributes);

	return n;
}
