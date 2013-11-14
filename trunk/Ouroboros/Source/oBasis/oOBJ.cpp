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
#include <oBasis/oOBJ.h>
#include <oBase/algorithm.h>
#include <oBase/atof.h>
#include <oBase/finally.h>
#include <oBase/fixed_string.h>
#include <oBase/assert.h>
#include <oBase/macros.h>
#include <oStd/for.h>
#include <oBase/timer.h>
#include <oBase/unordered_map.h>
#include <oBasis/oContainer.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oMath.h>
#include <oBasis/oMeshUtil.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oStdLinearAllocator.h>

using namespace ouro;

// To translate from several index streams - one per vertex element stream - to 
// a single index buffer, we'll need to replicate vertices by their unique 
// combination of all vertex elements. This basically means an index is uniquely
// identified by a hash of the streams it indexes. We'll build the hash as well
// parse faces, but then it all can be freed at once. To prevent a length series
// of deallocs for indices - i.e. int values - use a linear allocator and then
// free all entries once the map goes out of scope.
typedef unsigned long long key_t;
typedef unsigned int val_t;
typedef std::pair<const key_t, val_t> pair_type;
typedef oStdLinearAllocator<pair_type> allocator_type;
typedef unordered_map<key_t, val_t, oNoopHash<key_t>, std::equal_to<key_t>, std::less<key_t>, allocator_type> index_map_t;

oDEFINE_WHITESPACE_PARSING();

static inline const char* ParseString(char* _StrDestination, size_t _SizeofStrDestination, const char* _S)
{
	move_next_word(&_S);
	const char* start = _S;
	move_to_line_end(&_S);
	size_t len = std::distance(start, _S);
	strncpy(_StrDestination, _SizeofStrDestination, start, len);
	return _S;
}

template<size_t size> static inline const char* ParseString(ouro::fixed_string<char, size>& _StrDestination, const char* r) { return ParseString(_StrDestination, _StrDestination.capacity(), r); }

enum oOBJ_VERTEX_ELEMENT
{
	oOBJ_POSITIONS,
	oOBJ_TEXCOORDS,
	oOBJ_NORMALS,

	oOBJ_VERTEX_NUM_ELEMENTS,
};

static const size_t oOBJ_MAX_NUM_VERTICES_PER_FACE = 4;

struct oOBJ_FACE
{
	oOBJ_FACE() 
		: NumIndices(0)
		, GroupIndex(0)
	{
		memset(Index, oInvalid, sizeof(Index));
	}

	unsigned int Index[oOBJ_MAX_NUM_VERTICES_PER_FACE][oOBJ_VERTEX_NUM_ELEMENTS];
	unsigned int NumIndices; // 3 for tris, 4 for quads
	unsigned int GroupIndex;
};

struct oOBJ_ELEMENTS
{
	void Reserve(size_t _NewVertexCount, size_t _NewFaceCount)
	{
		Positions.reserve(_NewVertexCount);
		Normals.reserve(_NewVertexCount);
		Texcoords.reserve(_NewVertexCount);
		Faces.reserve(_NewFaceCount);
		Groups.reserve(20);
	}

	oAABoxf Bound;

	std::vector<float3> Positions;
	std::vector<float3> Normals;
	std::vector<float3> Texcoords;
	
	// these are assigned after negative indices have been handled, but are 
	// otherwise directly-from-file values.
	std::vector<oOBJ_FACE> Faces;

	// On from-disk first-pass only the GroupName and MaterialName fields are 
	// valid. The range is calculated in the second reduction pass.
	std::vector<oOBJ_GROUP> Groups;

	path_string MTLPath;
};

// Given a string that starts with the letter 'v', parse as a line of vector
// values and push_back into the appropriate vector.
static const char* oOBJParseVLine(const char* _V, bool _FlipHandedness, oOBJ_ELEMENTS* _pElements)
{
	float3 temp;

	_V++;
	switch(*_V)
	{
		case ' ':
			atof(&_V, &temp.x);
			atof(&_V, &temp.y);
			atof(&_V, &temp.z); if (_FlipHandedness) temp.z = -temp.z;
			_pElements->Positions.push_back(temp);
			oExtendBy(_pElements->Bound, temp);
			break;
		case 't':
			move_to_whitespace(&_V);
			atof(&_V, &temp.x);
			atof(&_V, &temp.y);
			if (!atof(&_V, &temp.z)) temp.z = 0.0f;
			if (_FlipHandedness) temp.y = 1.0f - temp.y;
			_pElements->Texcoords.push_back(temp);
			break;
		case 'n':
			move_to_whitespace(&_V);
			atof(&_V, &temp.x);
			atof(&_V, &temp.y);
			atof(&_V, &temp.z); if (_FlipHandedness) temp.z = -temp.z;
			_pElements->Normals.push_back(temp);
			break;
		oNODEFAULT;
	}

	return _V;
}

// Fills the specified face with data from a line in an OBJ starting with the 
// 'f' (face) character. This returns a pointer into the string _F that is 
// either the end of the string, or the end of the line.
static const char* oOBJParseFLine(const char* _F, oOBJ_ELEMENTS* _pElements)
{
	move_to_whitespace(&_F);
	move_past_line_whitespace(&_F);
	oOBJ_FACE face;
	face.NumIndices = 0;
	face.GroupIndex = oUInt(_pElements->Groups.size());
	while (face.NumIndices < 4 && *_F != 0 && !is_newline(*_F))
	{
		bool foundSlash = false;
		int element = oOBJ_POSITIONS;
		do
		{
			// @tony: GAH! This is relative to the vertex parsing up to this 
			// point, not absolute from the end. /sigh...

			unsigned int ZeroBasedIndexFromFile = 0;

			// Negative indices implies the max indexable vertex OBJ will be INT_MAX, 
			// not UINT_MAX.
			if (*_F == '-')
			{
				int NegativeIndex = atoi(_F);
				unsigned int NumElements = 0;
				switch (element)
				{
					case oOBJ_POSITIONS: NumElements = oUInt(_pElements->Positions.size()); break;
					case oOBJ_TEXCOORDS: NumElements = oUInt(_pElements->Texcoords.size()); break;
					case oOBJ_NORMALS: NumElements = oUInt(_pElements->Normals.size()); break;
					oNODEFAULT;
				}
				ZeroBasedIndexFromFile = NumElements + NegativeIndex;
			}
			else
				ZeroBasedIndexFromFile = atoi(_F) - 1;

			face.Index[face.NumIndices][element] = ZeroBasedIndexFromFile;

			if (element == (oOBJ_VERTEX_NUM_ELEMENTS-1))
				break;

			while (isdigit(*_F) || *_F == '-')
				_F++;

			// support case where the texcoord channel is empty
			
			if (*_F == '/')
			{
				do
				{
					_F++;
					element++;
					foundSlash = true;
				} while(*_F == '/');
			}

			else
				break;

			move_past_line_whitespace(&_F);

		} while(foundSlash);

		move_next_word(&_F);
		face.NumIndices++;
	}

	_pElements->Faces.push_back(face);

	return _F;
}

// Scans an OBJ string and appends vertex element data
static bool oOBJParseElements(const char* _OBJString, bool _FlipHandedness, oOBJ_ELEMENTS* _pElements)
{
	unsigned int NumGroups = 0;
	oOBJ_GROUP group;

	const char* line = _OBJString;
	while (*line)
	{
		move_past_line_whitespace(&line);
		switch (*line)
		{
			case 'v':
				line = oOBJParseVLine(line, _FlipHandedness, _pElements);
				break;
			case 'f':
				line = oOBJParseFLine(line, _pElements);
				break;
			case 'g':
				// close out previous group
				if (NumGroups)
					_pElements->Groups.push_back(group); 
				NumGroups++;
				ParseString(group.GroupName, line);
				break;
			case 'u':
				ParseString(group.MaterialName, line);
				break;
			case 'm':
				ParseString(_pElements->MTLPath, line);
				break;
			default:
				break;
		}
		move_to_line_end(&line);
		move_past_newline(&line);
	}

	// close out a remaining group one last time
	if (NumGroups)
	{
		//group.Range.NumTriangles = oUInt(Indices.size() / 3) - group.Range.StartTriangle;
		_pElements->Groups.push_back(group);
	}

	else
	{
		group.GroupName = "Default Group";
		//group.Range.StartTriangle = 0;
		//group.Range.NumTriangles = oUInt(Indices.size() / 3);
		_pElements->Groups.push_back(group);
	}

	return true;
}

// Using config data and an index hash, reduce all duplicate/face data into 
// unique indexed data. This must be called after ALL SourceElements have been
// populated.
static void ReduceElements(const oOBJ_INIT& _Init
	, index_map_t* _pIndexMap
	, const oOBJ_ELEMENTS& _SourceElements
	, std::vector<unsigned int>* _pIndices
	, oOBJ_ELEMENTS* _pSinglyIndexedElements
	, std::vector<unsigned int>* _pDegenerateNormals
	, std::vector<unsigned int>* _pDegenerateTexcoords)
{
	unsigned int resolvedIndices[4];
	_pSinglyIndexedElements->Bound = _SourceElements.Bound;
	_pSinglyIndexedElements->Groups = _SourceElements.Groups;
	_pSinglyIndexedElements->MTLPath = _SourceElements.MTLPath;
	unsigned int LastGroupIndex = oInvalid;

	oFOR(const oOBJ_FACE& Face, _SourceElements.Faces)
	{
		oOBJ_GROUP& Group = _pSinglyIndexedElements->Groups[Face.GroupIndex];

		if (LastGroupIndex != Face.GroupIndex)
		{
			if (LastGroupIndex != oInvalid)
			{
				oOBJ_GROUP& LastGroup = _pSinglyIndexedElements->Groups[LastGroupIndex];
				LastGroup.Range.NumPrimitives = oUInt(_pIndices->size() / 3) - LastGroup.Range.StartPrimitive;
				Group.Range.StartPrimitive = oUInt(_pIndices->size() / 3);
			}
			LastGroupIndex = Face.GroupIndex;
		}

		for (unsigned int p = 0; p < Face.NumIndices; p++)
		{
			const key_t hash = Face.Index[p][oOBJ_POSITIONS] + _SourceElements.Positions.size() * Face.Index[p][oOBJ_TEXCOORDS] + _SourceElements.Positions.size() * _SourceElements.Texcoords.size() * Face.Index[p][oOBJ_NORMALS];
			index_map_t::iterator it = _pIndexMap->find(hash);

			if (it == _pIndexMap->end())
			{
				unsigned int NewIndex = oUInt(_pSinglyIndexedElements->Positions.size());
				_pSinglyIndexedElements->Positions.push_back(_SourceElements.Positions[Face.Index[p][oOBJ_POSITIONS]]);
			
				if (!_SourceElements.Normals.empty())
				{
					if (Face.Index[p][oOBJ_NORMALS] != oInvalid)
						_pSinglyIndexedElements->Normals.push_back(_SourceElements.Normals[Face.Index[p][oOBJ_NORMALS]]);
					else
					{
						_pDegenerateNormals->push_back(NewIndex);
						_pSinglyIndexedElements->Normals.push_back(oZERO3);
					}
				}

				if (!_SourceElements.Texcoords.empty())
				{
					if(Face.Index[p][oOBJ_TEXCOORDS] != oInvalid)
						_pSinglyIndexedElements->Texcoords.push_back(_SourceElements.Texcoords[Face.Index[p][oOBJ_TEXCOORDS]]);
					else
					{
						_pDegenerateTexcoords->push_back(NewIndex);
						_pSinglyIndexedElements->Texcoords.push_back(oZERO3);
					}
				}
			
				(*_pIndexMap)[hash] = resolvedIndices[p] = NewIndex;
			}
		
			else
				resolvedIndices[p] = (*it).second;
		} // for each face element

		// Now that the index has either been added or reset to a pre-existing index, 
		// add the face definition to the index list

		static const unsigned int CCWindices[6] = { 0, 2, 1, 2, 0, 3, };
		static const unsigned int CWindices[6] = { 0, 1, 2, 2, 3, 0, };
	
		bool UseCCW = _Init.CounterClockwiseFaces;
		if (_Init.FlipHandedness)
			UseCCW = !UseCCW;
	
		const unsigned int* pOrder = UseCCW ? CCWindices : CWindices;

		_pIndices->push_back(resolvedIndices[pOrder[0]]);
		_pIndices->push_back(resolvedIndices[pOrder[1]]);
		_pIndices->push_back(resolvedIndices[pOrder[2]]);

		// Add another triangle for the rest of the quad
		if (Face.NumIndices == 4)
		{
			_pIndices->push_back(resolvedIndices[pOrder[3]]);
			_pIndices->push_back(resolvedIndices[pOrder[4]]);
			_pIndices->push_back(resolvedIndices[pOrder[5]]);
		}
	} // for each face

	// close out last group
	unsigned int LastFaceGroupIndex = _SourceElements.Faces.back().GroupIndex;
	oGPU_RANGE& r = _pSinglyIndexedElements->Groups[LastFaceGroupIndex].Range;
	r.NumPrimitives = oUInt(_pIndices->size() / 3) - r.StartPrimitive;

	// Go back through groups and calc min/max verts
	oFOR(oOBJ_GROUP& g, _pSinglyIndexedElements->Groups)
		oCalcMinMaxVertices(data(*_pIndices), g.Range.StartPrimitive*3, g.Range.NumPrimitives*3, oUInt(_pSinglyIndexedElements->Positions.size()), &g.Range.MinVertex, &g.Range.MaxVertex);
}

static uint oOBJGetVertexElements(oGPU_VERTEX_ELEMENT* _pElements, uint _MaxNumElements, const oOBJ_DESC& _OBJDesc)
{
	uchar n = 0;

	if (_OBJDesc.pPositions)
	{
		_pElements[n].Semantic = 'POS0';
		_pElements[n].Format = ouro::surface::r32g32b32_float;
		_pElements[n].InputSlot = 0;
		_pElements[n].Instanced = false;
		n++;
	}

	if (_OBJDesc.pTexcoords)
	{
		_pElements[n].Semantic = 'TEX0';
		_pElements[n].Format = ouro::surface::r32g32b32_float;
		_pElements[n].InputSlot = n;
		_pElements[n].Instanced = false;
		n++;
	}

	if (_OBJDesc.pNormals)
	{
		_pElements[n].Semantic = 'NRM0';
		_pElements[n].Format = ouro::surface::r32g32b32_float;
		_pElements[n].InputSlot = n;
		_pElements[n].Instanced = false;
		n++;
	}

	oASSERT(_MaxNumElements >= 3, "Too many elements for destination");
	return n;
}

struct oOBJImpl : oOBJ
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oOBJImpl(const char* _OBJPath, const char* _OBJString, const oOBJ_INIT& _Init, bool* _pSuccess);
	void GetDesc(oOBJ_DESC* _pDesc) const threadsafe override;

protected:
	void GetDesc(oOBJ_DESC* _pDesc) const;
	oOBJ_ELEMENTS VertexElements;
	std::vector<unsigned int> Indices;
	oGPU_VERTEX_ELEMENT GPUVertexElements[3];
	uint NumGPUVertexElements;
	path_string OBJPath;
	oRefCount RefCount;
};

oOBJImpl::oOBJImpl(const char* _OBJPath, const char* _OBJString, const oOBJ_INIT& _Init, bool* _pSuccess)
	: OBJPath(_OBJPath)
{
	*_pSuccess = false;

	const size_t kInitialReserve = _Init.EstimatedNumIndices * sizeof(unsigned int);
	size_t IndexMapMallocBytes = 0;
	
	std::vector<unsigned int> DegenerateNormals, DegenerateTexcoords;
	DegenerateNormals.reserve(1000);
	DegenerateTexcoords.reserve(1000);

	// Scope memory usage... more might be used below when calculating normals, 
	// etc. so free this stuff up rather than leaving it all around.
	
	{
		void* pArena = malloc(kInitialReserve);
		finally FreeArena([&] { if (pArena) free(pArena); });
		concurrent_linear_allocator Allocator(pArena, kInitialReserve);
		index_map_t IndexMap(0, index_map_t::hasher(), index_map_t::key_equal(), std::less<key_t>()
			, allocator_type(&Allocator, &IndexMapMallocBytes));

		// OBJ files don't contain a same-sized vertex streams for each elements, 
		// and indexing occurs uniquely between vertex elements. Eventually we will 
		// create same-sized vertex data - even by replication of data - so that a 
		// single index buffer can be used. That's the this->VertexElements, but 
		// first to get the raw vertex data off disk, use this.
		oOBJ_ELEMENTS OffDiskElements;
		OffDiskElements.Reserve(_Init.EstimatedNumVertices, _Init.EstimatedNumIndices / 3);

		if (!oOBJParseElements(_OBJString, _Init.FlipHandedness, &OffDiskElements))
		{
			oErrorSetLast(std::errc::io_error, "Error parsing vertex elements in %s", _OBJPath);
			return;
		}

		const size_t kEstMaxVertexElements = __max(__max(OffDiskElements.Positions.size(), OffDiskElements.Normals.size()), OffDiskElements.Texcoords.size());
		VertexElements.Reserve(kEstMaxVertexElements, _Init.EstimatedNumIndices / 3);
		Indices.reserve(_Init.EstimatedNumIndices);

		ReduceElements(_Init, &IndexMap, OffDiskElements, &Indices, &VertexElements, &DegenerateNormals, &DegenerateTexcoords);

	} // End of life for from-disk vertex elements and index hash

	#ifdef _DEBUG
		if (IndexMapMallocBytes)
		{
			mstring reserved, additional;
			oTRACE("oOBJ: %s index map allocated %s additional indices beyond the initial EstimatedNumIndices=%s", oSAFESTRN(_OBJPath), format_commas(additional, (uint)oUInt(IndexMapMallocBytes / sizeof(unsigned int))), format_commas(reserved, _Init.EstimatedNumVertices));
		}
	#endif

	if (_Init.CalcNormalsOnError)
	{
		bool CalcNormals = false;
		if (VertexElements.Normals.empty())
		{
			oTRACE("oOBJ: No normals found in %s...", _OBJPath);
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
			VertexElements.Normals.resize(VertexElements.Positions.size());
			oCalcVertexNormals(data(VertexElements.Normals), data(Indices), Indices.size(), data(VertexElements.Positions), VertexElements.Positions.size(), _Init.CounterClockwiseFaces, true);
			format_duration(StrTime, t.seconds(), true, true);
			oTRACE("Calculating vertex normals done in %s. (%s)", StrTime.c_str(), oSAFESTRN(_OBJPath));
		}
	}

	if (_Init.CalcTexcoordsOnError)
	{
		bool CalcTexcoords = false;
		if (VertexElements.Texcoords.empty())
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
			VertexElements.Texcoords.resize(VertexElements.Positions.size());
			sstring StrTime;
			double SolverTime = 0.0;
			oTRACE("Calculating texture coordinates... (%s)", oSAFESTRN(_OBJPath));
			if (!oCalcTexcoords(VertexElements.Bound, data(Indices), oUInt(Indices.size()), data(VertexElements.Positions), data(VertexElements.Texcoords), oUInt(VertexElements.Texcoords.size()), &SolverTime))
			{
				VertexElements.Texcoords.clear();
				oTRACE("Calculating texture coordinates failed. %s (%s)", oErrorGetLastString(), _OBJPath);
			}

			format_duration(StrTime, SolverTime, true, true);
			oTRACE("Calculating texture coordinates done in %s. (%s)", StrTime.c_str(), oSAFESTRN(_OBJPath));
		}
	}

	oOBJ_DESC d;
	oOBJImpl::GetDesc(&d);
	NumGPUVertexElements = oOBJGetVertexElements(GPUVertexElements, oCOUNTOF(GPUVertexElements), d);

	*_pSuccess = true;
}

void oOBJImpl::GetDesc(oOBJ_DESC* _pDesc) const
{
	_pDesc->OBJPath = OBJPath;
	_pDesc->MTLPath = VertexElements.MTLPath;
	_pDesc->pPositions = data(VertexElements.Positions);
	_pDesc->pNormals = data(VertexElements.Normals);
	_pDesc->pTexcoords = data(VertexElements.Texcoords);
	_pDesc->pIndices = data(Indices);
	_pDesc->pGroups = data(VertexElements.Groups);
	_pDesc->pVertexElements = GPUVertexElements;
	_pDesc->NumVertexElements = NumGPUVertexElements;
	_pDesc->NumVertices = oUInt(VertexElements.Positions.size());
	_pDesc->NumIndices = oUInt(Indices.size());
	_pDesc->NumGroups = oUInt(VertexElements.Groups.size());
	_pDesc->Bound = VertexElements.Bound;
}

void oOBJImpl::GetDesc(oOBJ_DESC* _pDesc) const threadsafe
{
	oThreadsafe(this)->GetDesc(_pDesc); // safe because the object is const throughout its lifetime
}

bool oOBJCreate(const char* _OBJPath, const char* _OBJString, const oOBJ_INIT& _Init, threadsafe oOBJ** _ppOBJ)
{
	bool success = false;
	oCONSTRUCT(_ppOBJ, oOBJImpl(_OBJPath, _OBJString, _Init, &success));
	return success;
}

namespace ouro {

bool from_string(oOBJ_TEXTURE_TYPE* _pType, const char* _StrSource)
{
	static const char* sStrings[] = 
	{
		"",
		"cube_right",
		"cube_left",
		"cube_top",
		"cube_bottom",
		"cube_back",
		"cube_front",
		"sphere",
	};
	static_assert(oOBJ_TEXTURE_TYPE_COUNT == oCOUNTOF(sStrings), "array mismatch");

	for (size_t i = 0; i < oCOUNTOF(sStrings); i++)
	{
		if (!_stricmp(sStrings[i], _StrSource))
		{
			*_pType = (oOBJ_TEXTURE_TYPE)i;
			return true;
		}
	}

	return false;
}

} // namespace ouro

bool oFromStringOptVec(float3* _pDest, const char** _pStart)
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

static bool oOBJParseTextureDesc(const char* _TextureLine, oOBJ_TEXTURE* _pTexture)
{
	*_pTexture = oOBJ_TEXTURE();
	const char* c = _TextureLine;
	while (*c)
	{
		if (*c == '-' && *(c-1) == ' ')
		{
			c++;
			if (*c == 'o')
			{
				if (!oFromStringOptVec(&_pTexture->OriginOffset, &c))
					return false;
			}

			else if (*c == 's')
			{
				if (!oFromStringOptVec(&_pTexture->Scale, &c))
					return false;
			}

			else if (*c == 't')
			{
				if (!oFromStringOptVec(&_pTexture->Turbulance, &c))
					return false;
			}

			else if (*c == 'm' && *(c+1) == 'm')
			{
				c += 2;
				move_past_line_whitespace(&++c);
				if (!from_string(&_pTexture->BrightnessGain, c))
					return false;

				move_next_word(&c);
				move_to_whitespace(&c);
			}

			else if (*c == 'b' && *(c+1) == 'm')
			{
				c += 2;
				move_past_line_whitespace(&++c);
				if (!from_string(&_pTexture->BumpMultiplier, c))
					return false;

				move_to_whitespace(&c);
			}

			else if (!_memicmp(c, "blendu", 6))
			{
				c += 6;
				move_past_line_whitespace(&c);
				_pTexture->Blendu = !!_memicmp(c, "on", 2);
				c += _pTexture->Blendu ? 2 : 3;
			}

			else if (!_memicmp(c, "blendv", 6))
			{
				c += 6;
				move_past_line_whitespace(&c);
				_pTexture->Blendv = !!_memicmp(c, "on", 2);
				c += _pTexture->Blendv ? 2 : 3;
			}

			else if (!_memicmp(c, "boost", 5))
			{
				c += 5;
				move_past_line_whitespace(&c);
				if (!from_string(&_pTexture->Boost, c))
					return false;

				move_to_whitespace(&c);
			}

			else if (!_memicmp(c, "texres", 6))
			{
				c += 6;
				move_past_line_whitespace(&c);
				if (!from_string(&_pTexture->Resolution, c))
					return false;

				move_next_word(&c);
				move_to_whitespace(&c);
			}

			else if (!_memicmp(c, "clamp", 5))
			{
				c += 5;
				move_past_line_whitespace(&c);
				_pTexture->Blendv = !!_memicmp(c, "on", 2);
				c += _pTexture->Blendv ? 2 : 3;
			}

			else if (!_memicmp(c, "imfchan", 7))
			{
				c += 7;
				move_past_line_whitespace(&c);
				_pTexture->IMFChan = *c++;
			}

			else if (!_memicmp(c, "type", 4))
			{
				c += 4;
				move_past_line_whitespace(&c);

				if (!from_string(&_pTexture->Type, c))
					return false;
			}
		}
		else
		{
			move_past_line_whitespace(&c);
			_pTexture->Path = c;
			return true;
		}
	}

	return false;
}

// @tony: NOTE: _MTLPath is not used yet, but leave this in case we need
// to report better errors.
static bool oMTLParse(const char* _MTLPath, const char* _MTLString, std::vector<oOBJ_MATERIAL>* _pMTLLibrary)
{
	_pMTLLibrary->clear();

	float3* pColor = 0;
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
				_pMTLLibrary->resize(_pMTLLibrary->size() + 1);
				_pMTLLibrary->back().Name = buf;
				break;
			case 'N':
				type = *(++r);
				sscanf_s(++r, "%f", type == 's' ? &_pMTLLibrary->back().Specularity : &_pMTLLibrary->back().RefractionIndex);
				break;
			case 'T': // Tr
				type = *(++r);
				if (type == 'f')
				{
					float3* pColor = (float3*)&_pMTLLibrary->back().TransmissionColor;
					sscanf_s(++r, "%f %f %f", &pColor->x, &pColor->y, &pColor->z);
				}
				else if (type == 'r') 
					sscanf_s(++r, "%f", &_pMTLLibrary->back().Transparency); // 'd' or 'Tr' are the same (transparency)
				break;
			case 'd':
				sscanf_s(++r, "%f", &_pMTLLibrary->back().Transparency);
				break;
			case 'K':
				if (r[1] == 'm') break; // I can't find Km documented anywhere.
				switch (r[1])
				{
					case 'a': pColor = (float3*)&_pMTLLibrary->back().AmbientColor; break;
					case 'e': pColor = (float3*)&_pMTLLibrary->back().EmissiveColor; break;
					case 'd': pColor = (float3*)&_pMTLLibrary->back().DiffuseColor; break;
					case 's': pColor = (float3*)&_pMTLLibrary->back().SpecularColor; break;
					default: break;
				}
				sscanf_s(r+2, "%f %f %f", &pColor->x, &pColor->y, &pColor->z);
				break;
			case 'm':
				sscanf_s(r + strcspn(r, "\t "), "%[^\r|^\n]", buf, oCOUNTOF(buf));

				#define oOBJTEX(_Name) do \
				{	if (!oOBJParseTextureDesc(buf + 1, &_pMTLLibrary->back()._Name)) \
						return oErrorSetLast(std::errc::io_error, "Failed to parse \"%s\"", r); \
				} while(false)
				
				if (!_memicmp("map_Ka", r, 6)) oOBJTEX(Ambient);
				else if (!_memicmp("map_Kd", r, 6)) oOBJTEX(Diffuse);
				else if (!_memicmp("map_Ks", r, 6)) oOBJTEX(Specular);
				else if (!_memicmp("map_d", r, 5) || 0 == _memicmp("map_opacity", r, 11)) oOBJTEX(Alpha);
				else if (!_memicmp("map_Bump", r, 8) || 0 == _memicmp("bump", r, 4)) oOBJTEX(Bump);
				break;
			case 'i':
				r += 5; // "illum"
				move_past_line_whitespace(&r);
				sscanf_s(r, "%d", &_pMTLLibrary->back().Illum);
				break;
			default:
				break;
		}
		move_to_line_end(&r);
		move_past_newline(&r);
	}

	return true;
}

struct oMTLImpl : oMTL
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	oMTLImpl(const char* _MTLPath, const char* _MTLString, bool* _pSuccess);
	void GetDesc(oMTL_DESC* _pDesc) const threadsafe override;
	std::vector<oOBJ_MATERIAL> Materials;
	path_string MTLPath;
	oRefCount RefCount;
};

oMTLImpl::oMTLImpl(const char* _MTLPath, const char* _MTLString, bool* _pSuccess)
	: MTLPath(_MTLPath)
{
	Materials.reserve(16);
	*_pSuccess = oMTLParse(_MTLPath, _MTLString, &Materials);
}

void oMTLImpl::GetDesc(oMTL_DESC* _pDesc) const threadsafe
{
	auto& m = oThreadsafe(Materials); // safe because data is const throughout this object's lifetime

	_pDesc->MTLPath = MTLPath;
	_pDesc->pMaterials = data(m);
	_pDesc->NumMaterials = oUInt(m.size());
}

bool oMTLCreate(const char* _MTLPath, const char* _MTLString, threadsafe oMTL** _ppMTL)
{
	bool success = false;
	oCONSTRUCT(_ppMTL, oMTLImpl(_MTLPath, _MTLString, &success));
	return success;
}

bool oOBJCopyRanges(oGPU_RANGE* _pDestination, size_t _NumRanges, const oOBJ_DESC& _Desc)
{
	if (_Desc.NumGroups > _NumRanges)
		return oErrorSetLast(std::errc::invalid_argument);
	for (uint i = 0; i < _Desc.NumGroups; i++)
		_pDestination[i] = _Desc.pGroups[i].Range;
	return true;
}
