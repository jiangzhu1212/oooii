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
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/oModule.h>

static oOption sOptions[] = 
{
	{ "FileDescription", 'd', 0, "File description" },
	{ "Type", 't', 0, "File type" },
	{ "FileVersion", 'f', 0, "File version" },
	{ "ProductName", 'p', 0, "Product name" },
	{ "ProductVersion", 'v', 0, "Product version" },
	{ "Copyright", 'c', 0, "Copyright" },
	{ 0, 0, 0, 0 },
};

// TODO: Expose all these as user options
struct oVER_DESC
{
	bool PrintFileDescription;
	bool PrintType;
	bool PrintFileVersion;
	bool PrintProductName;
	bool PrintProductVersion;
	bool PrintCopyright;
	const char* InputPath;
};

bool ParseCommandLine(int argc, const char* argv[], oVER_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(oVER_DESC));

	const char* value = 0;
	char ch = oOptTok(&value, argc, argv, sOptions);
	int count = 0;
	while (ch)
	{
		switch (ch)
		{
			case 'd': _pDesc->PrintFileDescription = true; break;
			case 't': _pDesc->PrintType = true; break;
			case 'f': _pDesc->PrintFileVersion = true; break;
			case 'p': _pDesc->PrintProductName = true; break;
			case 'v': _pDesc->PrintProductVersion = true; break;
			case 'c': _pDesc->PrintCopyright = true; break;
			case ' ': _pDesc->InputPath = value; break;
			case ':': return oErrorSetLast(std::errc::invalid_argument, "The %d%s option is missing a parameter (does it begin with '-' or '/'?)", count, oStd::ordinal(count));
		}

		ch = oOptTok(&value, 0, 0, 0);
		count++;
	}

	return true;
}

bool Main(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		char buf[1024];
		printf("%s", oOptDoc(buf, oGetFilebase(argv[0]), sOptions));
		return oErrorSetLast(std::errc::invalid_argument, ""); // don't print any other complaint
	}

	oVER_DESC opts;
	if (!ParseCommandLine(argc, argv, &opts))
	{
		oStd::xlstring temp = oErrorGetLastString();
		return oErrorSetLast(std::errc::invalid_argument, "bad command line: %s", temp.c_str());
	}

	if (!opts.InputPath)
		return oErrorSetLast(std::errc::invalid_argument, "no input file specified.");

	oMODULE_DESC md;
	if (!oModuleGetDesc(opts.InputPath, &md))
		return oErrorSetLast(std::errc::no_such_file_or_directory, "failed to get information for file \"%s\"", opts.InputPath);

	oStd::sstring FBuf, PBuf;
	oVERIFY(oStd::to_string(FBuf, md.FileVersion));
	oVERIFY(oStd::to_string(PBuf, md.ProductVersion));

	if (opts.PrintFileDescription)
		printf("%s\n", md.Description.c_str());
	if (opts.PrintType)
		printf("%s\n", oStd::as_string(md.Type));
	if (opts.PrintFileVersion)
		printf("%s\n", FBuf.c_str());
	if (opts.PrintProductName)
		printf("%s\n", md.ProductName.c_str());
	if (opts.PrintProductVersion)
		printf("%s\n", PBuf.c_str());
	if (opts.PrintCopyright)
		printf("%s\n", md.Copyright.c_str());

	return true;
}

int main(int argc, const char* argv[])
{
	int err = 0;
	if (!Main(argc, argv))
	{
		printf("%s\n", oErrorGetLastString());
		err = -1;
	}
	return err;
};
