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
// Structures and utilities for loading the Wavefront OBJ 3D object file format
#pragma once
#ifndef oMesh_obj_h
#define oMesh_obj_h

#include <oBase/invalid.h>
#include <oBase/colors.h>
#include <oBase/path.h>
#include <oBase/rgb.h>
#include <oMesh/mesh.h>
#include <memory>

namespace ouro {
	namespace mesh {
		namespace obj {

// http://local.wasp.uwa.edu.au/~pbourke/dataformats/mtl/
namespace illumination
{ enum value {
			
	color_on_ambient_off,
	color_on_ambient_on,
	highlight_on,
	reflection_on_ray_trace_on,
	transparency_glass_on_reflection_ray_trace_on,
	reflection_fresnel_on_ray_trace_on,
	transparency_refraction_on_reflection_fresnel_off_ray_trace_on,
	transparency_refraction_on_reflection_frensel_on_ray_trace_on,
	reflection_on_ray_trace_off,
	transparency_glass_on_reflection_ray_trace_off,
	casts_shadows_onto_invisible_surfaces,

	count

};}

namespace texture_type
{ enum value {

	regular,
	cube_right,
	cube_left,
	cube_top,
	cube_bottom,
	cube_back,
	cube_front,
	sphere,
	
	count,

};}

struct group
{
	mstring group_name;
	mstring material_name;
};

struct info
{
	// positions/texcoords/normals will all have the same count (i.e. 
	// reduction/duplication is done internally). If there is no data for one of 
	// the channels, then the pointer will be null and pVertexElements will be 
	// collapsed (no null element between others).

	info()
		: obj_path(nullptr)
		, mtl_path(nullptr)
		, groups(nullptr)
		, indices(nullptr)
		, positions(nullptr)
		, normals(nullptr)
		, texcoords(nullptr)
	{}

	path obj_path;
	path mtl_path;
	const group* groups; // the size of this array is the same as mesh_info::num_ranges
	const range* ranges;
	const uint* indices;
	const float3* positions;
	const float3* normals;
	const float3* texcoords;
	mesh::info mesh_info;
};

struct init
{
	init()
		: est_num_vertices(100000)
		, est_num_indices(100000)
		, flip_handedness(false)
		, counter_clockwide_faces(true)
		, calc_normals_on_error(true)
		, calc_texcoords_on_error(false)
	{}

	// Estimates are used to pre-allocate memory in order to minimize reallocs
	uint est_num_vertices;
	uint est_num_indices;
	bool flip_handedness;
	bool counter_clockwide_faces;
	bool calc_normals_on_error; // either for no loaded normals or degenerates
	bool calc_texcoords_on_error; // uses LCSM if no texcoords in source
};

class mesh
{
public:
	// given the path and the loaded-to-memory string contents of the file, parse into 3D data
	static std::shared_ptr<mesh> make(const init& _Init, const path& _OBJPath, const char* _OBJString);

	virtual info get_info() const = 0;
};

struct texture_info
{
	texture_info()
		: boost(0.0f)
		, brightness_gain(0.0f, 1.0f)
		, origin_offset(0.0f, 0.0f, 0.0f)
		, scale(1.0f, 1.0f, 1.0f)
		, turbulance(0.0f, 0.0f, 0.0f)
		, resolution(ouro::invalid, ouro::invalid)
		, bump_multiplier(1.0f)
		, type(texture_type::regular)
		, imfchan('l')
		, blendu(true)
		, blendv(true)
		, clamp(false)
	{}

	path path;
	float boost;
	float2 brightness_gain;
	float3 origin_offset;
	float3 scale;
	float3 turbulance;
	uint2 resolution;
	float bump_multiplier;
	texture_type::value type;
	char imfchan;
	bool blendu;
	bool blendv;
	bool clamp;
};

struct material_info
{
	material_info()
		: ambient_color(black)
		, emissive_color(black)
		, diffuse_color(white_smoke)
		, specular_color(white)
		, transmission_color(black)
		, specularity(0.04f)
		, transparency(1.0f) // (1 means opaque 0 means transparent)
		, refraction_index(1.0f)
		, illum(illumination::color_on_ambient_off)
	{}

	rgbf ambient_color;
	rgbf emissive_color;
	rgbf diffuse_color;
	rgbf specular_color;
	rgbf transmission_color;
	float specularity;
	float transparency;
	float refraction_index;
	illumination::value illum;

	path name;
	texture_info ambient;
	texture_info diffuse;
	texture_info alpha;
	texture_info specular;
	texture_info transmission;
	texture_info bump;
};

struct material_lib_info
{
	material_lib_info()
		: mtl_path(nullptr)
		, materials(nullptr)
		, num_materials(0)
	{}

	path mtl_path;
	const material_info* materials;
	uint num_materials;
};

class material_lib
{
public:
	static std::shared_ptr<material_lib> make(const path& _MTLPath, const char* _MTLString);

	virtual material_lib_info get_info() const = 0;
};

		} // namespace obj
	} // namespace mesh 
} // namespace ouro

#endif
