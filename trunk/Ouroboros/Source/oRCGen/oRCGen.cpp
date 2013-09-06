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

static oStd::option sOptions[] = 
{
	{ 'v', "version", "maj.min", "Major/minor version to use" },
	{ 'a', "auto-revision", nullptr, "Obtain revision information from scc" },
	{ 'r', "revision", "scc-revision", "override auto-determination of source code control revision" },
	{ 'p', "ProductName", "product name", "Product name" },
	{ 'f', "FileName", "filename", "File name" },
	{ 'c', "Company", "name", "Company name" },
	{ 'C', "Copyright", "string", "Copyright" },
	{ 'd', "Description", "string", "Description" },
	{ 'o', "output", "path", "Output file path" },
	{ 'h', "help", 0, "Displays this message" },
};

struct oOPT
{
	const char* ProductName;
	const char* FileName;
	const char* Company;
	const char* Copyright;
	const char* Description;
	const char* Output;
	unsigned short Major;
	unsigned short Minor;
	unsigned int Revision;
	bool AutoRevision;
	bool ShowHelp;
};

oOPT ParseOptions(int argc, const char* argv[])
{
	oOPT o;
	memset(&o, 0, sizeof(oOPT));
	const char* value = nullptr;
	char ch = oStd::opttok(&value, argc, argv, sOptions);
	while (ch)
	{
		switch (ch)
		{
			case 'v':
			{
				oStd::sstring v(value);
				char* majend = strchr(v, '.');
				if (majend)
				{
					*majend++ = '\0';
					if (oStd::from_string(&o.Major, v) && oStd::from_string(&o.Minor, majend))
						break;
				}
				oTHROW(invalid_argument, "version number not well-formatted");
			}
			case 'a': o.AutoRevision = true; break;
			case 'r':
			{
				if (!oStd::from_string(&o.Revision, value))
					oTHROW(invalid_argument, "unrecognized -r value: it must be an non-negative integer", value);
				break;
			}
			case 'p': o.ProductName = value; break;
			case 'f': o.FileName = value; break;
			case 'c': o.Company = value; break;
			case 'C': o.Copyright = value; break;
			case 'd': o.Description = value; break;
			case 'h': o.ShowHelp = true; break;
			case 'o': o.Output = value; break;
			case '?': oTHROW(invalid_argument, "unrecognized switch \"%s\"", value);
			case ':': oTHROW(invalid_argument, "missing parameter option for argument %d", (int)value);
		}

		ch = oStd::opttok(&value);
	}

	if (!o.Output)
		oTHROW(invalid_argument, "-o specifying an output file is required");

	return o;
}

int ShowHelp(const char* argv0)
{
	oStd::path path = argv0;
	oStd::lstring doc;
	optdoc(doc, path.filename().c_str(), sOptions);
	printf("%s\n", doc.c_str());
	return 0;
}

int main(int argc, const char* argv[])
{
	try
	{
		oOPT opt = ParseOptions(argc, argv);

		if (opt.ShowHelp)
			return ShowHelp(argv[0]);

		unsigned int Revision = opt.Revision;
		if (opt.AutoRevision)
		{
			auto scc = oStd::make_scc(oStd::scc_protocol::svn, oBIND(oSystemExecute, oBIND1, oBIND2, oBIND3, false, oBIND4));
			oStd::path_string DevPath;
			oCHECK0(oSystemGetPath(DevPath, oSYSPATH_DEV));
			printf("scc revision...", DevPath.c_str());
			oStd::lstring RevStr;
			try { Revision = scc->revision(DevPath); }
			catch (std::exception& e) { printf("\n  %s", e.what()); }
			printf("\b\b\b %s\n", Revision ? oStd::to_string(RevStr, Revision) : "");
		}

		oStd::version v(opt.Major, opt.Minor, Revision);
		oStd::sstring VerStrMS, VerStrLX;
		oStd::xlstring s;
		int w = snprintf(s, 
			"// generated file - do not modify\n" \
			"#define oRC_SCC_REVISION %u\n" \
			"#define oRC_VERSION_VAL %u,%u,%u,%u\n" \
			"#define oRC_VERSION_STR_MS \"%s\"\n" \
			"#define oRC_VERSION_STR_LINUX \"%u.%u.%u\"\n" \
			, Revision
			, v.major, v.minor, v.build, v.revision
			, oStd::to_string4(VerStrMS, v)
			, oStd::to_string3(VerStrLX, v));

		if (opt.ProductName) sncatf(s, "#define oRC_PRODUCTNAME \"%s\"\n", opt.ProductName);
		if (opt.FileName) sncatf(s, "#define oRC_FILENAME \"%s\"\n", opt.FileName);
		if (opt.Company) sncatf(s, "#define oRC_COMPANY \"%s\"\n", opt.Company);
		if (opt.Copyright) sncatf(s, "#define oRC_COPYRIGHT \"%s\"\n", opt.Copyright);
		if (opt.Description) sncatf(s, "#define oRC_DESCRIPTION \"%s\"\n", opt.Description);

		if (!oFileSave(opt.Output, s, s.length(), true))
			oTHROW(io_error, "failed to save '%s'\n", opt.Output);
	}

	catch (std::exception& e)
	{
		printf("%s\n", e.what());
		std::system_error* se = dynamic_cast<std::system_error*>(&e);
		return se ? se->code().value() : -1;
	}

	return 0;
}
