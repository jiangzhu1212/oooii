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

static oOption sOptions[] = 
{
	{ "version", 'v', "major.minor", "File version (pass 0.0 to only update revision)" },
	{ "productname", 'p', "name", "Product name" },
	{ "company", 'c', "name", "Company" },
	{ "copyright", 't', "string", "Copyright" },
	{ "comments", 'm', "string", "Comments" },
	{ "privatebuild", 'b', "string", "File type" },
	{ "specialbuild", 's', "string", "File type" },
	{ "sccroot", 'r', "path", "source code control repository root" },
	{ "file", 'f', "path", "executable file to modify" },
	{ "verpatch", '@', "path", "path to verpatch.exe" },
	{ 0, 0, 0, 0 },
};

struct oVERPATCH_DESC
{
	const char* Version;
	const char* ProductName;
	const char* Company;
	const char* Copyright;
	const char* Comments;
	const char* PrivateBuild;
	const char* SpecialBuild;
	const char* SCCRoot;
	const char* File;
	const char* VerPatch;
};

static oStd::path_string sDefaultSCCRoot;

static bool ParseCommandLine(int argc, const char* argv[], oVERPATCH_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(oVERPATCH_DESC));
	#if 0
		_pDesc->SCCRoot = "//...";
	#else
	oSystemGetPath(sDefaultSCCRoot, oSYSPATH_APP);
	_pDesc->SCCRoot = sDefaultSCCRoot;
	#endif
	_pDesc->VerPatch = "./verpatch.exe";

	const char* value = 0;
	char ch = oOptTok(&value, argc, argv, sOptions);
	int count = 0;
	while (ch)
	{
		switch (ch)
		{
			case 'v': _pDesc->Version = value; break;
			case 'p': _pDesc->ProductName = value; break;
			case 'c': _pDesc->Company = value; break;
			case 't': _pDesc->Copyright = value; break;
			case 'm': _pDesc->Comments = value; break;
			case 'b': _pDesc->PrivateBuild = value; break;
			case 's': _pDesc->SpecialBuild = value; break;
			case 'r': _pDesc->SCCRoot = value; break;
			case 'f': _pDesc->File = value; break;
			case '@': _pDesc->VerPatch = value; break;
			case ':': return oErrorSetLast(std::errc::invalid_argument, "The %d%s option is missing a parameter (does it begin with '-' or '/'?)", count, oStd::ordinal(count));
		}

		ch = oOptTok(&value, 0, 0, 0);
		count++;
	}

	return true;
}

static bool CreateVersionString(oStd::mstring& _StrDestination, const oVERPATCH_DESC& _Desc)
{
	oMODULE_DESC d;
	if (!oModuleGetDesc(_Desc.File, &d))
		return false; // pass through error

	oVersion v(1,0);
	if (oStd::from_string(&v, _Desc.Version) && !v.IsValid())
		v = d.FileVersion;

	auto scc = oStd::make_scc(oStd::scc_protocol::svn, oBIND(oSystemExecute, oBIND1, oBIND2, oBIND3, false, oBIND4));
	uint Revision = scc->revision(_Desc.SCCRoot);

	// make revision readable, but fit Microsoft's standards
	v.Build = static_cast<unsigned short>(Revision / 10000);
	v.Revision = Revision % 10000;

	// Now convert it to string
	oStd::to_string(_StrDestination, v);

	// Mark if this is a modified build
	oStd::scc_file f;
	bool Special = !scc->is_up_to_date(_Desc.SCCRoot);

	if (d.IsDebugBuild)
		oStrcat(_StrDestination, " (Debug)");
	if (Special)
		oStrcat(_StrDestination, " (Special)");

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

	oVERPATCH_DESC opts;
	if (!ParseCommandLine(argc, argv, &opts))
	{
		oStd::xlstring temp = oErrorGetLastString();
		return oErrorSetLast(std::errc::invalid_argument, "bad command line: %s", temp.c_str());
	}

	if (!opts.File)
		return oErrorSetLast(std::errc::invalid_argument, "A file to modify (-f) must be specified.");

	// Prepare a command line to spawn for verpatch.exe

	std::string verpatch;
	verpatch.reserve(oKB(2));
	verpatch = "\"";
	verpatch += opts.VerPatch;
	verpatch += "\" \"";
	verpatch += opts.File;
	verpatch += "\"";

	if (opts.Version)
	{
		oStd::mstring StrVersion;
		if (!CreateVersionString(StrVersion, opts))
			return false; // pass through error

		verpatch += " \"";
		verpatch += StrVersion;
		verpatch += "\"";

		verpatch += " -pv \"";
		verpatch += StrVersion;
		verpatch += "\"";
	}

	#define oVERVAL(Val) do \
	{	if (opts.Val) { \
		verpatch += " /s " #Val " \""; \
		verpatch += opts.Val; \
		verpatch += "\""; \
	}} while (false)

	oVERVAL(ProductName);
	oVERVAL(Company);
	oVERVAL(Copyright);
	oVERVAL(Comments);
	oVERVAL(PrivateBuild);
	oVERVAL(SpecialBuild);

	oStd::xlstring Response;
	int ExitCode = 0;
	if (!oSystemExecute(verpatch.c_str(), [&](char* _Line) { strlcat(Response, _Line, Response.capacity()); }, &ExitCode, false, 5000))
		return false; // pass through error

	if (ExitCode)
	{
		printf("verpatch exited with code %d\n%s", ExitCode, Response.c_str());
		return oErrorSetLast(std::errc::io_error, Response);
	}

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