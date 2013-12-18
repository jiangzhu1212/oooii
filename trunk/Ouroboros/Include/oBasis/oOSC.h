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

// NOTE: This API tries to conform to the vocabulary used in the definition of 
// both OSC 1.0 and OSC 1.1 specifications.
// http://opensoundcontrol.org/spec-1_0
// http://cnmat.berkeley.edu/system/files/attachments/Nime09OSCfinal.pdf
// IN ADDITION:
// This implementation supports the following type tags:
// [1-9]: A single digit will be interpreted as a fixed char array in multiples 
//        of 64 chars. This way, small fixed-size strings can be inlined. The 
//        buffer will be treated as a string, so serialization to a buffer is no 
//        different than with 's', but this allows some avoidance of char 
//        pointers in relatively simple cases such as small names and paths. The
//        _SizeofField param will still be strlen()+1.

#pragma once
#ifndef oOSC_h
#define oOSC_h

#include <oBasis/oFunction.h>
#include <oBase/date.h>

static const int oOSC_MAX_FIXED_STRING_LENGTH = 9*64;

// _Type will be an OSC Type Tag identifier. _pField will point to the buffer
// containing the data, i.e. it will point to an intrinsic type for intrinsics,
// or directly to a string or blob. _SizeofField represents the size of the 
// buffer pointed at by _pField. For strings, this will just be the sizeof(char*)
// as _pField must be dereferenced to get to the actual ASCII string.
// For blobs, this is the size of the blob. For char, the size is sizeof(char).
// No OSC padding requirements are considered in data preparation, only use of
// OSC type tags to describe traversal of a C++ struct.
typedef std::function<void(int _Type, void* _pField, size_t _SizeofField)> oOSCVisitorFn;
typedef std::function<void(int _Type, const void* _pField, size_t _SizeofField)> oOSCVisitorConstFn;

// This function correctly traverses fields in a specified struct that is 
// trivial, meaning:
// o Struct does not inherit from any base struct, so has no end-of-struct 
//   alignment issues
// o Struct does not have a vtable, either inherited or direct
// o Struct does not compile under any user-specialized pragma packs (though this may work, it's just not been tested at this time)
//
// Visiting fields is done according to specified the OSC 1.1 compliant TypeTags 
// string. Support for the optional common types described in the OSC 1.0
// specification under "OSC Type Tag String" EXCEPT for MIDI message m because 
// currently I don't time/a way for testing proper receipt of MIDI.
//
// Fields such as '[' or 'T' are also traversed with a nullptr value for _pField 
// and 0 value for _SizeofField.
//
// Important Reminders from the OSC 1.1 spec:
// o All blob data must be described in the struct by an int followed by a void*
// o All ints and longlongs are considered signed - there is no guarantee of 
//   unsigned support, though most operations are just size-based pass-through 
//   operations/copies to client code.
bool oOSCVisitStructFields(const char* _TypeTags, void* _pStruct, size_t _SizeofStruct, oOSCVisitorFn _Visitor);
bool oOSCVisitStructFields(const char* _TypeTags, const void* _pStruct, size_t _SizeofStruct, oOSCVisitorConstFn _Visitor);

// This function traverses an OSC 1.1 compliant message's arguments list 
// according to the specified type tags and calls the specified visitor on each
// field.
bool oOSCVisitMessageTypeTags(const char* _TypeTags, void* _pMessageArguments, oOSCVisitorFn _Visitor);
bool oOSCVisitMessageTypeTags(const char* _TypeTags, const void* _pMessageArguments, oOSCVisitorConstFn _Visitor);

// Returns the number of fields described by the type tags (ignores commands,
// array delimiters, counts arrays as one field etc.)
size_t oOSCCalculateNumFields(const char* _TypeTags);

// Given the specified type tags and struct (need to traverse string lengths and
// blob sizes) return the size of a buffer fit for use in allocating the 
// arguments portion of an OSC message. This takes into consideration the extra
// bytes imposed by OSC alignment requirements such as extra nuls on the end of
// a string to ensure 32-bit alignment.
size_t oOSCCalculateArgumentsDataSize(const char* _TypeTags, const void* _pStruct, size_t _SizeofStruct);

// Calculate the size of a buffer required to store the specified message. 
// _ArgumentsDataSize is the result of the oOSCCalculateArgumentsDataSize()
// function. This takes into consideration the extra bytes imposed by OSC 
// alignment requirements such as extra nuls on the end of a string to ensure 
// 32-bit alignment.
size_t oOSCCalculateMessageSize(const char* _Address, const char* _TypeTags, size_t _ArgumentsDataSize);

// Calculate the size of a buffer required to store the specified bundles. 
// The value for _SumOfAllSubbundleSizes should be the sum of various calls to
// oOSCCalculateMessageSize() and oOSCCalculateBundleSize() for recursive 
// bundles.
size_t oOSCCalculateBundleSize(size_t _NumSubbundles, size_t _SumOfAllSubbundleSizes);

// Calculate the size of the struct needed to hold the deserialized messgage.
// This does not include the memory needed to hold strings or blobs, just
// the size of the pointers and sizes.
size_t oOSCCalculateDeserializedStructSize(const char* _TypeTags);

// Returns the message's address so it can be redirected as appropriate. This
// will return nullptr if the message is not well formed (address doesn't begin
// with a '/')
const char* oOSCGetMessageAddress(const void* _pMessage);

// Returns the message's type tags. This will return a nullptr if the specified
// message does not have a valid address or if the type tags is not specified/
// valid (does not begin with ',').
const char* oOSCGetMessageTypeTags(const void* _pMessage);

// Takes the contents of the specified struct and creates an OSC 1.1 compliant
// message in _pMessage. The _TypeTags are used to interpret the data in
// _pStruct. When T or F is encountered in the type tags the type tags in the
// message are patched to represent either true or false based on the underlying
// data in the structure. Size of the entire message written is returned.
size_t oOSCSerializeStructToMessage(const char* _Address, const char* _TypeTags, const void* _pStruct, size_t _SizeofStruct, void* _pMessage, size_t _SizeofMessage);

// Interpret the specified buffer as an OSC 1.1 compliant message and fill the 
// specified buffer with its contents. _pStruct is required to be
// 4-byte aligned. Any strings and or blobs will point back into the original message.
bool oOSCDeserializeMessageToStruct(const void* _pMessage, void* _pStruct, size_t _SizeofStruct);

// Prefer using this to verify an expected type tag versus a messages type tags
// over a string compare because we bake bool values into the type tag itself
// using T/F since there is no bool type in OSC, so T==F F==T T==T F==F in this
// compare.
bool oOSCTypeTagsMatch(const char* _TypeTags0, const char* _TypeTags1);

// Returns true if the specified packet is an OSC message as defined in the spec.
inline bool oOSCIsMessage(const void* _pOSCPacket) { return _pOSCPacket && *(const char*)_pOSCPacket == '/'; }

// Returns true if the specified packet is an OSC bundle as defined in the spec.
inline bool oOSCIsBundle(const void* _pOSCPacket) { return _pOSCPacket && *(const char*)_pOSCPacket == '#'; }

// Returns the timestamp in the specified bundle.
ouro::ntp_timestamp oOSCGetBundleTimestamp(const void* _pOSCBundle);

// Like strtok, this iterates through all subbundles found inside an OSC packet
// that IS a bundle. Per the spec, size must be provided by either the packet or
// the underlying transit protocol. This function is not threadsafe. If 
// iteration exists before the last subbundle is reached, then oOSCTokClose must 
// be called explicitly, otherwise it is called automatically.
// NOTE: Because bundles can contain other bundles, oOSCTok should be used to 
// set up each one, and each one should have its own context.
const void* oOSCTok(const void* _pOSCPacket, size_t _SizeofOSCPacket, void** _ppContext);

// This is only required if iteration using oOSCTok is exited early (error?). 
// A context is automatically closed when oOSCTok returns nullptr due to being
// finished iterating.
void oOSCTokClose(void** _ppContext);


// Templated on type of struct versions of the above API
template<typename T> inline bool oOSCVisitStructFields(const char* _TypeTags, const T* _pStruct, oOSCVisitorFn _Visitor) { return oOSCVisitStructFields(_TypeTags, _pStruct, sizeof(T), _Visitor); }
template<typename T> inline size_t oOSCCalculateArgumentsDataSize(const char* _TypeTags, const T& _Struct) { return oOSCCalculateArgumentsDataSize(_TypeTags, &_Struct, sizeof(T)); }
template<typename T> inline size_t oOSCSerializeStructToMessage(const char* _Address, const char* _TypeTags, const T& _Struct, void* _pDestination, size_t _SizeofDestination) { return oOSCSerializeStructToMessage(_Address, _TypeTags, &_Struct, sizeof(T), _pDestination, _SizeofDestination); }
template<typename T> inline bool oOSCDeserializeMessageToStruct(const void* _pMessage, T* _pStruct ) { return oOSCDeserializeMessageToStruct(_pMessage, _pStruct, sizeof(T)); }
#endif
