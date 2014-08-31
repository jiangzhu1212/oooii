// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include "d3d_include.h"
#include <oCore/filesystem.h>

namespace ouro {
	namespace gpu {
		namespace d3d {

HRESULT include::Close(LPCVOID pData)
{
	//free((void*)pData); // don't destroy cached files
	return S_OK;
}

HRESULT include::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, unsigned int* pBytes)
{
	try
	{
		path Filename(pFileName);

		scoped_allocation* c = nullptr;
		if (Cache.get_ptr(Filename.hash(), &c))
		{
			*ppData = *c;
			*pBytes = as_uint(c->size());
			return S_OK;
		}

		path FullPath(Filename);
		bool exists = filesystem::exists(FullPath);
		if (!exists)
		{
			for (const path& p : SearchPaths)
			{
				FullPath = p / Filename;
				exists = filesystem::exists(FullPath);
				if (exists)
					break;
			}
		}

		if (!exists)
		{
			std::string err(Filename);
			err += " not found in search path: ";
			if (SearchPaths.empty())
				err += "<empty>";
			else
				for (const path& p : SearchPaths)
				{
					err += p.c_str();
					err += ";";
				}
			
			oTHROW(no_such_file_or_directory, "%s", err.c_str());
		}

		scoped_allocation source = filesystem::load(FullPath);

		*ppData = source;
		*pBytes = as_uint(source.size());
		oCHECK(Cache.add(Filename.hash(), std::move(source)), "add failed");
	}

	catch (std::exception&)
	{
		*ppData = nullptr;
		*pBytes = 0;
		return E_FAIL;
	}

	return S_OK;
}

		} // namespace d3d
	} // namespace gpu
} // namespace ouro
