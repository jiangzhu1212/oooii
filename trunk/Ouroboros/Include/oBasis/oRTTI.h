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
#pragma once
#ifndef oRTTI_h
#define oRTTI_h

#include <oStd/assert.h>
#include <oStd/byte.h>
#include <oStd/fourcc.h>
#include <oStd/macros.h>
#include <oStd/path.h>
#include <oStd/uri.h>
#include <oBasis/oTypeInfo.h>
#include <typeinfo>
#include <vector>

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
	oRTTI_CONTAINED_TYPE_##operation(ostd_fixed_vector, type_name)

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
oRTTI_CONTAINER_DECLARATION(ostd_fixed_vector)

oRTTI_ATOM_DECLARATION(oRTTI_CAPS_NONE, oVersion)
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
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::color, ostd_color)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, oRGBf)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, float4x4)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, quatf)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, oPlanef)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, oSpheref)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, oAABoxf)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::fourcc, ostd_fourcc)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, std::string, std_string)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::sstring, ostd_sstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::mstring, ostd_mstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::lstring, ostd_lstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::xlstring, ostd_xlstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::xxlstring, ostd_xxlstring)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::path_string, ostd_path_string)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::uri_string, ostd_uri_string)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::path, ostd_path)
oRTTI_ATOM_DECLARATION_NON_CANONICAL(oRTTI_CAPS_ARRAY, oStd::uri, ostd_uri)

#endif oRTTI_h
