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
#include <oPlatform/oVersionUpdate.h>
#include <oPlatform/oStream.h>
#include <oBase/algorithm.h>
#include <oBase/fixed_string.h>

using namespace ouro;

char* oSystemURIPartsToPath(char* _Path, size_t _SizeofPath, const oURIParts& _URIParts);
template<size_t size> char* oSystemURIPartsToPath(char (&_ResultingFullPath)[size], const oURIParts& _URIParts) { return oSystemURIPartsToPath(_ResultingFullPath, size, _URIParts); }
template<size_t capacity> char* oSystemURIPartsToPath(ouro::fixed_string<char, capacity>& _ResultingFullPath, const oURIParts& _URIParts) { return oSystemURIPartsToPath(_ResultingFullPath, _ResultingFullPath.capacity(), _URIParts); }

static std::regex reAppName("(.+?)(" oMODULE_DEBUG_SUFFIX_A ")?(?:-([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+))?(\\.exe)(.*)", std::regex_constants::optimize);

bool oVUDecompose(const uri_string& _URI, bool _VersionFromFilename, oVU_URI_PARTS* _pParts)
{
	if (!oURIDecompose(_URI, &_pParts->URIParts))
		return false; // pass through error

	path_string FilePath;
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

	ouro::module::info md;
	if (!_VersionFromFilename)
	{
		ouro::module::info mi = ouro::module::get_info(path(FilePath));
		_pParts->Version = mi.version;
	}

	mstring Filename = oGetFilebase(FilePath);
	oTrimFilename(FilePath);
	uri_string FilePathURI;
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
			sstring StrVersion(matches[3].first, matches[3].second);
			oVERIFY(from_string(&_pParts->Version, StrVersion)); // the regex ensures this is correct
		}
		else
			return oErrorSetLast(std::errc::invalid_argument, "version could not be extracted");
	}

	return true;
}

static bool oVURecomposePath(oURIParts& _URIParts, const oVU_URI_PARTS& _Parts)
{
	const char* BuildSuffix = _Parts.IsDebugBuild ? oMODULE_DEBUG_SUFFIX_A : "";
	uri_string FilePath;
	oURIRecompose(FilePath, _URIParts);
	if (_Parts.Version != version())
	{
		sstring ver;
		sncatf(FilePath, "%s%s-%s%s", _Parts.Filebase.c_str(), BuildSuffix, to_string(ver, _Parts.Version), _Parts.Extension.c_str());
	}
	else
		sncatf(FilePath, "%s%s%s", _Parts.Filebase.c_str(), BuildSuffix, _Parts.Extension.c_str());

	oURIDecompose(FilePath, &_URIParts);
	return true;
}

char* oVURecompose(uri_string& _URI, const oVU_URI_PARTS& _Parts)
{
	oURIParts p = _Parts.URIParts;
	if (!oVURecomposePath(p, _Parts))
		return false; // pass through error
	return oURIRecompose(_URI, p);
}

bool oVUGetThisAppsInstallerFilename(mstring& _StrDestination)
{
	uri AppURI(ouro::filesystem::app_path(true));

	// by decomposing and recomposing, we've included the version into the exe
	// name for the installer's full name.
	oVU_URI_PARTS parts;
	oVERIFY(oVUDecompose(AppURI, false, &parts));
	oVERIFY(oVURecomposePath(parts.URIParts, parts));
	uri_string FilePathURI;
	oURIRecompose(FilePathURI, parts.URIParts);
	path_string FilePath;
	oURIToPath(FilePath, FilePathURI);
	_StrDestination = oGetFilebase(FilePath);
	return true;
}

static bool oVUGetLatestInstallerFilename(mstring& _StrDestination, const char* _InstallersFolder)
{
	path Path(_InstallersFolder);
	Path /= "/*";
	std::vector<oVU_URI_PARTS> Installers;
	Installers.reserve(10);

	ouro::filesystem::enumerate(Path, [&](const path& _FullPath, const ouro::filesystem::file_status& _Status, unsigned long long _Size)->bool
	{
		uri_string URI;
		oVERIFY(oURIFromAbsolutePath(URI, _FullPath));
		oVU_URI_PARTS parts;
		if (oVUDecompose(_FullPath, true, &parts))
			Installers.push_back(parts);
		
		return true;
	});

	if (Installers.empty())
		return oErrorSetLast(std::errc::no_such_file_or_directory, "No installers found");
	
	std::sort(Installers.begin(), Installers.end(), [=](const oVU_URI_PARTS& _S1, const oVU_URI_PARTS& _S2)->bool { return _S1.Version < _S2.Version; });
	
	uri_string LatestURI;
	oVERIFY(oVURecompose(LatestURI, Installers.back()));

	oURIParts URIParts;
	oVERIFY(oURIDecompose(LatestURI, &URIParts));
	Path = URIParts.Path;
	_StrDestination = Path.filename().c_str();
	return true;
}

bool oVUGetLatestInstallerFilename(mstring& _StrDestination)
{
	path AppPath = ouro::filesystem::app_path();
	AppPath /= "../";
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
		mstring ThisInstallerFilename;
		oVERIFY(oVUGetThisAppsInstallerFilename(ThisInstallerFilename));
		oVERIFY(oVUDecompose(ThisInstallerFilename, true, &DestParts));
	}

	DestParts.Version = SourceParts.Version = version();
	DestParts.Extension = SourceParts.Extension;
	DestParts.URIParts = SourceParts.URIParts;

	uri_string S, D;
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
	path ModuleName = ouro::this_module::path();
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

	ouro::process::info pi;
	pi.command_line = _CommandLine;
	pi.show = ouro::process::minimized;
	oTRACE("Spawning process: %s", _CommandLine);
	auto NewProcess = ouro::process::make(pi);
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

bool oVUIsUpdateInstallerValid(const uri_string& _SfxURI, bool _IsNewer /*= true*/, bool _MatchInstallerAndExecutableNames /*= false*/)
{
	oVU_URI_PARTS SfxParts;
	if (!oVUDecompose(_SfxURI, true, &SfxParts))
		return false; // pass through error
	
	uri AppURI(ouro::filesystem::app_path(true));
	oVU_URI_PARTS AppParts;
	if (!oVUDecompose(AppURI, false, &AppParts))
		return false; // pass through error

	if (SfxParts.Version != version() || (_MatchInstallerAndExecutableNames && _stricmp(SfxParts.Filebase, AppParts.Filebase)))
		return oErrorSetLast(std::errc::invalid_argument, "ignoring %s: not an installer for process %s", _SfxURI.c_str(), AppParts.Filebase.c_str());

	if ((_IsNewer && (SfxParts.Version < AppParts.Version)) ||
		(!_IsNewer && (SfxParts.Version == AppParts.Version)))
	{
		sstring ver;
		return oErrorSetLast(std::errc::invalid_argument, "ignoring %s: The current version is up-to-date at %s", _SfxURI.c_str(), to_string(ver, AppParts.Version));
	}

	return true;
}

static bool oVUGetModuleShortName(mstring& _ModuleShortName)
{
	path ModulePath = ouro::this_module::path();
	_ModuleShortName = ModulePath.filename().c_str();
	size_t suffixsize = strlen(oMODULE_DEBUG_SUFFIX_A);
	if (0 == memcmp(_ModuleShortName.c_str() + _ModuleShortName.size() - suffixsize, oMODULE_DEBUG_SUFFIX_A, suffixsize))
		*(_ModuleShortName.c_str() + _ModuleShortName.size() - suffixsize) = 0;
	return true;
}

static const char* sGenericLauncherName = "oLauncher";

char* oVUGetLauncherName(char* _StrDestination, size_t _SizeofStrDestination)
{
	path LauncherPath = ouro::filesystem::app_path();
	LauncherPath /= "../";

	mstring ModuleName;
	oVERIFY(oVUGetModuleShortName(ModuleName));

	// keep big fixed strings off stack
	path_string tmp;
	std::vector<std::string> Paths;
	snprintf(tmp, "%s%s%s.exe", LauncherPath.c_str(), ModuleName.c_str(), oMODULE_DEBUG_SUFFIX_A);
	Paths.push_back(std::string(clean_path(tmp, tmp)));

	snprintf(tmp, "%s%s%s.exe", LauncherPath.c_str(), sGenericLauncherName, oMODULE_DEBUG_SUFFIX_A);
	Paths.push_back(std::string(clean_path(tmp, tmp)));

	snprintf(tmp, "%s%s.exe", LauncherPath.c_str(), ModuleName.c_str());
	Paths.push_back(std::string(clean_path(tmp, tmp)));

	snprintf(tmp, "%s%s.exe", LauncherPath.c_str(), sGenericLauncherName);
	Paths.push_back(std::string(clean_path(tmp, tmp)));

	oFOR(const auto& path, Paths)
	{
		if (oStreamExists(path.c_str()))
		{
			strlcpy(_StrDestination, path.c_str(), _SizeofStrDestination);
			return _StrDestination;
		}
	}

	oVUNotFound(Paths, Paths.cend());
	return nullptr;
}

template<size_t size> char* oVUGetLauncherName(char (&_StrDestination)[size]) { return oVUGetLauncherName(_StrDestination, size); }
template<size_t capacity> char* oVUGetLauncherName(ouro::fixed_string<char, capacity>& _StrDestination) { return oVUGetLauncherName(_StrDestination, _StrDestination.capacity()); }

bool oVULaunchLauncher(unsigned int _ExpectedTimeToShutdownMS, const version& _Version)
{
	xlstring CmdLine;
	if (!oVUGetLauncherName(CmdLine))
		return false; // pass through error

	const char* filebase = oGetFilebase(CmdLine);
	if (strstr(filebase, sGenericLauncherName))
	{
		// Strip any suffix due to debug (this complicates user-specified prefixes)
		mstring ModuleName;
		oVERIFY(oVUGetModuleShortName(ModuleName));

		// @oooii-tony: this is probably reason enough to encapsulate the cmdline
		// params inside oVersionUpdate.cpp...
		sncatf(CmdLine, " -e\"%s\"", ModuleName.c_str());
	}

	sstring ForcedVersion;
	if (_Version != version())
	{
		sstring StrVer;
		snprintf(ForcedVersion, "-v %s", to_string(StrVer, _Version));
	}

	sncatf(CmdLine, " -w %u -t %u %s", ouro::this_process::get_id(), _ExpectedTimeToShutdownMS, ForcedVersion.c_str());

	xlstring ModuleCmdLine;
	ouro::this_process::command_line(ModuleCmdLine, true);
	if (!ModuleCmdLine.empty())
		sncatf(CmdLine, " -c\"%s\"", ModuleCmdLine.c_str());

	return oVUFireAndForget(CmdLine);
}

bool oVUUnzip(const char* _SfxURI)
{
	oURIParts URIParts;
	if (!oURIDecompose(_SfxURI, &URIParts))
		return false; // pass through error

	if (_stricmp(URIParts.Scheme, "file"))
		return oErrorSetLast(std::errc::invalid_argument, "invalid scheme: %s", oSAFESTRN(_SfxURI));

	path_string Path, TargetDir;
	oSystemURIPartsToPath(Path, URIParts);

	// Extract to ./ wherever the installer is
	TargetDir = Path;
	*oGetFilebase(TargetDir) = 0;

	lstring CmdLine;
	snprintf(CmdLine, "%s x -y -o\"%s\"", Path.c_str(), TargetDir.c_str());
	ouro::fixed_string<char, oKB(16)> StdOut;
	const unsigned int kTwoMinutes = 2 * 60 * 1000;
	int ExitCode = ouro::system::spawn(CmdLine, [&](char* _Line) { strlcat(StdOut, _Line, StdOut.capacity()); }, false, kTwoMinutes);
	if (ExitCode)
	{
		path Path = ouro::filesystem::app_path();
		Path /= "oYUVUnzip-Error.log";
		ouro::filesystem::save(Path, StdOut.c_str(), StdOut.size(), ouro::filesystem::save_option::text_write);

		return oErrorSetLast(std::errc::protocol_error, "7ZSfx exited with code %d: %s", ExitCode, StdOut.c_str());
	}

	return true;
}

static bool oVUFindVersionPath(path_string& _StrPath, const char* _SpecificVersion)
{
	_StrPath = ouro::filesystem::app_path();
	if (oSTRVALID(_SpecificVersion))
	{
		sncatf(_StrPath, "%s/", _SpecificVersion);
		if (!oStreamExists(_StrPath))
			return oErrorSetLast(std::errc::invalid_argument, "not found: specified version folder \"%s\"", _StrPath.c_str());
	}

	else
	{
		// Search for most recent version folder
		path Wildcard(_StrPath);
		Wildcard /= "/*";
		std::vector<mstring> VersionFolders;
		VersionFolders.reserve(10);
		ouro::filesystem::enumerate(Wildcard, [&](const path& _FullPath, const ouro::filesystem::file_status& _Status, unsigned long long _Size)->bool
		{
			path_string ver = _FullPath.basename();
			version v;
			if (from_string(&v, ver))
				VersionFolders.push_back(ver);
			return true;
		});

		// if empty, leave _StrPath alone, thus having the .\ path in it described 
		// above
		if (!VersionFolders.empty())
		{
			std::sort(VersionFolders.begin(), VersionFolders.end(), [=](const sstring& _S1, const sstring& _S2)->bool { return _stricmp(_S1, _S2) < 1; });
			sstring StrVer;
			sncatf(_StrPath, "%s/", VersionFolders.back().c_str());
		}
	}
	
	if (!clean_path(_StrPath, _StrPath))
		return false; // pass thr
	return true;
}

static bool oVUFindExePath(path_string& _ExePath, const path_string& _VersionPath, const oVERSIONED_LAUNCH_DESC& _Desc)
{
	// Now check for:
	// 0. If no specified module name, use launcher name
	// 1. Specified module name with prefix (if valid)
	// 2. Specific module name without prefix

	mstring ModuleName;
	if (oSTRVALID(_Desc.SpecificModuleName))
	{
		ModuleName = _Desc.SpecificModuleName;
	}
	else
	{
		oVERIFY(oVUGetModuleShortName(ModuleName));
	}

	mstring ModuleNameMinusExtension;
	sstring ModuleNameExtension;
	oGetFilebase(ModuleNameMinusExtension, ModuleName.c_str());
	ModuleNameExtension = oGetFileExtension(ModuleName.c_str());
	if (ModuleNameExtension.empty())
		ModuleNameExtension = ".exe";

	const char* Prefix = oSAFESTR(_Desc.ModuleNamePrefix);

	// Keep the stack small...
	std::vector<std::string> Paths;
	path_string tmp;

	// This is the evaluation order: All things debug first...

	snprintf(tmp, "%s%s%s" oMODULE_DEBUG_SUFFIX_A "%s", _VersionPath.c_str(), Prefix, ModuleNameMinusExtension.c_str(), ModuleNameExtension.c_str());
	
	Paths.push_back(std::string(clean_path(tmp, tmp)));

	snprintf(tmp, "%s%s%s" oMODULE_DEBUG_SUFFIX_A "%s", _VersionPath.c_str(), "", ModuleNameMinusExtension.c_str(), ModuleNameExtension.c_str());
	Paths.push_back(std::string(clean_path(tmp, tmp)));

	if (!oSTRVALID(_Desc.SpecificVersion))
	{
		snprintf(tmp, "%s%s" oMODULE_DEBUG_SUFFIX_A "%s", Prefix, ModuleNameMinusExtension.c_str(), ModuleNameExtension.c_str());
		Paths.push_back(std::string(clean_path(tmp, tmp)));

		snprintf(tmp, "%s%s" oMODULE_DEBUG_SUFFIX_A "%s", "", ModuleNameMinusExtension.c_str(), ModuleNameExtension.c_str());
		Paths.push_back(std::string(clean_path(tmp, tmp)));
	}

	// release...
	snprintf(tmp, "%s%s%s%s", _VersionPath.c_str(), Prefix, ModuleName.c_str(), ModuleNameExtension.c_str());
	Paths.push_back(std::string(clean_path(tmp, tmp)));

	snprintf(tmp, "%s%s%s%s", _VersionPath.c_str(), "", ModuleName.c_str(), ModuleNameExtension.c_str());
	Paths.push_back(std::string(clean_path(tmp, tmp)));

	if (!oSTRVALID(_Desc.SpecificVersion))
	{
		snprintf(tmp, "%s%s%s", Prefix, ModuleName.c_str(), ModuleNameExtension.c_str());
		Paths.push_back(std::string(clean_path(tmp, tmp)));

		snprintf(tmp, "%s%s%s", "", ModuleName.c_str(), ModuleNameExtension.c_str());
		Paths.push_back(std::string(clean_path(tmp, tmp)));
	}

	// reduce duplicates
	auto it = Paths.begin() + 1;
	while (it != Paths.end())
	{
		auto itPrev = it - 1;

		if (!_stricmp(itPrev->c_str(), it->c_str()))
			it = Paths.erase(it);
		else
			++it;
	}

	path ThisModuleName = ouro::this_module::path();
	std::vector<std::string>::const_iterator itSelf = Paths.cend();
	
	oFOR(const auto& path, Paths)
	{
		if (_stricmp(ThisModuleName, path.c_str()) && oStreamExists(path.c_str())) // don't execute self (infinite loop)
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

	path_string VersionPath;
	if (!oVUFindVersionPath(VersionPath, _Desc.SpecificVersion))
		return false; // pass through error

	path_string ExePath;
	if (!oVUFindExePath(ExePath, VersionPath, _Desc))
		return false; // pass through error

	// After all this, VersionPath has the full path to the new exe to spawn

	bool KeepChecking = true;
	while (_Desc.WaitForPID && KeepChecking)
	{
		path ProcessName = ouro::process::get_name(_Desc.WaitForPID);

		oTRACE("Waiting for process exit %d %s (timeout = %.02f sec)...", _Desc.WaitForPID, ProcessName.c_str(), _Desc.WaitForPIDTimeout * 1000.0f);
		if (ouro::process::wait_for(_Desc.WaitForPID, oStd::chrono::milliseconds(_Desc.WaitForPIDTimeout)))
			KeepChecking = false;
		else
		{
			oTRACE("Lasterr: %s", oErrorGetLastString());
			if (!ouro::process::has_debugger_attached(_Desc.WaitForPID))
				ouro::process::terminate(_Desc.WaitForPID, 0xdeadbeef);
		}

		if (!ouro::this_process::has_debugger_attached())
		{
			oSleep(1000);
			KeepChecking = false;
		}
	}

	xlstring CmdLine;
	if (oSTRVALID(_Desc.PassThroughCommandLine))
		snprintf(CmdLine, "%s %s", ExePath.c_str(), _Desc.PassThroughCommandLine);
	else
		snprintf(CmdLine, "%s", ExePath.c_str());
	
	return oVUFireAndForget(CmdLine);
}
