// $(header)
#include <oooii/oDebugger.h>
#include <oooii/oErrno.h>
#include <oooii/oPath.h>
#include <oooii/oFile.h>
#include <oooii/oString.h>

static oOption sOptions[] = 
{
	{ "pch", 'p', "precompiled-header", "Specify a precompiled header" },
	{ "input", 'i', "path", "Input source file" },
	{ "output", 'o', "path", "Output cpp file" },
	{ "name", 'n', "buffer-name", "Name of created buffer" },
	{ "64bit", '6', 0, "Use 64-bit words" },
	{ "bigendian", 'b', 0, "Write big endian (defaults to little endian)" },
	{ 0, 0, 0, 0 },
};

// TODO: Expose all these as user options
struct FILE2CPP_DESC
{
	const char* InputPath;
	const char* OutputPath;
	const char* PCHName;
	const char* BufferName;
	bool Use64BitWords;
	bool BigEndian;
};

void ParseCommandLine(int argc, const char* argv[], FILE2CPP_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(FILE2CPP_DESC));

	const char* value = 0;
	char ch = oOptTok(&value, argc, argv, sOptions);
	while (ch)
	{
		switch (ch)
		{
			case 'p': _pDesc->PCHName = value; break;
			case 'i': _pDesc->InputPath = value; break;
			case 'o': _pDesc->OutputPath = value; break;
			case 'n': _pDesc->BufferName = value; break;
			case '6': _pDesc->Use64BitWords = !!value; break;
			case 'b': _pDesc->BigEndian = !!value; break;
		}

		ch = oOptTok(&value, 0, 0, 0);
	}
}

char* GetBufferName(char* _StrDestination, size_t _SizeofStrDestination, const char* _Path)
{
	if (oReplace(_StrDestination, _SizeofStrDestination, oGetFilebase(_Path), ".", "_"))
		return 0;
	return _StrDestination;
}

template<size_t size> inline char* GetBufferName(char (&_StrDestination)[size], const char* _Path) { return GetBufferName(_StrDestination, size, _Path); }

int main(int argc, const char* argv[])
{
	// Set this to debug leaks
	long id = 0;
	oDebugger::BreakOnAlloc(id);
	oDebugger::ReportLeaksOnExit();

	FILE2CPP_DESC opts;
	ParseCommandLine(argc, argv, &opts);

	int rv = EINVAL;
	if (opts.InputPath && opts.OutputPath)
	{
		void* pFileBuffer = 0;
		size_t fileBufferSize = 0;
		if (oFile::LoadBuffer(&pFileBuffer, &fileBufferSize, malloc, opts.InputPath, oFile::IsText(opts.InputPath)))
		{
			char pchHeader[256];
			*pchHeader = 0;
			if (opts.PCHName)
				sprintf_s(pchHeader, "#include \"%s\"\n", opts.PCHName);

			// guestimate output size.
			size_t cppBufferSize = __max(500 * 1024, fileBufferSize * 5);
			char* pCppBuffer = static_cast<char*>(malloc(cppBufferSize));
			char* pCppBufferEnd = pCppBuffer + cppBufferSize;

			char* w = pCppBuffer + sprintf_s(pCppBuffer, cppBufferSize, "// $(header)\n%s", pchHeader);
			w += oCodifyData(w, std::distance(w, pCppBufferEnd), opts.InputPath, pFileBuffer, fileBufferSize, sizeof(unsigned int));

			if (oFile::SaveBuffer(opts.OutputPath, pCppBuffer, strlen(pCppBuffer), true, false))
				rv = 0;

			free(pFileBuffer);
			free(pCppBuffer);
		}

		else
			printf("Failed to load %s\n", oSAFESTRN(opts.InputPath));
	}

	else
	{
		char buf[1024];
		printf("%s", oOptDoc(buf, oGetFilebase(argv[0]), sOptions));
	}

	return rv;
};
