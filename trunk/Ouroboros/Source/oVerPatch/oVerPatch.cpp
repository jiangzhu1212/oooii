// $(header)

static oOption sOptions[] = 
{
	{ "version", 'v', "major.minor", "File version (pass 0.0 to only update revision)" },
	{ "productname", 'p', "name", "Product name" },
	{ "company", 'c', "name", "Company" },
	{ "copyright", 't', "string", "Copyright" },
	{ "comments", 'm', "string", "Comments" },
	{ "privatebuild", 'b', "string", "File type" },
	{ "specialbuild", 's', "string", "File type" },
	{ "p4root", 'r', "path", "P4 root" },
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
	const char* P4Root;
	const char* File;
	const char* VerPatch;
};

static bool ParseCommandLine(int argc, const char* argv[], oVERPATCH_DESC* _pDesc)
{
	memset(_pDesc, 0, sizeof(oVERPATCH_DESC));
	_pDesc->P4Root = "//...";
	_pDesc->VerPatch = "./";

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
			case 'r': _pDesc->P4Root = value; break;
			case 'f': _pDesc->File = value; break;
			case '@': _pDesc->VerPatch = value; break;
			case ':': return oErrorSetLast(oERROR_INVALID_PARAMETER, "The %d%s option is missing a parameter (does it begin with '-' or '/'?)", count, oOrdinal(count));
		}

		ch = oOptTok(&value, 0, 0, 0);
		count++;
	}

	return true;
}

static bool CreateVersionString(oStringM& _StrDestination, const oVERPATCH_DESC& _Desc)
{
	oMODULE_DESC d;
	if (!oModuleGetDesc(_Desc.File, &d))
		return false; // pass through error

	// First get the revision from P4.
	oVersion v;
	v.Major = 1;
	if (oFromString(&v, _Desc.Version) && !v.IsValid())
		v = d.FileVersion;

	uint Revision = oP4GetCurrentChangelist(_Desc.P4Root);
	bool Special = oErrorGetLast() == oERROR_CORRUPT;
	if (Revision == oInvalid)
		Revision = 0;
	oP4SetRevision(Revision, &v);

	// Now convert it to string
	oToString(_StrDestination, v);
	
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
		return oErrorSetLast(oERROR_INVALID_PARAMETER, ""); // don't print any other complaint
	}

	oVERPATCH_DESC opts;
	if (!ParseCommandLine(argc, argv, &opts))
	{
		oStringXL temp = oErrorGetLastString();
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "bad command line: %s", temp.c_str());
	}

	if (!opts.File)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "A file to modify (-f) must be specified.");

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
		oStringM StrVersion;
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

	oStringXL Response;
	int ExitCode = 0;
	if (!oSystemExecute(verpatch.c_str(), Response, &ExitCode, 5000))
		return false; // pass through error

	if (ExitCode)
	{
		printf("verpatch exited with code %d\n%s", ExitCode, Response.c_str());
		return oErrorSetLast(oERROR_IO, Response);
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
