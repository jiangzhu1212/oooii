// $(header)
#include <oBasis/oDispatchQueue.h>

const oGUID& oGetGUID(threadsafe const oDispatchQueue* threadsafe const*)
{
	// {85260463-6AA5-4BAB-951F-E1B044E9F692}
	static const oGUID IIDDispatchQueue = { 0x85260463, 0x6aa5, 0x4bab, { 0x95, 0x1f, 0xe1, 0xb0, 0x44, 0xe9, 0xf6, 0x92 } };
	return IIDDispatchQueue;
}
