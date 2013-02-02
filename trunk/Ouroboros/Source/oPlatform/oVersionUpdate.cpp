/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oPlatform/oVersionUpdate.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oFixedString.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oSystem.h>

static std::regex reAppName("(" oMODULE_DEBUG_PREFIX_A ")?([^-]+)(-([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+))?(\\.exe)", std::regex_constants::optimize);

bool oVUDecompose(const oStringURI& _URI, bool _VersionFromFilename, oVU_URI_PARTS* _pParts)
{
	if (!oURIDecompose(_URI, &_pParts->URIParts))
		return false; // pass through error

	oMODULE_DESC md;
	if (!_VersionFromFilename)
	{
		if (oModuleGetDesc(_pParts->URIParts.Path, &md))
			_pParts->Version = md.ProductVersion;
		else
			return false; // pass through error
	}

	oStringM Filename = oGetFilebase(_pParts->URIParts.Path);
	oTrimFilename(_pParts->URIParts.Path);

	std::cmatch matches;
	if (!regex_match(Filename.c_str(), matches, reAppName))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "malformed app/installer name: %s", _URI.c_str());

	_pParts->IsDebugBuild = matches[1].first != matches[1].second;
	_pParts->Filebase.assign(matches[2].first, matches[2].second);
	_pParts->Extension.assign(matches[5].first, matches[5].second);

	if (_VersionFromFilename)
	{
		if (matches.length(5))
		{
			oStringS StrVersion(matches[4].first, matches[4].second);
			oVERIFY(oFromString(&_pParts->Version, StrVersion)); // the regex ensures this is correct
		}

		else
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "version could not be extracted");
	}

	return true;
}

static bool oVURecomposePath(oURIParts& _URIParts, const oVU_URI_PARTS& _Parts)
{
	const char* BuildPrefix = _Parts.IsDebugBuild ? oMODULE_DEBUG_PREFIX_A : "";
	if (_Parts.Version.IsValid())
	{
		oStringS ver;
		oStrAppendf(_URIParts.Path, "%s%s-%s%s", BuildPrefix, _Parts.Filebase.c_str(), oToString(ver, _Parts.Version), _Parts.Extension.c_str());
	}

	else
		oStrAppendf(_URIParts.Path, "%s%s%s", BuildPrefix, _Parts.Filebase.c_str(), _Parts.Extension.c_str());

	return true;
}

char* oVURecompose(oStringURI& _URI, const oVU_URI_PARTS& _Parts)
{
	oURIParts p = _Parts.URIParts;
	if (!oVURecomposePath(p, _Parts))
		return false; // pass through error
	return oURIRecompose(_URI, p);
}

bool oVUGetThisAppsInstallerFilename(oStringM& _StrDestination)
{
	oStringURI AppURI;
	oVERIFY(oSystemGetURI(AppURI, oSYSPATH_APP_FULL));

	// by decomposing and recomposing, we've included the version into the exe
	// name for the installer's full name.
	oVU_URI_PARTS parts;
	oVERIFY(oVUDecompose(AppURI, false, &parts));
	oVERIFY(oVURecomposePath(parts.URIParts, parts));
	_StrDestination = oGetFilebase(parts.URIParts.Path);
	return true;
}

static bool oVUEnumInstallers(const char* _FullPath, const oSTREAM_DESC& _Desc, std::vector<oVU_URI_PARTS>* _pInstallers)
{
	oStringURI URI;
	oVERIFY(oURIFromAbsolutePath(URI, _FullPath));
	oVU_URI_PARTS parts;
	if (oVUDecompose(_FullPath, true, &parts))
		_pInstallers->push_back(parts);
	return true;
}

static bool oVUGetLatestInstallerFilename(oStringM& _StrDestination, const char* _InstallersFolder)
{
	oStringPath Path;
	oPrintf(Path, "%s/*", _InstallersFolder); // all folders
	std::vector<oVU_URI_PARTS> Installers;
	Installers.reserve(10);
	if (!oFileEnum(Path, oBIND(oVUEnumInstallers, oBIND1, oBIND2, &Installers)))
		return false; // pass through error

	if (Installers.empty())
		return oErrorSetLast(oERROR_NOT_FOUND, "No installers found");
	
	std::sort(Installers.begin(), Installers.end(), [=](const oVU_URI_PARTS& _S1, const oVU_URI_PARTS& _S2)->bool { return _S1.Version < _S2.Version; });
	
	oStringURI LatestURI;
	oVERIFY(oVURecompose(LatestURI, Installers.back()));

	oURIParts URIParts;
	oVERIFY(oURIDecompose(LatestURI, &URIParts));
	oVERIFY(oSystemURIPartsToPath(Path, URIParts));
	_StrDestination = oGetFilebase(Path);
	return true;
}

bool oVUGetLatestInstallerFilename(oStringM& _StrDestination)
{
	oStringPath AppPath;
	oVERIFY(oSystemGetPath(AppPath, oSYSPATH_APP));
	oStrAppendf(AppPath, "../");
	if (!oVUGetLatestInstallerFilename(_StrDestination, AppPath))
		return false; // pass through error
	return true;
}

bool oVURenameLauncher(const char* _SfxURI)
{
	oVU_URI_PARTS SourceParts;
	oVERIFY(oVUDecompose(_SfxURI, true, &SourceParts));

	oVU_URI_PARTS DestParts;
	{
		oStringM ThisInstallerFilename;
		oVERIFY(oVUGetThisAppsInstallerFilename(ThisInstallerFilename));
		oVERIFY(oVUDecompose(ThisInstallerFilename, true, &DestParts));
	}

	DestParts.Version = SourceParts.Version = oVersion();
	DestParts.Extension = SourceParts.Extension;
	DestParts.URIParts = SourceParts.URIParts;

	oStringURI S, D;
	oVERIFY(oVURecomposePath(SourceParts.URIParts, SourceParts));
	oVERIFY(oURIRecompose(S, SourceParts.URIParts));

	oVERIFY(oSystemURIPartsToPath(S, SourceParts.URIParts));

	oVERIFY(oVURecomposePath(DestParts.URIParts, DestParts));
	oVERIFY(oURIRecompose(D, DestParts.URIParts));

	if (!oStreamMove(S, D, true))
	{
		oTRACE("%s", oErrorGetLastString());
		return false; // pass through error
	}

	return true;
}

static bool oVUFireAndForget(const char* _CommandLine)
{
	oStringPath ModuleName;
	oVERIFY(oModuleGetName(ModuleName));
	oCleanPath(ModuleName, ModuleName);

	oStringPath MDir, MFile, CDir, CFile;
	MDir = ModuleName;
	*oGetFilebase(MDir) = 0;
	
	const char* MFB = oGetFilebase(ModuleName);
	if (!_memicmp("DEBUG-", MFB, 6))
		MFB += 6;
	MFile = MFB;
	CDir = _CommandLine;
	*oGetFilebase(CDir) = 0;
	const char* CFB = oGetFilebase(_CommandLine);
	if (!_memicmp("DEBUG-", CFB, 6))
		CFB += 6;
	CFile = CFB;

	if (0 == strcmp(MDir, CDir) && 0 == strcmp(MFile, CFile))
		return oErrorSetLast(oERROR_REDUNDANT, "Asked to execute self, this will create an infinite loop.");

	oProcess::DESC pd;
	pd.CommandLine = _CommandLine;
	pd.StartMinimized = true;
	#ifndef _DEBUG
		pd.StartSuspended = false; // approved usage of non-start-need API.
	#endif
	oTRACE("Spawning process: %s", _CommandLine);
	oRef<threadsafe oProcess> NewProcess;
	if (!oProcessCreate(pd, &NewProcess))
		return false; // pass through error
	NewProcess->Start();
	return true;
}

static bool oVUNotFound(const std::vector<std::string>& _Paths, std::vector<std::string>::const_iterator _ItSelf)
{
	std::string all;
	for (auto it = _Paths.cbegin(); it != _Paths.cend(); ++it)
	{
		all += "  " + *it;
		if (_ItSelf == it)
			all += " (self)";
		all += "\n";
	}
	return oErrorSetLast(oERROR_NOT_FOUND, "not found:\n%s", all.c_str());
}

// Returns true if the specified URI is an OOOii standard sfx zip installer for 
// the currently running process, i.e. <curprocname>-maj.min.build.rev.exe
// To support active development, this returns true if the versions are equal
// because unzipping and more inspection is required to see if it's worth 
// updating. This will return false for versions that are less than the current
// version or the specified URI is otherwise malformed.
bool oVUIsNewerVersion(const oStringURI& _SfxURI, bool _MatchInstallerAndExecutableNames)
{
	oVU_URI_PARTS SfxParts;
	if (!oVUDecompose(_SfxURI, true, &SfxParts))
		return false; // pass through error
	
	oStringPath AppURI;
	oVU_URI_PARTS AppParts;
	oVERIFY(oSystemGetURI(AppURI, oSYSPATH_APP_FULL));
	if (!oVUDecompose(AppURI, true, &AppParts))
		return false; // pass through error

	if (!SfxParts.Version.IsValid() || (_MatchInstallerAndExecutableNames && oStricmp(SfxParts.Filebase, AppParts.Filebase)))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "ignoring %s: not an installer for process %s", _SfxURI.c_str(), AppParts.Filebase.c_str());

	if (SfxParts.Version < AppParts.Version)
	{
		oStringS ver;
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "ignoring %s: The current version is up-to-date at %s", _SfxURI.c_str(), oToString(ver, AppParts.Version));
	}

	return true;
}

static bool oVUGetModuleShortName(oStringM& _ModuleShortName)
{
	oStringPath ModulePath;
	oModuleGetName(ModulePath);
	char* filebase = oGetFilebase(ModulePath);
	char* p = strstr(filebase, oMODULE_DEBUG_PREFIX_A);
	if (p)
		_ModuleShortName = p + oStrlen(oMODULE_DEBUG_PREFIX_A);
	else
		_ModuleShortName = filebase;
	return true;
}

static const char* sGenericLauncherName = "oLauncher.exe";

char* oVUGetLauncherName(char* _StrDestination, size_t _SizeofStrDestination)
{
	oStringPath LauncherPath;
	oSystemGetPath(LauncherPath, oSYSPATH_APP);
	oStrAppendf(LauncherPath, "../");

	oStringM ModuleName;
	oVERIFY(oVUGetModuleShortName(ModuleName));

	// keep big fixed strings off stack
	oStringPath tmp;
	std::vector<std::string> Paths;
	oPrintf(tmp, "%s%s%s", LauncherPath.c_str(), oMODULE_DEBUG_PREFIX_A, ModuleName.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s%s", LauncherPath.c_str(), oMODULE_DEBUG_PREFIX_A, sGenericLauncherName);
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s", LauncherPath.c_str(), ModuleName.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s", LauncherPath.c_str(), sGenericLauncherName);
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oFOR(const auto& path, Paths)
	{
		if (oStreamExists(path.c_str()))
		{
			oStrcpy(_StrDestination, _SizeofStrDestination, path.c_str());
			return _StrDestination;
		}
	}

	oVUNotFound(Paths, Paths.cend());
	return nullptr;
}

template<size_t size> char* oVUGetLauncherName(char (&_StrDestination)[size]) { return oVUGetLauncherName(_StrDestination, size); }
template<size_t capacity> char* oVUGetLauncherName(oFixedString<char, capacity>& _StrDestination) { return oVUGetLauncherName(_StrDestination, _StrDestination.capacity()); }

bool oVULaunchLauncher(unsigned int _ExpectedTimeToShutdownMS, const oVersion& _Version)
{
	oStringXL CmdLine;
	if (!oVUGetLauncherName(CmdLine))
		return false; // pass through error

	const char* filebase = oGetFilebase(CmdLine);
	if (strstr(filebase, sGenericLauncherName))
	{
		// Strip any prefix due to debug (this complicates user-specified prefixes)
		oStringM ModuleName;
		oVERIFY(oVUGetModuleShortName(ModuleName));

		oStringM SpecificModuleName;
		char* p = strstr(ModuleName, oMODULE_DEBUG_PREFIX_A);
		if (p)
			SpecificModuleName = p + oStrlen(oMODULE_DEBUG_PREFIX_A);
		else
			SpecificModuleName = ModuleName;

		// @oooii-tony: this is probably reason enough to encapsulate the cmdine
		// params inside oVersionUpdate.cpp...
		oStrAppendf(CmdLine, " -e\"%s\"", SpecificModuleName.c_str());
	}

	oStringS ForcedVersion;
	if (_Version.IsValid())
	{
		oStringS StrVer;
		oPrintf(ForcedVersion, "-v %s", oToString(StrVer, _Version));
	}

	oStrAppendf(CmdLine, " -w %u -t %u %s", oProcessGetCurrentID(), _ExpectedTimeToShutdownMS, ForcedVersion.c_str());

	oStringXL ModuleCmdLine;
	oProcessGetCommandLine(ModuleCmdLine, true);
	if (!ModuleCmdLine.empty())
		oStrAppendf(CmdLine, " -c\"%s\"", ModuleCmdLine.c_str());

	return oVUFireAndForget(CmdLine);
}

bool oVUUnzip(const char* _SfxURI)
{
	oURIParts URIParts;
	if (!oURIDecompose(_SfxURI, &URIParts))
		return false; // pass through error

	if (oStricmp(URIParts.Scheme, "file"))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "invalid scheme: %s", oSAFESTRN(_SfxURI));

	oStringPath Path, TargetDir;
	oSystemURIPartsToPath(Path, URIParts);

	// Extract to ./ wherever the installer is
	TargetDir = Path;
	*oGetFilebase(TargetDir) = 0;

	oStringL CmdLine;
	oPrintf(CmdLine, "%s x -y -o\"%s\"", Path.c_str(), TargetDir.c_str());
	oStringXXL StdOut;
	int ExitCode = 0;
	const unsigned int kTwoMinutes = 2 * 60 * 1000;
	if (!oSystemExecute(CmdLine, StdOut, &ExitCode, kTwoMinutes))
		return false; // pass through error

	if (ExitCode)
		return oErrorSetLast(oERROR_GENERIC, "%s", StdOut.c_str());

	return true;
}

static bool oVUEnumVersionFolders(const char* _FullPath, const oSTREAM_DESC& _Desc, std::vector<oStringM>* _pVersionFolders)
{
	const char* ver = oGetFilebase(_FullPath);
	oVersion v;
	if (oFromString(&v, ver))
		_pVersionFolders->push_back(ver);
	return true;
}

static bool oVUFindVersionPath(oStringPath& _StrPath, const char* _SpecificVersion)
{
	oSystemGetPath(_StrPath, oSYSPATH_APP);
	if (oSTRVALID(_SpecificVersion))
	{
		oStrAppendf(_StrPath, "%s/", _SpecificVersion);
		if (!oStreamExists(_StrPath))
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "not found: specified version folder \"%s\"", _StrPath.c_str());
	}

	else
	{
		// Search for most recent version folder
		oStringPath Wildcard;
		oPrintf(Wildcard, "%s*",_StrPath.c_str()); // all folders
		std::vector<oStringM> VersionFolders;
		VersionFolders.reserve(10);
		if (!oFileEnum(Wildcard, oBIND(oVUEnumVersionFolders, oBIND1, oBIND2, &VersionFolders)))
			return false; // pass through error

		// if empty, leave _StrPath alone, thus having the .\ path in it described 
		// above
		if (!VersionFolders.empty())
		{
			std::sort(VersionFolders.begin(), VersionFolders.end(), [=](const oStringS& _S1, const oStringS& _S2)->bool { return oStricmp(_S1, _S2) < 1; });
			oStringS StrVer;
			oStrAppendf(_StrPath, "%s/", VersionFolders.back().c_str());
		}
	}
	
	if (!oCleanPath(_StrPath, _StrPath))
		return false; // pass thr
	return true;
}

static bool oVUFindExePath(oStringPath& _ExePath, const oStringPath& _VersionPath, const oVERSIONED_LAUNCH_DESC& _Desc)
{
	// Now check for:
	// 0. If no specified module name, use launcher name
	// 1. Specified module name with prefix (if valid)
	// 2. Specific module name without prefix

	oStringM ModuleName;
	if (oSTRVALID(_Desc.SpecificModuleName))
	{
		ModuleName = _Desc.SpecificModuleName;
		if (!oSTRVALID(oGetFileExtension(ModuleName)))
			oStrAppendf(ModuleName, ".exe");
	}
	else
		oVERIFY(oVUGetModuleShortName(ModuleName));

	const char* Prefix = oSAFESTR(_Desc.ModuleNamePrefix);

	// Keep the stack small...
	std::vector<std::string> Paths;
	oStringPath tmp;

	// This is the evaluation order: All things debug first...

	oPrintf(tmp, "%s%s" oMODULE_DEBUG_PREFIX_A "%s", _VersionPath.c_str(), Prefix, ModuleName.c_str());
	
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s" oMODULE_DEBUG_PREFIX_A "%s", _VersionPath.c_str(), "", ModuleName.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	if (!oSTRVALID(_Desc.SpecificVersion))
	{
		oPrintf(tmp, "%s" oMODULE_DEBUG_PREFIX_A "%s", Prefix, ModuleName.c_str());
		Paths.push_back(std::string(oCleanPath(tmp, tmp)));

		oPrintf(tmp, "%s" oMODULE_DEBUG_PREFIX_A "%s", "", ModuleName.c_str());
		Paths.push_back(std::string(oCleanPath(tmp, tmp)));
	}

	// release...
	oPrintf(tmp, "%s%s%s", _VersionPath.c_str(), Prefix, ModuleName.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s%s", _VersionPath.c_str(), "", ModuleName.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	if (!oSTRVALID(_Desc.SpecificVersion))
	{
		oPrintf(tmp, "%s%s", Prefix, ModuleName.c_str());
		Paths.push_back(std::string(oCleanPath(tmp, tmp)));

		oPrintf(tmp, "%s%s", "", ModuleName.c_str());
		Paths.push_back(std::string(oCleanPath(tmp, tmp)));
	}

	// reduce duplicates
	auto it = Paths.begin() + 1;
	while (it != Paths.end())
	{
		auto itPrev = it - 1;

		if (!oStricmp(itPrev->c_str(), it->c_str()))
			it = Paths.erase(it);
		else
			++it;
	}

	oStringPath ThisModuleName;
	oVERIFY(oModuleGetName(ThisModuleName));
	oCleanPath(ThisModuleName, ThisModuleName);
	std::vector<std::string>::const_iterator itSelf = Paths.cend();
	
	oFOR(const auto& path, Paths)
	{
		if (oStricmp(ThisModuleName, path.c_str()) && oStreamExists(path.c_str())) // don't execute self (infinite loop)
		{
			_ExePath = path.c_str();
			return true;
		}
	}

	return oVUNotFound(Paths, itSelf);
}

bool oVURelaunch(const oVERSIONED_LAUNCH_DESC& _Desc)
{
	// Resolve the path we're going to execute. Either use the .\ dir, or a subdir
	// of .\ that is the FFI version number string. So first get the path to the 
	// version files to check for the exe.

	oStringPath VersionPath;
	if (!oVUFindVersionPath(VersionPath, _Desc.SpecificVersion))
		return false; // pass through error

	oStringPath ExePath;
	if (!oVUFindExePath(ExePath, VersionPath, _Desc))
		return false; // pass through error

	// After all this, VersionPath has the full path to the new exe to spawn

	bool KeepChecking = true;
	while (_Desc.WaitForPID && KeepChecking)
	{
		oStringPath ProcessName;
		if (!oProcessGetName(ProcessName, _Desc.WaitForPID))
			ProcessName = "???";

		oTRACE("Waiting for process exit %d %s (timeout = %.02f sec)...", _Desc.WaitForPID, ProcessName.c_str(), _Desc.WaitForPIDTimeout * 1000.0f);
		if (oProcessWaitExit(_Desc.WaitForPID, _Desc.WaitForPIDTimeout))
			KeepChecking = false;
		else
		{
			oTRACE("Lasterr: %s", oErrorGetLastString());
			if (!oProcessHasDebuggerAttached(_Desc.WaitForPID))
				oProcessTerminate(_Desc.WaitForPID, 0xdeadbeef);
		}

		if (!oProcessHasDebuggerAttached(oProcessGetCurrentID()))
		{
			oSleep(1000);
			KeepChecking = false;
		}
	}

	oStringXL CmdLine;
	if (oSTRVALID(_Desc.PassThroughCommandLine))
		oPrintf(CmdLine, "%s %s", ExePath.c_str(), _Desc.PassThroughCommandLine);
	else
		oPrintf(CmdLine, "%s", ExePath.c_str());
	
	return oVUFireAndForget(CmdLine);
}
