/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include "pch.h"
#include <oooii/oAssert.h>
#include <oooii/oConsole.h>
#include <oooii/oDebugger.h>
#include <oooii/oImage.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oMsgBox.h>
#include <oooii/oPath.h>
#include <oooii/oTest.h>
#include <oooii/oRef.h>

static const char* sTITLE = "OOOii Unit Test Suite";

static const oOption sCmdLineOptions[] = 
{
	{ "include", 'i', "regex", "regex matching a test name to include" },
	{ "exclude", 'e', "regex", "regex matching a test name to exclude" },
	{ "path", 'p', "path", "Path where all test data is loaded from. The current working directory is used by default." },
	{ "special-mode", 's', "mode", "Run the test harness in a special mode (used mostly by multi-process/client-server unit tests)" },
	{ "random-seed", 'r', "seed", "Set the random seed to be used for this run. This is reset at the start of each test." },
	{ "golden-images", 'g', "path", "Path where all known-good \"golden\" images are stored. The current working directory is used by default." },
	{ "output", 'o', "path", "Path where all logging and error images are created." },
	{ "repeat-number", 'n', "Repeat the test a certain number of times." },
	{ 0,0,0,0 }
};

void InitEnv()
{
	oConsole::SetTitle(sTITLE);

	#if defined(_WIN32) || defined (_WIN64)
		// Load/test OOOii lib icon:
		extern void GetDescoooii_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);
		const char* BufferName = 0;
		const void* pBuffer = 0;
		size_t bufferSize = 0;
		GetDescoooii_ico(&BufferName, &pBuffer, &bufferSize);

		oRef<oImage> ico;
		oVERIFY(oImage::Create(pBuffer, bufferSize, &ico));

		HICON hIcon = ico->AsIco();

		oSetIcon(GetConsoleWindow(), false, hIcon);

		DeleteObject(hIcon);
	#endif

	oDebugger::ReportLeaksOnExit(true);

	// Resize console
	{
		oConsole::DESC desc;
		desc.BufferWidth = 255;
		desc.BufferHeight = 1024;
		desc.Top = 10;
		desc.Left = 10;
		desc.Width = 120;
		desc.Height = 50;
		desc.Foreground = std::LimeGreen;
		desc.Background = std::Black;
		oConsole::SetDesc(&desc);
	}
}

struct PARAMETERS
{
	std::vector<oFilterChain::FILTER> Filters;
	const char* SpecialMode;
	const char* DataPath;
	const char* GoldenPath;
	const char* OutputPath;
	unsigned int RandomSeed;
	unsigned int RepeatNumber;
};

void ParseCommandLine(int _Argc, const char* _Argv[], PARAMETERS* _pParameters)
{
	_pParameters->Filters.clear();
	_pParameters->SpecialMode = NULL;
	_pParameters->DataPath = NULL;
	_pParameters->GoldenPath = NULL;
	_pParameters->OutputPath = NULL;
	_pParameters->RandomSeed = 0;
	_pParameters->RepeatNumber = 1;

	const char* value = 0;
	char ch = oOptTok(&value, _Argc, _Argv, sCmdLineOptions);
	while (ch)
	{
		switch (ch)
		{
			case 'i':
			{
				_pParameters->Filters.resize(_pParameters->Filters.size() + 1);
				_pParameters->Filters.back().FilterType = oFilterChain::INCLUDE1;
				_pParameters->Filters.back().RegularExpression = value;
				break;
			}

			case 'e':
			{
				_pParameters->Filters.resize(_pParameters->Filters.size() + 1);
				_pParameters->Filters.back().FilterType = oFilterChain::EXCLUDE1;
				_pParameters->Filters.back().RegularExpression = value;
				break;
			}

			case 'p':
				_pParameters->DataPath = value;
				break;

			case 's':
				_pParameters->SpecialMode = value;
				break;

			case 'r':
				_pParameters->RandomSeed = atoi(value) ? atoi(value) : 1; // ensure it's not 0, meaning choose randomly
				break;

			case 'g':
				_pParameters->GoldenPath = value;
				break;

			case 'o':
				_pParameters->OutputPath = value;
				break;

			case 'n':
				_pParameters->RepeatNumber = atoi(value);
				break;

			default:
				break;
		}

		ch = oOptTok(&value, 0, 0, 0);
	}
}

bool GetDataPath(char* _DataPath, size_t _SizeofDataPath, const char* _UserDataPath)
{
	// @oooii-tony: Some hard-coded ones that are typical in our dev environment. This should go away
	static const char* sFallbackPaths[] = 
	{
		"../../oooii/",
	};

	if (_UserDataPath && oFile::Exists(_UserDataPath))
	{
		return 0 == strcpy_s(_DataPath, _SizeofDataPath, _UserDataPath);
	}

	else
	{
		for (size_t i = 0; i < oCOUNTOF(sFallbackPaths); i++)
		{
			if (oFile::Exists(sFallbackPaths[i]))
			{
				return 0 == strcpy_s(_DataPath, _SizeofDataPath, sFallbackPaths[i]);
			}
		}
	}

	// If here, give up to CWD.
	return oGetSysPath(_DataPath, _SizeofDataPath, oSYSPATH_CWD);
}

template<size_t size> inline bool GetDataPath(char (&_DataPath)[size], const char* _UserDataPath) { return GetDataPath(_DataPath, size, _UserDataPath); }

void EnableLogFile(const char* _SpecialModeName)
{
	#ifdef _DEBUG
		char logFilePath[_MAX_PATH];
		oGetLogFilePath(logFilePath, _SpecialModeName);
		oAssert::DESC desc;
		oAssert::GetDesc(&desc);
		desc.LogFilePath = logFilePath;
		oAssert::SetDesc(&desc);
	#endif
}

void SetTestManagerDesc(const PARAMETERS* _pParameters)
{
	char dataPath[_MAX_PATH];
	GetDataPath(dataPath, _pParameters->DataPath);

	oTestManager::DESC desc;
	desc.TestSuiteName = sTITLE;
	desc.DataPath = dataPath;
	desc.GoldenPath = _pParameters->GoldenPath;
	desc.OutputPath = _pParameters->OutputPath;
	desc.NameColumnWidth = 36;
	desc.StatusColumnWidth = 10;
	desc.RandomSeed = _pParameters->RandomSeed ? _pParameters->RandomSeed : oTimerMS();
	desc.NumRunIterations = _pParameters->RepeatNumber ? _pParameters->RepeatNumber : 1;

	oTestManager::Singleton()->SetDesc(&desc);
}

int main(int argc, const char* argv[])
{
	InitEnv();

	PARAMETERS parameters;
	ParseCommandLine(argc, argv, &parameters);
	SetTestManagerDesc(&parameters);
	EnableLogFile(parameters.SpecialMode);

	int result = 0;

	if (parameters.SpecialMode)
		result = oTestManager::Singleton()->RunSpecialMode(parameters.SpecialMode);
	else
	{
		result = oTestManager::Singleton()->RunTests(parameters.Filters.empty() ? 0 : &parameters.Filters[0], parameters.Filters.size());
		if (oDebugger::IsAttached())
			oMsgBox::printf(oMsgBox::INFO, sTITLE, "oooii_unittests completed%s", result ? " with errors" : " successfully");
	}

	return result;
}
