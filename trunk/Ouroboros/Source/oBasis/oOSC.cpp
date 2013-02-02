/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oAssert.h>
#include <oBasis/oByte.h>
#include <oBasis/oError.h>
#include <oBasis/oStringize.h>

#define oUNKNOWN_SIZE SIZE_MAX

#define oASSERT_ALIGNED(_pDestination) oASSERT(oIsByteAligned(_pDestination, 4), "The destination pointer must be 4-byte aligned");

// If a nullptr is serialized as a string, we still need to identify that the
// occurance happened, so write a special character.
static const char NULL_STRING = -1;

template<typename T> static T oOSCPlatformEndianSwap(const T& _X)
{
	#ifdef oLITTLEENDIAN
		return oByteSwap(_X);
	#else
		return _X;
	#endif
}

// Field alignment is in terms of offset from base struct pointer, not absolute
// memory. This is really apparent when there are double-word types, such as 
// long longs on 32-bit builds.
void* MoveToNextField(const void* _pStructBase, const void* _pUnalignedEndOfLastField, size_t _AlignmentOfNextField)
{
	ptrdiff_t offset = oByteDiff(_pUnalignedEndOfLastField, _pStructBase);
	return oByteAdd((void*)_pStructBase, oByteAlign(offset, _AlignmentOfNextField));
}

size_t SizeofFixedString(int _Type)
{
	return (_Type - '0') * 64 * sizeof(char);
}

template<typename ptr_t>
struct Visitor
{
	typedef oFUNCTION<void(int _Type, ptr_t _pField, size_t _SizeofField)> Fn;
};

namespace STRUCT {

// STRUCT* Aligns to the size of type and visits data at that address and then
// returns a pointer just after the read field

template<typename ptr_t>
static ptr_t VisitNextIntrinsic(int _Type, ptr_t _pStructBase, ptr_t _pField, size_t _SizeofField, const typename Visitor<ptr_t>::Fn& _Visitor)
{
	void* p = MoveToNextField(_pStructBase, _pField, _SizeofField);
	_Visitor(_Type, p, _SizeofField);
	return oByteAdd(p, _SizeofField);
}

template<typename ptr_t>
static ptr_t VisitNextChar(ptr_t _pStructBase, ptr_t _pField, const typename Visitor<ptr_t>::Fn& _Visitor)
{
	_Visitor('c', _pField, sizeof(char));
	return oByteAdd(_pField, sizeof(char));
}

template<typename ptr_t>
static ptr_t VisitNextString(ptr_t _pStructBase, ptr_t _pField, const typename Visitor<ptr_t>::Fn& _Visitor)
{
	void* p = MoveToNextField(_pStructBase, _pField, sizeof(const char*));
	const char* s = *(const char**)p;
	_Visitor('s', p, s ? (oStrlen(s)+1) : sizeof(NULL_STRING));
	return oByteAdd(p, sizeof(ptr_t));
}

template<typename ptr_t>
static ptr_t VisitNextFixedString(int _Type, ptr_t _pStructBase, ptr_t _pField, const typename Visitor<ptr_t>::Fn& _Visitor)
{
	void* p = MoveToNextField(_pStructBase, _pField, sizeof(char));
	_Visitor(_Type, p, oStrlen((const char*)p)+1);
	return oByteAdd(p, SizeofFixedString(_Type));
}

template<typename ptr_t>
static ptr_t VisitNextBlob(ptr_t _pStructBase, ptr_t _pField, const typename Visitor<ptr_t>::Fn& _Visitor)
{
	void* p = MoveToNextField(_pStructBase, _pField, sizeof(int));
	int size = *(int*)p;
	p = MoveToNextField(_pStructBase, oByteAdd(p, sizeof(int)), sizeof(ptr_t));
	_Visitor('b', *(void**)p, size);
	return oByteAdd(p, sizeof(ptr_t));
}

} // namespace STRUCT

template<typename ptr_t>
bool oOSCVisitStructFieldsInternal(const typename Visitor<ptr_t>::Fn& _Visitor, const char* _TypeTags, ptr_t _pStruct, size_t _SizeofStruct, char* _pOptionalPatchTags = nullptr)
{
	if (!_Visitor || !_TypeTags || !_pStruct || !_SizeofStruct)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "null value");

	if (*_TypeTags != ',')
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "TypeTags must start with a ',' character");

	auto tag = _TypeTags;
	auto patchTag = _pOptionalPatchTags;
	ptr_t p = _pStruct;
	auto end = oByteAdd(p, _SizeofStruct);

	while (*(++tag))
	{
		if (p >= end)
		{
			if( p == end && ( *tag == 0 || *tag == '[' || *tag == ']' ) )  // If we finish on any of these tags it's ok since they do not expect valid data
				return true;

			return oErrorSetLast(oERROR_INVALID_PARAMETER, "Tag string \"%s\" has run past the size of the struct pointed to by pointer 0x%p", _TypeTags, _pStruct);
		}

		if(patchTag)
			++patchTag;

		switch (*tag)
		{
			case 'r': case 'i': case 'f': p = STRUCT::VisitNextIntrinsic(*tag, _pStruct, p, 4, _Visitor); break;
			case 'h': case 't': case 'd': p = STRUCT::VisitNextIntrinsic(*tag, _pStruct, p, 8, _Visitor); break;
			case 'c': p = STRUCT::VisitNextChar(_pStruct, p, _Visitor); break;
			case 's': p = STRUCT::VisitNextString(_pStruct, p, _Visitor); break;
			case 'b': p = STRUCT::VisitNextBlob(_pStruct, p, _Visitor); break;
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': p = STRUCT::VisitNextFixedString(*tag, _pStruct, p, _Visitor); break;
			case 'T': case 'F': 
			{
				// Special case for boolean types
				bool* pBTag = (bool*)p;
				int bits = 0;
				while(*tag == 'T' || *tag == 'F')
				{
					_Visitor(*tag, nullptr, 0);

					if( patchTag )
					{
						*patchTag = *pBTag ? 'T' : 'F';
						++patchTag;
					}

					++bits;
					++tag;
					++pBTag;
					p = oByteAdd( p, 1);
				}
				// Offset number of bytes plus padding
				//p = oByteAdd( p, ( bits / 8 ) + 1);

				// Back the tags up so we increment correctly in the next pass
				--tag; 
				if( patchTag )
					--patchTag;
			}
			break;
			default: _Visitor(*tag, nullptr, 0); break;
		}
	}

	return true;
}
bool oOSCVisitStructFields(const char* _TypeTags, void* _pStruct, size_t _SizeofStruct, oOSCVisitorFn _Visitor)
{
	return oOSCVisitStructFieldsInternal<void*>(_Visitor, _TypeTags, _pStruct, _SizeofStruct, nullptr);
}
bool oOSCVisitStructFields(const char* _TypeTags, const void* _pStruct, size_t _SizeofStruct, oOSCVisitorConstFn _Visitor)
{
	return oOSCVisitStructFieldsInternal<const void*>(_Visitor, _TypeTags, _pStruct, _SizeofStruct, nullptr);
}

namespace TAG {
	// TAG functions assume 4-byte alignment, assumes char is stored as an int and
	// does endian swapping on the data before visiting.

template<typename T, typename ptr_t, typename fn_t> 
static ptr_t VisitNextIntrinsic(int _Type, ptr_t _pArgument, const fn_t& _Visitor)
{
	T swapped = oOSCPlatformEndianSwap(*(const T*)_pArgument);
	_Visitor(_Type, &swapped, sizeof(T));
	return oByteAdd(_pArgument, sizeof(T));
}

template<typename ptr_t, typename fn_t>
static ptr_t VisitNextChar( ptr_t _pArgument, const fn_t& _Visitor)
{
	int cAsInt = oOSCPlatformEndianSwap(*(const int*)_pArgument);
	char c = (char)cAsInt;
	_Visitor('c', &c, sizeof(char)); // be honest about data and describe as a char
	return oByteAdd(_pArgument, sizeof(int)); // move past the original int in the buffer
}

template<typename ptr_t, typename fn_t>
static ptr_t VisitNextString(int _Type, ptr_t _pArgument, const fn_t& _Visitor)
{
	size_t size = oStrlen((const char*)_pArgument) + 1;
	_Visitor(_Type, _pArgument, size);
	return oByteAdd(_pArgument, oByteAlign(size, 4));
}

template<typename ptr_t, typename fn_t>
static ptr_t VisitNextBlob( ptr_t _pArgument, const fn_t& _Visitor)
{
	int size = oOSCPlatformEndianSwap(*(const int*)_pArgument);
	ptr_t p = oByteAdd(_pArgument, sizeof(int));
	_Visitor('b', p, size);
	return oByteAdd(p, oByteAlign(size, 4));
}

} // namespace TAG

template<typename ptr_t, typename fn_t>
bool oOSCVisitMessageTypeTagsInternal(const char* _TypeTags, ptr_t _pMessageArguments, fn_t _Visitor)
{
	if (!_Visitor || !_TypeTags || *_TypeTags != ',' || !_pMessageArguments)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	auto tag = _TypeTags;
	auto p = _pMessageArguments;
	while (*(++tag))
	{
		switch (*tag)
		{
			case 'r': case 'i': case 'f': p = TAG::VisitNextIntrinsic<int>(*tag, p, _Visitor); break;
			case 'h': case 't': case 'd': p = TAG::VisitNextIntrinsic<long long>(*tag, p, _Visitor); break;
			case 'c': p = TAG::VisitNextChar(p, _Visitor); break;
			case 'b': p = TAG::VisitNextBlob(p, _Visitor); break;
			case 's': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': p = TAG::VisitNextString(*tag, p, _Visitor); break;
			default: _Visitor(*tag, nullptr, 0); break;
		}
	}

	return true;
}

bool oOSCVisitMessageTypeTags(const char* _TypeTags, const void* _pMessageArguments, oOSCVisitorConstFn _Visitor)
{
	return oOSCVisitMessageTypeTagsInternal(_TypeTags, _pMessageArguments, _Visitor);
}

bool oOSCVisitMessageTypeTags(const char* _TypeTags, void* _pMessageArguments, oOSCVisitorFn _Visitor)
{
	return oOSCVisitMessageTypeTagsInternal(_TypeTags, _pMessageArguments, _Visitor);
}

static void oOSCSumFieldSizes(int _Type, const void* _pField, size_t _SizeofField, size_t* _pOutSizeSum)
{
	*_pOutSizeSum += oByteAlign(_SizeofField, 4);
	if (_Type == 'b') *_pOutSizeSum += 4; // for the size item
}

size_t oOSCCalculateNumFields(const char* _TypeTags)
{
	bool InArray = false;
	size_t nFields = 0;
	const char* tag = _TypeTags;
	while (*(++tag))
	{
		if (*tag == '[')
			InArray = true;
		else if (*tag == ']')
		{
			nFields++;
			InArray = false;
		}

		if (!InArray)
		{
			static const char* countedTags = "rifhtdcsb123456789P";
			if (strchr(countedTags, *tag))
				nFields++;
		}
	}

	return nFields;
}

size_t oOSCCalculateArgumentsDataSize(const char* _TypeTags, const void* _pStruct, size_t _SizeofStruct)
{
	size_t size = 0;
	oVERIFY(oOSCVisitStructFields(_TypeTags, _pStruct, _SizeofStruct, oBIND(oOSCSumFieldSizes, oBIND1, oBIND2, oBIND3, &size)));
	return size;
}

size_t oOSCCalculateMessageSize(const char* _Address, const char* _TypeTags, size_t _ArgumentsDataSize)
{
	size_t size = oByteAlign(oStrlen(_Address) + 1, 4);
	if (_TypeTags)
		size += oByteAlign(oStrlen(_TypeTags) + 1, 4);
	return size + _ArgumentsDataSize;
}

size_t oOSCCalculateBundleSize(size_t _NumSubbundles, size_t _SumOfAllSubbundleSizes)
{	// sizeof "#bundle\0" + sizeof time-tag + a size per bundle + size of all bundles
	return 8 + 8 + (sizeof(int) * _NumSubbundles) + _SumOfAllSubbundleSizes;
}

template<typename T>
void AlignedIncrement(size_t* pSize)
{
	*pSize = oByteAlign(*pSize, sizeof(T)) + sizeof(T);
}

size_t oOSCCalculateDeserializedStructSize(const char* _TypeTags)
{
	if (!_TypeTags || *_TypeTags != ',')
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return 0;
	}

	size_t size = 0;
	const char* tag = _TypeTags;
	while (*(++tag))
	{
		switch(*tag)
		{
			case 'r': case 'i': case 'f': AlignedIncrement<int>(&size); break;
			case 'h': case 't': case 'd': AlignedIncrement<long long>(&size); break;
			case 'c': AlignedIncrement<char>(&size); break;
			case 'b': AlignedIncrement<int>(&size); AlignedIncrement<void*>(&size); break;
			case 's': AlignedIncrement<void*>(&size); break;
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': size += atoi(tag) * 64; break;
			case 'T': case 'F': 
			{
					int bits = 1;
					while(tag[1] == 'T' || tag[1] == 'F')
				{
					++bits;
					++tag;
				}
				size += (bits / 8) + 1;
			}
		}
	}

	return oByteAlign(size, sizeof(int));
}
const char* oOSCGetMessageAddress(const void* _pMessage)
{
	return *(const char*)_pMessage == '/' ? (const char*)_pMessage : nullptr;
}

const char* oOSCGetMessageTypeTags(const void* _pMessage)
{
	const char* Address = oOSCGetMessageAddress(_pMessage);
	if (Address)
	{
		const char* TypeTags = oByteAdd(Address, oByteAlign(oStrlen(Address)+1, 4));
		return *TypeTags == ',' ? TypeTags : nullptr;
	}
	
	return nullptr;
}

namespace SERIALIZE {

template<typename T> static void* NextIntrinsic(const void* _pStructBase, void* _pDestination, const void* _pUnalignedEndOfLastField)
{
	oASSERT_ALIGNED(_pDestination);
	const void* s = MoveToNextField(_pStructBase, _pUnalignedEndOfLastField, std::alignment_of<T>::value);
	*(T*)_pDestination = oOSCPlatformEndianSwap(*(const T*)s);
	return oByteAdd(_pDestination, sizeof(T));
}

static void* NextChar(void* _pDestination, const void* _pSource)
{
	oASSERT_ALIGNED(_pDestination);
	int cAsInt = *(const char*)_pSource;
	*(int*)_pDestination = oOSCPlatformEndianSwap(cAsInt);
	return oByteAdd(_pDestination, sizeof(int));
}

static void* CopyNextBuffer(void* _pDestination, size_t _SizeofDestination, const void* _pBuffer, size_t _SizeofBuffer)
{
	oASSERT_ALIGNED(_pDestination);
	oASSERT(_SizeofDestination >= oByteAlign(_SizeofBuffer, 4), "");
	oASSERT(_pBuffer, "A valid buffer must be specified");
	memcpy(_pDestination, _pBuffer, _SizeofBuffer);
	// pad with zeros out to 4-byte alignment
	char* p = (char*)oByteAdd(_pDestination, _SizeofBuffer);
	char* pend = oByteAlign(p, 4);
	while (p < pend)
		*p++ = 0;
	return p;
}

static void* NextString(void* _pDestination, size_t _SizeofDestination, const char* _String)
{
	oASSERT_ALIGNED(_pDestination);
	if (_String)
		return CopyNextBuffer(_pDestination, _SizeofDestination, _String, oStrlen(_String) + 1);
	return CopyNextBuffer(_pDestination, _SizeofDestination, &NULL_STRING, 1);
}

static void* NextBlob(void* _pDestination, size_t _SizeofDestination, const void* _pBuffer, size_t _SizeofBuffer)
{
	oASSERT_ALIGNED(_pDestination);
	*(int*)_pDestination = oOSCPlatformEndianSwap((int)_SizeofBuffer);
	return CopyNextBuffer(oByteAdd(_pDestination, sizeof(int)), oUNKNOWN_SIZE, _pBuffer, _SizeofBuffer);
}

} // namespace SERIALIZE

static void oOSCSerializer(int _Type, const void* _pField, size_t _SizeofField, const void* _pDestStructBase, void** _ppDestination, void* _pDestinationEnd)
{
	oASSERT(*_ppDestination < _pDestinationEnd, "Writing past end of buffer");
	switch (_Type)
	{
		case 'r': case 'i': case 'f': *_ppDestination = SERIALIZE::NextIntrinsic<int>(_pDestStructBase, *_ppDestination, _pField); break;
		case 'h': case 't': case 'd': *_ppDestination = SERIALIZE::NextIntrinsic<long long>(_pDestStructBase, *_ppDestination, _pField); break;
		case 'c': *_ppDestination = SERIALIZE::NextChar(*_ppDestination, _pField); break;
		case 's': *_ppDestination = SERIALIZE::NextString(*_ppDestination, oUNKNOWN_SIZE, *(const char**)_pField); break; // oUNKNOWN_SIZE is unsafe, ignoring buffer boundaries but this should already be allocated correctly
		case 'b': *_ppDestination = SERIALIZE::NextBlob(*_ppDestination, oUNKNOWN_SIZE, _pField, _SizeofField); break;
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': *_ppDestination = SERIALIZE::NextString(*_ppDestination, oUNKNOWN_SIZE, (const char*)_pField);
		default: break;
	}
}

namespace DESERIALIZE {

template<typename T> static void* NextIntrinsic(void* _pStructBase, void* _pDestination, const void* _pSource)
{
	void* p = MoveToNextField(_pStructBase, _pDestination, std::alignment_of<T>::value);
	*(T*)p = *(const T*)_pSource;
	return oByteAdd(p, sizeof(T));
}

static void* NextString(void* _pStructBase, void* _pDestination, const char* _String)
{
	// assign pointer into message buffer
	const char** s = (const char**)MoveToNextField(_pStructBase, _pDestination, std::alignment_of<const char**>::value);
	*s = *(const char*)_String == NULL_STRING ? nullptr : (const char*)_String;
	return oByteAdd(s, sizeof(const char*));
}

static void* NextFixedString(void* _pStructBase, void* _pDestination, const char* _String, size_t _NumChars)
{
	// assign pointer into message buffer
	char* s = (char*)MoveToNextField(_pStructBase, _pDestination, std::alignment_of<char>::value);
	oStrcpy(s, _NumChars, _String);
	return oByteAdd(s, _NumChars * sizeof(char));
}

static void* NextBlob(void* _pStructBase, void* _pDestination, const void* _pSource, size_t _SizeofSource)
{
	int* p = (int*)MoveToNextField(_pStructBase, _pDestination, std::alignment_of<int>::value);
	*p = (int)_SizeofSource;
	p = (int*)MoveToNextField(_pStructBase, oByteAdd(p, sizeof(int)), std::alignment_of<void*>::value);
	*(const void**)p = _pSource;
	return oByteAdd(p, sizeof(const void*));
}

} // namespace DESERIALIZE

static void oOSCDeserializer(int _Type, const void* _pField, size_t _SizeofField, void* _pDestStructBase, void** _ppDestination, void* _pDestinationEnd)
{
	oASSERT(*_ppDestination < _pDestinationEnd || _Type == '[' || _Type == ']', "Writing past end of buffer");
	if('T' != _Type && 'F' != _Type)
		*_ppDestination = oByteAlign(*_ppDestination, sizeof(char));

	switch (_Type)
	{
		case 'r': case 'i': case 'f': *_ppDestination = DESERIALIZE::NextIntrinsic<int>(_pDestStructBase, *_ppDestination, _pField); break;
		case 'h': case 't': case 'd': *_ppDestination = DESERIALIZE::NextIntrinsic<long long>(_pDestStructBase, *_ppDestination, _pField); break;
		case 'c': *_ppDestination = DESERIALIZE::NextIntrinsic<char>(_pDestStructBase, *_ppDestination, _pField); break;
		case 's': *_ppDestination = DESERIALIZE::NextString(_pDestStructBase, *_ppDestination, (const char*)_pField); break;
		case 'b': *_ppDestination = DESERIALIZE::NextBlob(_pDestStructBase, *_ppDestination, _pField, _SizeofField); break;
		case 'T': *(bool*)(*_ppDestination) = true; (*_ppDestination) = (bool*)(*_ppDestination) + 1; break;
		case 'F': *(bool*)(*_ppDestination) = false; (*_ppDestination) = (bool*)(*_ppDestination) + 1; break;
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': *_ppDestination = DESERIALIZE::NextFixedString(_pDestStructBase, *_ppDestination, (const char*)_pField, SizeofFixedString(_Type)); break;
		default: break;
	}
}

size_t oOSCSerializeStructToMessage(const char* _Address, const char* _TypeTags, const void* _pStruct, size_t _SizeofStruct, void* _pMessage, size_t _SizeofMessage)
{
	if (!_Address || *_Address != '/' || !_pMessage || !_pStruct || _SizeofStruct == 0)
		return (size_t)oErrorSetLast(oERROR_INVALID_PARAMETER);

	void* p = _pMessage;
	void* pend = oByteAdd(p, _SizeofMessage);

	p = SERIALIZE::NextString(p, _SizeofMessage, _Address);
	char* pPatchTag = (char*)p;
	p = SERIALIZE::NextString(p, oByteDiff(pend, p), _TypeTags);

	size_t szSerializedMessage = 0;
	oASSERT((size_t)oByteDiff(pend, p) >= oOSCCalculateArgumentsDataSize(_TypeTags, _pStruct, _SizeofStruct), "");
	if( !oOSCVisitStructFieldsInternal(oBIND(oOSCSerializer, oBIND1, oBIND2, oBIND3, _pStruct, &p, pend), _TypeTags, _pStruct, _SizeofStruct, pPatchTag) )
		return 0;

	return (char*)p - (char*)_pMessage;
}

bool oOSCDeserializeMessageToStruct(const void* _pMessage, void* _pStruct, size_t _SizeofStruct)
{
	// if struct ptr is not aligned, all the alignment logic will be off
	if (!_pMessage || !oIsByteAligned(_pStruct, 4))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	const char* tags = oOSCGetMessageTypeTags(_pMessage);
	if (!tags)
		return oErrorSetLast(oERROR_CORRUPT, "failed to read message type tags");

	const void* args = (const void*)oByteAdd(tags, oByteAlign(oStrlen(tags)+1, 4));
	void* p = _pStruct;
	void* pend = oByteAdd(_pStruct, _SizeofStruct);

	return oOSCVisitMessageTypeTags(tags, args, oBIND(oOSCDeserializer, oBIND1, oBIND2, oBIND3, _pStruct, &p, pend));
}

static bool IsBoolTag(char _TypeTag)
{
	return _TypeTag == 'T' || _TypeTag == 'F' || _TypeTag == 't' || _TypeTag == 'f';
}

bool oOSCTypeTagsMatch(const char* _TypeTags0, const char* _TypeTags1)
{
	if (!_TypeTags0 || *_TypeTags0 != ',' || !_TypeTags1 || *_TypeTags1 != ',')
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	size_t len0 = oStrlen(_TypeTags0);
	size_t len1 = oStrlen(_TypeTags1);
	if (len0 != len1)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	while (*_TypeTags0)
	{
		if (*_TypeTags0 != *_TypeTags1 && (!IsBoolTag(*_TypeTags0) || !IsBoolTag(*_TypeTags1)))
			return oErrorSetLast(oERROR_GENERIC, "tags mismatch");
		_TypeTags0++;
		_TypeTags1++;
	}

	return true;
}

oNTPTimestamp oOSCGetBundleTimestamp(const void* _pOSCBundle)
{
	oASSERT(oOSCIsBundle(oOSCIsBundle), "The specified pointer is not a bundle");
	return oOSCPlatformEndianSwap(*(oNTPTimestamp*)oByteAdd(_pOSCBundle, 8));
}

struct oOSCTokContext
{
	const void* pSubbundle;
	const void* pEnd;
	size_t Size;
	int Cookie;
};

const void* oOSCTok(const void* _pOSCPacket, size_t _SizeofOSCPacket, void** _ppContext)
{
	if (_pOSCPacket)
	{
		*_ppContext = nullptr;
		if (!oOSCIsBundle(_pOSCPacket))
			return nullptr;
		const void* p = oByteAdd(_pOSCPacket, 16); // +8 #bundle + 8 NTPTime
		oOSCTokContext* ctx = new oOSCTokContext();
		ctx->Size = oOSCPlatformEndianSwap(*(int*)p);
		ctx->pSubbundle = oByteAdd(p, sizeof(int));
		ctx->pEnd = oByteAdd(_pOSCPacket, _SizeofOSCPacket);
		ctx->Cookie = 'OSCT';
		*_ppContext = ctx;
		return ctx->pSubbundle;
	}
	
	oASSERT(_ppContext, "A valid context must be specified");
	oOSCTokContext* ctx = (oOSCTokContext*)*_ppContext;
	oASSERT(ctx && ctx->Cookie == 'OSCT', "Invalid context");

	const void* pNext = oByteAdd(ctx->pSubbundle, ctx->Size);
	if (pNext >= ctx->pEnd)
	{
		delete ctx;
		*_ppContext = nullptr;
		return nullptr;
	}

	ctx->Size = oOSCPlatformEndianSwap(*(int*)pNext);
	ctx->pSubbundle = oByteAdd(pNext, sizeof(int));
	return ctx->pSubbundle;
}

void oOSCTokClose(void** _ppContext)
{
	oASSERT(_ppContext, "A valid context must be specified");
	oOSCTokContext* ctx = (oOSCTokContext*)*_ppContext;
	oASSERT(ctx && ctx->Cookie == 'OSCT', "Invalid context");
	delete ctx;
	*_ppContext = nullptr;
}
