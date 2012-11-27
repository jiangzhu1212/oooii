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
// This header is designed to cross-compile in both C++ and HLSL. This defines
// for C++ HLSL specialized structs (mostly non-rasterization-related).
// 
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSLStructs_h
#define oHLSLStructs_h

#ifndef oHLSL

#include <oBasis/oAssert.h>
#include <oBasis/oHLSLAtomics.h>
#include <oBasis/oHLSLTypes.h>

#define oDEFINE_HLSL_OP_ARRAY() T& operator[](uint _Index) { oASSERT(_Index < Capacity, "Out of range"); return Buffer[_Index]; }
#define oDEFINE_HLSL_CONST_OP_ARRAY() const T& operator[](uint _Index) const { oASSERT(_Index < Capacity, "Out of range"); return Buffer[_Index]; }

template<typename T> class HLSLBufferBase
{
	HLSLBufferBase(const HLSLBufferBase&);
	const HLSLBufferBase& operator=(const HLSLBufferBase&);

public:
	~HLSLBufferBase() { Deallocate(); }

	void CopyFrom(const oSURFACE_MAPPED_SUBRESOURCE& _MappedSubresource)
	{
		oASSERT(_MappedSubresource.RowPitch <= (Capacity * sizeof(T)), "Invalidly sized buffer");
		memcpy(Buffer, _MappedSubresource.pData, _MappedSubresource.RowPitch);
	}

	void Reallocate(uint _Capacity) { if (_Capacity > Capacity) { Deallocate(); Buffer = new T[_Capacity]; Capacity = _Capacity; } }
	void Deallocate() { if (Buffer) delete [] Buffer; }

	void GetDimensions(uint& _NumStructs, uint& _Stride) const { _NumStructs = Capacity; _Stride = sizeof(T); }

	oDEFINE_HLSL_CONST_OP_ARRAY();

protected:
	HLSLBufferBase() : Buffer(nullptr), Capacity(0) {}
	HLSLBufferBase(uint _Capacity) : Counter(_Counter) { Reallocate(_Capacity); }

	T* Buffer;
	uint Capacity;
};

template<typename T> class HLSLCounterBufferBase : public HLSLBufferBase<T>
{
	HLSLCounterBufferBase(const HLSLCounterBufferBase&);
	const HLSLCounterBufferBase& operator=(const HLSLCounterBufferBase&);

public:
	void SetCounter(uint _Counter) { Counter = _Counter; }
	uint GetCounter() const { return Counter; }

	oDEFINE_HLSL_OP_ARRAY();
	oDEFINE_HLSL_CONST_OP_ARRAY();

protected:
	HLSLCounterBufferBase() : Counter(0) {}
	HLSLCounterBufferBase(uint _Capacity, uint _Counter = 0) : HLSLBufferBase(_Capacity), Counter(_Counter) {}

	uint Counter;
};

template<typename T> class RWBuffer : public HLSLBufferBase<T>
{
public:
	RWBuffer() {}
	RWBuffer(uint _Capacity) : HLSLBufferBase(_Capacity) {}

	oDEFINE_HLSL_OP_ARRAY();
	oDEFINE_HLSL_CONST_OP_ARRAY();
};

template<typename T> class StructuredBuffer : public HLSLBufferBase<T>
{
	StructuredBuffer(const StructuredBuffer&);
	const StructuredBuffer& operator=(const StructuredBuffer&);

public:
	StructuredBuffer() {}
	StructuredBuffer(uint _Capacity) : HLSLBufferBase(_Capacity) {}

	oDEFINE_HLSL_CONST_OP_ARRAY();
};

template<typename T> class RWStructuredBuffer : public HLSLCounterBufferBase<T>
{
	RWStructuredBuffer(const RWStructuredBuffer&);
	const RWStructuredBuffer& operator=(const RWStructuredBuffer&);

public:
	RWStructuredBuffer() {}
	RWStructuredBuffer(uint _Capacity, uint _Counter = 0) : HLSLCounterBufferBase(_Capacity, _Counter) {}

	oDEFINE_HLSL_OP_ARRAY();
	oDEFINE_HLSL_CONST_OP_ARRAY();

	uint IncrementCounter() { uint Orig = 0; InterlockedAdd(Counter, 1, Orig); return Orig; }
	uint DecrementCounter() { uint Orig = 0; InterlockedAdd(Counter, -1, Orig); return Orig; }
};

template<typename T> class AppendStructuredBuffer : public HLSLCounterBufferBase<T>
{
public:
	AppendStructuredBuffer() {}
	AppendStructuredBuffer(uint _Capacity, uint _Counter = 0) : HLSLCounterBufferBase(_Capacity, _Counter) {}

	void Append(const T& _Value) { uint i = 0; InterlockedAdd(Counter, 1, i); (*this)[i] = _Value; }
};

template<typename T> class ConsumeStructuredBuffer : public HLSLCounterBufferBase<T>
{
public:
	ConsumeStructuredBuffer() {}
	ConsumeStructuredBuffer(uint _Capacity, uint _Counter = 0) : HLSLCounterBufferBase(_Capacity, _Counter) {}

	T& Consume() const { uint i = 0; InterlockedAdd(Counter, -1, i); return (*this)[i-1]; }
};

template<typename T> struct ByteAddressBuffer
{
	inline void GetDimensions(uint& _Dimensions) const { _Dimensions = sizeof(T); }
	inline uint Load(uint _Address) const { return *Ptr<uint>(_Address); }
	inline uint2 Load2(uint _Address) const { return *Ptr<uint2>(_Address); }
	inline uint3 Load3(uint _Address) const { return *Ptr<uint3>(_Address); }
	inline uint4 Load4(uint _Address) const { return *Ptr<uint4>(_Address); }

protected:
	template<typename T> T* Ptr(uint _Address) { oASSERT(oIsByteAligned(_Address, 4), "_Address must be 4-byte aligned"); return (T*)((unsigned char*)this + _Address); }
	template<typename T> const T* Ptr(uint _Address) const { oASSERT(oIsByteAligned(_Address, 4), "_Address must be 4-byte aligned"); return (const T*)((unsigned char*)this + _Address); }
};

template<typename T> struct RWByteAddressBuffer : ByteAddressBuffer<T>
{
	inline void InterlockedAdd(uint _DestAddress, uint _Value, uint& _OriginalValue) { ::InterlockedAdd(*Ptr<uint>(_DestAddress), _Value, _OriginalValue); }
	inline void InterlockedAdd(uint _DestAddress, uint _Value) { ::InterlockedAdd(*Ptr<uint>(_DestAddress), _Value); }
	inline void InterlockedAnd(uint _DestAddress, uint _Value, uint& _OriginalValue) { ::InterlockedAnd(*Ptr<uint>(_DestAddress), _Value, _OriginalValue); }
	inline void InterlockedAnd(uint _DestAddress, uint _Value) { ::InterlockedAnd(*Ptr<uint>(_DestAddress), _Value); }
	inline void InterlockedCompareExchange(uint _DestAddress, uint _CompareValue, uint _Value, uint& _OriginalValue) { ::InterlockedCompareExchange(*Ptr<uint>(_DestAddress), _CompareValue, _Value, _OriginalValue); }
	inline void InterlockedCompareExchange(uint _DestAddress, uint _CompareValue, uint _Value) { ::InterlockedCompareExchange(*Ptr<uint>(_DestAddress), _CompareValue, _Value); }
	//inline void InterlockedCompareStore(uint _DestAddress, uint _CompareValue, uint _Value);
	inline void InterlockedExchange(uint _DestAddress, uint _Value, uint& _OriginalValue) { ::InterlockedExchange(*Ptr<uint>(_DestAddress), _Value, _OriginalValue); }
	inline void InterlockedExchange(uint _DestAddress, uint _Value) { ::InterlockedExchange(*Ptr<uint>(_DestAddress), _Value); }
	inline void InterlockedMax(uint _DestAddress, uint _Value, uint& _OriginalValue) { ::InterlockedMax(*Ptr<uint>(_DestAddress), _Value, _OriginalValue); }
	inline void InterlockedMax(uint _DestAddress, uint _Value) { ::InterlockedMax(*Ptr<uint>(_DestAddress), _Value); }
	inline void InterlockedMin(uint _DestAddress, uint _Value, uint& _OriginalValue) { ::InterlockedMin(*Ptr<uint>(_DestAddress), _Value, _OriginalValue); }
	inline void InterlockedMin(uint _DestAddress, uint _Value) { ::InterlockedMin(*Ptr<uint>(_DestAddress), _Value); }
	inline void InterlockedOr(uint _DestAddress, uint _Value, uint& _OriginalValue) { ::InterlockedOr(*Ptr<uint>(_DestAddress), _Value, _OriginalValue); }
	inline void InterlockedOr(uint _DestAddress, uint _Value) { ::InterlockedOr(*Ptr<uint>(_DestAddress), _Value); }
	inline void InterlockedXor(uint _DestAddress, uint _Value, uint& _OriginalValue) { ::InterlockedXor(*Ptr<uint>(_DestAddress), _Value, _OriginalValue); }
	inline void InterlockedXor(uint _DestAddress, uint _Value) { ::InterlockedXor(*Ptr<uint>(_DestAddress), _Value); }

	inline void Store(uint _Address, uint _Value) { InterlockedExchange(_Address, _Value); }
	inline void Store2(uint _Address, const uint2& _Value) { Store(_Address, _Value.x); Store(_Address + 4, _Value.y); }
	inline void Store3(uint _Address, const uint3& _Value) { Store(_Address, _Value.x); Store(_Address + 4, _Value.y); Store(_Address + 8, _Value.z); }
	inline void Store4(uint _Address, const uint4& _Value) { Store(_Address, _Value.x); Store(_Address + 4, _Value.y); Store(_Address + 8, _Value.z); Store(_Address + 12, _Value.w); }
};

#endif
#endif
