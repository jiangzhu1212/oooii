// $(header)
#include <oooii/oGUID.h>
#include <oooii/oInterface.h>

const oGUID& oGetGUID( threadsafe const oInterface* threadsafe const * )
{
	// {9370A4C3-B863-40c7-87F7-614474F40C20}
	static const oGUID oIIDInterface = { 0x9370a4c3, 0xb863, 0x40c7, { 0x87, 0xf7, 0x61, 0x44, 0x74, 0xf4, 0xc, 0x20 } };
	return oIIDInterface;
}
