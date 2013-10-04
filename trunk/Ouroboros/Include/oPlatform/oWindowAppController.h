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
#ifndef oWindowAppController_h
#define oWindowAppController_h

#include <oBasis/oGUI.h>
#include <oPlatform/oImage.h>

// Provides control of the top level window in
// another process.
interface oWindowAppController : oInterface
{
	virtual bool SendX11Keys(oGUI_KEY* _pKeys, int _NumberKeys) = 0;
	template<int KEY_COUNT>
	inline bool SendX11Keys(oGUI_KEY (&_Keys)[KEY_COUNT])
	{
		return SendX11Keys(_Keys, KEY_COUNT);
	}

	virtual bool SendMouseButtonAtPosition(const float2 &_Position, oGUI_KEY _MouseButton, bool _MouseUp) = 0;
	virtual bool SendKeyDown(oGUI_KEY _Key, bool _KeyUp) = 0;
	virtual bool SendASCII(const char* _pASCII) = 0;

	virtual bool CreateSnapshot( oImage** _ppSnapshot ) = 0;
};

// Process name is required while window name is only recommended (since processes can have more than one top level window)
bool oWindowAppControllerCreate(const char* _pProcessName, const char* _pWindowName, oWindowAppController** _ppWindowAppController);

#endif