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
#include <oBasis/oURIQuerySerialize.h>
#include <oBasis/oError.h>
#include <oBase/fixed_string.h>

using namespace ouro;

bool oURIQueryReadValue(void* _pDest, int _SizeOfDest, const char* _StrSource, const oRTTI& _RTTI)
{
	if ((_RTTI.GetTraits() & (type_trait_flag::is_integralf | type_trait_flag::is_floating_pointf)) != 0 && (_RTTI.GetNumStringTokens() == 1) && (*_StrSource != '\"'))
	{
		sstring Typename;
		if (strstr(_RTTI.GetName(Typename), "char"))
		{
			int Value;
			if (!from_string(&Value, _StrSource)) 
				return false;
			*((char*)_pDest) = (Value & 0xff);
			return true;
		}
		return _RTTI.FromString(_StrSource, _pDest, _SizeOfDest);
	}
	else
	{
		return _RTTI.FromString(_StrSource, _pDest, _SizeOfDest);
	}
}

bool oURIQueryReadCompound(void* _pDestination, const oRTTI& _RTTI, const char* _Query, bool _FailOnMissingValues)
{
	if (_RTTI.GetType() != oRTTI_TYPE_COMPOUND)
		return oErrorSetLast(std::errc::invalid_argument);

	bool Found = false;
	oURIQueryEnumKeyValuePairs(_Query, [&](const char* _key, const char* _Value){

		// Enumerating RTTI compound attributes is lighter weight than enumerating
		// the query pairs, so let's do that in the inner loop
		_RTTI.EnumAttrs(true, [&](const oRTTI_ATTR& _Attr)->bool{
			if (0 == _stricmp(_Attr.Name, _key))
			{
				Found = true;
				switch(_Attr.RTTI->Type)
				{
					case oRTTI_TYPE_ENUM:
					case oRTTI_TYPE_ATOM:
					{
						oURIQueryReadValue(_Attr.GetDestPtr(_pDestination), as_int(_Attr.Size), _Value, *_Attr.RTTI);
						break;
					}

					default:
						break;
				}
				// Stop enumerating
				return false;
			}
			// Continue enumerating
			return true;
		});

	});
	
	if (_FailOnMissingValues && !Found)
		return oErrorSetLast(std::errc::no_such_file_or_directory);

	return true;
}

bool oURIQueryWriteValue(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, const oRTTI& _RTTI)
{
	xxlstring buf;
	sstring Typename;
	if (_RTTI.ToString(buf, _pSource))
	{
		if ((_RTTI.GetTraits() & (type_trait_flag::is_integralf | type_trait_flag::is_floating_pointf)) != 0  && (_RTTI.GetNumStringTokens() == 1))
		{
			if (strstr(_RTTI.GetName(Typename), "uchar"))
				sncatf(_StrDestination, _SizeofStrDestination, "%d", (int)*(uchar*)_pSource);
			else if (strstr(_RTTI.GetName(Typename), "char"))
				sncatf(_StrDestination, _SizeofStrDestination, "%d", (int)*(char*)_pSource);
			else
				sncatf(_StrDestination, _SizeofStrDestination, "%s", buf.c_str());
		}
		else
			strlcpy(_StrDestination, buf.c_str(), _SizeofStrDestination);
	}
	else
		return false;

	return true;
}

bool oURIQueryWriteCompound(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, const oRTTI& _RTTI)
{
	if (_RTTI.GetType() != oRTTI_TYPE_COMPOUND)
		return oErrorSetLast(std::errc::invalid_argument);

	bool firstNameValuePair = true;
	_RTTI.EnumAttrs(true, [&](const oRTTI_ATTR& _Attr)->bool{

		switch(_Attr.RTTI->Type)
		{
		case oRTTI_TYPE_ENUM:
		case oRTTI_TYPE_ATOM:
			{
				xxlstring buf;
				if (oURIQueryWriteValue(buf.c_str(), buf.capacity(), _Attr.GetSrcPtr(_pSource), *_Attr.RTTI))
				{
					if (!firstNameValuePair) sncatf(_StrDestination, _SizeofStrDestination, "&"); firstNameValuePair = false;
					sncatf(_StrDestination, _SizeofStrDestination, "%s=%s", _Attr.Name, buf.c_str());
				}
				break;
			}

		default:
			{
				sstring rttiName;
				return oErrorSetLast(std::errc::not_supported, "No support for RTTI type: %s", _Attr.RTTI->TypeToString(rttiName));
			}
		}
		return true;
	});

	return true;
}

