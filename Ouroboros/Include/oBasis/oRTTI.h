/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#pragma once
#ifndef oRTTI_h
#define oRTTI_h

#include <oBase/assert.h>
#include <oBase/aabox.h>
#include <oBase/byte.h>
#include <oBase/color.h>
#include <oBase/fourcc.h>
#include <oBase/macros.h>
#include <oBase/path.h>
#include <oBase/rgb.h>
#include <oBase/type_info.h>
#include <oBase/uri.h>
#include <typeinfo>
#include <vector>

#include <oBase/input.h> // @tony fixme: needed to convert namespace::enum -> namespace_enum. Maybe there's a more generic way to support namespaces?

// _____________________________________________________________________________
// oRTTI_TYPE - RTTI atomic building block types

enum oRTTI_TYPE
{
	oRTTI_TYPE_INVALID,
	oRTTI_TYPE_ATOM,
	oRTTI_TYPE_POINTER,
	oRTTI_TYPE_CONTAINER,
	oRTTI_TYPE_ENUM,
	oRTTI_TYPE_COMPOUND,
};

// _____________________________________________________________________________
// oRTTI_OF - Get an oRTTI interface for a type (using its canonical type name)
// The canonical type name is generally equal to the type name, with exceptions
// for atom types which can have spaces in them, look for which canonical name
// has been assigned to atom types with either the oRTTI_ATOM_DESCRIPTION or
// oRTTI_ATOM_DEFAULT_DESCRIPTION macros.

#define oRTTI_OF(type_name) (*((const oRTTI*)&oRTTI_##type_name))

// _____________________________________________________________________________
// oRTTI_CAPS_xxx - Add capabilities to a type, like containers or pointers
// operation = DECLARATION | DESCRIPTION
// type_name = canonical type name

#define oRTTI_CAPS_NONE(operation, type_name)

#define oRTTI_CAPS_ARRAY_NO_STD(operation, type_name) \
	oRTTI_CONTAINED_TYPE_##operation(c_array, type_name) \
	oRTTI_CONTAINED_TYPE_##operation(ouro_fixed_vector, type_name)

#define oRTTI_CAPS_ARRAY(operation, type_name) \
	oRTTI_CAPS_ARRAY_NO_STD(operation, type_name) \
	oRTTI_STD_CONTAINED_TYPE_##operation(std_vector, type_name, std::vector<type_name>)

#define oRTTI_CAPS_POINTER(operation, type_name) \
	oRTTI_POINTER_TYPE_##operation(c_ptr, type_name)

#define oRTTI_CAPS_DEFAULT(operation, type_name) \
	oRTTI_CAPS_POINTER(operation, type_name) \
	oRTTI_CAPS_ARRAY(operation, type_name) \
	oRTTI_CAPS_ARRAY(operation, c_ptr_##type_name)
// _____________________________________________________________________________

class oRTTI_OBJECT
{
};

// _____________________________________________________________________________

#include <oBasis/oRTTITypedefs.h>
#include <oBasis/oRTTIForContainers.h>
#include <oBasis/oRTTIForEnums.h>
#include <oBasis/oRTTIForAtoms.h>
#include <oBasis/oRTTIForFunctions.h>
#include <oBasis/oRTTIForCompounds.h>
#include <oBasis/oRTTIForPointers.h>
#include <oBasis/oRTTIStructs.h>

// _____________________________________________________________________________
// Declaration of atom types and standard array and pointer types
// TODO: These should probably move to their appropriate headers

oRTTI_CONTAINER_DECLARATION(c_array)
oRTTI_CONTAINER_DECLARATION(ouro_fixed_vector)

oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY_NO_STD, bool)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, char)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, uchar)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, short)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, ushort)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, int)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, int2)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, int3)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, int4)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, uint)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, uint2)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, uint3)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, uint4)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, llong)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, ullong)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, float)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, float2)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, float3)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, float4)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, double)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::color, ouro_color)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, rgbf, ouro_rgbf)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, float4x4)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, quatf)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, planef)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, spheref)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, aaboxf)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::version, ouro_version)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::fourcc, ouro_fourcc)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, std::string, std_string)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::sstring, ouro_sstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::mstring, ouro_mstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::lstring, ouro_lstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::xlstring, ouro_xlstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::xxlstring, ouro_xxlstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::path_string, ouro_path_string)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::uri_string, ouro_uri_string)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::path, ouro_path)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::uri, ouro_uri)

// @tony fixme: find a way to make namespace support more generic or reduce 
// usage of reflection in these cases.
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::input::key, ouro_input_key)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, ouro::input::skeleton_bone, ouro_input_skeleton_bone)

#endif oRTTI_h
