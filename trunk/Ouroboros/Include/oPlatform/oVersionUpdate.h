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
// Utilities for deploying updated versions of an application.
// This system is modeled from observation of Google Chrome's application folder
// and update process. Basically:
// The app folder has a shim launcher. That launcher executes 
// ".\<version>\<shim-name>_launcher.exe"
// Installers are just zip files that unzip a new .\<version> folder, so old
// versions can be executed with ease.

#ifndef oVersionUpdate_h
#define oVersionUpdate_h

#include <oBase/fixed_string.h>
#include <oBase/version.h>
#include <oCore/process.h>

// _____________________________________________________________________________
// Utilities to be called from the non-launcher/target executable

struct oVU_URI_PARTS
{
	oURIParts URIParts; // Path is truncated to have the filename removed
	ouro::mstring Filebase;
	ouro::sstring Extension;
	ouro::version Version;
	bool IsDebugBuild;
};

oAPI bool oVUDecompose(const ouro::uri_string& _URI, bool _VersionFromFilename, oVU_URI_PARTS* _pParts);

oAPI char* oVURecompose(ouro::uri_string& _URI, const oVU_URI_PARTS& _Parts);

// Returns true if the installer advertises a different version than the 
// currently running executable. If _IsNewer is true, the version will have to
// be newer than the current one. If _MatchInstallerAndExecutableNames is false, 
// then no verification that this installer is meant for this executable is done.
oAPI bool oVUIsUpdateInstallerValid(const ouro::uri_string& _SfxURI, bool _IsNewer = true, bool _MatchInstallerAndExecutableNames = true);

// This expects a file:// URI to a 7-zip console sfx executable specified as 
// _SfxURI matching the regex format:
// "(.*?)("oMODULE_DEBUG_SUFFIX_A")?-([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)\\.exe"
// This will unzip to the oSYSPATH_APP folder of the specified _SfxURI. The zip 
// should be set up to unzip to a sub folder that is its version number.
oAPI bool oVUUnzip(const char* _SfxURI);

// Usually the launcher app that is installed to the same dir as the install 
// sfx is named the same as the main executable in the version dir, but 
// sometimes another exe is desired (i.e. THIS exe as a new version), so use 
// this to rename the launcher to the same as the currently running executable.
oAPI bool oVURenameLauncher(const char* _SfxURI);

// Returns the filename of the installer that might've installed this executable
// by using its name and version. This is a convenient function built on top of
// oVUDecompose/oVURecompose.
oAPI bool oVUGetThisAppsInstallerFilename(ouro::mstring& _StrDestination);

oAPI bool oVUGetLatestInstallerFilename(ouro::mstring& _StrDestination);

// This should be called from a file in <some-path>\<version>\Exe.exe, so this
// will look for the launcher in ../
oAPI bool oVULaunchLauncher(unsigned int _ExpectedTimeToShutdownMS, const ouro::version& _Version = ouro::version());

// _____________________________________________________________________________
// Utilities to be called from the launcher/lightweight executable

struct oVERSIONED_LAUNCH_DESC
{
	oVERSIONED_LAUNCH_DESC()
		: WaitForPIDTimeout(oInfiniteWait)
		, SpecificModuleName(nullptr)
		, SpecificVersion(nullptr)
		, ModuleNamePrefix(nullptr)
		, PassThroughCommandLine(nullptr)
	{}

	ouro::process::id WaitForPID;
	unsigned int WaitForPIDTimeout;
	const char* SpecificModuleName; // if null, use this launcher's module name
	const char* SpecificVersion; // if null, use latest version
	const char* ModuleNamePrefix; // if module name can't be found, try prepending this
	const char* PassThroughCommandLine; // a single string containing the command line to pass onto the spawned exe
};

// Call this from launcher code. Client code should parse options from command
// line/ini/registry, or they can be hard-coded for situations that warrant it.
// This will use a specific search order while looking for the executable.
// That order is:
// 1. If a specific module version is specified, find ./<ver>, else find greatest 
//    valid version folder in ./
// 2. If a specific module name is specified, search for that in the version 
//    folder and ./ If that doesn't exist, fail
// 3. If no module name, use launcher's current name
// 4. Check: version path + prefix + module name
// 5. Check: version path + module name.
// 6. Check: prefix + ./ + module name.
// 7. Check: ./ + module name.
// Steps 4-7 are first checked with the debug module name and then again with
// the release module name.
// The debug module name is generated as module name minus extension + 
// oMODULE_DEBUG_SUFFIX_A + module name extension.
oAPI bool oVURelaunch(const oVERSIONED_LAUNCH_DESC& _Desc);

#endif
