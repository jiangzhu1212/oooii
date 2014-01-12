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
#include <oCore/reporting.h>
#include <oBase/algorithm.h>
#include <oBase/fnv1a.h>
#include <oBase/fixed_vector.h>
#include <oCore/debugger.h>
#include <oCore/filesystem.h>
#include <oCore/process.h>
#include <oCore/process_heap.h>
#include <oCore/system.h>
#include <oCore/windows/win_exception_handler.h>
#include <oStd/for.h>
#include <oStd/mutex.h>

#include <oGUI/msgbox.h>

namespace ouro {

const char* as_string(const assert_type::value& _Type)
{
	switch (_Type)
	{
		case assert_type::trace: return "Trace";
		case assert_type::assertion: return "Error";
		default: break;
	}
	return "?";
}

	namespace reporting {

void report_and_exit()
{
	oTRACEA("std::terminate called");
	oASSERT(false, "std::terminate called");
}

void emit_debugger(const assert_context& _Assertion, const char* _Message)
{
	debugger::print(_Message);
}

void emit_log(const assert_context& _Assertion, const char* _Message, filesystem::file_handle _hFile)
{
	if (_hFile)
		filesystem::write(_hFile, _Message, strlen(_Message), true);
}

// _____________________________________________________________________________
// Nove this code out of this module so as not to depend on oGUI.

static assert_action::value to_action(msg_result::value _Result)
{
	switch (_Result)
	{
		case msg_result::abort: return assert_action::abort;
		case msg_result::debug: return assert_action::debug;
		case msg_result::ignore: return assert_action::ignore_always;
		default: break;
	}

	return assert_action::ignore;
}

static assert_action::value show_msgbox(const assert_context& _Assertion, msg_type::value _Type, const char* _String)
{
	#ifdef _DEBUG
		#define MSGBOX_BUILD_TYPE "Debug"
	#else
		#define MSGBOX_BUILD_TYPE "Release"
	#endif
	static const char* DIALOG_BOX_TITLE = "Ouroboros " MSGBOX_BUILD_TYPE " Library";

	char format[oKB(16)];
	char cmdline[oKB(2)];
	*format = 0;
	char* end = format + sizeof(format);
	char* cur = format;
	cur += snprintf(format, MSGBOX_BUILD_TYPE " %s!\nFile: %s(%d)\nCommand Line: %s\n"
		, _Type == msg_type::warn ? "Warning" : "Error"
		, _Assertion.filename
		, _Assertion.line
		, this_process::command_line(cmdline)
		, _Assertion.expression);

	if (oSTRVALID(_Assertion.expression))
		cur += snprintf(cur, std::distance(cur, end), "Expression: %s\n", _Assertion.expression);

	*cur++ = '\n';

	strlcpy(cur, _String, std::distance(cur, end));

	path AppPath = filesystem::app_path(true);
	char title[64];
	snprintf(title, "%s (%s)", DIALOG_BOX_TITLE, AppPath.c_str());
	return to_action(msgbox(_Type, nullptr, title, "%s", format));
}

assert_action::value prompt_msgbox(const assert_context& _Assertion, const char* _Message)
{
	// Output message
	assert_action::value action = _Assertion.default_response;
	switch (_Assertion.type)
	{
		default:
		case assert_type::trace:
			break;

		case assert_type::assertion:
			action = show_msgbox(_Assertion, msg_type::debug, _Message);
			break;
	}

	return action;
}

// End of code to possibly move
// _____________________________________________________________________________

#define LOCK() oStd::lock_guard<oStd::recursive_mutex> lock(Mutex)

class context
{
public:
	static context& singleton();

	inline void set_info(const info& _Info) { LOCK(); Info = _Info; }
	inline info get_info() const { LOCK(); return Info; }

	assert_action::value vformatf(
		const assert_context& _Assertion
		, const char* _Format
		, va_list _Args) const;

	inline int add_emitter(const emitter& _Emitter) { LOCK(); return (int)sparse_set(Emitters, _Emitter); }
	inline void remove_emitter(int _Emitter) { LOCK(); safe_set(Emitters, _Emitter, nullptr); }
	void set_log(const path& _Path);
	inline path get_log() const { return filesystem::get_path(Log); }
	inline void set_prompter(const prompter& _Prompter) { LOCK(); Prompter = _Prompter; }
	inline void add_filter(unsigned int _ID) { LOCK(); push_back_unique(Filters, _ID); }
	inline void remove_filter(unsigned int _ID) { LOCK(); find_and_erase(Filters, _ID); }

private:
	context();
	~context();

	info Info;
	mutable oStd::recursive_mutex Mutex;

	fixed_vector<unsigned int, 256> Filters;
	fixed_vector<emitter, 16> Emitters;
	prompter Prompter;
	filesystem::scoped_file Log;
	int LogEmitter;
};

context::context()
	: LogEmitter(-1)
{
	std::set_terminate(report_and_exit);
	set_info(Info);

	set_prompter(prompt_msgbox);
	add_emitter(emit_debugger);
}

context::~context()
{
}

oDEFINE_PROCESS_SINGLETON("ouro::reporting", context);

// Returns a pointer to the nul terminator of _StrDestination, or beyond its 
// size if filled.
static char* format_message(char* _StrDestination
	, size_t _SizeofStrDestination
	, const assert_context& _Assertion
	, const info& _Info
	, char** _pUserMessageStart // points into _StrDestination at end of prefix info
	, const char* _Format
	, va_list _Args)
{
	if (_pUserMessageStart)
		*_pUserMessageStart = _StrDestination;

	#define oACCUMF(_Format, ...) do \
	{	res = snprintf(_StrDestination + len, _SizeofStrDestination - len - 1, _Format, ## __VA_ARGS__); \
		if (res == -1) goto TRUNCATION; \
		len += res; \
	} while(false)

	#define oVACCUMF(_Format, _Args) do \
	{	res = ouro::vsnprintf(_StrDestination + len, _SizeofStrDestination - len - 1, _Format, _Args); \
		if (res == -1) goto TRUNCATION; \
		len += res; \
	} while(false)

	*_StrDestination = '\0';

	int res = 0;
	size_t len = 0;
	if (_Info.prefix_file_line)
	{
		#if _MSC_VER < 1600
			static const char* kClickableFileLineFormat = "%s(%u) : ";
		#else
			static const char* kClickableFileLineFormat = "%s(%u): ";
		#endif
		oACCUMF(kClickableFileLineFormat, _Assertion.filename, _Assertion.line);
	}

	if (_Info.prefix_timestamp)
	{
		ntp_timestamp now = 0;
		system::now(&now);
		res = (int)strftime(_StrDestination + len
			, _SizeofStrDestination - len - 1
			, sortable_date_ms_format
			, now
			, date_conversion::to_local);
		oACCUMF(" ");
		if (res == 0) goto TRUNCATION;
		len += res;
	}

	if (_Info.prefix_type)
		oACCUMF("%s: ", as_string(_Assertion.type));

	if (_Info.prefix_thread_id)
	{
		mstring exec;
		oACCUMF("%s ", system::exec_path(exec));
	}

	if (_Info.prefix_id)
		oACCUMF("{0x%08x} ", fnv1a<unsigned int>(_Format));

	if (_pUserMessageStart)
		*_pUserMessageStart = _StrDestination + len;

	oVACCUMF(_Format, _Args);

	return _StrDestination + len;

TRUNCATION:
	static const char* kStackTooLargeMessage = "\n... truncated ...";
	size_t TLMLength = strlen(kStackTooLargeMessage);
	snprintf(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
	return _StrDestination + _SizeofStrDestination;

	#undef oACCUMF
	#undef oVACCUMF
}

assert_action::value context::vformatf(const assert_context& _Assertion
	, const char* _Format
	, va_list _Args) const
{
	assert_action::value action = _Assertion.default_response;

	{
		unsigned int ID = fnv1a<unsigned int>(_Format);
		LOCK();
		if (contains(Filters, ID))
			return action;
	}

	char msg[oKB(8)];
	char* usr = nullptr;
	char* cur = format_message(msg, sizeof(msg), _Assertion, Info, &usr, _Format, _Args);
	char* end = msg + sizeof(msg);

	if (Info.callstack && _Assertion.type == assert_type::assertion)
		debugger::print_callstack(cur, std::distance(cur, end), 6, true);

	{
		LOCK();

		oFOR(auto& e, Emitters)
			if (e) e(_Assertion, msg);

		if (Prompter)
			action = Prompter(_Assertion, usr);
	}

	if (_Assertion.type == assert_type::assertion)
	{
		oTRACE("%s (%i): %s", _Assertion.filename, _Assertion.line, msg);
		if (!this_process::has_debugger_attached())
			debugger::dump_and_terminate(nullptr, nullptr);
	}

	return action;
}

void context::set_log(const path& _Path)
{
	if (LogEmitter >= 0)
	{
		remove_emitter(LogEmitter);
		LogEmitter = -1;
	}

	Log = std::move(filesystem::scoped_file(_Path, filesystem::open_option::text_append));
	if (Log)
		LogEmitter = add_emitter(std::bind(emit_log, std::placeholders::_1, std::placeholders::_2, (filesystem::file_handle)Log));
}

void ensure_initialized()
{
	context::singleton();
}

void set_info(const info& _Info)
{
	context::singleton().set_info(_Info);
}

info get_info()
{
	return context::singleton().get_info();
}

void vformatf(
	const assert_context& _Assertion
	, const char* _Format
	, va_list _Args)
{
	context::singleton().vformatf(_Assertion, _Format, _Args);
}

int add_emitter(const emitter& _Emitter)
{
	return context::singleton().add_emitter(_Emitter);
}

void remove_emitter(int _Emitter)
{
	context::singleton().remove_emitter(_Emitter);
}

void set_log(const path& _Path)
{
	context::singleton().set_log(_Path);
}

path get_log()
{
	return context::singleton().get_log();
}

void set_prompter(const prompter& _Prompter)
{
	context::singleton().set_prompter(_Prompter);
}

void add_filter(unsigned int _ID)
{
	context::singleton().add_filter(_ID);
}

void remove_filter(unsigned int _ID)
{
	context::singleton().remove_filter(_ID);
}

	} // namespace reporting

// attach to callback in assert.h
assert_action::value vtracef(const assert_context& _Assertion, const char* _Format, va_list _Args)
{
	return reporting::context::singleton().vformatf(_Assertion, _Format, _Args);
}

} // namespace ouro