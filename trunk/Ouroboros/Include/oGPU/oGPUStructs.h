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
// The major structs used in oGPU.
#pragma once
#ifndef oGPUStructs_h
#define oGPUStructs_h

#include <oBasis/oFixedString.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oSurface.h>
#include <oBasis/oVersion.h>
#include <oBasis/oGPUEnums.h>
#include <oGPU/oGPUConstants.h>

struct oGPU_INSTANCE_LIST_DESC
{
	// Elements points to a full pipeline's expected inputs. InputSlots 
	// specifies which of these elements will be populated in this instance 
	// list.

	oGPU_INSTANCE_LIST_DESC()
		: pElements(nullptr)
		, NumElements(0)
		, InputSlot(oInvalid)
		, MaxNumInstances(0)
		, NumInstances(0)
	{}

	const oGPU_VERTEX_ELEMENT* pElements;
	uint NumElements;
	uint InputSlot;
	uint MaxNumInstances;
	uint NumInstances;
};

// A line list's reserved memory or a user source for a commit must be in this
// format.
struct oGPU_LINE_VERTEX
{
	float3 Position;
	oColor Color;
};

struct oGPU_LINE_LIST_DESC
{
	oGPU_LINE_LIST_DESC()
		: MaxNumLines(0)
		, NumLines(0)
	{}

	uint MaxNumLines;
	uint NumLines;
};

struct oGPU_MESH_DESC
{
	oGPU_MESH_DESC()
		: NumIndices(0)
		, NumVertices(0)
		, NumRanges(0)
		, pElements(nullptr)
		, NumElements(0)
	{}

	uint NumIndices;
	uint NumVertices;
	uint NumRanges;
	oAABoxf LocalSpaceBounds;

	// A copy of the underlying data is not made for this pointer because the 
	// intended usage pattern is that all possible geometry layouts are defined 
	// statically in code and referenced from there.
	const oGPU_VERTEX_ELEMENT* pElements;
	uint NumElements;
};

struct oGPU_BUFFER_DESC
{
	// Description of a constant buffer (view, draw, material). Client code can 
	// defined whatever value are to be passed to a shader that expects them.
	// StructByteSize must be 16-byte aligned.
	// Instead of structured (StructByteSize=0), you can provide a Format

	oGPU_BUFFER_DESC()
		: Type(oGPU_BUFFER_DEFAULT)
		, Format(oSURFACE_UNKNOWN)
		, StructByteSize(oInvalid)
		, ArraySize(1)
	{}

	// Specifies the type of the constant buffer. Normally the final buffer size
	// is StructByteSize*ArraySize. If the type is specified as 
	// oGPU_BUFFER_UNORDERED_UNSTRUCTURED, then StructByteSize must be 
	// oInvalid and the size is calculated as (size of Format) * ArraySize.
	oGPU_BUFFER_TYPE Type;

	// This must be valid for oGPU_BUFFER_UNORDERED_UNSTRUCTURED types, and 
	// oSURFACE_UNKNOWN for all other types.
	oSURFACE_FORMAT Format;

	// This must be oInvalid for oGPU_BUFFER_UNORDERED_UNSTRUCTURED types,
	// but valid for all other types.
	uint StructByteSize;

	// The number of format elements or structures in the buffer.
	uint ArraySize;
};

struct oGPU_TEXTURE_DESC
{
	oGPU_TEXTURE_DESC()
		: Dimensions(oInvalid, oInvalid)
		, NumSlices(1)
		, Format(oSURFACE_B8G8R8A8_UNORM)
		, Type(oGPU_TEXTURE_2D_MAP)
	{}

	int2 Dimensions;
	int NumSlices;
	oSURFACE_FORMAT Format;
	oGPU_TEXTURE_TYPE Type;
};

// Same as oGPU_PIPELINE_BYTECODE but with no ctor so it can be statically 
// initialized.
struct oGPU_STATIC_PIPELINE_BYTECODE
{
	const char* DebugName;
	const oGPU_VERTEX_ELEMENT* pElements;
	int NumElements;
	const void* pVertexShader;
	const void* pHullShader;
	const void* pDomainShader;
	const void* pGeometryShader;
	const void* pPixelShader;
};

// Abstracts the raw data readied for use by a GPU. This might be text source
// that is compiled by the system, intermediate bytecode that is further 
// compiled by the driver, or direct assembly.
struct oGPU_PIPELINE_BYTECODE : oGPU_STATIC_PIPELINE_BYTECODE
{
	oGPU_PIPELINE_BYTECODE()
	{
		DebugName = "oGPU_PIPELINE_BYTECODE";
		pElements = nullptr;
		NumElements = oInvalid;
		pVertexShader = nullptr;
		pHullShader = nullptr;
		pDomainShader = nullptr;
		pGeometryShader = nullptr;
		pPixelShader = nullptr;
	}

	const oGPU_PIPELINE_BYTECODE& operator=(const oGPU_STATIC_PIPELINE_BYTECODE& _That) { *(oGPU_STATIC_PIPELINE_BYTECODE*)this = _That; return *this; }
};

struct oGPU_STATIC_COMPUTE_SHADER_BYTECODE
{
	const char* DebugName;
	const void* pComputeShader;
};

struct oGPU_COMPUTE_SHADER_BYTECODE : oGPU_STATIC_COMPUTE_SHADER_BYTECODE
{
	oGPU_COMPUTE_SHADER_BYTECODE()
	{
		DebugName = "oGPU_COMPUTE_SHADER_BYTECODE";
		pComputeShader = nullptr;
	}
};

struct oGPU_CLEAR_DESC
{
	oGPU_CLEAR_DESC()
		: DepthClearValue(1.0f)
		, StencilClearValue(0)
	{ oINIT_ARRAY(ClearColor, 0); }

	oColor ClearColor[oGPU_MAX_NUM_MRTS];
	float DepthClearValue;
	uchar StencilClearValue;
};

struct oGPU_RENDER_TARGET_DESC
{
	oGPU_RENDER_TARGET_DESC()
		: Dimensions(oInvalid, oInvalid)
		, NumSlices(1)
		, MRTCount(1)
		, DepthStencilFormat(oSURFACE_UNKNOWN)
		, GenerateMips(false)
	{ oINIT_ARRAY(Format, oSURFACE_UNKNOWN); }

	int2 Dimensions;
	int NumSlices;
	int MRTCount;
	oSURFACE_FORMAT Format[oGPU_MAX_NUM_MRTS];
	oSURFACE_FORMAT DepthStencilFormat; // Use UNKNOWN for no depth
	oGPU_CLEAR_DESC ClearDesc;
	bool GenerateMips;
};

struct oGPU_COMMAND_LIST_DESC
{
	int DrawOrder;
};

struct oGPU_DEVICE_INIT
{
	oGPU_DEVICE_INIT(const char* _DebugName = "oGPUDevice")
		: DebugName(_DebugName)
		, Version(11,0)
		, AdapterIndex(0)
		, VirtualDesktopPosition(oDEFAULT, oDEFAULT)
		, UseSoftwareEmulation(false)
		, EnableDebugReporting(false)
		, DriverVersionMustBeExact(false)
	{}

	// Name associated with this device in debug output
	oStringS DebugName;

	// The minimum version of the driver required to successfully create the 
	// device. If the driver version is 0.0.0.0 (the default) then a hard-coded
	// internal value is used which represents a pivot point where QA was done at 
	// OOOii that showed significant problems or missing features before that 
	// default version.
	oVersion MinDriverVersion;

	// The version of the underlying API to use.
	oVersion Version;

	// If VirtualDesktopPosition is oDEFAULT, oDEFAULT, then use the nth found
	// device as specified by this Index. If VirtualDesktopPosition is anything
	// valid, then the device used to handle that desktop position will be used
	// and Index is ignored.
	int AdapterIndex;

	// Position on the desktop and thus on a monitor to be used to determine which 
	// GPU is used for that monitor and create a device for that GPU.
	int2 VirtualDesktopPosition;

	// Allow SW emulation for the specified version. If false, a create will fail
	// if HW acceleration is not available.
	bool UseSoftwareEmulation;

	// Reports errors in usage to the debugger if true.
	bool EnableDebugReporting;

	// If true, == is used to match MinDriverVersion to the specified GPU's 
	// driver. If false, CurVer >= MinDriverVersion is used.
	bool DriverVersionMustBeExact;
};

struct oGPU_DEVICE_DESC
{
	oGPU_DEVICE_DESC()
		: NativeMemory(0)
		, DedicatedSystemMemory(0)
		, SharedSystemMemory(0)
		, AdapterIndex(0)
		, API(oGPU_API_UNKNOWN)
		, Vendor(oGPU_VENDOR_UNKNOWN)
		, IsSoftwareEmulation(false)
		, DebugReportingEnabled(false)
	{}

	// Name associated with this device in debug output
	oStringS DebugName;
	
	// Description as provided by the device vendor
	oStringM DeviceDescription;

	// Description as provided by the driver vendor
	oStringM DriverDescription;

	// Number of bytes present on the device (AKA VRAM)
	unsigned long long NativeMemory;

	// Number of bytes reserved by the system to accommodate data transfer to the 
	// device
	unsigned long long DedicatedSystemMemory;

	// Number of bytes reserved in system memory used instead of a separate bank 
	// of NativeMemory 
	unsigned long long SharedSystemMemory;

	// The version for the software that supports the native API. This depends on 
	// the API type being used.
	oVersion DriverVersion;

	// The feature level the device supports. This depends on the API type being 
	// used.
	oVersion FeatureVersion; 

	// The driver/software interface version that might be different than the 
	// capabilities of the device. This depends on the API type being used.
	oVersion InterfaceVersion;

	// The zero-based index of the adapter. This may be different than what is 
	// specified in oGPU_DEVICE_INIT in certain debug/development modes.
	int AdapterIndex;

	// Describes the API used to implement the oGPU API
	oGPU_API API;

	// Describes the company that made the device
	oGPU_VENDOR Vendor;

	// True if the device was created in software emulation mode
	bool IsSoftwareEmulation;

	// True if the device was created with debug reporting enalbed.
	bool DebugReportingEnabled;
};

#endif
