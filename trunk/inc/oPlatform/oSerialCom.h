// $(header)
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