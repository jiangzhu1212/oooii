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
#include <oBasis/oCppParsing.h>
#include <oBasis/oError.h>
#include <oBasis/oPath.h>
#include <oBasis/oString.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oFile.h>

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
	FILE2CPP_DESC opts;
	ParseCommandLine(argc, argv, &opts);

	int rv = EINVAL;
	if (opts.InputPath && opts.OutputPath)
	{
		oRef<oBuffer> b;
		if (oBufferLoad(opts.InputPath, &b, oASCII == oStreamGetUTFType(opts.InputPath)))
		{
			oStringL pchHeader;
			if (opts.PCHName)
				oPrintf(pchHeader, "#include \"%s\"\n", opts.PCHName);

			// guestimate output size.
			size_t cppBufferSize = b->GetSize() * 5;
			char* pCppBuffer = static_cast<char*>(malloc(cppBufferSize));
			char* pCppBufferEnd = oByteAdd(pCppBuffer, cppBufferSize);

			char* w = pCppBuffer + oPrintf(pCppBuffer, cppBufferSize, "// $(header)\n%s", pchHeader.c_str());
			w += oCodifyData(w, (size_t)std::distance(w, pCppBufferEnd), opts.InputPath, b->GetData(), b->GetSize(), sizeof(unsigned int));

			if (oFileSave(opts.OutputPath, pCppBuffer, oStrlen(pCppBuffer), true))
				rv = 0;

			free(pCppBuffer);
		}

		else
			printf("Failed to load %s\n", oSAFESTRN(opts.InputPath));
	}

	else
	{
		oStringXL buf;
		printf("%s", oOptDoc(buf, oGetFilebase(argv[0]), sOptions));
	}

	return rv;
};
