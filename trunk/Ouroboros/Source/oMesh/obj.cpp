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
#include <oMesh/obj.h>
#include <oMesh/mesh.h>
#include <oBase/algorithm.h>
#include <oBase/atof.h>
#include <oBase/finally.h>
#include <oBase/fixed_string.h>
#include <oBase/assert.h>
#include <oBase/macros.h>
#include <oBase/std_linear_allocator.h>
#include <oBase/timer.h>
#include <oBase/unordered_map.h>

oDEFINE_WHITESPACE_PARSING();

static const float3 ZERO3(0.0f, 0.0f, 0.0f);

namespace ouro {

bool from_string(mesh::obj::texture_type::value* _pType, const char* _StrSource)
{
	static const char* sStrings[] = 
	{
		"regular",
		"cube_right",
		"cube_left",
		"cube_top",
		"cube_bottom",
		"cube_back",
		"cube_front",
		"sphere",
	};
	static_assert(mesh::obj::texture_type::count == oCOUNTOF(sStrings), "array mismatch");

	if (*_StrSource == '\0')
	{
		*_pType = mesh::obj::texture_type::regular;
		return true;
	}

	for (size_t i = 0; i < oCOUNTOF(sStrings); i++)
	{
		if (!_stricmp(sStrings[i], _StrSource))
		{
			*_pType = (mesh::obj::texture_type::value)i;
			return true;
		}
	}

	return false;
}

	namespace mesh {
		namespace obj {

// To translate from several index streams - one per vertex element stream - to 
// a single index buffer we'll need to replicate vertices by their unique 
// combination of all vertex elements. This basically means an index is uniquely
// identified by a hash of the streams it indexes. We'll build the hash as well
// parse faces but then it all can be freed at once. To prevent a length series
// of deallocs for indices - i.e. int values - use a linear allocator and then
// free all entries once the map goes out of scope.
typedef unsigned long long key_t;
typedef uint val_t;
typedef std::pair<const key_t, val_t> pair_type;
typedef ouro::std_linear_allocator<pair_type> allocator_type;
typedef unordered_map<key_t, val_t, std_noop_hash<key_t>, std::equal_to<key_t>, std::less<key_t>, allocator_type> index_map_t;

static inline const char* parse_string(char* _StrDestination, size_t _SizeofStrDestination, const char* _S)
{
	move_next_word(&_S);
	const char* start = _S;
	move_to_line_end(&_S);
	size_t len = std::distance(start, _S);
	strncpy(_StrDestination, _SizeofStrDestination, start, len);
	return _S;
}

template<size_t size> static inline const char* parse_string(fixed_string<char, size>& _StrDestination, const char* r) { return parse_string(_StrDestination, _StrDestination.capacity(), r); }

static const uint kMaxNumVertsPerFace = 4;

struct face
{
	face() 
		: num_indices(0)
		, group_range_index(0)
	{
		for (auto& i : index)
			i.fill(invalid);
	}

	enum semantic
	{
		position,
		texcoord,
		normal,

		semantic_count,
	};

	std::array<std::array<uint, semantic_count>, kMaxNumVertsPerFace> index;
	uint num_indices; // 3 for tris, 4 for quads
	uint group_range_index; // groups and ranges are indexed with the same value
};

struct vertex_data
{
	void reserve(uint _NewVertexCount, uint _NewFaceCount)
	{
		positions.reserve(_NewVertexCount);
		normals.reserve(_NewVertexCount);
		texcoords.reserve(_NewVertexCount);
		faces.reserve(_NewFaceCount);
		groups.reserve(20);
	}

	aaboxf bound;

	std::vector<float3> positions;
	std::vector<float3> normals;
	std::vector<float3> texcoords;
	
	// these are assigned after negative indices have been handled, but are 
	// otherwise directly-from-file values.
	std::vector<face> faces;

	std::vector<group> groups;
	std::vector<range> ranges;

	path_string mtl_path;
};

// Given a string that starts with the letter 'v', parse as a line of vector
// values and push_back into the appropriate vector.
static const char* parse_vline(const char* _V
	, bool _FlipHandedness
	, std::vector<float3>& _Positions
	, std::vector<float3>& _Normals
	, std::vector<float3>& _Texcoords
	, float3& _Min
	, float3& _Max)
{
	float3 temp;

	_V++;
	switch (*_V)
	{
		case ' ':
			atof(&_V, &temp.x);
			atof(&_V, &temp.y);
			atof(&_V, &temp.z); if (_FlipHandedness) temp.z = -temp.z;
			_Positions.push_back(temp);
			_Min = min(_Min, temp);
			_Max = max(_Max, temp);
			break;
		case 't':
			move_to_whitespace(&_V);
			atof(&_V, &temp.x);
			atof(&_V, &temp.y);
			if (!atof(&_V, &temp.z)) temp.z = 0.0f;
			if (_FlipHandedness) temp.y = 1.0f - temp.y;
			_Texcoords.push_back(temp);
			break;
		case 'n':
			move_to_whitespace(&_V);
			atof(&_V, &temp.x);
			atof(&_V, &temp.y);
			atof(&_V, &temp.z); if (_FlipHandedness) temp.z = -temp.z;
			_Normals.push_back(temp);
			break;
		oNODEFAULT;
	}

	return _V;
}

// Fills the specified face with data from a line in an OBJ starting with the 
// 'f' (face) character. This returns a pointer into the string _F that is 
// either the end of the string, or the end of the line.
static const char* parse_fline(const char* _F
	, uint _NumPositions
	, uint _NumNormals
	, uint _NumTexcoords
	, uint _NumGroups
	, std::vector<face>& _Faces)
{
	move_to_whitespace(&_F);
	move_past_line_whitespace(&_F);
	face f;
	f.num_indices = 0;
	f.group_range_index = _NumGroups;
	while (f.num_indices < 4 && *_F != '\0' && !is_newline(*_F))
	{
		bool foundSlash = false;
		int semantic = face::position;
		do
		{
			// parsing faces is relative to vertex parsing up to this point,
			// so figure out where the data's at.
			uint ZeroBasedIndexFromFile = 0;

			// Negative indices implies the max indexable vertex OBJ will be INT_MAX, 
			// not UINT_MAX.
			if (*_F == '-')
			{
				int NegativeIndex = atoi(_F);
				uint NumElements = 0;
				switch (semantic)
				{
					case face::position: NumElements = _NumPositions; break;
					case face::normal: NumElements = _NumNormals; break;
					case face::texcoord: NumElements = _NumTexcoords; break;
					oNODEFAULT;
				}
				ZeroBasedIndexFromFile = NumElements + NegativeIndex;
			}
			else
				ZeroBasedIndexFromFile = atoi(_F) - 1;

			f.index[f.num_indices][semantic] = ZeroBasedIndexFromFile;

			if (semantic == (face::semantic_count-1))
				break;

			while (isdigit(*_F) || *_F == '-')
				_F++;

			// support case where the texcoord channel is empty
			
			if (*_F == '/')
			{
				do
				{
					_F++;
					semantic++;
					foundSlash = true;
				} while(*_F == '/');
			}

			else
				break;

			move_past_line_whitespace(&_F);

		} while(foundSlash);

		move_next_word(&_F);
		f.num_indices++;
	}

	_Faces.push_back(f);

	return _F;
}

// Scans an OBJ string and appends vertex element data
static void parse_elements(const char* _OBJString, bool _FlipHandedness, vertex_data* _pElements)
{
	uint NumGroups = 0;
	group group;

	const char* line = _OBJString;
	while (*line)
	{
		move_past_line_whitespace(&line);
		switch (*line)
		{
			case 'v':
				line = parse_vline(line, _FlipHandedness, _pElements->positions, _pElements->normals, _pElements->texcoords, _pElements->bound.Min, _pElements->bound.Max);
				break;
			case 'f':
				line = parse_fline(line
					, as_uint(_pElements->positions.size())
					, as_uint(_pElements->normals.size())
					, as_uint(_pElements->texcoords.size())
					, as_uint(_pElements->groups.size())
					, _pElements->faces);
				break;
			case 'g':
				// close out previous group
				if (NumGroups)
					_pElements->groups.push_back(group); 
				NumGroups++;
				parse_string(group.group_name, line);
				break;
			case 'u':
				parse_string(group.material_name, line);
				break;
			case 'm':
				parse_string(_pElements->mtl_path, line);
				break;
			default:
				break;
		}
		move_to_line_end(&line);
		move_past_newline(&line);
	}

	// close out a remaining group one last time
	// NOTE: start prim / num prims are handled later after vertices have been reduced
	if (NumGroups)
		_pElements->groups.push_back(group);

	else
	{
		group.group_name = "Default Group";
		_pElements->groups.push_back(group);
	}
}

// Using config data and an index hash, reduce all duplicate/face data into 
// unique indexed data. This must be called after ALL SourceElements have been
// populated.
static void reduce_elements(const init& _Init
	, index_map_t* _pIndexMap
	, const vertex_data& _SourceElements
	, std::vector<uint>* _pIndices
	, vertex_data* _pSinglyIndexedElements
	, std::vector<uint>* _pDegenerateNormals
	, std::vector<uint>* _pDegenerateTexcoords)
{
	_pSinglyIndexedElements->ranges.resize(_SourceElements.groups.size());

	uint resolvedIndices[4];
	_pSinglyIndexedElements->bound = _SourceElements.bound;
	_pSinglyIndexedElements->groups = _SourceElements.groups;
	_pSinglyIndexedElements->mtl_path = _SourceElements.mtl_path;
	uint LastRangeIndex = invalid;

	for (const face& f : _SourceElements.faces)
	{
		range& Range = _pSinglyIndexedElements->ranges[f.group_range_index];

		if (LastRangeIndex != f.group_range_index)
		{
			if (LastRangeIndex != invalid)
			{
				range& LastRange = _pSinglyIndexedElements->ranges[LastRangeIndex];
				LastRange.num_primitives = as_uint(_pIndices->size() / 3) - LastRange.start_primitive;
				Range.start_primitive = as_uint(_pIndices->size() / 3);
			}
			LastRangeIndex = f.group_range_index;
		}

		for (uint p = 0; p < f.num_indices; p++)
		{
			const key_t hash = f.index[p][face::position] + _SourceElements.positions.size() * f.index[p][face::texcoord] + _SourceElements.positions.size() * _SourceElements.texcoords.size() * f.index[p][face::normal];
			index_map_t::iterator it = _pIndexMap->find(hash);

			if (it == _pIndexMap->end())
			{
				uint NewIndex = as_uint(_pSinglyIndexedElements->positions.size());
				_pSinglyIndexedElements->positions.push_back(_SourceElements.positions[f.index[p][face::position]]);
			
				if (!_SourceElements.normals.empty())
				{
					if (f.index[p][face::normal] != invalid)
						_pSinglyIndexedElements->normals.push_back(_SourceElements.normals[f.index[p][face::normal]]);
					else
					{
						_pDegenerateNormals->push_back(NewIndex);
						_pSinglyIndexedElements->normals.push_back(ZERO3);
					}
				}

				if (!_SourceElements.texcoords.empty())
				{
					if(f.index[p][face::texcoord] != invalid)
						_pSinglyIndexedElements->texcoords.push_back(_SourceElements.texcoords[f.index[p][face::texcoord]]);
					else
					{
						_pDegenerateTexcoords->push_back(NewIndex);
						_pSinglyIndexedElements->texcoords.push_back(ZERO3);
					}
				}
			
				(*_pIndexMap)[hash] = resolvedIndices[p] = NewIndex;
			}
		
			else
				resolvedIndices[p] = (*it).second;
		} // for each face element

		// Now that the index has either been added or reset to a pre-existing index, 
		// add the face definition to the index list

		static const uint CCWindices[6] = { 0, 2, 1, 2, 0, 3, };
		static const uint CWindices[6] = { 0, 1, 2, 2, 3, 0, };
	
		bool UseCCW = _Init.counter_clockwide_faces;
		if (_Init.flip_handedness)
			UseCCW = !UseCCW;
	
		const uint* pOrder = UseCCW ? CCWindices : CWindices;

		_pIndices->push_back(resolvedIndices[pOrder[0]]);
		_pIndices->push_back(resolvedIndices[pOrder[1]]);
		_pIndices->push_back(resolvedIndices[pOrder[2]]);

		// Add another triangle for the rest of the quad
		if (f.num_indices == 4)
		{
			_pIndices->push_back(resolvedIndices[pOrder[3]]);
			_pIndices->push_back(resolvedIndices[pOrder[4]]);
			_pIndices->push_back(resolvedIndices[pOrder[5]]);
		}
	} // for each face

	// close out last group
	LastRangeIndex = _SourceElements.faces.back().group_range_index;
	range& r = _pSinglyIndexedElements->ranges[LastRangeIndex];
	r.num_primitives = as_uint(_pIndices->size() / 3) - r.start_primitive;

	// Go back through ranges and calc min/max verts
	for (range& r : _pSinglyIndexedElements->ranges)
		calc_min_max_indices(_pIndices->data(), r.start_primitive*3, r.num_primitives*3, as_uint(_pSinglyIndexedElements->positions.size()), &r.min_vertex, &r.max_vertex);
}

static layout::value get_vertex_layout(const vertex_data& _Data)
{
	layout::value Layout = layout::pos;
	if (_Data.positions.empty())
		oTHROW_INVARG("vertices must have positions");
	if (!_Data.normals.empty())
		Layout = _Data.texcoords.empty() ? layout::pos_nrm : layout::pos_nrm_tan_uv0;
	if (!_Data.texcoords.empty())
		Layout = layout::pos_uv0;
	return Layout;
}

class mesh_impl : public mesh
{
public:
	mesh_impl(const init& _Init, const path& _OBJPath, const char* _OBJString);
	info get_info() const override;

private:
	vertex_data Data;
	std::vector<uint> Indices;
	layout::value VertexLayout;
	path Path;
};

mesh_impl::mesh_impl(const init& _Init, const path& _OBJPath, const char* _OBJString)
	: Path(_OBJPath)
{
	const size_t kInitialReserve = _Init.est_num_indices * sizeof(uint);
	size_t IndexMapMallocBytes = 0;
	
	std::vector<uint> DegenerateNormals, DegenerateTexcoords;
	DegenerateNormals.reserve(1000);
	DegenerateTexcoords.reserve(1000);

	// Scope memory usage... more might be used below when calculating normals, 
	// etc. so free this stuff up rather than leaving it all around.
	
	{
		void* pArena = malloc(kInitialReserve);
		finally FreeArena([&] { if (pArena) free(pArena); });
		linear_allocator Allocator(pArena, kInitialReserve);
		index_map_t IndexMap(0, index_map_t::hasher(), index_map_t::key_equal(), std::less<key_t>()
			, allocator_type(&Allocator, &IndexMapMallocBytes));

		// OBJ files don't contain a same-sized vertex streams for each elements, 
		// and indexing occurs uniquely between vertex elements. Eventually we will 
		// create same-sized vertex data - even by replication of data - so that a 
		// single index buffer can be used. That's the this->VertexElements, but 
		// first to get the raw vertex data off disk, use this.
		vertex_data OffDiskElements;
		OffDiskElements.reserve(_Init.est_num_vertices, _Init.est_num_indices / 3);
		parse_elements(_OBJString, _Init.flip_handedness, &OffDiskElements);

		const uint kEstMaxVertexElements = as_uint(__max(__max(OffDiskElements.positions.size(), OffDiskElements.normals.size()), OffDiskElements.texcoords.size()));
		Data.reserve(kEstMaxVertexElements, _Init.est_num_indices / 3);
		Indices.reserve(_Init.est_num_indices);

		reduce_elements(_Init, &IndexMap, OffDiskElements, &Indices, &Data, &DegenerateNormals, &DegenerateTexcoords);

	} // End of life for from-disk vertex elements and index hash

	#ifdef _DEBUG
		if (IndexMapMallocBytes)
		{
			mstring reserved, additional;
			oTRACE("obj: %s index map allocated %s additional indices beyond the initial EstimatedNumIndices=%s", oSAFESTRN(_OBJPath), format_commas(additional, as_uint(IndexMapMallocBytes / sizeof(uint))), format_commas(reserved, _Init.est_num_vertices));
		}
	#endif

	if (_Init.calc_normals_on_error)
	{
		bool CalcNormals = false;
		if (Data.normals.empty())
		{
			oTRACE("obj: No normals found in %s...", _OBJPath);
			CalcNormals = true;
		}

		else if (!DegenerateNormals.empty())
		{
			// @tony: Is there a way to calculate only the degenerates?
			oTRACE("oOBJ: %u degenerate normals in %s...", DegenerateNormals.size(), _OBJPath);
			CalcNormals = true;
		}

		if (CalcNormals)
		{
			oTRACE("Calculating vertex normals... (%s)", oSAFESTRN(_OBJPath));
			sstring StrTime;
			timer t;
			Data.normals.resize(Data.positions.size());
			calc_vertex_normals(Data.normals.data(), Indices.data(), as_uint(Indices.size()), Data.positions.data(), as_uint(Data.positions.size()), _Init.counter_clockwide_faces, true);
			format_duration(StrTime, t.seconds(), true, true);
			oTRACE("Calculating vertex normals done in %s. (%s)", StrTime.c_str(), oSAFESTRN(_OBJPath));
		}
	}

	if (_Init.calc_texcoords_on_error)
	{
		bool CalcTexcoords = false;
		if (Data.texcoords.empty())
		{
			oTRACE("oOBJ: No texcoords found in %s...", oSAFESTRN(_OBJPath));
			CalcTexcoords = true;
		}

		else if (!DegenerateTexcoords.empty())
		{
			oTRACE("oOBJ: %u degenerate texcoords in %s...", DegenerateTexcoords.size(), oSAFESTRN(_OBJPath));
			CalcTexcoords = true;
		}

		if (CalcTexcoords)
		{
			Data.texcoords.resize(Data.positions.size());
			sstring StrTime;
			double SolverTime = 0.0;
			oTRACE("Calculating texture coordinates... (%s)", oSAFESTRN(_OBJPath));
			try { calc_texcoords(Data.bound, Indices.data(), as_uint(Indices.size()), Data.positions.data(), Data.texcoords.data(), as_uint(Data.texcoords.size()), &SolverTime); }
			catch (std::exception& e)
			{
				Data.texcoords.clear();
				oTRACEA("Calculating texture coordinates failed. %s (%s)", _OBJPath, e.what());
			}

			format_duration(StrTime, SolverTime, true, true);
			oTRACE("Calculating texture coordinates done in %s. (%s)", StrTime.c_str(), oSAFESTRN(_OBJPath));
		}
	}
}

std::shared_ptr<mesh> mesh::make(const init& _Init, const path& _OBJPath, const char* _OBJString)
{
	return std::make_shared<mesh_impl>(_Init, _OBJPath, _OBJString);
}

info mesh_impl::get_info() const
{
	info i;
	i.obj_path = Path;
	i.mtl_path = Data.mtl_path;
	i.groups = Data.groups.data();

	i.mesh_info.local_space_bound = Data.bound;
	i.mesh_info.num_indices = as_uint(Indices.size());
	i.mesh_info.num_vertices = as_uint(Data.positions.size());
	i.mesh_info.vertex_layouts[0] = get_vertex_layout(Data);
	i.mesh_info.primitive_type = primitive_type::triangles;
	i.mesh_info.face_type = face_type::unknown;
	i.mesh_info.num_ranges = (uchar)Data.groups.size();
	i.mesh_info.vertex_scale_shift = 0;

	i.data.indicesi = Indices.data();
	i.data.ranges = Data.ranges.data();
	i.data.range_pitch = sizeof(range);
	i.data.positionsf = Data.positions.data();
	i.data.positionf_pitch = sizeof(float3);
	i.data.normalsf = Data.normals.data();
	i.data.normalf_pitch = sizeof(float3);
	i.data.uvw0sf = Data.texcoords.data();
	i.data.uvw0f_pitch = sizeof(float3);
	i.data.vertex_layout = i.mesh_info.vertex_layouts[0];
	return i;
}

bool from_string_opt(float3* _pDest, const char** _pStart)
{
	const char* c = *_pStart;
	move_past_line_whitespace(&++c);
	if (!from_string(_pDest, c))
	{
		if (!from_string((float2*)_pDest, c))
		{
			if (!from_string((float*)&_pDest, c))
				return false;

			move_to_whitespace(&c);
		}

		else
		{
			move_next_word(&c);
			move_to_whitespace(&c);
		}
	}

	else
	{
		move_next_word(&c);
		move_next_word(&c);
		move_to_whitespace(&c);
	}

	*_pStart = c;
	return true;
}

static texture_info parse_texture_info(const char* _TextureLine)
{
	texture_info i;
	const char* c = _TextureLine;
	while (*c)
	{
		if (*c == '-' && *(c-1) == ' ')
		{
			c++;
			if (*c == 'o')
			{
				if (!from_string_opt(&i.origin_offset, &c))
					break;
			}

			else if (*c == 's')
			{
				if (!from_string_opt(&i.scale, &c))
					break;
			}

			else if (*c == 't')
			{
				if (!from_string_opt(&i.turbulance, &c))
					break;
			}

			else if (*c == 'm' && *(c+1) == 'm')
			{
				c += 2;
				move_past_line_whitespace(&++c);
				if (!from_string(&i.brightness_gain, c))
					break;

				move_next_word(&c);
				move_to_whitespace(&c);
			}

			else if (*c == 'b' && *(c+1) == 'm')
			{
				c += 2;
				move_past_line_whitespace(&++c);
				if (!from_string(&i.bump_multiplier, c))
					break;

				move_to_whitespace(&c);
			}

			else if (!_memicmp(c, "blendu", 6))
			{
				c += 6;
				move_past_line_whitespace(&c);
				i.blendu = !!_memicmp(c, "on", 2);
				c += i.blendu ? 2 : 3;
			}

			else if (!_memicmp(c, "blendv", 6))
			{
				c += 6;
				move_past_line_whitespace(&c);
				i.blendv = !!_memicmp(c, "on", 2);
				c += i.blendv ? 2 : 3;
			}

			else if (!_memicmp(c, "boost", 5))
			{
				c += 5;
				move_past_line_whitespace(&c);
				if (!from_string(&i.boost, c))
					break;

				move_to_whitespace(&c);
			}

			else if (!_memicmp(c, "texres", 6))
			{
				c += 6;
				move_past_line_whitespace(&c);
				if (!from_string(&i.resolution, c))
					break;

				move_next_word(&c);
				move_to_whitespace(&c);
			}

			else if (!_memicmp(c, "clamp", 5))
			{
				c += 5;
				move_past_line_whitespace(&c);
				i.blendv = !!_memicmp(c, "on", 2);
				c += i.blendv ? 2 : 3;
			}

			else if (!_memicmp(c, "imfchan", 7))
			{
				c += 7;
				move_past_line_whitespace(&c);
				i.imfchan = *c++;
			}

			else if (!_memicmp(c, "type", 4))
			{
				c += 4;
				move_past_line_whitespace(&c);

				if (!from_string(&i.type, c))
					break;
			}
		}
		else
		{
			move_past_line_whitespace(&c);
			i.path = c;
			return i;
		}
	}

	oTHROW(io_error, "error parsing obj texture info");
}

static void parse_materials(std::vector<material_info>& _Materials, const path& _MTLPath, const char* _MTLString)
{
	// NOTE: _MTLPath is not used yet, but leave this in case we need
	// to report better errors.

	material_info i;

	float3* pColor = nullptr;
	char type = 0;
	char buf[_MAX_PATH];

	const char* r = _MTLString;
	while (*r)
	{
		move_past_line_whitespace(&r);
		switch (*r)
		{
			case 'n':
				r += 6; // "newmtl"
				move_past_line_whitespace(&r);
				sscanf_s(r, "%[^\r|^\n]", buf, oCOUNTOF(buf));
				_Materials.resize(_Materials.size() + 1);
				_Materials.back().name = buf;
				break;
			case 'N':
				type = *(++r);
				sscanf_s(++r, "%f", type == 's' ? &_Materials.back().specularity : &_Materials.back().refraction_index);
				break;
			case 'T': // Tr
				type = *(++r);
				if (type == 'f')
				{
					float3* pColor = (float3*)&_Materials.back().transmission_color;
					sscanf_s(++r, "%f %f %f", &pColor->x, &pColor->y, &pColor->z);
				}
				else if (type == 'r') 
					sscanf_s(++r, "%f", &_Materials.back().transparency); // 'd' or 'Tr' are the same (transparency)
				break;
			case 'd':
				sscanf_s(++r, "%f", &_Materials.back().transparency);
				break;
			case 'K':
				if (r[1] == 'm') break; // I can't find Km documented anywhere.
				switch (r[1])
				{
					case 'a': pColor = (float3*)&_Materials.back().ambient_color; break;
					case 'e': pColor = (float3*)&_Materials.back().emissive_color; break;
					case 'd': pColor = (float3*)&_Materials.back().diffuse_color; break;
					case 's': pColor = (float3*)&_Materials.back().specular_color; break;
					default: break;
				}
				sscanf_s(r+2, "%f %f %f", &pColor->x, &pColor->y, &pColor->z);
				break;
			case 'm':
				sscanf_s(r + strcspn(r, "\t "), "%[^\r|^\n]", buf, oCOUNTOF(buf));

				#define oOBJTEX(_Name) do \
				{	if (!parse_texture_info(buf + 1, &_Materials.back()._Name)) \
						oTHROW(io_error, "Failed to parse \"%s\"", r); \
				} while(false)
				
				if (!_memicmp("map_Ka", r, 6)) _Materials.back().ambient = parse_texture_info(buf + 1);
				else if (!_memicmp("map_Kd", r, 6)) _Materials.back().diffuse = parse_texture_info(buf + 1);
				else if (!_memicmp("map_Ks", r, 6)) _Materials.back().specular = parse_texture_info(buf + 1);
				else if (!_memicmp("map_d", r, 5) || 0 == _memicmp("map_opacity", r, 11)) _Materials.back().alpha = parse_texture_info(buf + 1);
				else if (!_memicmp("map_Bump", r, 8) || 0 == _memicmp("bump", r, 4)) _Materials.back().bump = parse_texture_info(buf + 1);
				break;
			case 'i':
				r += 5; // "illum"
				move_past_line_whitespace(&r);
				sscanf_s(r, "%d", &_Materials.back().illum);
				break;
			default:
				break;
		}
		move_to_line_end(&r);
		move_past_newline(&r);
	}
}

class material_lib_impl : public material_lib
{
public:
	material_lib_impl(const path& _MTLPath, const char* _MTLString);
	material_lib_info get_info() const override;
private:
	std::vector<material_info> Materials;
	path Path;
};

material_lib_impl::material_lib_impl(const path& _MTLPath, const char* _MTLString)
{
	Materials.reserve(16);
	parse_materials(Materials, _MTLPath, _MTLString);
}

std::shared_ptr<material_lib> material_lib::make(const path& _MTLPath, const char* _MTLString)
{
	return std::make_shared<material_lib_impl>(_MTLPath, _MTLString);
}

material_lib_info material_lib_impl::get_info() const
{
	material_lib_info i;
	i.materials = Materials.data();
	i.num_materials = as_uint(Materials.size());
	i.mtl_path = Path;
	return i;
}

		} // namespace obj
	} // namespace mesh
} // namespace ouro 