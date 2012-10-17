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
#include <oBasis/oOBJ.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oAssert.h>
#include <oBasis/oAtof.h>
#include <oBasis/oFor.h>
#include <oBasis/oHash.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oLinearAllocator.h>
#include <oBasis/oLockThis.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMeshUtil.h>
#include <oBasis/oOnScopeExit.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oString.h>
#include <oBasis/oTimer.h>
#include <oBasis/oUnorderedMap.h>

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
typedef oUnorderedMap<key_t, val_t, oNoopHash<key_t>, std::equal_to<key_t>, std::less<key_t>, allocator_type> index_map_t;

namespace performance_detail {
	// This was benchmarked on buddha.obj (Standford buddha) to be 120 ms compared
	// with a strcspn/strspn implementation of the Move* functions at 150 ms
	static bool gToWhitespace[256] = {0};
	static bool gPastWhitespace[256] = {0};
	static bool gNewline[256] = {0};

	struct SetupWhitespace
	{
		SetupWhitespace()
		{
			gToWhitespace[' '] = true;
			gToWhitespace['\t'] = true;
			gToWhitespace['\n'] = true;
			gToWhitespace['\r'] = true;
			gToWhitespace['\v'] = true;

			gPastWhitespace[' '] = true;
			gPastWhitespace['\t'] = true;
			gPastWhitespace['\v'] = true;

			gNewline['\n'] = true;
			gNewline['\r'] = true;
		}
	};
	static SetupWhitespace AutoSetupWhitespace;
} // namespace performance_detail

inline bool IsNewline(int c) { return performance_detail::gNewline[c]; }

inline void MovePastWhitespace(const char** _CurPos) { while(**_CurPos && performance_detail::gPastWhitespace[**_CurPos]) ++*_CurPos; }
inline void MoveToWhitespace(const char** _CurPos) { while(**_CurPos && !performance_detail::gToWhitespace[**_CurPos]) ++*_CurPos; }
inline void MoveToEndOfLine(const char** _CurPos) { while(**_CurPos && !IsNewline(**_CurPos)) ++*_CurPos; }
inline void MovePastNewline(const char** _CurPos) { while(**_CurPos && IsNewline(**_CurPos)) ++*_CurPos; }

static inline const char* ParseString(char* _StrDestination, size_t _SizeofStrDestination, const char* _S)
{
	MoveToWhitespace(&_S); MovePastWhitespace(&_S);
	const char* start = _S;
	MoveToEndOfLine(&_S);
	size_t len = std::distance(start, _S);
	oStrncpy(_StrDestination, _SizeofStrDestination, start, len);
	return _S;
}

template<size_t size> static inline const char* ParseString(oFixedString<char, size>& _StrDestination, const char* r) { return ParseString(_StrDestination, _StrDestination.capacity(), r); }

struct oOBJ_VERTEX_ELEMENTS
{
	void Reserve(size_t _NewCount)
	{
		Positions.reserve(_NewCount);
		Normals.reserve(_NewCount);
		Texcoords.reserve(_NewCount);
	}

	std::vector<float3> Positions;
	std::vector<float3> Normals;
	std::vector<float2> Texcoords;
};

// Given a string that starts with the letter 'v', parse as a line of vector
// values and push_back into the appropriate vector.
static const char* oOBJParseVLine(const char* _V, bool _FlipHandedness, oOBJ_VERTEX_ELEMENTS* _pElements)
{
	float3 temp;

	_V++;
	switch(*_V)
	{
		case ' ':
			oAtof(_V, &temp.x); MovePastWhitespace(&_V); MoveToWhitespace(&_V);
			oAtof(_V, &temp.y); MovePastWhitespace(&_V); MoveToWhitespace(&_V);
			oAtof(_V, &temp.z); if (_FlipHandedness) temp.z = -temp.z;
			_pElements->Positions.push_back(temp);
			break;
		case 't':
			MoveToWhitespace(&_V);
			oAtof(_V, &temp.x); MovePastWhitespace(&_V); MoveToWhitespace(&_V);
			oAtof(_V, &temp.y); if (_FlipHandedness) temp.y = 1.0f - temp.y;
			_pElements->Texcoords.push_back(temp.xy());
			break;
		case 'n':
			MoveToWhitespace(&_V);
			oAtof(_V, &temp.x); MovePastWhitespace(&_V); MoveToWhitespace(&_V);
			oAtof(_V, &temp.y); MovePastWhitespace(&_V); MoveToWhitespace(&_V);
			oAtof(_V, &temp.z); if (_FlipHandedness) temp.z = -temp.z;
			_pElements->Normals.push_back(temp);
			break;
		default:
			oASSERT_NOEXECUTION;
	}

	return _V;
}

// Scans an OBJ string and appends vertex element data
static bool oOBJParseVertexElements(const char* _OBJString, bool _FlipHandedness, oOBJ_VERTEX_ELEMENTS* _pElements)
{
	const char* line = _OBJString;
	while (*line)
	{
		MovePastWhitespace(&line);
		switch (*line)
		{
			case 'v':
				line = oOBJParseVLine(line, _FlipHandedness, _pElements);
				break;
			default:
				break;
		}
		MoveToEndOfLine(&line);
		MovePastNewline(&line);
	}

	return true;
}

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
	{
		memset(Index, oInvalid, sizeof(Index));
	}

	unsigned int Index[oOBJ_MAX_NUM_VERTICES_PER_FACE][oOBJ_VERTEX_NUM_ELEMENTS];
	unsigned int NumIndices; // 3 for tris, 4 for quads
};

// Fills the specified face with data from a line in an OBJ starting with the 
// 'f' (face) character. This returns a pointer into the string _F that is 
// either the end of the string, or the end of the line.
static const char* oOBJParseFLine(const char* _F, oOBJ_FACE* _pFace)
{
	MoveToWhitespace(&_F);
	MovePastWhitespace(&_F);

	_pFace->NumIndices = 0;
	while (_pFace->NumIndices < 4 && *_F != 0 && !IsNewline(*_F))
	{
		bool foundSlash = false;

		int element = oOBJ_POSITIONS;
		do
		{
			_pFace->Index[_pFace->NumIndices][element] = atoi(_F) - 1;

			if (element == (oOBJ_VERTEX_NUM_ELEMENTS-1))
				break;

			while(isdigit(*_F))
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

			MovePastWhitespace(&_F);

		} while(foundSlash);

		MoveToWhitespace(&_F);
		MovePastWhitespace(&_F);
		_pFace->NumIndices++;
	}

	return _F;
}

// Using config data and an index hash, add vertex and index data from 
// _SourceElements and the specified face to _pIndices and 
// _pSinglyIndexedElements.
static void AddFace(const oOBJ_INIT& _Init
	, const oOBJ_FACE& _Face
	, index_map_t* _pIndexMap
	, const oOBJ_VERTEX_ELEMENTS& _SourceElements
	, std::vector<unsigned int>* _pIndices
	, oOBJ_VERTEX_ELEMENTS* _pSinglyIndexedElements
	, std::vector<unsigned int>* _pDegenerateNormals
	, std::vector<unsigned int>* _pDegenerateTexcoords)
{
	unsigned int resolvedIndices[4];

	for (unsigned int p = 0; p < _Face.NumIndices; p++)
	{
		const key_t hash = _Face.Index[p][oOBJ_POSITIONS] + _SourceElements.Positions.size() * _Face.Index[p][oOBJ_TEXCOORDS] + _SourceElements.Positions.size() * _SourceElements.Texcoords.size() * _Face.Index[p][oOBJ_NORMALS];
		index_map_t::iterator it = _pIndexMap->find(hash);

		if (it == _pIndexMap->end())
		{
			unsigned int NewIndex = oUInt(_pSinglyIndexedElements->Positions.size());
			_pSinglyIndexedElements->Positions.push_back(_SourceElements.Positions[_Face.Index[p][oOBJ_POSITIONS]]);
			
			if (!_SourceElements.Normals.empty())
			{
				if (_Face.Index[p][oOBJ_NORMALS] != oInvalid)
					_pSinglyIndexedElements->Normals.push_back(_SourceElements.Normals[_Face.Index[p][oOBJ_NORMALS]]);
				else
				{
					_pDegenerateNormals->push_back(NewIndex);
					_pSinglyIndexedElements->Normals.push_back(oZERO3);
				}
			}

			if (!_SourceElements.Texcoords.empty())
			{
				if(_Face.Index[p][oOBJ_TEXCOORDS] != oInvalid)
					_pSinglyIndexedElements->Texcoords.push_back(_SourceElements.Texcoords[_Face.Index[p][oOBJ_TEXCOORDS]]);
				else
				{
					_pDegenerateTexcoords->push_back(NewIndex);
					_pSinglyIndexedElements->Texcoords.push_back(oZERO2);
				}
			}
			
			(*_pIndexMap)[hash] = resolvedIndices[p] = NewIndex;
		}
		
		else
			resolvedIndices[p] = (*it).second;
	}
	
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
	if (_Face.NumIndices == 4)
	{
		_pIndices->push_back(resolvedIndices[pOrder[3]]);
		_pIndices->push_back(resolvedIndices[pOrder[4]]);
		_pIndices->push_back(resolvedIndices[pOrder[5]]);
	}
}

struct oOBJImpl : oOBJ
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oOBJImpl(const char* _OBJPath, const char* _OBJString, const oOBJ_INIT& _Init, bool* _pSuccess);
	void GetDesc(oOBJ_DESC* _pDesc) const threadsafe override;

protected:
	void GetDesc(oOBJ_DESC* _pDesc) const;
	oOBJ_VERTEX_ELEMENTS VertexElements;
	std::vector<unsigned int> Indices;
	std::vector<oOBJ_GROUP> Groups;
	oStringPath OBJPath;
	oStringPath MTLPath;
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
	unsigned int NumGroups = 0;
	oOBJ_GROUP group;
	oOBJ_FACE face;

	// Scope memory usage... more might be used below when calculating normals, 
	// etc. so free this stuff up rather than leaving it all around.
	
	{
		void* pIndexAllocatorArena = malloc(kInitialReserve);
		oOnScopeExit FreeArena([&] { if (pIndexAllocatorArena) free(pIndexAllocatorArena); });
		index_map_t IndexMap(0, index_map_t::hasher(), index_map_t::key_equal(), std::less<key_t>(), allocator_type(pIndexAllocatorArena, kInitialReserve, &IndexMapMallocBytes));

		// OBJ files don't contain a same-sized vertex streams for each elements, and 
		// indexing occurs uniquely between vertex elements. Eventually we will create
		// same-sized vertex data - even by replication of data - so that a single 
		// index buffer can be used. That's the this->VertexElements, but first to get 
		// the raw vertex data off dish, use this.
		oOBJ_VERTEX_ELEMENTS OffDiskElements;
		OffDiskElements.Reserve(_Init.EstimatedNumVertices);

		if (!oOBJParseVertexElements(_OBJString, _Init.FlipHandedness, &OffDiskElements))
		{
			oErrorSetLast(oERROR_IO, "Error parsing vertex elements in %s", _OBJPath);
			return;
		}

		// Use position's indexing since most likely positions are all unique, and 
		// other attributes can be shared.
		VertexElements.Reserve(OffDiskElements.Positions.size());
		Indices.reserve(_Init.EstimatedNumIndices);

		const char* r = _OBJString;
		while (*r)
		{
			MovePastWhitespace(&r);
			switch (*r)
			{
				case 'f':
				{
					oOBJParseFLine(r, &face);
					AddFace(_Init, face, &IndexMap, OffDiskElements, &Indices, &VertexElements, &DegenerateNormals, &DegenerateTexcoords);
					break;
				}

				case 'g':
					// close out previous group
					if (NumGroups) { group.Range.NumTriangles = oUInt(Indices.size() / 3) - group.Range.StartTriangle; Groups.push_back(group); }
					NumGroups++;
					ParseString(group.GroupName, r);
					group.Range.StartTriangle = oUInt(Indices.size() / 3);
					break;
				case 'u':
					ParseString(group.MaterialName, r);
					break;
				case 'm':
					ParseString(MTLPath, r);
					break;
				default:
					break;
			}
			MoveToEndOfLine(&r);
			MovePastNewline(&r);
		}

	} // End of life for from-disk vertex elements and index hash

	#ifdef _DEBUG
		if (IndexMapMallocBytes)
		{
			oStringM reserved, additional;
			oTRACE("oOBJ: %s index map allocated %s additional indices beyond the initial EstimatedNumIndices=%s", oSAFESTRN(_OBJPath), oFormatCommas(additional, oUInt(IndexMapMallocBytes / sizeof(unsigned int))), oFormatCommas(reserved, _Init.EstimatedNumVertices));
		}
	#endif

	// close out a remaining group one last time
	if (NumGroups)
	{
		group.Range.NumTriangles = oUInt(Indices.size() / 3) - group.Range.StartTriangle;
		Groups.push_back(group);
	}

	else
	{
		group.GroupName = "Default Group";
		group.Range.StartTriangle = 0;
		group.Range.NumTriangles = oUInt(Indices.size() / 3);
		Groups.push_back(group);
	}

	// Go back through groups and calc min/max verts
	oFOR(oOBJ_GROUP& g, Groups)
		oCalculateMinMaxVertices(oGetData(Indices), g.Range.StartTriangle*3, g.Range.NumTriangles*3, oUInt(VertexElements.Positions.size()), &g.Range.MinVertex, &g.Range.MaxVertex);

	if (_Init.CalcNormalsOnError)
	{
		bool CalcNormals = false;
		if (VertexElements.Normals.empty())
		{
			oTRACE("oOBJ: No normals loaded from %s; calculating now...", _OBJPath);
			CalcNormals = true;
		}

		else if (!DegenerateNormals.empty())
		{
			// @oooii-tony: Is there a way to calculate only the degenerates?
			oTRACE("oOBJ: %u degenerate normals in %s; recalculating all normals now...", DegenerateNormals.size(), _OBJPath);
			CalcNormals = true;
		}

		if (CalcNormals)
		{
			VertexElements.Normals.resize(VertexElements.Positions.size());
			oStringS StrTime;
			oLocalTimer t;
			oCalculateVertexNormals(oGetData(VertexElements.Normals), oGetData(Indices), Indices.size(), oGetData(VertexElements.Positions), VertexElements.Positions.size(), _Init.CounterClockwiseFaces, true);
			oTRACE("oOBJ: %u normals calculated in %s for %s", VertexElements.Normals.size(), oFormatTimeSize(StrTime, t.Seconds(), true, true), oSAFESTRN(_OBJPath));
		}
	}

	// @oooii-tony: What to do in this case? Can they be recalculated?
	if (!DegenerateTexcoords.empty())
		oTRACE("oOBJ: There are %u degenerate/missing texcoords in %s", DegenerateTexcoords.size(), oSAFESTRN(_OBJPath));

	*_pSuccess = true;
}

void oOBJImpl::GetDesc(oOBJ_DESC* _pDesc) const
{
	_pDesc->OBJPath = OBJPath;
	_pDesc->MTLPath = MTLPath;
	_pDesc->pPositions = oGetData(VertexElements.Positions);
	_pDesc->pNormals = oGetData(VertexElements.Normals);
	_pDesc->pTexcoords = oGetData(VertexElements.Texcoords);
	_pDesc->pIndices = oGetData(Indices);
	_pDesc->pGroups = oGetData(Groups);
	_pDesc->NumVertices = oUInt(VertexElements.Positions.size());
	_pDesc->NumIndices = oUInt(Indices.size());
	_pDesc->NumGroups = oUInt(Groups.size());
}

void oOBJImpl::GetDesc(oOBJ_DESC* _pDesc) const threadsafe
{
	oInherentlyThreadsafe()->GetDesc(_pDesc); // safe because the object is const throughout its lifetime
}

bool oOBJCreate(const char* _OBJPath, const char* _OBJString, const oOBJ_INIT& _Init, threadsafe oOBJ** _ppOBJ)
{
	bool success = false;
	oCONSTRUCT(_ppOBJ, oOBJImpl(_OBJPath, _OBJString, _Init, &success));
	return success;
}

// @oooii-tony: NOTE: _MTLPath is not used yet, but leave this in case we need
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
		MovePastWhitespace(&r);
		switch (*r)
		{
			case 'n':
				r += 6; // "newmtl"
				MovePastWhitespace(&r);
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
				pColor = r[1] == 'a' ? (float3*)&_pMTLLibrary->back().AmbientColor : (r[1] == 'd' ? (float3*)&_pMTLLibrary->back().DiffuseColor : (float3*)&_pMTLLibrary->back().SpecularColor);
				sscanf_s(r+2, "%f %f %f", &pColor->x, &pColor->y, &pColor->z);
				break;
			case 'm':
				sscanf_s(r + strcspn(r, "\t "), "%[^\r|^\n]", buf, oCOUNTOF(buf)); // +7 skips "map_K? "
				if (0 == _memicmp("map_Ka", r, 6)) _pMTLLibrary->back().AmbientTexturePath = buf + 1; // Drop the whitespace
				else if (0 == _memicmp("map_Kd", r, 6)) _pMTLLibrary->back().DiffuseTexturePath = buf + 1; // Drop the whitespace
				else if (0 == _memicmp("map_Ks", r, 6)) _pMTLLibrary->back().SpecularTexturePath = buf + 1; // Drop the whitespace
				else if (0 == _memicmp("map_d", r, 5) || 0 == _memicmp("map_opacity", r, 11)) _pMTLLibrary->back().AlphaTexturePath = buf + 1; // Drop the whitespace
				else if (0 == _memicmp("map_Bump", r, 8) || 0 == _memicmp("bump", r, 4)) _pMTLLibrary->back().BumpTexturePath = buf + 1; // Drop the whitespace
				break;
			case 'i':
				r += 5; // "illum"
				MovePastWhitespace(&r);
				sscanf_s(r, "%d", &_pMTLLibrary->back().Illum);
				break;
			default:
				break;
		}
		MoveToEndOfLine(&r);
		MovePastNewline(&r);
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
	oRefCount RefCount;
};

oMTLImpl::oMTLImpl(const char* _MTLPath, const char* _MTLString, bool* _pSuccess)
{
	Materials.reserve(16);
	*_pSuccess = oMTLParse(_MTLPath, _MTLString, &Materials);
}

void oMTLImpl::GetDesc(oMTL_DESC* _pDesc) const threadsafe
{
	const std::vector<oOBJ_MATERIAL>& m = oInherentlyThreadsafe()->Materials; // safe because data is const throughout this object's lifetime

	_pDesc->pMaterials = oGetData(m);
	_pDesc->NumMaterials = oUInt(m.size());
}

bool oMTLCreate(const char* _MTLPath, const char* _MTLString, threadsafe oMTL** _ppMTL)
{
	bool success = false;
	oCONSTRUCT(_ppMTL, oMTLImpl(_MTLPath, _MTLString, &success));
	return success;
}
