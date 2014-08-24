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
#include <malloc.h>

namespace ouro {

/** <citation
	usage="Implementation" 
	reason="9/7 CDF Wavlet transform"
	author="Gregoire Pau"
	description="http://www.embl.de/~gpau/misc/dwt97.c"
	license="*** Assumed Public Domain ***"
	licenseurl="http://www.embl.de/~gpau/misc/dwt97.c"
	modification="switched to floating point, renamed variables, removed malloc"
/>*/

// $(CitedCodeBegin)

void cdf97fwd(float* _pValues, size_t _NumValues)
{
	float a;
	size_t i;

	// Predict 1
	a=-1.586134342f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	} 
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Update 1
	a=-0.05298011854f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2.0f*a*_pValues[1];

	// Predict 2
	a=0.8829110762f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Update 2
	a=0.4435068522f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Scale
	a=1.0f/1.149604398f;
	for (i=0;i<_NumValues;i++) {
		if (i%2) _pValues[i]*=a;
		else _pValues[i]/=a;
	}

	// Pack
	float* TempBank = (float*)_alloca(sizeof(float) * _NumValues);

	for (size_t i = 0; i < _NumValues; i++) 
	{
		if (i%2==0) 
			TempBank[i/2]= _pValues[i];
		else 
			TempBank[_NumValues/2+i/2] = _pValues[i];
	}

	for (size_t i = 0; i < _NumValues; i++) 
		_pValues[i]=TempBank[i];
}

void cdf97inv(float* _pValues, size_t _NumValues) 
{
	float a;
	size_t i;

	// Unpack
	float* TempBank = (float*)_alloca(sizeof(float) * _NumValues);
	for (i=0;i<_NumValues/2;i++) {
		TempBank[i*2]=_pValues[i];
		TempBank[i*2+1]=_pValues[i+_NumValues/2];
	}
	for (i=0;i<_NumValues;i++) _pValues[i]=TempBank[i];

	// Undo scale
	a=1.149604398f;
	for (i=0;i<_NumValues;i++) {
		if (i%2) _pValues[i]*=a;
		else _pValues[i]/=a;
	}

	// Undo update 2
	a=-0.4435068522f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Undo predict 2
	a=-0.8829110762f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Undo update 1
	a=0.05298011854f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Undo predict 1
	a=1.586134342f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	} 
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];
}

// $(CitedCodeEnd)

} // namespace ouro

