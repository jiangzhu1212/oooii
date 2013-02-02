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
// A collection of useful hashes. An 'i' at the end indicates a case-insensitive 
// string hash.
#pragma once
#ifndef oHash_h
#define oHash_h

#include <oBasis/oUint128.h>

unsigned int oHash_djb2(const void* buf, unsigned int len, unsigned int seed = 5381);
unsigned int oHash_djb2(const char* s);
unsigned int oHash_djb2i(const char* s);

unsigned int oHash_sdbm(const void* buf, unsigned int len, unsigned int seed = 0);
unsigned int oHash_sdbm(const char* s);
unsigned int oHash_sdbmi(const char* s);

unsigned int oHash_stlp(const void* buf, unsigned int len, unsigned int seed = 0);
unsigned int oHash_stlp(const char* s);
unsigned int oHash_stlpi(const char* s);

unsigned int oHash_FNV1a(const void* buf, unsigned int len, unsigned int seed = 2166136261u);
unsigned int oHash_FNV1a(const char* s);
unsigned int oHash_FNV1ai(const char* s);

unsigned int oHash_superfast(const void* buf, unsigned int len, unsigned int seed = 0);
unsigned int oHash_superfasti(const void* buf, unsigned int len, unsigned int seed = 0);

unsigned int oHash_crc32(const void* buf, unsigned int len, unsigned int seed = 0);
unsigned int oHash_crc32(const char* s);
unsigned int oHash_crc32i(const char *s);

// Warning: there is an unfixable algo bug in murmur2. Read more: 
// https://sites.google.com/site/murmurhash/: Update October 28, 2010
// Use with caution (or prefer murmur3).
unsigned int oHash_murmur2(const void* buf, unsigned int len, unsigned int seed = 0);
unsigned int oHash_murmur2aligned(const void* buf, unsigned int len, unsigned int seed = 0);
unsigned int oHash_murmur2neutral(const void* buf, unsigned int len, unsigned int seed = 0);

uint128 oHash_murmur3_x64_128(const void* buf, unsigned int len, unsigned int seed = 0);

// fourcc uses the first four characters, ignoring the rest of the string.
unsigned int oHash_fourcc(const char* s);
unsigned int oHash_fourcci(const char* s);

// eightcc uses the first eight characters, ignoring the rest of the string.
unsigned long long oHash_eightcc(const char* s);
unsigned long long oHash_eightcci(const char* s);

#endif
