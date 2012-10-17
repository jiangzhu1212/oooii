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
#include <oBasis/oOSC.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oError.h>
#include <oBasis/oMemory.h>
#include <ctime>
#include <vector>

struct TEST
{
	static const char* GetTypeTags() { return ",Tc3Tih[fff]iTTs2f1sicbsi[iiii]dIfbt"; }

	bool blA;
	char c1;
	char OOOiiString[64 * 3];
	bool blB;
	int int1;
	long long long1;
	float floatArray[3];
	int int2;
	bool blC;
	bool blD;
	const char* p1;
	char TestString[64 * 2];
	float float1;
	char FinalTestString[64];
	const char* p2;
	int int3;
	char c2;
	// Expected padding
	int b1size;
	const void* b1;
	const char* NullString;
	int int4;
	int intArray[4];
	double double1;
	float float2;
	int b2size;
	const void* b2;
	oNTPTimestamp time;
};

void AssignTestValues(TEST* _pTest, const void* _pBuffer1, int _SizeofBuffer1, const void* _pBuffer2, int _SizeofBuffer2)
{
	_pTest->c1 = 'c';
	oStrcpy(_pTest->OOOiiString, "OOOiiString");
	_pTest->int1 = 1;
	_pTest->long1 = 11111111;
	_pTest->floatArray[0] = 1.00f;
	_pTest->floatArray[1] = 1.01f;
	_pTest->floatArray[2] = 1.02f;
	_pTest->int2 = 2;
	_pTest->blA = false;
	_pTest->blB = true;
	_pTest->blC = false;
	_pTest->blD = true;
	_pTest->p1 = "p1 a string larger than a pointer size (len=48)";
	oStrcpy(_pTest->TestString, "TestString");
	_pTest->float1 = 1.111f;
	// Expected padding
	_pTest->p2 = "p2";
	_pTest->int3 = 3;
	_pTest->c2 = 'K';
	_pTest->b1size = _SizeofBuffer1;
	_pTest->b1 = _pBuffer1;
	_pTest->NullString = nullptr;
	_pTest->int4 = 4;
	_pTest->intArray[0] = 100;
	_pTest->intArray[1] = 101;
	_pTest->intArray[2] = 102;
	_pTest->intArray[3] = 103;
	_pTest->double1 = 7.77777;
	_pTest->float2 = 2.222f;
	_pTest->b2size = _SizeofBuffer2;
	oStrcpy(_pTest->FinalTestString, "FinalTestString");
	_pTest->b2 = _pBuffer2;
	time_t t = time(nullptr);
	oVERIFY(oDateConvert(t, &_pTest->time));
}

bool operator==(const TEST& x, const TEST& y)
{
	return x.c1 == y.c1
		&& !oStrcmp(x.OOOiiString, y.OOOiiString)
		&& x.int1 == y.int1
		&& x.long1 == y.long1
		&& x.floatArray[0] == y.floatArray[0]
		&& x.floatArray[1] == y.floatArray[1]
		&& x.floatArray[2] == y.floatArray[2]
		&& x.int2 == y.int2
		&& x.blA == y.blA
		&& x.blB == y.blB
		&& x.blC == y.blC
		&& x.blD == y.blD
		&& !oStrcmp(x.p1, y.p1)
		&& !oStrcmp(x.TestString, y.TestString)
		&& x.float1 == y.float1
		&& !oStrcmp(x.p2, y.p2)
		&& x.int3 == y.int3
		&& x.c2 == y.c2
		&& x.b1size == y.b1size
		&& !memcmp(x.b1, y.b1, x.b1size)
		&& x.NullString == y.NullString
		&& x.int4 == y.int4
		&& x.intArray[0] == y.intArray[0]
		&& x.intArray[1] == y.intArray[1]
		&& x.intArray[2] == y.intArray[2]
		&& x.intArray[3] == y.intArray[3]
		&& x.double1 == y.double1
		&& x.float2 == y.float2
		&& x.b2size == y.b2size
		&& !oStrcmp(x.FinalTestString, y.FinalTestString)
		&& !memcmp(x.b2, y.b2, x.b2size)
		&& x.time == y.time
		;
}

bool operator!=(const TEST& x, const TEST& y) { return !(x == y); }

bool oBasisTest_oOSC()
{
	// Set up test object and a reference object we'll test against after going
	// through the OSC serialization and deserialization.
	char b1[10];
	memset(b1, 21, sizeof(b1));
	char b2[20];
	memset(b2, 22, sizeof(b2));

	TEST sent, received;
	oMemset4(&sent, 0xdeadc0de, sizeof(sent));
	oMemset4(&received, 0xdeadc0de, sizeof(received));
	AssignTestValues(&sent, b1, sizeof(b1), b2, sizeof(b2));

	const char* _MessageName = "/TESTOSC/Run/TEST/sent";
	if( !((void*)(&sent.c2 + 1) != (void*)(&sent.b1size)) )
		return oErrorSetLast(oERROR_GENERIC, "Expected padding");

	size_t TestSize = sizeof(TEST);
	if(TestSize != oOSCCalculateDeserializedStructSize(TEST::GetTypeTags()) )
		return oErrorSetLast(oERROR_GENERIC, "oOSCCalculateDeserializedStructSize failed to compute correct size");

	static const size_t kExpectedArgsSize = 220;

	size_t argsSize = oOSCCalculateArgumentsDataSize(TEST::GetTypeTags(), sent);
	if (argsSize != kExpectedArgsSize)
		return oErrorSetLast(oERROR_GENERIC, "oOSCCalculateArgumentsDataSize failed to compute correct size");

	size_t msgSize = oOSCCalculateMessageSize(_MessageName, TEST::GetTypeTags(), argsSize);

	{
		std::vector<char> ScopedBuffer(msgSize);
		size_t SerializedSize = oOSCSerializeStructToMessage(_MessageName, TEST::GetTypeTags(), sent, oGetData(ScopedBuffer), oGetDataSize(ScopedBuffer));
		if (SerializedSize <= 0)
			return oErrorSetLast(oERROR_GENERIC, "Failed to serialize buffer");
		if (!oOSCDeserializeMessageToStruct(oGetData(ScopedBuffer), &received))
			return oErrorSetLast(oERROR_GENERIC, "Deserialization failed");

		// NOTE: Remember that received has string and blob pointers that point
		// DIRECTLY into the ScopedBuffer, so either the client code here needs to
		// copy those values to their own buffers, or received can only remain in
		// the scope of the ScopedBuffer.
		if (sent != received)
			return oErrorSetLast(oERROR_GENERIC, "Sent and received buffers do not match");
	}

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

