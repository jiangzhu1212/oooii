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
#include <oBasis/oOSC.h>
#include <oStd/algorithm.h>
#include <oStd/memory.h>
#include <oBasis/oError.h>
#include "oBasisTestStruct.h"
#include <vector>

static const char* GetTypeTags() { return ",Tc3Tih[fff]iTTs2f1sicbsi[iiii]dIfbt"; }

bool oBasisTest_oOSC()
{
	// Set up test object and a reference object we'll test against after going
	// through the OSC serialization and deserialization.
	char b1[10];
	memset(b1, 21, sizeof(b1));
	char b2[20];
	memset(b2, 22, sizeof(b2));

	oBASIS_TEST_STRUCT sent, received;
	oStd::memset4(&sent, 0xdeadc0de, sizeof(sent));
	oStd::memset4(&received, 0xdeadc0de, sizeof(received));
	oBasisTestStructInit(&sent, b1, sizeof(b1), b2, sizeof(b2));

	const char* _MessageName = "/TESTOSC/Run/TEST/sent";
	if( !((void*)(&sent.c2 + 1) != (void*)(&sent.b1size)) )
		return oErrorSetLast(std::errc::protocol_error, "Expected padding");

	size_t TestSize = sizeof(oBASIS_TEST_STRUCT);
	if(TestSize != oOSCCalculateDeserializedStructSize(GetTypeTags()) )
		return oErrorSetLast(std::errc::protocol_error, "oOSCCalculateDeserializedStructSize failed to compute correct size");

	static const size_t kExpectedArgsSize = 220;

	size_t argsSize = oOSCCalculateArgumentsDataSize(GetTypeTags(), sent);
	if (argsSize != kExpectedArgsSize)
		return oErrorSetLast(std::errc::protocol_error, "oOSCCalculateArgumentsDataSize failed to compute correct size");

	size_t msgSize = oOSCCalculateMessageSize(_MessageName, GetTypeTags(), argsSize);

	{
		std::vector<char> ScopedBuffer(msgSize);
		size_t SerializedSize = oOSCSerializeStructToMessage(_MessageName, GetTypeTags(), sent, oStd::data(ScopedBuffer), oStd::size(ScopedBuffer));
		if (SerializedSize <= 0)
			return oErrorSetLast(std::errc::protocol_error, "Failed to serialize buffer");
		if (!oOSCDeserializeMessageToStruct(oStd::data(ScopedBuffer), &received))
			return oErrorSetLast(std::errc::protocol_error, "Deserialization failed");

		// NOTE: Remember that received has string and blob pointers that point
		// DIRECTLY into the ScopedBuffer, so either the client code here needs to
		// copy those values to their own buffers, or received can only remain in
		// the scope of the ScopedBuffer.
		if (sent != received)
			return oErrorSetLast(std::errc::protocol_error, "Sent and received buffers do not match");
	}

	oErrorSetLast(0, "");
	return true;
}

