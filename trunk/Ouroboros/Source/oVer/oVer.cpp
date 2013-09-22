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

static oStd::option sOptions[] = 
{
	{ 'd', "FileDescription", 0, "File description" },
	{ 't', "Type", 0, "File type" },
	{ 'f', "FileVersion", 0, "File version" },
	{ 'p', "ProductName", 0, "Product name" },
	{ 'v', "ProductVersion", 0, "Product version" },
	{ 'c', "Copyright", 0, "Copyright" },
	{ 'h', "help", 0, "Displays this message" },
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
	bool ShowHelp;
	const char* InputPath;
};

bool ParseCommandLine(int argc, const char* argv[], oVER_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(oVER_DESC));

	const char* value = 0;
	char ch = oStd::opttok(&value, argc, argv, sOptions);
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
			case 'h': _pDesc->ShowHelp = true; break;
			case ' ': _pDesc->InputPath = value; break;
			case ':': return oErrorSetLast(std::errc::invalid_argument, "The %d%s option is missing a parameter (does it begin with '-' or '/'?)", count, oStd::ordinal(count));
		}

		ch = oStd::opttok(&value);
		count++;
	}

	return true;
}

bool Main(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		char buf[1024];
		printf("%s", oStd::optdoc(buf, oGetFilebase(argv[0]), sOptions));
		return true;
	}

	oVER_DESC opts;
	if (!ParseCommandLine(argc, argv, &opts))
	{
		oStd::xlstring temp = oErrorGetLastString();
		return oErrorSetLast(std::errc::invalid_argument, "bad command line: %s", temp.c_str());
	}

	if (opts.ShowHelp)
	{
		char buf[1024];
		printf("%s", oStd::optdoc(buf, oGetFilebase(argv[0]), sOptions));
		return true;
	}

	if (!opts.InputPath)
		return oErrorSetLast(std::errc::invalid_argument, "no input file specified.");

	oCore::module::info mi = oCore::module::get_info(oStd::path(opts.InputPath));
	oStd::sstring FBuf, PBuf;
	oVERIFY(oStd::to_string(FBuf, mi.version));
	oVERIFY(oStd::to_string(PBuf, mi.version));

	if (opts.PrintFileDescription)
		printf("%s\n", mi.description.c_str());
	if (opts.PrintType)
		printf("%s\n", oStd::as_string(mi.type));
	if (opts.PrintFileVersion)
		printf("%s\n", FBuf.c_str());
	if (opts.PrintProductName)
		printf("%s\n", mi.product_name.c_str());
	if (opts.PrintProductVersion)
		printf("%s\n", PBuf.c_str());
	if (opts.PrintCopyright)
		printf("%s\n", mi.copyright.c_str());

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
