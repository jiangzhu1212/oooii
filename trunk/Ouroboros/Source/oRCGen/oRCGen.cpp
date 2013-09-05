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

bool ParseOptions(int argc, const char* argv[], oOPT* _pOpt)
{
	memset(_pOpt, 0, sizeof(oOPT));
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
					if (oStd::from_string(&_pOpt->Major, v) && oStd::from_string(&_pOpt->Minor, majend))
						break;
				}
				return oErrorSetLast(std::errc::invalid_argument, "version number not well-formatted");
			}
			case 'a': _pOpt->AutoRevision = true; break;
			case 'r':
			{
				if (!oStd::from_string(&_pOpt->Revision, value))
					return oErrorSetLast(std::errc::invalid_argument, "error: unrecognized -r value: it must be an non-negative integer", value);
				break;
			}
			case 'p': _pOpt->ProductName = value; break;
			case 'f': _pOpt->FileName = value; break;
			case 'c': _pOpt->Company = value; break;
			case 'C': _pOpt->Copyright = value; break;
			case 'd': _pOpt->Description = value; break;
			case 'h': _pOpt->ShowHelp = true; break;
			case 'o': _pOpt->Output = value; break;
			case '?': return oErrorSetLast(std::errc::invalid_argument, "error: unrecognized switch \"%s\"", value);
			case ':': return oErrorSetLast(std::errc::invalid_argument, "missing parameter option for argument %d", (int)value);
		}

		ch = oStd::opttok(&value);
	}

	if (!_pOpt->Output)
		return oErrorSetLast(std::errc::invalid_argument, "-o specifying an output file is required");

	return true;
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
	oOPT opt;
	if (!ParseOptions(argc, argv, &opt))
	{
		printf("%s: %s\n", oErrorAsString(oErrorGetLast()), oErrorGetLastString());
		return oErrorGetLast();
	}

	if (opt.ShowHelp)
		return ShowHelp(argv[0]);

	unsigned int Revision = opt.Revision;
	if (opt.AutoRevision)
	{
		auto scc = oStd::make_scc(oStd::scc_protocol::svn, oBIND(oSystemExecute, oBIND1, oBIND2, oBIND3, false, oBIND4));
		oStd::path_string DevPath;
		oVERIFY(oSystemGetPath(DevPath, oSYSPATH_DEV));
		printf("scc revision for '%s'...", DevPath.c_str());
		oStd::lstring RevStr;
		try { Revision = scc->revision(DevPath); }
		catch (std::exception& e) { printf("\nerror: %s", e.what()); }
		printf("%s\n", Revision ? oStd::to_string(RevStr, Revision) : "");
	}

	oVersion v(opt.Major, opt.Minor, Revision);
	oStd::sstring VerStr;
	oStd::to_string(VerStr, v);
	oStd::xlstring s;
	int w = snprintf(s, 
		"// generated file - do not modify\n" \
		"#define oRC_SCC_REVISION %u\n" \
		"#define oRC_VERSION_VAL %u,%u,%u,%u\n" \
		"#define oRC_VERSION_STR_MS \"%s\"\n" \
		"#define oRC_VERSION_STR_LINUX \"%u.%u.%u\"\n" \
		, Revision
		, v.Major, v.Minor, v.Build, v.Revision
		, VerStr.c_str()
		, v.Major, v.Minor, Revision);

	if (opt.ProductName) sncatf(s, "#define oRC_PRODUCTNAME \"%s\"\n", opt.ProductName);
	if (opt.FileName) sncatf(s, "#define oRC_FILENAME \"%s\"\n", opt.FileName);
	if (opt.Company) sncatf(s, "#define oRC_COMPANY \"%s\"\n", opt.Company);
	if (opt.Copyright) sncatf(s, "#define oRC_COPYRIGHT \"%s\"\n", opt.Copyright);
	if (opt.Description) sncatf(s, "#define oRC_DESCRIPTION \"%s\"\n", opt.Description);

	if (!oFileSave(opt.Output, s, s.length(), true))
		return oErrorSetLast(std::errc::io_error, "failed to save '%s'\n", opt.Output);

	return 0;
}
