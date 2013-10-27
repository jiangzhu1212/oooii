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
#include <oPlatform/oConsole.h>
#include <oBase/assert.h>
#include <oBasis/oError.h>
#include <oBase/fixed_string.h>
#include <oConcurrency/mutex.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSingleton.h>
#include <oGUI/Windows/oGDI.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oPlatform/oStream.h>

using namespace ouro;

// TODO: Add GetConsoleMode support

struct oConsoleContext : public oProcessSingleton<oConsoleContext>
{
	struct Run { Run() { oProcessHeapEnsureRunning(); oConsoleContext::Singleton(); } };

	oConsoleContext()
		: CtrlHandlerSet(false)
	{}

	BOOL CtrlHandler(DWORD fdwCtrlType);

	static const oGUID GUID;
	oConcurrency::recursive_mutex ConsoleLock;
	bool CtrlHandlerSet;
	oConsole::EventFn Functions[5];
	path LogFilePath;
	intrusive_ptr<threadsafe oStreamWriter> LogFile;
};

// {145728A4-3A9A-47FD-BF88-8B61A1EC14AB}
const oGUID oConsoleContext::GUID = { 0x145728a4, 0x3a9a, 0x47fd, { 0xbf, 0x88, 0x8b, 0x61, 0xa1, 0xec, 0x14, 0xab } };
oSINGLETON_REGISTER(oConsoleContext);

static oConsoleContext::Run sInstantiateConsoleContext; // @tony: safe static, just meant to make sure singleton is instantiated

static BOOL WINAPI StaticCtrlHandler(DWORD fdwCtrlType)
{
	return oConsoleContext::Singleton()->CtrlHandler(fdwCtrlType);
}

BOOL oConsoleContext::CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
		case CTRL_C_EVENT: if (Functions[oConsole::CTRLC]) return Functions[oConsole::CTRLC]();
		case CTRL_BREAK_EVENT: if (Functions[oConsole::CTRLBREAK]) return Functions[oConsole::CTRLBREAK]();
		case CTRL_CLOSE_EVENT: if (Functions[oConsole::CLOSE]) return Functions[oConsole::CLOSE]();
		case CTRL_LOGOFF_EVENT: if (Functions[oConsole::LOGOFF]) return Functions[oConsole::LOGOFF]();
		case CTRL_SHUTDOWN_EVENT: if (Functions[oConsole::SHUTDOWN]) return Functions[oConsole::SHUTDOWN]();
		default: break;
	}
	
	return FALSE;
}

static void GetColor(WORD _wAttributes, color* _pForeground, color* _pBackground)
{
	{
		float r = 0.0f, g = 0.0f, b = 0.0f;
		bool intense = !!(_wAttributes & FOREGROUND_INTENSITY);
		if (_wAttributes & FOREGROUND_RED) r = intense ? 1.0f : 0.5f;
		if (_wAttributes & FOREGROUND_GREEN) g = intense ? 1.0f : 0.5f;
		if (_wAttributes & FOREGROUND_BLUE) b = intense ? 1.0f : 0.5f;
		*_pForeground = color(r, g, b, 1.0f);
	}
		
	{
		float r = 0.0f, g = 0.0f, b = 0.0f;
		bool intense = !!(_wAttributes & BACKGROUND_INTENSITY);
		if (_wAttributes & BACKGROUND_RED) r = intense ? 1.0f : 0.5f;
		if (_wAttributes & BACKGROUND_GREEN) g = intense ? 1.0f : 0.5f;
		if (_wAttributes & BACKGROUND_BLUE) b = intense ? 1.0f : 0.5f;
		*_pBackground = color(r, g, b, 1.0f);
	}
}

#define FOREGROUND_GRAY (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)
#define BACKGROUND_GRAY (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE)
#define FOREGROUND_MASK (FOREGROUND_INTENSITY|FOREGROUND_GRAY)
#define BACKGROUND_MASK (BACKGROUND_INTENSITY|BACKGROUND_GRAY)

// returns prior wAttributes
static WORD SetConsoleColor(HANDLE _hStream, color _Foreground, color _Background)
{
	#define RED__ FOREGROUND_RED|BACKGROUND_RED
	#define GREEN__ FOREGROUND_GREEN|BACKGROUND_GREEN
	#define BLUE__ FOREGROUND_BLUE|BACKGROUND_BLUE
	#define BRIGHT__ FOREGROUND_INTENSITY|BACKGROUND_INTENSITY

	static const color sConsoleColors[] = { Black, Navy, Green, Teal, Maroon, Purple, Olive, Silver, Gray, Blue, Lime, Aqua, Red, Fuchsia, Yellow, White };
	static const WORD sConsoleColorWords[] = { 0, BLUE__, GREEN__, BLUE__|GREEN__, RED__, RED__|BLUE__, RED__|GREEN__, RED__|GREEN__|BLUE__, BRIGHT__, BRIGHT__|BLUE__, BRIGHT__|GREEN__, BRIGHT__|BLUE__|GREEN__, BRIGHT__|RED__, BRIGHT__|RED__|BLUE__, BRIGHT__|RED__|GREEN__, BRIGHT__|RED__|GREEN__|BLUE__ };

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(_hStream, &csbi);
	WORD wOriginalAttributes = csbi.wAttributes;
	WORD wAttributes = csbi.wAttributes & ~(FOREGROUND_MASK|BACKGROUND_MASK);
	
	if (!_Foreground)
		wAttributes |= wOriginalAttributes & FOREGROUND_MASK;
	else
		wAttributes |= sConsoleColorWords[palettize(_Foreground, sConsoleColors)] & FOREGROUND_MASK;

	if (!_Background)
		wAttributes |= wOriginalAttributes & BACKGROUND_MASK;
	else
		wAttributes |= sConsoleColorWords[palettize(_Background, sConsoleColors)] & BACKGROUND_MASK;
	
	SetConsoleTextAttribute(_hStream, wAttributes);
	return wOriginalAttributes;
}

void* oConsole::GetNativeHandle()
{
	return GetConsoleWindow();
}

int2 oConsole::GetSizeInPixels()
{
	RECT r;
	GetWindowRect(GetConsoleWindow(), &r);
	return oWinRectSize(r);
}

int2 oConsole::GetSizeInCharacters()
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	oVB(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info));
	return int2(info.dwSize.X, info.dwSize.Y);
}

void oConsole::GetDesc(DESC* _pDesc)
{
	oConsoleContext* c = oConsoleContext::Singleton();

	// Set LogFilePath first, in case the standard pipes are captured
	// GetConsoleScreenBufferInfo may error out because there is no window.
	_pDesc->LogFilePath = c->LogFilePath;

	oConcurrency::lock_guard<oConcurrency::recursive_mutex> lock(c->ConsoleLock);
	CONSOLE_SCREEN_BUFFER_INFO info;
	oASSERT(GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE, "");

	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
	{
		if (GetLastError() == ERROR_INVALID_HANDLE && ouro::this_process::is_child())
		{
			oErrorSetLast(std::errc::permission_denied, "Failed to access console because this is a child process.");
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
	oConsoleContext* c = oConsoleContext::Singleton();
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> lock(c->ConsoleLock);

	// Set LogFilePath first, in case the standard pipes are captured
	// GetConsoleScreenBufferInfo may error out because there is no window.
	path_string OldLogPath = c->LogFilePath;
	if (!_pDesc->LogFilePath.empty())
	{
		c->LogFilePath = _pDesc->LogFilePath;
		if (0 != _stricmp(OldLogPath, c->LogFilePath))
		{
			c->LogFile = nullptr;
			if (!oStreamLogWriterCreate(c->LogFilePath, &c->LogFile))
			{
				oTRACE("WARNING: Failed to open log file \"%s\"\n%s: %s", c->LogFilePath.c_str(), oErrorAsString(oErrorGetLast()), oErrorGetLastString());
				c->LogFilePath.clear();
			}
		}
	}
	else
		c->LogFile = nullptr;

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
	COORD bufferDimension;
	bufferDimension.X = (SHORT)desc.BufferWidth;
	bufferDimension.Y = (SHORT)desc.BufferHeight;
	if (!SetConsoleScreenBufferSize(hConsole, bufferDimension))
	{
		if (GetLastError() == ERROR_INVALID_HANDLE && ouro::this_process::is_child())
		{
			oErrorSetLast(std::errc::permission_denied, "Failed to access console because this is a child process.");
			return;
		}
	}

	SMALL_RECT r;
	r.Left = 0;
	r.Top = 0;
	r.Right = (SHORT)(r.Left + desc.Width);
	r.Bottom = (SHORT)(r.Top + desc.Height);

	// Clamp to max size.
	CONSOLE_SCREEN_BUFFER_INFO info;
	oVB(GetConsoleScreenBufferInfo(hConsole, &info));
	if (r.Right >= info.dwMaximumWindowSize.X)
	{
		r.Right = info.dwMaximumWindowSize.X-1;
		if (bufferDimension.X <= desc.Width)
			oTRACE("Clamping console width (%d) to system max of %d due to a specified buffer width (%d) smaller than screen width", desc.Width, r.Right, bufferDimension.X);
		else
			oTRACE("Clamping console width (%d) to system max of %d", desc.Width, r.Right);
	}

	if (r.Bottom >= info.dwMaximumWindowSize.Y)
	{
		r.Bottom = info.dwMaximumWindowSize.Y-4; // take a bit more off for the taskbar
		if (bufferDimension.Y <= desc.Height)
			oTRACE("Clamping console height (%d) to system max of %d due to a specified buffer width (%d) smaller than screen width", desc.Height, r.Bottom, bufferDimension.Y);
		else
			oTRACE("Clamping console height (%d) to system max of %d", desc.Height, r.Bottom);
	}

	oVB(SetConsoleWindowInfo(hConsole, TRUE, &r));
	UINT show = desc.Show ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
	oVB(SetWindowPos(GetConsoleWindow(), HWND_TOP, desc.Left, desc.Top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|show));
	SetConsoleColor(hConsole, _pDesc->Foreground, _pDesc->Background);
}

void oConsole::SetTitle(const char* _Title)
{
	oVB(SetConsoleTitle(_Title));
}

char* oConsole::GetTitle(char* _StrDestination, size_t _SizeofStrDestination)
{
	if (!GetConsoleTitle(_StrDestination, static_cast<DWORD>(_SizeofStrDestination)))
		return nullptr;
	return _StrDestination;
}

void oConsole::SetCursorPosition(const int2& _Position)
{
	COORD c = { oUShort(_Position.x), oUShort(_Position.y) };
	if (c.X == DEFAULT || c.Y == DEFAULT)
	{
		int2 p = GetCursorPosition();
		if (c.X == DEFAULT) c.X = oUShort(p.x);
		if (c.Y == DEFAULT) c.Y = oUShort(p.y);
	}

	oVB(SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c));
}

int2 oConsole::GetCursorPosition()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return int2(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y);
}

void oConsole::Clear()
{
	::system("cls");
}

bool oConsole::HasFocus()
{
	return oWinHasFocus(GetConsoleWindow());
}

int oConsole::vfprintf(FILE* _pStream, color _Foreground, color _Background, const char* _Format, va_list _Args)
{
	HANDLE hConsole = 0;
	WORD wOriginalAttributes = 0;

	oConsoleContext* c = oConsoleContext::Singleton();
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> lock(c->ConsoleLock);

	if (_pStream == stdout || _pStream == stderr)
	{
		hConsole = GetStdHandle(_pStream == stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
		wOriginalAttributes = SetConsoleColor(hConsole, _Foreground, _Background);
	}

	char msg[oKB(8)];
	vsnprintf(msg, _Format, _Args);

	// Always print any message to _pStream
	int n = ::fprintf(_pStream, msg);

	// And to log file
	if (c->LogFile)
	{
		oSTREAM_WRITE w;
		w.pData = msg;
		w.Range = oSTREAM_RANGE(oSTREAM_APPEND, strlen(msg));
		c->LogFile->Write(w);
	}

	if (hConsole)
		SetConsoleTextAttribute(hConsole, wOriginalAttributes);

	return n;
}

void oConsole::HookEvent(EVENT _Event, EventFn _Function)
{
	oConsoleContext* c = oConsoleContext::Singleton();
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> lock(c->ConsoleLock);

	c->Functions[_Event] = _Function;

	if (!c->CtrlHandlerSet)
	{
		oVB(SetConsoleCtrlHandler(StaticCtrlHandler, TRUE));
		c->CtrlHandlerSet = true;
	}
}