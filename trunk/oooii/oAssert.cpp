// $(header)
#include <oooii/oAssert.h>
#include <oooii/oDebugger.h>
#include <oooii/oHash.h>
#include <oooii/oMsgBox.h>
#include <oooii/oPath.h>
#include <oooii/oSingleton.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oThreading.h>
#include <oooii/oString.h>
#ifdef _DEBUG
	#include <crtdbg.h>
#endif
#include <vector>

template<> const char* oAsString(const oAssert::TYPE& _Type)
{
	switch (_Type)
	{
		case oAssert::TYPE_TRACE: return "Trace";
		case oAssert::TYPE_WARNING: return "Warning";
		case oAssert::TYPE_ASSERT: return "Error";
		default: oASSUME(0);
	}
}

oAssert::ACTION OOOiiLowLevelPrintMessage(const oAssert::ASSERTION& _Assertion, void* _hLogFile, const char* _Format, va_list _Args)
{
	// @oooii-tony: NOTE: crtdbg is really a platform thing, but then so
	// is oDebugger.

	oAssert::ACTION action = oAssert::IGNORE_ONCE;

	char msg[oKB(2)];
	vsprintf_s(msg, _Format, _Args);

	// always print a message to the screen
	oDebugger::Print(msg);

	switch (_Assertion.Type)
	{
		case oAssert::TYPE_TRACE:
			break;

		#ifdef _DEBUG

			case oAssert::TYPE_WARNING:
				_CrtDbgReport(_CRT_WARN, _Assertion.Filename, _Assertion.Line, "OOOii Debug Library", msg);
				break;

			case oAssert::TYPE_ASSERT:
			{
				char msg2[oKB(2)];
				sprintf_s(msg2, "%s\n\n%s", _Assertion.Expression, msg);
				if (1 == _CrtDbgReport(_CRT_ASSERT, _Assertion.Filename, _Assertion.Line, "OOOii Debug Library", msg2))
					action = oAssert::BREAK;
				break;
			}

		#endif

		default:
			break;
	}

	return action;
}

// Requires oDebugger and oMsgbox as well as calls to oProcess::GetCurrentProcessID() and oThread::GetCurrentThreadID()
namespace RobustPrintMessage
{
	static oAssert::ACTION GetAction(oMsgBox::RESULT _Result)
	{
		switch (_Result)
		{
			case oMsgBox::ABORT: return oAssert::ABORT;
			case oMsgBox::BREAK: return oAssert::BREAK;
			case oMsgBox::IGNORE_ALWAYS: return oAssert::IGNORE_ALWAYS;
			default: break;
		}

		return oAssert::IGNORE_ONCE;
	}

	static oAssert::ACTION ShowMsgBox(const oAssert::ASSERTION& _Assertion, oMsgBox::TYPE _Type, const char* _String)
	{
		#ifdef _DEBUG
			static const char* MESSAGE_PREFIX = "Debug %s!\n\n";
			static const char* DIALOG_BOX_TITLE = "OOOii Debug Library";
		#else
			static const char* MESSAGE_PREFIX = "Release %s!\n\n";
			static const char* DIALOG_BOX_TITLE = "OOOii Release Library";
		#endif

		char format[64 * 1024];
		*format = 0;
		char* end = format + sizeof(format);
		char* cur = format;
		cur += sprintf_s(format, MESSAGE_PREFIX, _Type == oMsgBox::WARN ? "Warning" : "Error");
		strcpy_s(cur, std::distance(cur, end), _String);
		return GetAction(oMsgBox::printf(_Type, DIALOG_BOX_TITLE, "%s", format));
	}

	static void PrintCallStackToString(char* _StrDestination, size_t _SizeofStrDestination)
	{
		size_t offset = 9; // Start offset from where assert occurred, skipping any debug handling code

		size_t nSymbols = 0;
		unsigned long long address = 0;
		while (oDebugger::GetCallstack(&address, 1, offset++))
		{
			oDebugger::SYMBOL symbol;
			if (oDebugger::TranslateSymbol(&symbol, address))
			{
				if (nSymbols++ == 0) // if we have a callstack, label it
					sprintf_s(_StrDestination, _SizeofStrDestination, "\nCall Stack:\n");

				if (symbol.Line && symbol.CharOffset)
				{
					if (0 != oStrAppend(_StrDestination, _SizeofStrDestination, "%s!%s() ./%s Line %i + 0x%0x bytes\n", symbol.Module, symbol.Name, oGetFilebase(symbol.Filename), symbol.Line, symbol.CharOffset)) goto STACK_TOO_LARGE;
				}

				else if (symbol.Line)
				{
					if (0 != oStrAppend(_StrDestination, _SizeofStrDestination, "%s!%s() ./%s Line %i\n", symbol.Module, symbol.Name, oGetFilebase(symbol.Filename), symbol.Line) ) goto STACK_TOO_LARGE;
				}
				else
				{
					if (0 != oStrAppend(_StrDestination, _SizeofStrDestination, "%s!%s() ./%s\n", symbol.Module, symbol.Name, oGetFilebase(symbol.Filename))) goto STACK_TOO_LARGE;
				}
			}
		}

		return;

		STACK_TOO_LARGE:
			static const char* kStackTooLargeMessage = "... truncated ...";
			strcpy_s(_StrDestination + _SizeofStrDestination - strlen(kStackTooLargeMessage) + 1, strlen(kStackTooLargeMessage) + 1, kStackTooLargeMessage);
	}

	char* FormatAssertMessage(char* _StrDestination, size_t _SizeofStrDestination, const oAssert::DESC& _Desc, const oAssert::ASSERTION& _Assertion, const char* _Format, va_list _Args)
	{
		char* cur = _StrDestination;
		char* end = cur + _SizeofStrDestination;
		{
			if (_Desc.PrefixFileLine)
			{
				#ifdef oCLICKABLE_OUTPUT_REQUIRES_SPACE_COLON
					static const char* kClickableFileLineFormat = "%s(%u) : ";
				#else
					static const char* kClickableFileLineFormat = "%s(%u): ";
				#endif
				cur += sprintf_s(cur, std::distance(cur, end), kClickableFileLineFormat, _Assertion.Filename, _Assertion.Line);
			}

			if (_Desc.PrefixMsgType)
				cur += sprintf_s(cur, std::distance(cur, end), "%s ", oAsString(_Assertion.Type));

			if (_Desc.PrefixThreadId)
				cur += sprintf_s(cur, std::distance(cur, end), "[%s.%u.%u] ", oGetHostname(), oGetCurrentProcessID(), oGetCurrentThreadID());

			if (_Desc.PrefixMsgId)
				cur += sprintf_s(cur, std::distance(cur, end), "{0x%08x} ", _Assertion.MsgId);
		}

		cur += vsprintf_s(cur, std::distance(cur, end), _Format, _Args);
		return cur;
	}

	template<size_t size> inline char* FormatAssertMessage(char (&_StrDestination)[size], const oAssert::DESC& _Desc, const oAssert::ASSERTION& _Assertion, const char* _Format, va_list _Args) { return FormatAssertMessage(_StrDestination, size, _Desc, _Assertion, _Format, _Args); }

	oAssert::ACTION OOOiiDefaultPrintMessage(const oAssert::ASSERTION& _Assertion, void* _hLogFile, const char* _Format, va_list _Args)
	{
		oAssert::DESC desc;
		oAssert::GetDesc(&desc);

		bool addCallStack = desc.PrintCallstack && (_Assertion.Type == oAssert::TYPE_ASSERT);

		// add prefixes to original message
		char msg[oKB(8)];
		char* cur = FormatAssertMessage(msg, desc, _Assertion, _Format, _Args);
		char* end = msg + sizeof(msg);

		if(addCallStack)
			PrintCallStackToString(cur, std::distance(cur, end));

		// Always print any message to the debugger output
		oDebugger::Print(msg);

		// And to log file

		if (_hLogFile)
		{
			fwrite(msg, strlen(msg), 1, (FILE*)_hLogFile);
			fflush((FILE*)_hLogFile);
		}

		// Output message
		oAssert::ACTION action = oAssert::IGNORE_ONCE;
		switch (_Assertion.Type)
		{
			case oAssert::TYPE_TRACE:
				break;

			case oAssert::TYPE_WARNING:
				if (desc.EnableWarnings)
					action = ShowMsgBox(_Assertion, oMsgBox::WARN, msg);
				break;

			case oAssert::TYPE_ASSERT:
				action = ShowMsgBox(_Assertion, oMsgBox::DEBUG, msg);
				break;

			default: oASSUME(0);
		}

		return action;
	}
} // namespace RobustPrintMessage

struct oAssertContext : oProcessSingleton<oAssertContext>
{
	oAssertContext()
		: LogFile(0)
	{
		PushMessageHandler(OOOiiLowLevelPrintMessage);

		// @oooii-tony: about to be refactored into another file
		PushMessageHandler(RobustPrintMessage::OOOiiDefaultPrintMessage);

		oDebugger::Reference();
		*LogPath = 0;
	}

	~oAssertContext()
	{
		if (LogFile)
			fclose(LogFile);

		oDebugger::Release();
	}

	void SetDesc(const oAssert::DESC* _pDesc)
	{
		oAssert::DESC defaultDesc;

		if (!_pDesc)
			_pDesc = &defaultDesc;

		Desc = *_pDesc;

		if (Desc.LogFilePath)
		{
			char tmp[_MAX_PATH];
			oCleanPath(tmp, Desc.LogFilePath);
			if (0 != _stricmp(LogPath, tmp))
			{
				if (LogFile)
				{
					fclose(LogFile);
					LogFile = 0;
				}

				if (0 != fopen_s(&LogFile, tmp, "wt"))
				{
					*LogPath = 0;
					oWARN("Failed to open log file \"%s\"", oSAFESTR(tmp));

					// make a copy of the path and attach it to the desc for future
					// GetDesc() calls.
					strcpy_s(LogPath, tmp);
					Desc.LogFilePath = LogPath;
				}
			}
		}

		else if (LogFile)
		{
			fclose(LogFile);
			LogFile = 0;
		}
	}

	inline const oAssert::DESC& GetDesc() const { return Desc; }

	bool PushMessageHandler(oAssert::VPrintMessageFn _VPrintMessage)
	{
		if (VPrintMessageStack.size() < VPrintMessageStack.capacity())
		{
			VPrintMessageStack.push_back(_VPrintMessage);
			return true;
		}

		return false;
	}

	oAssert::VPrintMessageFn PopMessageHandler()
	{
		oAssert::VPrintMessageFn fn = 0;
		if (!VPrintMessageStack.empty())
		{
			fn = VPrintMessageStack.back();
			VPrintMessageStack.pop_back();
		}

		return fn;
	}
	
	oAssert::ACTION VPrintMessage(const oAssert::ASSERTION& _Assertion, const char* _Format, va_list _Args)
	{
		if (!oContains(FilteredMessages, _Assertion.MsgId) && !VPrintMessageStack.empty())
		{
			oAssert::VPrintMessageFn VPrintMessage = VPrintMessageStack.back();
			return VPrintMessage(_Assertion, LogFile, _Format, _Args);
		}

		return oAssert::IGNORE_ONCE;
	}

	void AddMessageFilter(int _MsgId) { oPushBackUnique(FilteredMessages, _MsgId); }
	void RemoveMessageFilter(int _MsgId) { oFindAndErase(FilteredMessages, _MsgId); }

protected:
	FILE* LogFile;
	char LogPath[_MAX_PATH];
	oAssert::DESC Desc;
	typedef oArray<int, 256> array_t;
	array_t FilteredMessages;
	oArray<oAssert::VPrintMessageFn, 8> VPrintMessageStack;
};

void oAssert::SetDesc(const DESC* _pDesc)
{
	oAssertContext::Singleton()->SetDesc(_pDesc);
}

void oAssert::GetDesc(DESC* _pDesc)
{
	*_pDesc = oAssertContext::Singleton()->GetDesc();
}

bool oAssert::PushMessageHandler(VPrintMessageFn _VPrintMessage)
{
	return oAssertContext::Singleton()->PushMessageHandler(_VPrintMessage);
}

oAssert::VPrintMessageFn oAssert::PopMessageHandler()
{
	return oAssertContext::Singleton()->PopMessageHandler();
}

void oAssert::Reference()
{
	oAssertContext::Singleton()->Reference();
}

void oAssert::Release()
{
	oAssertContext::Singleton()->Release();
}

int oAssert::GetMsgId(const char* _Format)
{
	return (int)oHash_superfast(_Format, static_cast<unsigned int>(strlen(_Format)), 0);
}

oAssert::ACTION oAssert::VPrintMessage(const ASSERTION& _Assertion, const char* _Format, va_list _Args)
{
	return oAssertContext::Singleton()->VPrintMessage(_Assertion, _Format, _Args);
}

void oAssert::AddMessageFilter(int _MsgId)
{
	oAssertContext::Singleton()->AddMessageFilter(_MsgId);
}

void oAssert::RemoveMessageFilter(int _MsgId)
{
	oAssertContext::Singleton()->RemoveMessageFilter(_MsgId);
}
