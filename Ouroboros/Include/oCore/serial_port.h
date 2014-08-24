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
// Serial communication interface
#pragma once
#ifndef oCore_serial_port
#define oCore_serial_port

namespace ouro {

class serial_port
{
public:
	enum port
	{
		com1,
		com2,
		com3,
		com4,
	};
	
	enum parity
	{
		none,
		odd,
		even,
		mark,
		space,
	};
	
	enum stop_bits
	{
		one,
		one5,
		two,
	};
	
	struct info
	{
		info()
			: com_port(com1)
			, baud(9600)
			, byte_size(8)
			, parity(none)
			, stop_bits(one)
			, read_timeout_ms(200)
			, per_byte_read_timeout_ms(10)
		{}
		
		port com_port;
		int baud;
		int byte_size;
		enum parity parity;
		enum stop_bits stop_bits;
		unsigned int read_timeout_ms;
		unsigned int per_byte_read_timeout_ms;
	};
	
	virtual info get_info() const = 0;
	virtual void send(const void* _pBuffer, size_t _SizeofBuffer) = 0;
	virtual size_t receive(void* _pBuffer, size_t _SizeofBuffer) = 0;

	static std::shared_ptr<serial_port> make(const info& _Info);
};

} // namespace ouro

#endif
