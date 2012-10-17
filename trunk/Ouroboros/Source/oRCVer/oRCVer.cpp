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
#include <oPlatform/Windows/oWindows.h>
#include <oBasis/oMemory.h>

static oOption sOptions[] = 
{
	{ "input", 'i', "path", "Input source file" },
	{ "output", 'o', "path", "Output cpp file" },
	{ "p4root", 'r', "path", "Root path used to determine changelist (ignores modifications outside this root)" },
	{ "product-name", 'p', "name", "Name of the product" },
	{ "version", 'v', "version", "Version in Major.Minor form (i.e. 1.2 or 7.9)" },
	{ "additional-includes", 'I', "path", "Extra includes to include in the template RC file after oRCProductName but before" },
	{ "app-icon", 'a', "path", "The icon to use for the application" },
	//{ "private", 'p', 0, "Mark as private build" },
	//{ "special", 's', 0, "Mark as special build" },
	{ 0, 0, 0, 0 },
};

// TODO: Expose all these as user options
struct oRCVER_DESC
{
	const char* InputPath;
	const char* OutputPath;
	const char* P4RootPath;
	const char* ProductName;
	const char* Version;
	const char* AppIcon;
	std::vector<std::string> AdditionalIncludes;
};

bool ParseCommandLine(int argc, const char* argv[], oRCVER_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(oRCVER_DESC));
	_pDesc->P4RootPath = "//...";

	const char* value = 0;
	char ch = oOptTok(&value, argc, argv, sOptions);
	int count = 0;
	while (ch)
	{
		switch (ch)
		{
			case 'i': _pDesc->InputPath = value; break;
			case 'o': _pDesc->OutputPath = value; break;
			case 'r': _pDesc->P4RootPath = value; break;
			case 'p': _pDesc->ProductName = value; break;
			case 'v': _pDesc->Version = value; break;
			case 'I': _pDesc->AdditionalIncludes.push_back(value); break;
			case 'a': _pDesc->AppIcon = value; break;
			case ':': return oErrorSetLast(oERROR_INVALID_PARAMETER, "The %d%s option is missing a parameter (does it begin with '-' or '/'?)", count, oOrdinal(count));
		}

		ch = oOptTok(&value, 0, 0, 0);
		count++;
	}

	return true;
}

template<typename T> void oByteSwap(T* _pBuffer, size_t _NumBytes)
{
	oASSERT(oByteAlign(_NumBytes, sizeof(T)), "_NumBytes must be aligned to sizeof(T)");
	for (size_t i = 0; i < _NumBytes / sizeof(T); i++)
		_pBuffer[i] = oByteSwap(_pBuffer[i]);
}

void oByteSwapUTF(void* _UTFFile, size_t _SizeofUTFFile)
{
	oUTF_TYPE type = oMemGetUTFType(_UTFFile, _SizeofUTFFile);
	#ifdef oLITTLEENDIAN
		if (type == oUTF16BE)
	#else
		if (type == oUTF16LE)
	#endif
			oByteSwap(static_cast<short*>(_UTFFile), _SizeofUTFFile);

	#ifdef oLITTLEENDIAN
		else if (type == oUTF32BE)
	#else
		else if (type == oUTF32LE)
	#endif
			oByteSwap(static_cast<int*>(_UTFFile), _SizeofUTFFile);
}

static bool oFileLoad(const char* _Path, std::wstring* _pWorkingBuffer, bool _IncludeBOM = true)
{
	oRef<oBuffer> b;
	if (!oBufferLoad(_Path, &b, true))
		return false; // pass through error

	oByteSwapUTF(b->GetData(), b->GetSize());
	
	oUTF_TYPE type = oMemGetUTFType(b->GetData(), b->GetSize());
	if (type >= oUTF32BE)
		return oErrorSetLast(oERROR_NOT_FOUND, "UTF32 is not supported");

	_pWorkingBuffer->reserve(b->GetSize());

	if (type == oASCII)
	{
		std::string s = b->GetData<const char>();
		oStrcpy(*_pWorkingBuffer, s);
	}
	else
		*_pWorkingBuffer = b->GetData<const wchar_t>() + (_IncludeBOM ? 0 : 1);

	return true;
}

static oVersion GetVersion(const oRCVER_DESC& _Desc, bool* _pIsPure)
{
	*_pIsPure = false;

	oVersion v;
	v.Major = 1;
	if (_Desc.Version && !oFromString(&v, _Desc.Version))
		return v;

	if (!v.Build && !v.Revision)
	{
		unsigned int Revision = oP4GetCurrentChangelist(_Desc.P4RootPath);
		if (Revision == oInvalid)
		{
			oP4SetRevision(0, &v);
			*_pIsPure = false;
		}

		else
		{
			oP4SetRevision(Revision, &v);
			*_pIsPure = oErrorGetLast() != oERROR_CORRUPT;
		}
	}

	return v;
}

static std::wregex reAdditionalIncludes(L"//[ \t\v]+#include[ \t\v]+\"additional-includes\"[^\n\r]*", std::regex_constants::optimize);
static std::wregex reProductName(L"#define[ \t\v]+oRCProductName[ \t\v]+\"[^\"]+\"[^\n\r]*", std::regex_constants::optimize);
static std::wregex reAppIcon(L"#define[ \t\v]+oRCAppIcon[ \t\v]+\"[^\"]+\"[^\n\r]*", std::regex_constants::optimize);
static std::wregex reIsSpecialBuild(L"#define[ \t\v]+oRCIsSpecialBuild[^\n\r]*", std::regex_constants::optimize);
static std::wregex reVersionComma(L"(FILE|PRODUCT)VERSION[ \t\v]+([0-9]+),([0-9]+),[0-9]+,[0-9]+", std::regex_constants::optimize);
static std::wregex reVersionDot(L"VALUE[ \t\v]+\"(File|Product)Version\",[ \t\v]+\"([0-9]+)\\.([0-9]+)\\.[0-9]+\\.[0-9]+\"", std::regex_constants::optimize);

bool Main(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		char buf[1024];
		printf("%s", oOptDoc(buf, oGetFilebase(argv[0]), sOptions));
		return oErrorSetLast(oERROR_INVALID_PARAMETER, ""); // don't print any other complaint
	}

	oRCVER_DESC opts;
	if (!ParseCommandLine(argc, argv, &opts))
	{
		oStringXL temp = oErrorGetLastString();
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "bad command line: %s", temp.c_str());
	}

	if (!opts.InputPath || !opts.OutputPath)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Both an input (-i) and an output (-o) file must be specified.");
	
	std::wstring buffers[2]; // do all work and save in UTF16
	if (!oFileLoad(opts.InputPath, &buffers[0]))
		return oErrorSetLast(oERROR_IO, "Failed to load \"%s\" into a UTF16 format", opts.InputPath);

	bool IsPure = false;
	oVersion Version = GetVersion(opts, &IsPure);
	oStringS StrVersion, StrVersionComma;
	oVERIFY(oToString(StrVersion, Version));
	oReplace(StrVersionComma, StrVersion, ".", ",");

	oStringXL DefineAppIcon;
	oStringM VerComma, VerDot, DefineProductName;
	oPrintf(VerComma, "$1VERSION %s", StrVersionComma.c_str());
	oPrintf(VerDot, "VALUE \"$1Version\", \"%s\"", StrVersion.c_str());
	oPrintf(DefineProductName, "#define oRCProductName \"%s\"", opts.ProductName);
	
	if (opts.AppIcon)
	{
		// @oooii-tony: There's something funny about oCleanPath in 32-bit builds:
		// there's an INTERMITTENT formation of a bad path. so try unwinding any 
		// complications possibly caused by templates/oFixedString by using the 
		// direct function call on a raw char array. Do not convert path to 
		// oFixedString until there's a real understood fix to what's going on here.
		char path[_MAX_PATH];
		memset(path, 0, _MAX_PATH);
		oCleanPath(path, _MAX_PATH, opts.AppIcon, '/', true);
		oPrintf(DefineAppIcon, "#define oRCAppIcon \"%s\"", path);
	}

	std::string AdditionalIncludes;
	AdditionalIncludes = "";
	for (size_t i = 0; i < opts.AdditionalIncludes.size(); i++)
	{
		AdditionalIncludes += "#include \"";
		AdditionalIncludes += opts.AdditionalIncludes[i];
		AdditionalIncludes += "\"\n";
	}

	std::wstring WVerComma, WVerDot, WProductName, WAppIcon, WAdditionalIncludes;
	oStrcpy(WVerComma, std::string(VerComma));
	oStrcpy(WVerDot, std::string(VerDot));
	oStrcpy(WProductName, std::string(DefineProductName));
	oStrcpy(WAdditionalIncludes, AdditionalIncludes);
	oStrcpy(WAppIcon, std::string(DefineAppIcon));

	// @oooii-tony: Might want to report out errors here if expected patterns 
	// aren't found. For example the oRCProductName replacement isn't a public
	// standard, but useful. So if it's not there, the -p command line option
	// won't work as expected.

	std::wstring Empty(L"");

	struct FindReplace
	{
		bool Enabled;
		const std::wregex* Find;
		const std::wstring* Replace;
	};

	FindReplace work[] =
	{
		{ true, &reVersionComma, &WVerComma },
		{ true, &reVersionDot, &WVerDot },
		{ true, &reProductName, &WProductName },
		{ true, &reAdditionalIncludes, &WAdditionalIncludes },
		{ true, &reAppIcon, &WAppIcon },
		{ IsPure, &reIsSpecialBuild, &Empty },
	};

	int CurBuffer = 0;
	for (size_t i = 0; i < oCOUNTOF(work); i++)
	{
		if (work[i].Enabled)
		{
			int NextBuffer = (CurBuffer + 1) % 2;
			buffers[NextBuffer] = std::regex_replace(buffers[CurBuffer], *work[i].Find, *work[i].Replace);
			CurBuffer = NextBuffer;
		}
	}

	if (!oFileSave(opts.OutputPath, buffers[CurBuffer].data(), buffers[CurBuffer].length() * 2, false))
		return false; // pass through error

	return true;
}

int main(int argc, const char* argv[])
{
	if (!Main(argc, argv))
	{
		printf("%s\n", oErrorGetLastString());
		return -1;
	}
	return 0;
};
