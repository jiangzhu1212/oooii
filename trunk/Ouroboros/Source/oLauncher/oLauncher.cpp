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

#include <oPlatform/oModule.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oVersionUpdate.h>
#include <oPlatform/Windows/oWindows.h>

static oOption sCmdOptions[] = 
{
	{ "launcher-wait", 'w', "process-id", "The launcher should wait for the specified process to terminate before starting any meaningful work." },
	{ "wait-timeout", 't', "milliseconds", "After this amount of time, give up waiting for the -w process to terminates and forcibly try to terminate it before moving on." },
	{ "version", 'v', "ver", "Force execution of the specified version. Format is Microsoft standard 4-component version: Major.Minor.Build.Revision" },
	{ "exe-name-override", 'e', "exe-name", "By default this launcher uses its own name to find the versioned exe to run. This switch uses the param explicitly." },
	{ "command-line", 'c', "options", "The command line to pass to the actual executable" },
	{ "prefix", 'p', "prefix", "A prefix to differentiate the actual exe from the launcher exe" },
	{ 0, 0, 0, 0 },
};

// @oooii-tony: potentially promote this to a util lib to be used more broadly...

namespace oStd {

	bool from_string(const char** _ppConstStr, const char* _Value) { *_ppConstStr = _Value; return true; }

} // namespace oStd

#define oOPT_CASE(_ShortNameConstant, _Value, _Dest) case _ShortNameConstant: { if (!oStd::from_string(&(_Dest), value)) { return oErrorSetLast(std::errc::invalid_argument, "-%c %s cannot be interpreted as a(n) %s", (_ShortNameConstant), (_Value), typeid(_Dest).name()); } break; }
#define oOPT_CASE_DEFAULT(_ShortNameVariable, _Value, _NthOption) \
	case ' ': { return oErrorSetLast(std::errc::invalid_argument, "There should be no parameters that aren't switches passed"); break; } \
	case '?': { return oErrorSetLast(std::errc::invalid_argument, "Parameter %d is not recognized", (_NthOption)); break; } \
	case ':': { return oErrorSetLast(std::errc::invalid_argument, "Parameter %d is missing a value", (_NthOption)); break; } \
	default: { oTRACE("Unhandled option -%c %s", (_ShortNameVariable), oSAFESTR(_Value)); break; }

bool oParseCmdLine(int argc, const char* argv[], oVERSIONED_LAUNCH_DESC* _pDesc)
{
	const char* value = 0;
	char ch = oOptTok(&value, argc, argv, sCmdOptions);
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
		}

		ch = oOptTok(&value, 0, 0, 0);
		count++;
	}

	return true;
}

static bool oLauncherMain1(int argc, const char* argv[])
{
	oVERSIONED_LAUNCH_DESC vld;

	if (!oParseCmdLine(argc, argv, &vld))
		return false; // pass through error

	return oVURelaunch(vld);
}

int oLauncherMain(int argc, const char* argv[])
{
	if (!oLauncherMain1(argc, argv))
	{
		oStd::mstring ModuleName;
		oMSGBOX_DESC d;
		d.Title = oGetFilebase(oModuleGetName(ModuleName));
		d.Type = oMSGBOX_ERR;
		oMsgBox(d, "%s", oErrorGetLastString());
		return oErrorGetLast();
	}

	return 0;
}

oMAIN()
{
	return oLauncherMain(argc, argv);
}
