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
// Wrappers for some commonly loaded file types
#pragma once
#ifndef oCore_filesystem_util_h
#define oCore_filesystem_util_h

#include <oCore/filesystem.h>
#include <oBase/ini.h>
#include <oBase/xml.h>

namespace ouro {
	namespace filesystem {

inline std::unique_ptr<xml> load_xml(const path& _Path)
{
	scoped_allocation a = load(_Path, load_option::text_read);
	return std::unique_ptr<xml>(new xml(_Path, (char*)a.release(), a.get_deallocate()));
}

inline std::unique_ptr<ini> load_ini(const path& _Path)
{
	scoped_allocation a = load(_Path, load_option::text_read);
	return std::unique_ptr<ini>(new ini(_Path, (char*)a.release(), a.get_deallocate()));
}

	} // namespace filesystem
} // namespace ouro

#endif
