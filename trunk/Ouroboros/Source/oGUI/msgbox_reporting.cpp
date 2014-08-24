/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oGUI/msgbox.h>

namespace ouro {

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

} // namespace ouro
