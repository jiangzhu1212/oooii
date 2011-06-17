#include <oooii/oHashString.h>


errno_t oFromString(oHashString* _pValue, const char* _StrSource)
{
	_pValue->Set(_StrSource);
	return 0;
}