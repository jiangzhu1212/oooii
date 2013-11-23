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
using namespace ouro;

#include <oGUI/oMsgBox.h>
//#include <oPlatform/oVersionUpdate.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static option sCmdOptions[] = 
{
	{ 'w', "launcher-wait", "process-id", "Wait for process to terminate before launching" },
	{ 't', "wait-timeout", "milliseconds", "Try to forcibly terminate the -w process after\nthis amount of time" },
	{ 'v', "version", "Maj.Min.Build.Rev", "Force execution of the specified version" },
	{ 'e', "exe-name-override", "exe-name", "Override the default of <version>/launcher-name\nwith an explicit exe name" },
	{ 'c', "command-line", "options", "The command line to pass to the actual executable" },
	{ 'p', "prefix", "prefix", "A prefix to differentiatethe actual exe from the\nlauncher exe" },
	{ 'h', "help",  0, "Displays this message" },
};

namespace ouro {

	bool from_string(const char** _ppConstStr, const char* _Value) { *_ppConstStr = _Value; return true; }

} // namespace ouro

#define oOPT_CASE(_ShortNameConstant, _Value, _Dest) case _ShortNameConstant: { if (!from_string(&(_Dest), value)) { return oErrorSetLast(std::errc::invalid_argument, "-%c %s cannot be interpreted as a(n) %s", (_ShortNameConstant), (_Value), typeid(_Dest).name()); } break; }
#define oOPT_CASE_DEFAULT(_ShortNameVariable, _Value, _NthOption) \
	case ' ': { return oErrorSetLast(std::errc::invalid_argument, "There should be no parameters that aren't switches passed"); break; } \
	case '?': { return oErrorSetLast(std::errc::invalid_argument, "Parameter %d is not recognized", (_NthOption)); break; } \
	case ':': { return oErrorSetLast(std::errc::invalid_argument, "Parameter %d is missing a value", (_NthOption)); break; } \
	default: { oTRACE("Unhandled option -%c %s", (_ShortNameVariable), oSAFESTR(_Value)); break; }
#if 0
bool oParseCmdLine(int argc, const char* argv[], oVERSIONED_LAUNCH_DESC* _pDesc, bool* _pShowHelp)
{
	*_pShowHelp = false;
	const char* value = 0;
	char ch = opttok(&value, argc, argv, sCmdOptions);
	int count = 1;
	while (ch)
	{
		switch (ch)
		{
			oOPT_CASE('w', value, _pDesc->WaitForPID);
			oOPT_CASE('t', value, _pDesc->WaitForPIDTimeout);
			oOPT_CASE('v', value, _pDesc->SpecificVersion);
			oOPT_CASE('e', value, _pDesc->SpecificModuleName);
			oOPT_CASE('c', value, _pDesc->PassThroughCommandLine);
			oOPT_CASE('p', value, _pDesc->ModuleNamePrefix);
			oOPT_CASE_DEFAULT(ch, value, count);
			case 'h': *_pShowHelp = true; break;
		}

		ch = opttok(&value);
		count++;
	}

	return true;
}

static bool oLauncherMain1(int argc, const char* argv[])
{
	oVERSIONED_LAUNCH_DESC vld;

	bool ShowHelp = false;
	if (!oParseCmdLine(argc, argv, &vld, &ShowHelp))
		return false; // pass through error

	if (ShowHelp)
	{
		char help[1024];
		if (optdoc(help, ouro::path(argv[0]).filename().c_str(), sCmdOptions))
			printf(help);
		return true;
	}

	return oVURelaunch(vld);
}
#endif
int oLauncherMain(int argc, const char* argv[])
{
	if (0 /*&& !oLauncherMain1(argc, argv)*/)
	{
		path ModuleName = ouro::this_module::path();
		oMSGBOX_DESC d;
		d.Title = ModuleName.filename();
		d.Type = oMSGBOX_ERR;
		oMsgBox(d, "%s", oErrorGetLastString());
		return oErrorGetLast();
	}

	return 0;
}

int main(int argc, const char* argv[])
{
	oASSERT(false, "Disabled until version update is resurrected");

	return oLauncherMain(argc, argv);
}
