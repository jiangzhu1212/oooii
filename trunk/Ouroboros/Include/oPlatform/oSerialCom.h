/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#ifndef oSerialCom_h
#define oSerialCom_h

#include <oBasis/oInterface.h>

interface oSerialCom : oInterface
{
	enum COM
	{
		COM1,
		COM2,
		COM3,
		COM4
	};

	enum PARITY
	{
		NONE,
		ODD,
		EVEN,
		MARK,
		SPACE
	};

	enum STOPBITS
	{
		ONE,
		ONE5,
		TWO
	};


	struct DESC
	{
		DESC()
			: Com(COM1)
			, BaudRate(9600)
			, ByteSize(8)
			, Parity(NONE)
			, StopBits(ONE)
		{}
		COM Com;
		unsigned int BaudRate;
		unsigned char ByteSize;
		PARITY Parity;
		STOPBITS StopBits;
	};

	virtual bool Send(const char* _pString, unsigned int _StrLen) = 0;

	template<unsigned int size> bool Send(char (&_Str)[size])
	{
		return Send(_Str, size);
	}

	virtual int Receive(char *_pString, unsigned int _BufferLength) = 0;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

oAPI bool oSerialComCreate(const oSerialCom::DESC& _Desc, oSerialCom** _ppSerialCom);

#endif