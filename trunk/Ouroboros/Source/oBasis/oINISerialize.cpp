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
#include <oBasis/oINISerialize.h>
#include <oBasis/oError.h>
#include <oBasis/oStrTok.h>
#include <oBase/fixed_string.h>
#include <vector>

using namespace ouro;

bool oINIReadCompound(void* _pDestination, const oRTTI& _RTTI, const ouro::ini& _INI, ouro::ini::section _Section, bool _FailOnMissingValues)
{
	if (_RTTI.GetType() != oRTTI_TYPE_COMPOUND)
		return oErrorSetLast(std::errc::invalid_argument);

	std::vector<ouro::sstring> FromStringFailed;

	for (int b = 0; b < _RTTI.GetNumBases(); b++)
	{
		const oRTTI* baseRTTI = _RTTI.GetBaseRTTI(b);
		oINIReadCompound(ouro::byte_add(_pDestination, _RTTI.GetBaseOffset(b)), *baseRTTI, _INI, _Section, _FailOnMissingValues);
	}

	for (int i = 0; i < _RTTI.GetNumAttrs(); i++)
	{
		const oRTTI_ATTR* f = _RTTI.GetAttr(i);
		bool notFound = true;
		switch(f->RTTI->Type)
		{
			case oRTTI_TYPE_ENUM:
			case oRTTI_TYPE_ATOM:
			{
				oCHECK_SIZE(int, f->Size);
				if (f->RTTI->FromString(_INI.find_value(_Section, f->Name), f->GetDestPtr(_pDestination), static_cast<int>(f->Size)))
					notFound = false;
				else
					FromStringFailed.push_back(f->Name);
				break;
			}

			case oRTTI_TYPE_CONTAINER:
			{
				const oRTTI* itemRTTI = f->RTTI->GetItemRTTI();

				char* ctx = nullptr;
				const char* token = oStrTok(_INI.find_value(_Section, f->Name), ",", &ctx);
				int i = 0;
				while (token)
				{
					token += strspn(token, oWHITESPACE);
					switch(itemRTTI->Type)
					{
						case oRTTI_TYPE_ENUM:
						case oRTTI_TYPE_ATOM:
						{
							if (!f->RTTI->SetItemCount(f->GetDestPtr(_pDestination), f->Size, i+1))
								FromStringFailed.push_back("item");
							oCHECK_SIZE(int, f->RTTI->GetItemSize());
							if (!itemRTTI->FromString(token, f->RTTI->GetItemPtr(f->GetDestPtr(_pDestination), f->Size, i++), static_cast<int>(f->RTTI->GetItemSize())))
								FromStringFailed.push_back(f->Name);
							break;
						}

						default:
						{
							break;
						}
					}
					token = oStrTok(nullptr, ",", &ctx);
				}
				break;
			}

			default:
			{
				notFound = false;
				ouro::sstring rttiName;
				oTRACE("No support for RTTI type: %s", f->RTTI->TypeToString(rttiName));
				break;
			}
		}

		if (notFound)
		{
			ouro::sstring compoundName;
			oTRACE("No INI value for: %s::%s in INI section %s in %s", _RTTI.GetName(compoundName), f->Name, _INI.section_name(_Section), _INI.name());
		}
	}

	if (!FromStringFailed.empty())
		return !_FailOnMissingValues;

	return true;
}

bool oINIWriteCompound(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, const oRTTI& _RTTI, const char* _Heading)
{
	if (_RTTI.GetType() != oRTTI_TYPE_COMPOUND)
		return oErrorSetLast(std::errc::invalid_argument);

	ouro::sncatf(_StrDestination, _SizeofStrDestination, "[%s]\r\n", _Heading);

	bool AtLeastOneMissing = false;
	ouro::xlstring Missing("The following fields were not written correctly: ");
	ouro::xlstring buf;

	for (int i=0; i<_RTTI.GetNumAttrs(); ++i)
	{
		const oRTTI_ATTR* f = _RTTI.GetAttr(i);
		switch(f->RTTI->Type)
		{
			case oRTTI_TYPE_ENUM:
			case oRTTI_TYPE_ATOM:
			{
				if (f->RTTI->ToString(buf, f->GetSrcPtr(_pSource)))
					ouro::sncatf(_StrDestination, _SizeofStrDestination, "%s = %s\r\n", f->Name, buf.c_str());
				else
				{	
					ouro::sncatf(Missing, "%s%s", AtLeastOneMissing ? ", " : "", f->Name);
					AtLeastOneMissing = true;
				}
				break;
			}

			case oRTTI_TYPE_CONTAINER:
			{
				const oRTTI* itemRTTI = f->RTTI->GetItemRTTI();
				bool first = true;
				for (int i = 0; i < f->RTTI->GetItemCount(f->GetSrcPtr(_pSource), f->Size); i++)
				{
					ouro::lstring elem_buf;
					switch(itemRTTI->Type)
					{
						case oRTTI_TYPE_ENUM:
						case oRTTI_TYPE_ATOM:
						{
							if (itemRTTI->ToString(elem_buf, f->RTTI->GetItemPtr(f->GetSrcPtr(_pSource), f->Size, i)))
							{
								if (first) snprintf(buf, "%s", elem_buf.c_str()); 
								else ouro::sncatf(buf, ",%s", elem_buf.c_str()); 
								first = false;
							}
							else
							{	
								ouro::sncatf(Missing, "%s%s", AtLeastOneMissing ? ", " : "", f->Name);
								AtLeastOneMissing = true;
							}
							break;
						}

						default:
						{
							break;
						}
					}
				}
				ouro::sncatf(_StrDestination, _SizeofStrDestination, "%s = %s\r\n", f->Name, buf.c_str());
				break;
			}

			default:
			{
				ouro::sstring rttiName;
				oTRACE("No support for RTTI type: %s", f->RTTI->TypeToString(rttiName));
				AtLeastOneMissing = true;
				break;
			}
		}
	}
	
	// Close with an empty line
	ouro::sncatf(_StrDestination, _SizeofStrDestination, "\r\n");

	if (AtLeastOneMissing)
		return oErrorSetLast(std::errc::io_error, Missing);
	return true;
}
