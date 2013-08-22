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
#pragma once
#ifndef oBasisTestOBJ_h
#define oBasisTestOBJ_h

#include <oBasis/oOBJ.h>

struct oBasisTestOBJ
{
	// Provides a hand-constructed OBJ and its binary equivalents. Use this to 
	// test OBJ handling code.

	// Returns an oOBJ_DESC to verified binary data as if the file contents had
	// been properly loaded.
	virtual void GetDesc(oOBJ_DESC* _pDesc) const threadsafe = 0;

	// Returns the text of an OBJ file as if it had been fread into memory. ('\n'
	// is the only newline here)
	virtual const char* GetFileContents() const threadsafe = 0;
};

enum oBASIS_TEST_OBJ
{
	oBASIS_TEST_CUBE_OBJ,

	oBASIS_TEST_OBJ_COUNT,
};

// Returns a pointer to a singleton instance. Do not delete the pointer.
oAPI bool oBasisTestOBJGet(oBASIS_TEST_OBJ _OBJ, const oBasisTestOBJ** _ppTestOBJ);

#endif
