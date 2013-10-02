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
#include <oBasis/oRefCount.h>
#include <oPlatform/oSerialCom.h>
#include <oPlatform/Windows/oWindows.h>

class oSerialComImpl : public oSerialCom
{
public:
	oSerialComImpl(const DESC& _Desc, bool* _pSuccess);
	~oSerialComImpl();
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();


	bool Send(const char* _pString, unsigned int _StrLen) override;
	int Receive(char *_pString, unsigned int _BufferLength) override;
private:
	static const unsigned char GetStopBits(STOPBITS _StopBits)
	{
		switch(_StopBits)
		{
		case ONE:
			return ONESTOPBIT;
		case ONE5:
			return ONE5STOPBITS;
		case TWO:
			return TWOSTOPBITS;
		}

		return ONESTOPBIT;
	}

	static unsigned char GetParity(PARITY _Parity)
	{
		switch(_Parity)
		{
		case EVEN:
			return EVENPARITY;
		case ODD:
			return ODDPARITY;
		case NONE:
			return NOPARITY;
		case MARK:
			return MARKPARITY;
		case SPACE:
			return SPACEPARITY;
		}

		return NOPARITY;
	}

	DESC Desc;
	oRefCount Refcount;

	HANDLE File;
};

namespace ouro {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oSerialCom::COM& _Value)
{
	switch(_Value)
	{
	case oSerialCom::COM1:
		strlcpy(_StrDestination, "COM1", _SizeofStrDestination);
		return _StrDestination;

	case oSerialCom::COM2:
		strlcpy(_StrDestination, "COM2", _SizeofStrDestination);
		return _StrDestination;

	case oSerialCom::COM3:
		strlcpy(_StrDestination, "COM3", _SizeofStrDestination);
		return _StrDestination;

	case oSerialCom::COM4:
		strlcpy(_StrDestination, "COM4", _SizeofStrDestination);
		return _StrDestination;
	}

	oErrorSetLast(std::errc::invalid_argument, "Unrecognized COM");
	return nullptr;
}

bool from_string(oSerialCom::COM* _pAddress, const char* _StrSource)
{
	if (0 == _stricmp(_StrSource, "COM1"))
	{
		*_pAddress = oSerialCom::COM1;
		return true;
	}

	if (0 == _stricmp(_StrSource, "COM2"))
	{
		*_pAddress = oSerialCom::COM2;
		return true;
	}

	if (0 == _stricmp(_StrSource, "COM3"))
	{
		*_pAddress = oSerialCom::COM3;
		return true;
	}

	if (0 == _stricmp(_StrSource, "COM4"))
	{
		*_pAddress = oSerialCom::COM4;
		return true;
	}

	return false;
}

} // namespace ouro

oSerialComImpl::oSerialComImpl( const DESC& _Desc, bool* _pSuccess )
	: Desc(_Desc)
	, File(nullptr)
{
	char com[64];
	if (!ouro::to_string(com, Desc.Com))
		return;

	File = CreateFile( com,
		GENERIC_READ | GENERIC_WRITE,
		0,      //  must be opened with exclusive-access
		nullptr,   //  default security attributes
		OPEN_EXISTING, //  must use OPEN_EXISTING
		0,      //  not overlapped I/O
		nullptr ); //  hTemplate must be NULL for comm devices

	if(File == INVALID_HANDLE_VALUE)
	{
		oErrorSetLast(std::errc::no_such_device, "Com %s does not exist", com);
		return; // CreateFile calls set last error
	}

	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	if (!GetCommState(File, &dcbSerialParams))
		return; // GetCommState calls set last error

	dcbSerialParams.BaudRate = Desc.BaudRate;
	dcbSerialParams.ByteSize = Desc.ByteSize;
	dcbSerialParams.StopBits = GetStopBits(Desc.StopBits);
	dcbSerialParams.Parity = GetParity(Desc.Parity);

	if(!SetCommState(File, &dcbSerialParams))
		return; // SetCommState sets last error

	COMMTIMEOUTS Timeouts;
	if( !GetCommTimeouts( File, &Timeouts ) )
		return;

	// @oooii-kevin: FIXME, magic number timeouts
	// for Planar commands
	Timeouts.ReadIntervalTimeout = 200;
	Timeouts.ReadTotalTimeoutConstant = 200;
	Timeouts.ReadTotalTimeoutMultiplier = 10;

	if( !SetCommTimeouts( File, &Timeouts ) )
		return;

	*_pSuccess = true;
}

oSerialComImpl::~oSerialComImpl()
{
	CloseHandle(File);
}

bool oSerialComImpl::Send( const char* _pString, unsigned int _StrLen )
{
	DWORD Written = 0;
	WriteFile(File, _pString, _StrLen, &Written, nullptr );
	return Written == _StrLen;
}

int oSerialComImpl::Receive(char *_pString, unsigned int _BufferLength)
{
	DWORD Read = 0;
	ReadFile(File, _pString, _BufferLength, &Read, nullptr);
	return (int)Read;
}

bool oSerialComCreate(const oSerialCom::DESC& _Desc, oSerialCom** _ppSerialCom)
{
	bool success = false;
	oCONSTRUCT(_ppSerialCom, oSerialComImpl(_Desc, &success));
	return success;
}