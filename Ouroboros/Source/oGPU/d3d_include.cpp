/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
