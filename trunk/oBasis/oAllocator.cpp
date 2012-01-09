// $(header)
#include <oBasis/oError.h>
#include "oAllocatorTLSF.h"

const oGUID& oGetGUID(threadsafe const oAllocator* threadsafe const * /* = 0 */)
{
	// {B429A4E8-B365-4890-AEB5-15E1BE64C573}
	static const oGUID guid = { 0xb429a4e8, 0xb365, 0x4890, { 0xae, 0xb5, 0x15, 0xe1, 0xbe, 0x64, 0xc5, 0x73 } };
	return guid;
}
