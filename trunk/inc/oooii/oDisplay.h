/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#ifndef oDisplay_h
#define oDisplay_h

struct oDisplay
{
	// Use this for any of the DESC values below to use current settings
	static const int DEFAULT = 0x80000000;

	struct MODE_DESC
	{
		MODE_DESC()
			: Width(DEFAULT)
			, Height(DEFAULT)
			, Bitdepth(DEFAULT)
			, RefreshRate(DEFAULT)
		{}

		int Width;
		int Height;
		
		// usually 16- or 32-bit
		// Use of DEFAULT means "use the current settings"
		int Bitdepth;

		// Usually 60, 75, 85, 120, 240
		// Use of DEFAULT means "use the current settings"
		int RefreshRate;
	};

	struct DESC
	{
		DESC()
			: NativeHandle(0)
			, Index(~0u)
			, X(DEFAULT)
			, Y(DEFAULT)
			, WorkareaX(DEFAULT)
			, WorkareaY(DEFAULT)
			, WorkareaWidth(DEFAULT)
			, WorkareaHeight(DEFAULT)
			, IsPrimary(false)
		{}

		void* NativeHandle; // HMONITOR on Windows

		MODE_DESC ModeDesc;

		// display ordinal
		unsigned int Index;
		
		// Offset of display in the virtual rect
		int X;
		int Y;

		// Workarea minus any system constructs like the Windows 
		// task bar
		int WorkareaX;
		int WorkareaY;
		int WorkareaWidth;
		int WorkareaHeight;

		bool IsPrimary;
	};

	static bool GetDesc(unsigned int _Index, DESC* _pDesc);
	static bool SetDesc(unsigned int _Index, const DESC* _pDesc);

	static unsigned int GetPrimary(); // returns ~0u if none found
	static unsigned int GetNumDisplays();
	static void GetVirtualRect(int* _pX, int* _pY, int* _pWidth, int* _pHeight);
};

#endif
