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
#include <oStd/mutex.h>

#include <oGUI/Windows/oWinWindowing.h>
#include <oGUI/Windows/oWinRect.h>

namespace ouro {
	namespace console {
static void get_color(WORD _wAttributes, color* _pForeground, color* _pBackground)
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
static WORD set_console_color(HANDLE _hStream, color _Foreground, color _Background)
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

class context
{
public:
	context()
		: CtrlHandlerSet(false)
		, hLog(nullptr)
	{}

	~context()
	{
		if (hLog)
			filesystem::close(hLog);
	}

	static context& singleton();

	oGUI_WINDOW native_handle() const { return (oGUI_WINDOW)GetConsoleWindow(); }
  info get_info() const;
  void set_info(const info& _Info);
	void set_title(const char* _Title) { oVB(SetConsoleTitle(_Title)); }
  char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const
	{
		if (!GetConsoleTitle(_StrDestination, static_cast<DWORD>(_SizeofStrDestination)))
			return nullptr;
		return _StrDestination;
	}

	void set_log(const path& _Path);
	path get_log() const;

	void icon(oGUI_ICON _hIcon) { oWinSetIcon(GetConsoleWindow(), (HICON)_hIcon); }
	oGUI_ICON icon() const { return (oGUI_ICON)oWinGetIcon(GetConsoleWindow()); }
	void focus(bool _Focus) { oWinSetFocus(GetConsoleWindow()); }
	bool has_focus() const { return oWinHasFocus(GetConsoleWindow()); }
  int2 size_pixels() const { RECT r; GetWindowRect(GetConsoleWindow(), &r); return int2(r.right - r.left, r.bottom - r.top); }
  int2 size_characters() const 
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		oVB(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info));
		return int2(info.dwSize.X, info.dwSize.Y);
	}

	void cursor_position(const int2& _Position);
  int2 cursor_position() const;
  void set_handler(signal _Signal, const std::function<bool()>& _Handler);
	void clear() { ::system("cls"); }
  int vfprintf(FILE* _pStream, color _Foreground, color _Background, const char* _Format, va_list _Args);
private:
	oStd::recursive_mutex Mutex;
	std::function<bool()> Handlers[5];
	bool CtrlHandlerSet;

	path LogPath;
	filesystem::file_handle hLog;
	
	BOOL ctrl_handler(DWORD fdwCtrlType);
	static BOOL CALLBACK static_ctrl_handler(DWORD fdwCtrlType) { return singleton().ctrl_handler(fdwCtrlType); }
};

context& context::singleton()
{
	static context* sInstance = nullptr;
	if (!sInstance)
	{
		process_heap::find_or_allocate(
			"console window"
			, process_heap::per_process
			, process_heap::leak_tracked
			, [=](void* _pMemory) { new (_pMemory) context(); }
			, [=](void* _pMemory) { ((context*)_pMemory)->~context(); }
			, &sInstance);
	}

	return *sInstance;
}

console::info context::get_info() const
{
	oStd::lock_guard<oStd::recursive_mutex> lock(const_cast<oStd::recursive_mutex&>(Mutex));
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	oASSERT(GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE, "");

	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &sbi))
	{
		if (GetLastError() == ERROR_INVALID_HANDLE && this_process::is_child())
			oTHROW(permission_denied, "Failed to access console because this is a child process.");
	}

	info i;
	i.window_position = int2(sbi.srWindow.Left, sbi.srWindow.Top);
	i.window_size = int2(sbi.srWindow.Right - sbi.srWindow.Left, sbi.srWindow.Bottom - sbi.srWindow.Top);
	i.buffer_size = int2(sbi.dwSize.X, sbi.dwSize.Y);
	get_color(sbi.wAttributes, &i.foreground, &i.background);
	i.show = !!IsWindowVisible(GetConsoleWindow());
	return i;
}

void context::set_info(const info& _Info)
{
	oStd::lock_guard<oStd::recursive_mutex> lock(Mutex);

	info i = get_info();
	#define DEF(x) if (_Info.x != use_default) i.x = _Info.x
	DEF(buffer_size.x);
	DEF(buffer_size.y);
	DEF(window_position.x);
	DEF(window_position.y);
	DEF(window_size.x);
	DEF(window_size.y);
	i.foreground = _Info.foreground;
	i.background = _Info.background;
	i.show = _Info.show;
	#undef DEF

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD bufferDimension = { static_cast<SHORT>(i.buffer_size.x), static_cast<SHORT>(i.buffer_size.y) };
	if (!SetConsoleScreenBufferSize(hConsole, bufferDimension))
	{
		if (GetLastError() == ERROR_INVALID_HANDLE && ouro::this_process::is_child())
			oTHROW(permission_denied, "Failed to access console because this is a child process.");
	}

	SMALL_RECT r;
	r.Left = 0;
	r.Top = 0;
	r.Right = static_cast<SHORT>(r.Left + i.window_size.x);
	r.Bottom = static_cast<SHORT>(r.Top + i.window_size.y);

	// Clamp to max size.
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	oVB(GetConsoleScreenBufferInfo(hConsole, &sbi));
	if (r.Right >= sbi.dwMaximumWindowSize.X)
	{
		r.Right = sbi.dwMaximumWindowSize.X - 1;
		if (bufferDimension.X <= i.window_size.x)
			oTRACE("Clamping console width (%d) to system max of %d due to a specified buffer width (%d) larger than screen width", i.buffer_size.x, r.Right, bufferDimension.X);
		else
			oTRACE("Clamping console width (%d) to system max of %d", i.buffer_size.x, r.Right);
	}

	if (r.Bottom >= sbi.dwMaximumWindowSize.Y)
	{
		r.Bottom = sbi.dwMaximumWindowSize.Y - 4; // take a bit more off for the taskbar
		if (bufferDimension.Y <= i.window_size.y)
			oTRACE("Clamping console height (%d) to system max of %d due to a specified buffer width (%d) larger than screen width", i.buffer_size.y, r.Bottom, bufferDimension.Y);
		else
			oTRACE("Clamping console height (%d) to system max of %d", i.buffer_size.y, r.Bottom);
	}

	oVB(SetConsoleWindowInfo(hConsole, TRUE, &r));
	UINT show = i.show ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
	oVB(SetWindowPos(GetConsoleWindow(), HWND_TOP, i.window_position.x, i.window_position.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|show));
	set_console_color(hConsole, _Info.foreground, _Info.background);
}

void context::set_log(const path& _Path)
{
	if (hLog)
	{
		filesystem::close(hLog);
		hLog = nullptr;
	}

	LogPath = _Path;
	if (!LogPath.empty())
		hLog = filesystem::open(LogPath, filesystem::open_option::text_append);
}

path context::get_log() const
{
	oStd::lock_guard<oStd::recursive_mutex> lock(const_cast<oStd::recursive_mutex&>(Mutex));
	return LogPath;
}

void context::cursor_position(const int2& _Position)
{
	COORD c = { static_cast<unsigned short>(_Position.x), static_cast<unsigned short>(_Position.y) };
	if (c.X == use_default || c.Y == use_default)
	{
		int2 p = cursor_position();
		if (c.X == use_default) c.X = static_cast<unsigned short>(p.x);
		if (c.Y == use_default) c.Y = static_cast<unsigned short>(p.y);
	}

	oVB(SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c));
}

int2 context::cursor_position() const
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	return int2(csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y);
}

int context::vfprintf(FILE* _pStream, color _Foreground, color _Background, const char* _Format, va_list _Args)
{
	HANDLE hConsole = 0;
	WORD wOriginalAttributes = 0;

	oStd::lock_guard<oStd::recursive_mutex> lock(Mutex);

	if (_pStream == stdout || _pStream == stderr)
	{
		hConsole = GetStdHandle(_pStream == stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
		wOriginalAttributes = set_console_color(hConsole, _Foreground, _Background);
	}

	xlstring msg;
	vsnprintf(msg, _Format, _Args);

	// Always print any message to _pStream
	int n = ::fprintf(_pStream, msg);

	// And to log file
	if (hLog)
		filesystem::write(hLog, msg, strlen(msg), true);

	if (hConsole)
		SetConsoleTextAttribute(hConsole, wOriginalAttributes);

	return n;
}

BOOL context::ctrl_handler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
		case CTRL_C_EVENT: if (Handlers[ctrl_c]) return Handlers[ctrl_c]();
		case CTRL_BREAK_EVENT: if (Handlers[ctrl_break]) return Handlers[ctrl_break]();
		case CTRL_CLOSE_EVENT: if (Handlers[close]) return Handlers[close]();
		case CTRL_LOGOFF_EVENT: if (Handlers[logoff]) return Handlers[logoff]();
		case CTRL_SHUTDOWN_EVENT: if (Handlers[shutdown]) return Handlers[shutdown]();
		default: break;
	}
	
	return FALSE;
}

void context::set_handler(signal _Signal, const std::function<bool()>& _Handler)
{
	oStd::lock_guard<oStd::recursive_mutex> lock(Mutex);
	Handlers[_Signal] = _Handler;
	if (!CtrlHandlerSet)
	{
		oVB(SetConsoleCtrlHandler(static_ctrl_handler, TRUE));
		CtrlHandlerSet = true;
	}
}

oGUI_WINDOW native_handle()
{
	return context::singleton().native_handle();
}
  
info get_info()
{
	return context::singleton().get_info();
}

void set_info(const info& _Info)
{
	context::singleton().set_info(_Info);
}

void set_title(const char* _Title)
{
	context::singleton().set_title(_Title);
}

char* get_title(char* _StrDestination, size_t _SizeofStrDestination)
{
	return context::singleton().get_title(_StrDestination, _SizeofStrDestination);
}

void set_log(const path& _Path)
{
	context::singleton().set_log(_Path);
}

path get_log()
{
	return context::singleton().get_log();
}

void icon(oGUI_ICON _hIcon)
{
	context::singleton().icon(_hIcon);
}

oGUI_ICON icon()
{
	return context::singleton().icon();
}

void focus(bool _Focus)
{
	context::singleton().focus(_Focus);
}

bool has_focus()
{
	return context::singleton().has_focus();
}

int2 size_pixels()
{
	return context::singleton().size_pixels();
}

int2 size_characters()
{
	return context::singleton().size_characters();
}

void cursor_position(const int2& _Position)
{
	context::singleton().cursor_position(_Position);
}

int2 cursor_position()
{
	return context::singleton().cursor_position();
}

void set_handler(signal _Signal, const std::function<bool()>& _Handler)
{
	context::singleton().set_handler(_Signal, _Handler);
}

void clear()
{
	context::singleton().clear();
}

int vfprintf(FILE* _pStream, color _Foreground, color _Background, const char* _Format, va_list _Args)
{
	return context::singleton().vfprintf(_pStream, _Foreground, _Background, _Format, _Args);
}

	} // namespace console
} // namespace ouro
