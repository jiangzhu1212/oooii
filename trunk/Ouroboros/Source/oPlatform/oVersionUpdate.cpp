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
#include <oStd/algorithm.h>
#include <oStd/fixed_string.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oSystem.h>

static std::regex reAppName("(.+?)(" oMODULE_DEBUG_SUFFIX_A ")?(?:-([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+))?(\\.exe)(.*)", std::regex_constants::optimize);

bool oVUDecompose(const oStd::uri_string& _URI, bool _VersionFromFilename, oVU_URI_PARTS* _pParts)
{
	if (!oURIDecompose(_URI, &_pParts->URIParts))
		return false; // pass through error

	oStd::path_string FilePath;
	if (!oURIToPath(FilePath, _URI))
	{
		// @oooii-tony: Revisit this... oURIToPath changed.
		if (FilePath.empty())
		{
			oURI uri = _URI;
			FilePath = uri.Path();
		}

		//return oErrorSetLast(std::errc::protocol_error, "cannot convert URI to path: %s", _URI.c_str());
	}

	oMODULE_DESC md;
	if (!_VersionFromFilename)
	{
		if (oModuleGetDesc(FilePath, &md))
			_pParts->Version = md.ProductVersion;
		else
			return false; // pass through error
	}

	oStd::mstring Filename = oGetFilebase(FilePath);
	oTrimFilename(FilePath);
	oStd::uri_string FilePathURI;
	oURIFromAbsolutePath(FilePathURI, FilePath);
	oURIDecompose(FilePathURI, &_pParts->URIParts);

	std::cmatch matches;
	if (!regex_match(Filename.c_str(), matches, reAppName))
		return oErrorSetLast(std::errc::invalid_argument, "malformed app/installer name: %s", _URI.c_str());

	_pParts->IsDebugBuild = matches.length(2) != 0;
	_pParts->Filebase.assign(matches[1].first, matches[1].second);
	_pParts->Extension.assign(matches[4].first, matches[4].second);

	if (_VersionFromFilename)
	{
		if (matches.length(3))
		{
			oStd::sstring StrVersion(matches[3].first, matches[3].second);
			oVERIFY(oStd::from_string(&_pParts->Version, StrVersion)); // the regex ensures this is correct
		}
		else
			return oErrorSetLast(std::errc::invalid_argument, "version could not be extracted");
	}

	return true;
}

static bool oVURecomposePath(oURIParts& _URIParts, const oVU_URI_PARTS& _Parts)
{
	const char* BuildSuffix = _Parts.IsDebugBuild ? oMODULE_DEBUG_SUFFIX_A : "";
	oStd::uri_string FilePath;
	oURIRecompose(FilePath, _URIParts);
	if (_Parts.Version.IsValid())
	{
		oStd::sstring ver;
		oStrAppendf(FilePath, "%s%s-%s%s", _Parts.Filebase.c_str(), BuildSuffix, oStd::to_string(ver, _Parts.Version), _Parts.Extension.c_str());
	}
	else
		oStrAppendf(FilePath, "%s%s%s", _Parts.Filebase.c_str(), BuildSuffix, _Parts.Extension.c_str());

	oURIDecompose(FilePath, &_URIParts);
	return true;
}

char* oVURecompose(oStd::uri_string& _URI, const oVU_URI_PARTS& _Parts)
{
	oURIParts p = _Parts.URIParts;
	if (!oVURecomposePath(p, _Parts))
		return false; // pass through error
	return oURIRecompose(_URI, p);
}

bool oVUGetThisAppsInstallerFilename(oStd::mstring& _StrDestination)
{
	oStd::uri_string AppURI;
	oVERIFY(oSystemGetURI(AppURI, oSYSPATH_APP_FULL));

	// by decomposing and recomposing, we've included the version into the exe
	// name for the installer's full name.
	oVU_URI_PARTS parts;
	oVERIFY(oVUDecompose(AppURI, false, &parts));
	oVERIFY(oVURecomposePath(parts.URIParts, parts));
	oStd::uri_string FilePathURI;
	oURIRecompose(FilePathURI, parts.URIParts);
	oStd::path_string FilePath;
	oURIToPath(FilePath, FilePathURI);
	_StrDestination = oGetFilebase(FilePath);
	return true;
}

static bool oVUEnumInstallers(const char* _FullPath, const oSTREAM_DESC& _Desc, std::vector<oVU_URI_PARTS>* _pInstallers)
{
	oStd::uri_string URI;
	oVERIFY(oURIFromAbsolutePath(URI, _FullPath));
	oVU_URI_PARTS parts;
	if (oVUDecompose(_FullPath, true, &parts))
		_pInstallers->push_back(parts);
	return true;
}

static bool oVUGetLatestInstallerFilename(oStd::mstring& _StrDestination, const char* _InstallersFolder)
{
	oStd::path_string Path;
	oPrintf(Path, "%s/*", _InstallersFolder); // all folders
	std::vector<oVU_URI_PARTS> Installers;
	Installers.reserve(10);
	if (!oFileEnum(Path, oBIND(oVUEnumInstallers, oBIND1, oBIND2, &Installers)))
		return false; // pass through error

	if (Installers.empty())
		return oErrorSetLast(std::errc::no_such_file_or_directory, "No installers found");
	
	std::sort(Installers.begin(), Installers.end(), [=](const oVU_URI_PARTS& _S1, const oVU_URI_PARTS& _S2)->bool { return _S1.Version < _S2.Version; });
	
	oStd::uri_string LatestURI;
	oVERIFY(oVURecompose(LatestURI, Installers.back()));

	oURIParts URIParts;
	oVERIFY(oURIDecompose(LatestURI, &URIParts));
	oVERIFY(oSystemURIPartsToPath(Path, URIParts));
	_StrDestination = oGetFilebase(Path);
	return true;
}

bool oVUGetLatestInstallerFilename(oStd::mstring& _StrDestination)
{
	oStd::path_string AppPath;
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
		oStd::mstring ThisInstallerFilename;
		oVERIFY(oVUGetThisAppsInstallerFilename(ThisInstallerFilename));
		oVERIFY(oVUDecompose(ThisInstallerFilename, true, &DestParts));
	}

	DestParts.Version = SourceParts.Version = oVersion();
	DestParts.Extension = SourceParts.Extension;
	DestParts.URIParts = SourceParts.URIParts;

	oStd::uri_string S, D;
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
	oStd::path_string ModuleName;
	oVERIFY(oModuleGetName(ModuleName));
	oCleanPath(ModuleName, ModuleName);

	std::cmatch ModuleNameMatches;
	if (!regex_match(ModuleName.c_str(), ModuleNameMatches, reAppName))
		return oErrorSetLast(std::errc::invalid_argument, "malformed module name: %s", ModuleName.c_str());

	std::cmatch CommandLineMatches;
	if (!regex_match(_CommandLine, CommandLineMatches, reAppName))
		return oErrorSetLast(std::errc::invalid_argument, "malformed command line: %s", _CommandLine);

	// Equal test the filename(1), version(3) and extension(4)
	if ((ModuleNameMatches.length(1) == CommandLineMatches.length(1) && 0 == _memicmp(ModuleNameMatches[1].first, CommandLineMatches[1].first, ModuleNameMatches.length(1))) &&
		(ModuleNameMatches.length(3) == CommandLineMatches.length(3) && 0 == _memicmp(ModuleNameMatches[3].first, CommandLineMatches[3].first, ModuleNameMatches.length(3))) &&
		(ModuleNameMatches.length(4) == CommandLineMatches.length(4) && 0 == _memicmp(ModuleNameMatches[4].first, CommandLineMatches[4].first, ModuleNameMatches.length(4))))
		return oErrorSetLast(std::errc::operation_in_progress, "Asked to execute self, this will create an infinite loop.");

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
	return oErrorSetLast(std::errc::no_such_file_or_directory, "not found:\n%s", all.c_str());
}

bool oVUIsUpdateInstallerValid(const oStd::uri_string& _SfxURI, bool _IsNewer /*= true*/, bool _MatchInstallerAndExecutableNames /*= false*/)
{
	oVU_URI_PARTS SfxParts;
	if (!oVUDecompose(_SfxURI, true, &SfxParts))
		return false; // pass through error
	
	oStd::path_string AppURI;
	oVU_URI_PARTS AppParts;
	oVERIFY(oSystemGetURI(AppURI, oSYSPATH_APP_FULL));
	if (!oVUDecompose(AppURI, false, &AppParts))
		return false; // pass through error

	if (!SfxParts.Version.IsValid() || (_MatchInstallerAndExecutableNames && oStricmp(SfxParts.Filebase, AppParts.Filebase)))
		return oErrorSetLast(std::errc::invalid_argument, "ignoring %s: not an installer for process %s", _SfxURI.c_str(), AppParts.Filebase.c_str());

	if ((_IsNewer && (SfxParts.Version < AppParts.Version)) ||
		(!_IsNewer && (SfxParts.Version == AppParts.Version)))
	{
		oStd::sstring ver;
		return oErrorSetLast(std::errc::invalid_argument, "ignoring %s: The current version is up-to-date at %s", _SfxURI.c_str(), oStd::to_string(ver, AppParts.Version));
	}

	return true;
}

static bool oVUGetModuleShortName(oStd::mstring& _ModuleShortName)
{
	oStd::path_string ModulePath;
	oModuleGetName(ModulePath);
	oGetFilebase(_ModuleShortName, ModulePath.c_str());
	size_t suffixsize = oStrlen(oMODULE_DEBUG_SUFFIX_A);
	if (0 == memcmp(_ModuleShortName.c_str() + _ModuleShortName.size() - suffixsize, oMODULE_DEBUG_SUFFIX_A, suffixsize))
		*(_ModuleShortName.c_str() + _ModuleShortName.size() - suffixsize) = 0;
	return true;
}

static const char* sGenericLauncherName = "oLauncher";

char* oVUGetLauncherName(char* _StrDestination, size_t _SizeofStrDestination)
{
	oStd::path_string LauncherPath;
	oSystemGetPath(LauncherPath, oSYSPATH_APP);
	oStrAppendf(LauncherPath, "../");

	oStd::mstring ModuleName;
	oVERIFY(oVUGetModuleShortName(ModuleName));

	// keep big fixed strings off stack
	oStd::path_string tmp;
	std::vector<std::string> Paths;
	oPrintf(tmp, "%s%s%s.exe", LauncherPath.c_str(), ModuleName.c_str(), oMODULE_DEBUG_SUFFIX_A);
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s%s.exe", LauncherPath.c_str(), sGenericLauncherName, oMODULE_DEBUG_SUFFIX_A);
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s.exe", LauncherPath.c_str(), ModuleName.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s.exe", LauncherPath.c_str(), sGenericLauncherName);
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
template<size_t capacity> char* oVUGetLauncherName(oStd::fixed_string<char, capacity>& _StrDestination) { return oVUGetLauncherName(_StrDestination, _StrDestination.capacity()); }

bool oVULaunchLauncher(unsigned int _ExpectedTimeToShutdownMS, const oVersion& _Version)
{
	oStd::xlstring CmdLine;
	if (!oVUGetLauncherName(CmdLine))
		return false; // pass through error

	const char* filebase = oGetFilebase(CmdLine);
	if (strstr(filebase, sGenericLauncherName))
	{
		// Strip any suffix due to debug (this complicates user-specified prefixes)
		oStd::mstring ModuleName;
		oVERIFY(oVUGetModuleShortName(ModuleName));

		// @oooii-tony: this is probably reason enough to encapsulate the cmdline
		// params inside oVersionUpdate.cpp...
		oStrAppendf(CmdLine, " -e\"%s\"", ModuleName.c_str());
	}

	oStd::sstring ForcedVersion;
	if (_Version.IsValid())
	{
		oStd::sstring StrVer;
		oPrintf(ForcedVersion, "-v %s", oStd::to_string(StrVer, _Version));
	}

	oStrAppendf(CmdLine, " -w %u -t %u %s", oProcessGetCurrentID(), _ExpectedTimeToShutdownMS, ForcedVersion.c_str());

	oStd::xlstring ModuleCmdLine;
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
		return oErrorSetLast(std::errc::invalid_argument, "invalid scheme: %s", oSAFESTRN(_SfxURI));

	oStd::path_string Path, TargetDir;
	oSystemURIPartsToPath(Path, URIParts);

	// Extract to ./ wherever the installer is
	TargetDir = Path;
	*oGetFilebase(TargetDir) = 0;

	oStd::lstring CmdLine;
	oPrintf(CmdLine, "%s x -y -o\"%s\"", Path.c_str(), TargetDir.c_str());
	oStd::fixed_string<char, oKB(16)> StdOut;
	int ExitCode = 0;
	const unsigned int kTwoMinutes = 2 * 60 * 1000;
	if (!oSystemExecute(CmdLine, StdOut, &ExitCode, kTwoMinutes))
		return false; // pass through error

	if (ExitCode)
	{
		oStd::path_string Path;
		oSystemGetPath(Path, oSYSPATH_APP);
		oStrAppendf(Path, "oYUVUnzip-Error.log");
		oFileSave(Path, StdOut.c_str(), StdOut.size(), true, false);

		return oErrorSetLast(std::errc::protocol_error, "7ZSfx exited with code %d: %s", ExitCode, StdOut.c_str());
	}

	return true;
}

static bool oVUEnumVersionFolders(const char* _FullPath, const oSTREAM_DESC& _Desc, std::vector<oStd::mstring>* _pVersionFolders)
{
	const char* ver = oGetFilebase(_FullPath);
	oVersion v;
	if (oStd::from_string(&v, ver))
		_pVersionFolders->push_back(ver);
	return true;
}

static bool oVUFindVersionPath(oStd::path_string& _StrPath, const char* _SpecificVersion)
{
	oSystemGetPath(_StrPath, oSYSPATH_APP);
	if (oSTRVALID(_SpecificVersion))
	{
		oStrAppendf(_StrPath, "%s/", _SpecificVersion);
		if (!oStreamExists(_StrPath))
			return oErrorSetLast(std::errc::invalid_argument, "not found: specified version folder \"%s\"", _StrPath.c_str());
	}

	else
	{
		// Search for most recent version folder
		oStd::path_string Wildcard;
		oPrintf(Wildcard, "%s*",_StrPath.c_str()); // all folders
		std::vector<oStd::mstring> VersionFolders;
		VersionFolders.reserve(10);
		if (!oFileEnum(Wildcard, oBIND(oVUEnumVersionFolders, oBIND1, oBIND2, &VersionFolders)))
			return false; // pass through error

		// if empty, leave _StrPath alone, thus having the .\ path in it described 
		// above
		if (!VersionFolders.empty())
		{
			std::sort(VersionFolders.begin(), VersionFolders.end(), [=](const oStd::sstring& _S1, const oStd::sstring& _S2)->bool { return oStricmp(_S1, _S2) < 1; });
			oStd::sstring StrVer;
			oStrAppendf(_StrPath, "%s/", VersionFolders.back().c_str());
		}
	}
	
	if (!oCleanPath(_StrPath, _StrPath))
		return false; // pass thr
	return true;
}

static bool oVUFindExePath(oStd::path_string& _ExePath, const oStd::path_string& _VersionPath, const oVERSIONED_LAUNCH_DESC& _Desc)
{
	// Now check for:
	// 0. If no specified module name, use launcher name
	// 1. Specified module name with prefix (if valid)
	// 2. Specific module name without prefix

	oStd::mstring ModuleName;
	if (oSTRVALID(_Desc.SpecificModuleName))
	{
		ModuleName = _Desc.SpecificModuleName;
	}
	else
	{
		oVERIFY(oVUGetModuleShortName(ModuleName));
	}

	oStd::mstring ModuleNameMinusExtension;
	oStd::sstring ModuleNameExtension;
	oGetFilebase(ModuleNameMinusExtension, ModuleName.c_str());
	ModuleNameExtension = oGetFileExtension(ModuleName.c_str());
	if (ModuleNameExtension.empty())
		ModuleNameExtension = ".exe";

	const char* Prefix = oSAFESTR(_Desc.ModuleNamePrefix);

	// Keep the stack small...
	std::vector<std::string> Paths;
	oStd::path_string tmp;

	// This is the evaluation order: All things debug first...

	oPrintf(tmp, "%s%s%s" oMODULE_DEBUG_SUFFIX_A "%s", _VersionPath.c_str(), Prefix, ModuleNameMinusExtension.c_str(), ModuleNameExtension.c_str());
	
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s%s" oMODULE_DEBUG_SUFFIX_A "%s", _VersionPath.c_str(), "", ModuleNameMinusExtension.c_str(), ModuleNameExtension.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	if (!oSTRVALID(_Desc.SpecificVersion))
	{
		oPrintf(tmp, "%s%s" oMODULE_DEBUG_SUFFIX_A "%s", Prefix, ModuleNameMinusExtension.c_str(), ModuleNameExtension.c_str());
		Paths.push_back(std::string(oCleanPath(tmp, tmp)));

		oPrintf(tmp, "%s%s" oMODULE_DEBUG_SUFFIX_A "%s", "", ModuleNameMinusExtension.c_str(), ModuleNameExtension.c_str());
		Paths.push_back(std::string(oCleanPath(tmp, tmp)));
	}

	// release...
	oPrintf(tmp, "%s%s%s%s", _VersionPath.c_str(), Prefix, ModuleName.c_str(), ModuleNameExtension.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	oPrintf(tmp, "%s%s%s%s", _VersionPath.c_str(), "", ModuleName.c_str(), ModuleNameExtension.c_str());
	Paths.push_back(std::string(oCleanPath(tmp, tmp)));

	if (!oSTRVALID(_Desc.SpecificVersion))
	{
		oPrintf(tmp, "%s%s%s", Prefix, ModuleName.c_str(), ModuleNameExtension.c_str());
		Paths.push_back(std::string(oCleanPath(tmp, tmp)));

		oPrintf(tmp, "%s%s%s", "", ModuleName.c_str(), ModuleNameExtension.c_str());
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

	oStd::path_string ThisModuleName;
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

	oStd::path_string VersionPath;
	if (!oVUFindVersionPath(VersionPath, _Desc.SpecificVersion))
		return false; // pass through error

	oStd::path_string ExePath;
	if (!oVUFindExePath(ExePath, VersionPath, _Desc))
		return false; // pass through error

	// After all this, VersionPath has the full path to the new exe to spawn

	bool KeepChecking = true;
	while (_Desc.WaitForPID && KeepChecking)
	{
		oStd::path_string ProcessName;
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

	oStd::xlstring CmdLine;
	if (oSTRVALID(_Desc.PassThroughCommandLine))
		oPrintf(CmdLine, "%s %s", ExePath.c_str(), _Desc.PassThroughCommandLine);
	else
		oPrintf(CmdLine, "%s", ExePath.c_str());
	
	return oVUFireAndForget(CmdLine);
}
