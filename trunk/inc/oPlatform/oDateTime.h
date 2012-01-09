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
// Extension to standard-C time functions so that milliseconds (and future 
// higher precision) can be supported.
#pragma once
#ifndef oDateTime_h
#define oDateTime_h

#include <oBasis/oMacros.h>

struct oDateTime
{
	unsigned short Year; // The full year.
	unsigned short Month; // [1,12] (January - December)
	unsigned short DayOfWeek; // [0,6] (Sunday - Saturday)
	unsigned short Day; // [1,31]
	unsigned short Hour; // [0,23]
	unsigned short Minute; // [0,59]
	unsigned short Second; // [0,59]
	unsigned short Milliseconds; // [0,999]
};

// 0 means equal
// >0 _DateTime1 > _DateTime2 (1 is later/more in the future than 2)
// <0 _DateTime1 < _DateTime2 (1 is eariler/more in the past than 2)
int oCompareDateTime(const oDateTime& _DateTime1, const oDateTime& _DateTime2);

// Get the current UTC time in oDataTime format
bool oGetDateTime(oDateTime* _pDateTime);

// Converts UTC time to the current local time
bool oGetLocalDateTime(const oDateTime& _UTCTime, oDateTime* _pLocalTime);

time_t oConvertDateTime(const oDateTime& _DateTime);
void oConvertDateTime(oDateTime* _DateTime, time_t _Time);

// oToString/oFromString uses this format:
// YYYY/MM/DD HH:MM:SS:MMS (3 digits of milliseconds)

#endif
