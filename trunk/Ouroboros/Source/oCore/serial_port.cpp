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
#include <oCore/serial_port.h>
#include <oBase/finally.h>
#include <oBase/throw.h>
#include "../oStd/win.h"

using namespace oStd;

namespace ouro {

const char* as_string(const serial_port::port& _Port)
{
	switch (_Port)
	{
		case serial_port::com1: return "COM1";
		case serial_port::com2: return "COM2";
		case serial_port::com3: return "COM3";
		case serial_port::com4: return "COM4";
		default: break;
	}
	return "?";
}

static unsigned char get_stop_bits(serial_port::stop_bits _StopBits)
{
	switch(_StopBits)
	{
		case serial_port::one: return ONESTOPBIT;
		case serial_port::one5: return ONE5STOPBITS;
		case serial_port::two: return TWOSTOPBITS;
		default: break;
	}
	return ONESTOPBIT;
}

static unsigned char get_parity(serial_port::parity _Parity)
{
	switch (_Parity)
	{
		case serial_port::even: return EVENPARITY;
		case serial_port::odd: return ODDPARITY;
		case serial_port::none: return NOPARITY;
		case serial_port::mark: return MARKPARITY;
		case serial_port::space: return SPACEPARITY;
		default: break;
	}
	return NOPARITY;
}

class serial_port_impl : public serial_port
{
public:
	serial_port_impl(const info& _Info);
	~serial_port_impl();
	info get_info() const override;
	void send(const void* _pBuffer, size_t _SizeofBuffer) override;
	size_t receive(void* _pBuffer, size_t _SizeofBuffer) override;

private:
	HANDLE hFile;
	info Info;
};

serial_port_impl::serial_port_impl(const info& _Info)
	: Info(_Info)
	, hFile(INVALID_HANDLE_VALUE)
{
	HANDLE hNewFile = CreateFile(as_string(_Info.com_port), GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hNewFile == INVALID_HANDLE_VALUE)
		oTHROW(no_such_device, "%s does not exist", as_string(_Info.com_port));
	finally Close([&] { if (hNewFile != INVALID_HANDLE_VALUE) CloseHandle(hNewFile); });

	DCB dcb = {0};
	dcb.DCBlength = sizeof(DCB);
	if (!GetCommState(hNewFile, &dcb))
		throw windows::error();

	oCHECK_SIZE(BYTE, Info.byte_size);
	dcb.BaudRate = Info.baud;
	dcb.ByteSize = (BYTE)Info.byte_size;
	dcb.StopBits = get_stop_bits(Info.stop_bits);
	dcb.Parity = get_parity(Info.parity);

	if (!SetCommState(hNewFile, &dcb))
		throw windows::error();

	COMMTIMEOUTS cto;
	if (!GetCommTimeouts(hNewFile, &cto))
		throw windows::error();

	cto.ReadIntervalTimeout = Info.read_timeout_ms;
	cto.ReadTotalTimeoutConstant = Info.read_timeout_ms;
	cto.ReadTotalTimeoutMultiplier = Info.per_byte_read_timeout_ms;

	if (!SetCommTimeouts(hNewFile, &cto))
		throw windows::error();

	hFile = hNewFile;
	hNewFile = INVALID_HANDLE_VALUE; // prevent finally from killing the successful file
}

std::shared_ptr<serial_port> serial_port::make(const serial_port_impl::info& _Info)
{
	return std::make_shared<serial_port_impl>(_Info);
}

serial_port_impl::~serial_port_impl()
{
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
}

serial_port_impl::info serial_port_impl::get_info() const
{
	return Info;
}

void serial_port_impl::send(const void* _pBuffer, size_t _SizeofBuffer)
{
	DWORD written = 0;
	if (!WriteFile(hFile, _pBuffer, static_cast<DWORD>(_SizeofBuffer), &written, nullptr))
		throw windows::error();
	if (written != static_cast<DWORD>(_SizeofBuffer))
	{
		sstring b1, b2;
		format_bytes(b1, _SizeofBuffer, 2);
		format_bytes(b2, written, 2);
		oTHROW(io_error, "requested send of %s, but sent %s", b1.c_str(), b2.c_str());
	}
}

size_t serial_port_impl::receive(void* _pBuffer, size_t _SizeofBuffer)
{
	DWORD read = 0;
	if (!ReadFile(hFile, _pBuffer, static_cast<DWORD>(_SizeofBuffer), &read, nullptr))
		throw windows::error();
	return read;
}

} // namespace ouro
