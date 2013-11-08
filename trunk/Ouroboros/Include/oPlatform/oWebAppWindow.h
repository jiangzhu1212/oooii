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
#ifndef oWebAppWindow_h
#define oWebAppWindow_h

#include <oPlatform/oWindow.h>

// Simple window that reports work load through a "job count"
// and CPU utilizaton.  Includes a link that opens http://localhost:_ServerPort
interface oWebAppWindow : public oInterface, ouro::window
{
	virtual void SetCurrentJobCount(uint _JobCount) = 0;

	virtual bool IsRunning() const = 0;
	virtual void Close() = 0;
	virtual bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait) = 0;
};	

bool oWebAppWindowCreate(const char* _pTitle, unsigned short _ServerPort, oWebAppWindow** _ppWebAppWindow);

#endif //oWebAppWindow_h