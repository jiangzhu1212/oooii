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
#ifndef oFileSchemeHandler_h
#define oFileSchemeHandler_h

#include <oPlatform/oStream.h>

// {684A582C-240E-4C7D-839C-CF4897F2F5D7}
oDEFINE_GUID_I(oFileSchemeHandler, 0x684a582c, 0x240e, 0x4c7d, 0x83, 0x9c, 0xcf, 0x48, 0x97, 0xf2, 0xf5, 0xd7);
interface oFileSchemeHandler : oSchemeHandler
{
	virtual bool CreateStreamReaderNonBuffered4K(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe = 0;
};

bool oFileSchemeHandlerCreate(threadsafe oFileSchemeHandler** _ppFileSchemeHandler);

#endif