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

namespace oStd {

bool is_ascii(const void* _pBuffer, size_t _SizeofBuffer)
{
	// http://code.activestate.com/recipes/173220-test-if-a-file-or-string-is-text-or-binary/
	// "The difference between text and binary is ill-defined, so this duplicates 
	// "the definition used by Perl's -T flag, which is: <br/> The first block 
	// "or so of the file is examined for odd characters such as strange control
	// "codes or characters with the high bit set. If too many strange characters 
	// (>30%) are found, it's a -B file, otherwise it's a -T file. Also, any file 
	// containing null in the first block is considered a binary file."
	static const float kThreshold = 0.10f; // 0.30f; // 30% seems too high to me.

	// Count non-text characters
	size_t nonTextCount = 0;
	const char* b = static_cast<const char*>(_pBuffer);
	for (size_t i = 0; i < _SizeofBuffer; i++)
		if (b[i] == 0 || (b[i] & 0x80))
			nonTextCount++;

	// Determine results
	float percentNonAscii = nonTextCount / static_cast<float>(_SizeofBuffer);
	return percentNonAscii < kThreshold;
}

} // namespace oStd
