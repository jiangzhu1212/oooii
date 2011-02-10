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
#include "pch.h"
#include <oooii/oHash.h>
#include <memory>
#include <ctype.h>

unsigned int oHash_djb2(const void* buf, unsigned int len, unsigned int seed)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this" 
		author="Ozan Yigit"
		description="http://www.cse.yorku.ca/~oz/hash.html"
		license="Public Domain"
		licenseurl="http://www.cse.yorku.ca/~oz/hash.html"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	const char* s = (const char*)buf;
	unsigned int h = seed;
	while(len--)
		h = (h + (h << 5)) ^ *s++;
	return (unsigned int)h;
	// $(CitedCodeEnd)
}

unsigned int oHash_djb2(const char* s)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this" 
		author="Ozan Yigit"
		description="http://www.cse.yorku.ca/~oz/hash.html"
		license="Public Domain"
		licenseurl="http://www.cse.yorku.ca/~oz/hash.html"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	unsigned int h = 5381;
	while(*s)
		h = (h + (h << 5)) ^ *s++;
	return (unsigned int)h;
	// $(CitedCodeEnd)
}

unsigned int oHash_djb2i(const char* s)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this" 
		author="Ozan Yigit"
		description="http://www.cse.yorku.ca/~oz/hash.html"
		license="Public Domain"
		licenseurl="http://www.cse.yorku.ca/~oz/hash.html"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	unsigned int h = 5381;
	while(*s)
		h = (h + (h << 5)) ^ tolower(*s++);
	return (unsigned int)h;
	// $(CitedCodeEnd)
}

unsigned int oHash_sdbm(const void* buf, unsigned int len, unsigned int seed)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this" 
		author="Ozan Yigit"
		description="http://www.cse.yorku.ca/~oz/hash.html"
		license="Public Domain"
		licenseurl="http://www.cse.yorku.ca/~oz/hash.html"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	const char* s = (const char*)buf;
	unsigned int h = seed;
	while (len--)
		h = (unsigned int)(*s++) + (h << 6) + (h << 16) - h;
	return (unsigned int)h;
	// $(CitedCodeEnd)
}

unsigned int oHash_sdbm(const char* s)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this" 
		author="Ozan Yigit"
		description="http://www.cse.yorku.ca/~oz/hash.html"
		license="Public Domain"
		licenseurl="http://www.cse.yorku.ca/~oz/hash.html"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	unsigned int h = 0;
	while (*s)
		h = (unsigned int)(*s++) + (h << 6) + (h << 16) - h;
	return (unsigned int)h;
	// $(CitedCodeEnd)
}

unsigned int oHash_sdbmi(const char* s)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this" 
		author="Ozan Yigit"
		description="http://www.cse.yorku.ca/~oz/hash.html"
		license="Public Domain"
		licenseurl="http://www.cse.yorku.ca/~oz/hash.html"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	unsigned int h = 0;
	while (*s)
		h = (unsigned int)(tolower(*s++)) + (h << 6) + (h << 16) - h;
	return (unsigned int)h;
}

unsigned int oHash_stlp(const void* buf, unsigned int len, unsigned int seed)
{
	/** <citation
		usage="Implementation" 
		reason="STLPort's hash so it can be used in other non-STLPort code" 
		author="Boris Fomitchev"
		description="http://sourceforge.net/projects/stlport/"
		license="STLPort License"
		licenseurl="http://www.stlport.org/doc/license.html"
		modification="STLPort's hash algo was extracted from <_hash_fun> and altered to use more explict types and operate on a void*"
	/>*/
	/*
	 * Copyright (c) 1996,1997
	 * Silicon Graphics Computer Systems, Inc.
	 *
	 * Copyright (c) 1999 
	 * Boris Fomitchev
	 *
	 * This material is provided "as is", with absolutely no warranty expressed
	 * or implied. Any use is at your own risk.
	 *
	 * Permission to use or copy this software for any purpose is hereby granted 
	 * without fee, provided the above notices are retained on all copies.
	 * Permission to modify the code and to distribute modified code is granted,
	 * provided the above notices are retained, and a notice that the code was
	 * modified is included with the above copyright notice.
	 */
	// $(CitedCodeBegin)
	const char* s = (const char*)buf;
	unsigned int h = seed; 
	while (len--)
		h = 5 * h + *s++;
	return (unsigned int)h;
	// $(CitedCodeEnd)
}

unsigned int oHash_stlp(const char* s)
{
	/** <citation
		usage="Implementation" 
		reason="STLPort's hash so it can be used in other non-STLPort code" 
		author="Boris Fomitchev"
		description="http://sourceforge.net/projects/stlport/"
		license="STLPort License"
		licenseurl="http://www.stlport.org/doc/license.html"
		modification="STLPort's hash algo was extracted from <_hash_fun> and altered to use more explict types and operate on a void*"
	/>*/
	/*
	 * Copyright (c) 1996,1997
	 * Silicon Graphics Computer Systems, Inc.
	 *
	 * Copyright (c) 1999 
	 * Boris Fomitchev
	 *
	 * This material is provided "as is", with absolutely no warranty expressed
	 * or implied. Any use is at your own risk.
	 *
	 * Permission to use or copy this software for any purpose is hereby granted 
	 * without fee, provided the above notices are retained on all copies.
	 * Permission to modify the code and to distribute modified code is granted,
	 * provided the above notices are retained, and a notice that the code was
	 * modified is included with the above copyright notice.
	 */
	// $(CitedCodeBegin)
	unsigned int h = 0; 
	while (*s)
		h = 5 * h + *s++;
	return (unsigned int)h;
	// $(CitedCodeEnd)
}

unsigned int oHash_stlpi(const char* s)
{
	/** <citation
		usage="Implementation" 
		reason="STLPort's hash so it can be used in other non-STLPort code" 
		author="Boris Fomitchev"
		description="http://sourceforge.net/projects/stlport/"
		license="STLPort License"
		licenseurl="http://www.stlport.org/doc/license.html"
		modification="STLPort's hash algo was extracted from <_hash_fun> and altered to use more explict types and operate on a void*"
	/>*/
	/*
	 * Copyright (c) 1996,1997
	 * Silicon Graphics Computer Systems, Inc.
	 *
	 * Copyright (c) 1999 
	 * Boris Fomitchev
	 *
	 * This material is provided "as is", with absolutely no warranty expressed
	 * or implied. Any use is at your own risk.
	 *
	 * Permission to use or copy this software for any purpose is hereby granted 
	 * without fee, provided the above notices are retained on all copies.
	 * Permission to modify the code and to distribute modified code is granted,
	 * provided the above notices are retained, and a notice that the code was
	 * modified is included with the above copyright notice.
	 */
	// $(CitedCodeBegin)
	unsigned int h = 0; 
	while (*s)
		h = 5 * h + tolower(*s++);
	return (unsigned int)h;
	// $(CitedCodeEnd)
}

#define get16bits(d) (*((const unsigned short*) (d)))
unsigned int oHash_superfast(const void* buf, unsigned int len, unsigned int seed)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this"
		author="Paul Hsieh"
		description="http://www.azillionmonkeys.com/qed/hash.html"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.azillionmonkeys.com/qed/hash.html"
		modification="Use more explicit types"
	/>*/
	// $(CitedCodeBegin)
	const char* s = (const char*)buf;

	unsigned int h = seed + len, tmp;
	unsigned int rem;

	if (len <= 0 || s == 0)
		return 0;

	rem = len & 3;
	len >>= 2;

	// Main loop
	for (;len > 0; len--)
	{
		h += get16bits (s);
		tmp = (get16bits (s + 2) << 11) ^ h;
		h = (h << 16) ^ tmp;
		s += 2 * sizeof(unsigned short);
		h += h >> 11;
	}

	// Handle end cases
	switch (rem)
	{
		case 3:
			h += get16bits (s);
			h ^= h << 16;
			h ^= s[sizeof (unsigned short)] << 18;
			h += h >> 11;
			break;
		case 2:
			h += get16bits (s);
			h ^= h << 11;
			h += h >> 17;
			break;
		case 1:
			h += *s;
			h ^= h << 10;
			h += h >> 1;
	}

	// Force "avalanching" of final 127 bits
	h ^= h << 3;
	h += h >> 5;
	h ^= h << 4;
	h += h >> 17;
	h ^= h << 25;
	h += h >> 6;

	return h;
	// $(CitedCodeEnd)
}

inline unsigned short get16bitsi(const char* s)
{
	char low = (*s)-'A'+'a';
	char hi = (*(s+1))-'A'+'a';
	return (unsigned short)((hi << 8) | low);
}

unsigned int oHash_superfasti(const void* buf, unsigned int len, unsigned int seed)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this"
		author="Paul Hsieh"
		description="http://www.azillionmonkeys.com/qed/hash.html"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.azillionmonkeys.com/qed/hash.html"
		modification="Use more explicit types, case-insensitive and add seed"
	/>*/
	// $(CitedCodeBegin)
	const char* s = (const char*)buf;

	unsigned int h = seed + len, tmp;
	unsigned int rem;

	if (len <= 0 || s == 0)
		return 0;

	rem = len & 3;
	len >>= 2;

	// Main loop
	for (;len > 0; len--)
	{
		h += get16bitsi (s);
		tmp = (get16bitsi (s + 2) << 11) ^ h;
		h = (h << 16) ^ tmp;
		s += 2 * sizeof(unsigned short);
		h += h >> 11;
	}

	// Handle end cases
	switch (rem)
	{
		case 3:
			h += get16bitsi (s);
			h ^= h << 16;
			h ^= s[sizeof (unsigned short)] << 18;
			h += h >> 11;
			break;
		case 2:
			h += get16bitsi (s);
			h ^= h << 11;
			h += h >> 17;
			break;
		case 1:
			h += *s;
			h ^= h << 10;
			h += h >> 1;
	}

	// Force "avalanching" of final 127 bits
	h ^= h << 3;
	h += h >> 5;
	h ^= h << 4;
	h += h >> 17;
	h ^= h << 25;
	h += h >> 6;

	return h;
	// $(CitedCodeEnd)
}

/** <citation
	usage="Implementation" 
	reason="STL's hash is decent, but there are better ones, such as this"
	author="Bob Jenkins"
	description="http://burtleburtle.net/bob/c/crc.c"
	license="Public Domain"
	licenseurl="http://burtleburtle.net/bob/c/crc.c"
	modification=""
/>*/
// $(CitedCodeBegin)
static const unsigned int crctab[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};
// $(CitedCodeEnd)

unsigned int oHash_crc32(const void *buf, unsigned int len, unsigned int seed)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this"
		author="Bob Jenkins"
		description="http://burtleburtle.net/bob/c/crc.c"
		license="Public Domain"
		licenseurl="http://burtleburtle.net/bob/c/crc.c"
		modification="change around masking hash to ensure an index into LUT [0,255]"
	/>*/
	// $(CitedCodeBegin)
	const char* k = static_cast<const char*>(buf);
	unsigned int hash = seed;
	for (unsigned int i=0; i<len; ++i)
		hash = (hash >> 8) ^ crctab[(hash ^ k[i]) & 0xff];
	return hash;
	// $(CitedCodeEnd)
}

unsigned int oHash_crc32(const char *s)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this"
		author="Bob Jenkins"
		description="http://burtleburtle.net/bob/c/crc.c"
		license="Public Domain"
		licenseurl="http://burtleburtle.net/bob/c/crc.c"
		modification="change around masking hash to ensure an index into LUT [0,255]"
	/>*/
	// $(CitedCodeBegin)
	unsigned int hash = 0;
	while (*s)
	{
		hash = (hash >> 8) ^ crctab[(hash ^ *s) & 0xff];
		s++;
	}

	return hash;
	// $(CitedCodeEnd)
}

unsigned int oHash_crc32i(const char *s)
{
	/** $(Citation)
		<citation>
			<usage type="Implementation" />
			<author name="Bob Jenkins" />
			<description url="http://burtleburtle.net/bob/c/crc.c" />
			<license type="Public Domain" url="http://burtleburtle.net/bob/c/crc.c" />
		</citation>
	*/
	// $(CitedCodeBegin)
	unsigned int hash = 0;
	while (*s)
	{
		hash = (hash >> 8) ^ crctab[(hash & 0xff) ^ tolower(*s)];
		s++;
	}

	return hash;
	// $(CitedCodeEnd)
}

namespace orig_murmur_hash
{
/** $(Citation)
	<citation>
		<usage type="Implementation" />
		<author name="Austin Appleby" />
		<description url="http://murmurhash.googlepages.com/" />
		<license type="Public Domain" url="http://murmurhash.googlepages.com/" />
	</citation>
*/
// $(CitedCodeBegin)
//-----------------------------------------------------------------------------
// MurmurHash2, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

inline unsigned int MurmurHash2 ( const void * key, int len, unsigned int seed )
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	unsigned int h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h *= m; 
		h ^= k;

		data += 4;
		len -= 4;
	}
	
	// Handle the last few bytes of the input array

	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
} 

//-----------------------------------------------------------------------------
// MurmurHashAligned2, by Austin Appleby

// Same algorithm as MurmurHash2, but only does aligned reads - should be safer
// on certain platforms. 

// Performance will be lower than MurmurHash2

#define MIX(h,k,m) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

inline unsigned int MurmurHashAligned2 ( const void * key, int len, unsigned int seed )
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	const unsigned char * data = (const unsigned char *)key;

	unsigned int h = seed ^ len;

	int BYTEAlign = (int)data & 3;

	if(BYTEAlign && (len >= 4))
	{
		// Pre-load the temp registers

		unsigned int t = 0, d = 0;

		switch(BYTEAlign)
		{
			case 1: t |= data[2] << 16;
			case 2: t |= data[1] << 8;
			case 3: t |= data[0];
		}

		t <<= (8 * BYTEAlign);

		data += 4-BYTEAlign;
		len -= 4-BYTEAlign;

		int sl = 8 * (4-BYTEAlign);
		int sr = 8 * BYTEAlign;

		// Mix

		while(len >= 4)
		{
			d = *(unsigned int *)data;
			t = (t >> sr) | (d << sl);

			unsigned int k = t;

			MIX(h,k,m);

			t = d;

			data += 4;
			len -= 4;
		}

		// Handle leftover data in temp registers

		d = 0;

		if(len >= BYTEAlign)
		{
			switch(BYTEAlign)
			{
			case 3: d |= data[2] << 16;
			case 2: d |= data[1] << 8;
			case 1: d |= data[0];
			}

			unsigned int k = (t >> sr) | (d << sl);
			MIX(h,k,m);

			data += BYTEAlign;
			len -= BYTEAlign;

			//----------
			// Handle tail bytes

			switch(len)
			{
			case 3: h ^= data[2] << 16;
			case 2: h ^= data[1] << 8;
			case 1: h ^= data[0];
					h *= m;
			};
		}
		else
		{
			switch(len)
			{
			case 3: d |= data[2] << 16;
			case 2: d |= data[1] << 8;
			case 1: d |= data[0];
			case 0: h ^= (t >> sr) | (d << sl);
					h *= m;
			}
		}

		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;

		return h;
	}
	else
	{
		while(len >= 4)
		{
			unsigned int k = *(unsigned int *)data;

			MIX(h,k,m);

			data += 4;
			len -= 4;
		}

		//----------
		// Handle tail bytes

		switch(len)
		{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
				h *= m;
		};

		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;

		return h;
	}
}

//-----------------------------------------------------------------------------
// MurmurHashNeutral2, by Austin Appleby

// Same as MurmurHash2, but endian- and alignment-neutral.
// Half the speed though, alas.

inline unsigned int MurmurHashNeutral2 ( const void * key, int len, unsigned int seed )
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h = seed ^ len;

	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		unsigned int k;

		k  = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= m; 
		k ^= k >> r; 
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}
	
	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= m;
	};

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
} 

// $(CitedCodeEnd)

} // namespace orig_murmurhash

unsigned int oHash_murmur2(const void* buf, unsigned int len, unsigned int seed)
{
	return orig_murmur_hash::MurmurHash2(buf, len, seed);
}

unsigned int oHash_murmur2aligned(const void* buf, unsigned int len, unsigned int seed)
{
	return orig_murmur_hash::MurmurHashAligned2(buf, len, seed);
}

unsigned int oHash_murmur2neutral(const void* buf, unsigned int len, unsigned int seed)
{
	return orig_murmur_hash::MurmurHashNeutral2(buf, len, seed);
}

unsigned int oHash_fourcc(const char* s)
{
	return static_cast<unsigned int>((tolower(s[0]) << 24) | (tolower(s[1]) << 16) | (tolower(s[2]) << 8) | tolower(s[3]));
}

unsigned int oHash_fourcci(const char* s)
{
	return static_cast<unsigned int>((tolower(tolower(s[0])) << 24) | (tolower(tolower(s[1])) << 16) | (tolower(tolower(s[2])) << 8) | tolower(tolower(s[3])));
}

unsigned long long oHash_eightcc(const char* s)
{
	return static_cast<unsigned long long>(((unsigned long long)tolower(s[0]) << 56) | ((unsigned long long)tolower(s[1]) << 48) | ((unsigned long long)tolower(s[2]) << 40) | ((unsigned long long)tolower(s[3]) << 32) | ((unsigned long long)tolower(s[4]) << 24) | ((unsigned long long)tolower(s[5]) << 16) | ((unsigned long long)tolower(s[6]) << 8) | (unsigned long long)tolower(s[7]));
}

unsigned long long oHash_eightcci(const char* s)
{
	return static_cast<unsigned long long>(((unsigned long long)tolower(s[0]) << 56) | ((unsigned long long)tolower(s[1]) << 48) | ((unsigned long long)tolower(s[2]) << 40) | ((unsigned long long)tolower(s[3]) << 32) | ((unsigned long long)tolower(s[4]) << 24) | ((unsigned long long)tolower(s[5]) << 16) | ((unsigned long long)tolower(s[6]) << 8) | (unsigned long long)tolower(s[7]));
}
