// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/path.h>
#include <oBase/throw.h>
#include <oCore/module.h>
#include <oString/opttok.h>
#include <memory.h>

using namespace ouro;

static option sOptions[] = 
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

void ParseCommandLine(int argc, const char* argv[], oVER_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(oVER_DESC));

	const char* value = 0;
	char ch = opttok(&value, argc, argv, sOptions);
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
			case ':': oTHROW_INVARG("The %d%s option is missing a parameter (does it begin with '-' or '/'?)", count, ordinal(count));
		}

		ch = opttok(&value);
		count++;
	}
}

void Main(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		char buf[1024];
		printf("%s", optdoc(buf, path(argv[0]).filename().c_str(), sOptions));
		return;
	}

	oVER_DESC opts;
	ParseCommandLine(argc, argv, &opts);

	if (opts.ShowHelp)
	{
		char buf[1024];
		printf("%s", optdoc(buf, path(argv[0]).filename().c_str(), sOptions));
		return;
	}

	if (!opts.InputPath)
		oTHROW_INVARG("no input file specified");

	module::info mi = module::get_info(path(opts.InputPath));
	sstring FBuf, PBuf;
	oCHECK0(to_string(FBuf, mi.version));
	oCHECK0(to_string(PBuf, mi.version));

	if (opts.PrintFileDescription)
		printf("%s\n", mi.description.c_str());
	if (opts.PrintType)
		printf("%s\n", as_string(mi.type));
	if (opts.PrintFileVersion)
		printf("%s\n", FBuf.c_str());
	if (opts.PrintProductName)
		printf("%s\n", mi.product_name.c_str());
	if (opts.PrintProductVersion)
		printf("%s\n", PBuf.c_str());
	if (opts.PrintCopyright)
		printf("%s\n", mi.copyright.c_str());
}

int main(int argc, const char* argv[])
{
	int err = 0;
	try { Main(argc, argv); }
	catch (std::exception& e)
	{
		printf("%s\n", e.what());
		err = -1;
	}
	return err;
};
