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
#include <oooii/oOBJLoader.h>
#include <oooii/oAssert.h>
#include <oooii/oHash.h>
#include <unordered_map>
#include <oooii/oAtof.h>

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

inline void MovePastWhitespace(const char** _CurPos) { while(**_CurPos && gPastWhitespace[**_CurPos]) ++*_CurPos; }
inline void MoveToWhitespace(const char** _CurPos) { while(**_CurPos && !gToWhitespace[**_CurPos]) ++*_CurPos; }
inline void MoveToEndOfLine(const char** _CurPos) { while(**_CurPos && **_CurPos != '\n') ++*_CurPos; }

bool oOBJLoadMTL(const char* _MTLPath, const char* _MTLString, std::vector<oOBJ::MATERIAL>* _pMTLLibrary)
{
	_pMTLLibrary->clear();

	float3* pColor = 0;
	char type = 0;
	char buf[256];

	const char* r = _MTLString;
	while (*r)
	{
		MovePastWhitespace(&r);
		switch (*r)
		{
			case 'n':
				sscanf_s(r+7, "%[^\r|^\n]", buf, oCOUNTOF(buf)); // +7 is for "newmtl "
				_pMTLLibrary->resize(_pMTLLibrary->size() + 1);
				_pMTLLibrary->back().Name = buf;
				break;
			case 'N':
				type = *(++r);
				sscanf_s(++r, "%f", type == 's' ? &_pMTLLibrary->back().Specularity : &_pMTLLibrary->back().RefractionIndex);
				break;
			case 'd':
				sscanf_s(++r, "%f", &_pMTLLibrary->back().DiffuseColor.w);
				break;
			case 'K':
				if (r[1] == 'm') break; // I can't find Km documented anywhere.
				pColor = r[1] == 'a' ? &_pMTLLibrary->back().AmbientColor : (r[1] == 'd' ? (float3*)&_pMTLLibrary->back().DiffuseColor : &_pMTLLibrary->back().SpecularColor);
				sscanf_s(r+2, "%f %f %f", &pColor->x, &pColor->y, &pColor->z);
				break;
			case 'm':
				sscanf_s(r + strcspn(r, "\t "), "%[^\r|^\n]", buf, oCOUNTOF(buf)); // +7 skips "map_K? "
				if (0 == memcmp("map_Ka ", r, 6)) _pMTLLibrary->back().AmbientTexturePath = buf + 1; // Drop the whitespace
				else if (0 == memcmp("map_Kd ", r, 7)) _pMTLLibrary->back().DiffuseTexturePath = buf + 1; // Drop the whitespace
				else if (0 == memcmp("map_Ks ", r, 7)) _pMTLLibrary->back().SpecularTexturePath = buf + 1; // Drop the whitespace
				else if (0 == memcmp("map_d ", r, 6)) _pMTLLibrary->back().AlphaTexturePath = buf + 1; // Drop the whitespace
				else if (0 == memcmp("map_Bump ", r, 10)) _pMTLLibrary->back().AlphaTexturePath = buf + 1; // Drop the whitespace
				else oASSUME(0);
				break;
			default:
				break;
		}
		MoveToEndOfLine(&r);
		while(*r == '\n' || *r == '\r')
			r++;
	}

	return true;
}

inline void ParseString(std::string& str, const char* r)
{
	MoveToWhitespace(&r); MovePastWhitespace(&r);
	const char* start = r;
	MoveToEndOfLine(&r);
	str.assign(start, std::distance(start, r));
}

class ObjLoader
{
public:
	ObjLoader(const char* _OBJPath, const char* _OBJString, bool _FlipFaces, oOBJ* _pOBJ)
		: pOBJ(_pOBJ)
		, nDegenerateNormals(0)
	{
		_pOBJ->OBJPath = _OBJPath;

		float3		tempF3;
		FACEINDICES	fi;
		oOBJ::GROUP	g;

		bool	isQuad = false;
		int		majorIndexType = P;
		bool	parsedAGroup = false;

		const char* r = _OBJString;
		while (*r)
		{
			MovePastWhitespace(&r);
			switch (*r)
			{
			case 'v':
				ParseVLine(r);
				break;
			default:
				break;
			}
			MoveToEndOfLine(&r);
			while(*r == '\n' || *r == '\r')
				r++;
		}

		r = _OBJString;
		while (*r)
		{
			MovePastWhitespace(&r);
			switch (*r)
			{
			case 'f':
				{
					ParseFLine(&fi, r, &isQuad);
					ProcessFaces(&fi, majorIndexType, isQuad, _FlipFaces);
				}
				break;
				// @oooii-mike: no idea what this is for
				//case 's':
				//	sscanf_s(++r, "%u", &s);
				//	break;
			case 'g':
				if (parsedAGroup) { g.NumIndices = static_cast<unsigned int>(_pOBJ->Indices.size() - g.StartIndex); _pOBJ->Groups.push_back(g); }
				parsedAGroup = true;
				ParseString(g.GroupName, r);
				g.StartIndex = static_cast<unsigned int>(_pOBJ->Indices.size());
				break;
			case 'u':
				ParseString(g.MaterialName, r);
				break;
			case 'm':
				ParseString(_pOBJ->MaterialLibraryPath, r);
				break;
			default:
				break;
			}
			MoveToEndOfLine(&r);
			while(*r == '\n' || *r == '\r')
				r++;
		}

		if (parsedAGroup)
		{
			g.NumIndices = static_cast<unsigned int>(_pOBJ->Indices.size() - g.StartIndex);
			_pOBJ->Groups.push_back(g);
		}

		else
		{
			oOBJ::GROUP g;
			g.GroupName = "Default Group";
			g.StartIndex = 0;
			g.NumIndices = static_cast<unsigned int>(_pOBJ->Indices.size());
			_pOBJ->Groups.push_back(g);
		}

		if (nDegenerateNormals)
		{
			// @oooii-tony: Come back to this. For some reason doing this didn't work
			// right out of the box, so punt and say there are no normals for now...
			// (someone somewhere will probably recalc them if no normals are found)
			//oCalculateVertexNormals(oGetData(_pOBJ->Normals), oGetData(_pOBJ->Indices), _pOBJ->Indices.size(), oGetData(_pOBJ->Positions), _pOBJ->Positions.size(), !_FlipFaces, false);
			_pOBJ->Normals.clear();
		}
	}

private:
	static const unsigned int INVALID_INDEX = 0xFFFFFFFF;

	enum {
		P = 0,
		T,
		N
	};

	struct FACEINDICES
	{
		FACEINDICES()
		{
			memset(Index, INVALID_INDEX, sizeof(Index));
		}

		unsigned int Index[4][3];
	};

	inline void ParseVLine(const char* r)
	{
		float3 tempF3;
		float2 tempF2;

		r++;
		switch(*r)
		{
		case ' ':
			oAtof(r, &tempF3.x); MovePastWhitespace(&r); MoveToWhitespace(&r);
			oAtof(r, &tempF3.y); MovePastWhitespace(&r); MoveToWhitespace(&r);
			oAtof(r, &tempF3.z);
			tempPositions.push_back(tempF3);
			break;
		case 't':
			MoveToWhitespace(&r);
			oAtof(r, &tempF2.x); MovePastWhitespace(&r); MoveToWhitespace(&r);
			oAtof(r, &tempF2.y);
			tempTexCoords.push_back(tempF2);
			break;
		case 'n':
			MoveToWhitespace(&r);
			oAtof(r, &tempF3.x); MovePastWhitespace(&r); MoveToWhitespace(&r);
			oAtof(r, &tempF3.y); MovePastWhitespace(&r); MoveToWhitespace(&r);
			oAtof(r, &tempF3.z);
			tempNormals.push_back(tempF3);
			break;
		}
	}

	inline void ParseFLine(FACEINDICES* _pFace, const char* r, bool* pIsQuad)
	{
		MoveToWhitespace(&r);
		MovePastWhitespace(&r);

		int p = 0;
		while(p < 4 && *r != 0 && !gNewline[*r])
		{
			bool foundSlash = false;

			int indexType = P;
			do
			{
				_pFace->Index[p][indexType] = atoi(r) - 1;

				if(indexType == 2)
					break;

				while(*r != '/' && *r != ' ' && *r != 0 && *r != '\n')
					r++;

				while(*r == '/')
				{
					r++;
					indexType++;
					foundSlash = true;
				}
			} while(foundSlash);

			MoveToWhitespace(&r);
			MovePastWhitespace(&r);
			p++;
		}

		*pIsQuad = (p == 4);
	}

	inline float2 GetDegenerateTexCoord()
	{
		return float2(0.0f,0.0f);
	}

	inline float3 GetDegenerateNormal()
	{
		nDegenerateNormals++;
		return float3(0.0f,0.0f,0.0f);
	}

	inline void ProcessFaces(FACEINDICES* _pFace, int majorIndexType, bool isQuad, bool _FlipFaces)
	{
		unsigned int indices[4];

		for(int p = 0; p < (isQuad ? 4 : 3); p++)
		{
			unsigned int pIndex = _pFace->Index[p][P];
			unsigned int tIndex = _pFace->Index[p][T];
			unsigned int nIndex = _pFace->Index[p][N];
			uint64 hash = FaceToHash(pIndex, tIndex, nIndex);

			tVertIndexMap::iterator it = vertIndexMap.find(hash);
			if(it == vertIndexMap.end())
			{
				pOBJ->Positions.push_back(tempPositions[_pFace->Index[p][P]]);
				if(!tempTexCoords.empty())
				{
					if(_pFace->Index[p][T] != INVALID_INDEX)
						pOBJ->Texcoords.push_back(tempTexCoords[_pFace->Index[p][T]]);
					else
						pOBJ->Texcoords.push_back(GetDegenerateTexCoord());
				}
				if(!tempNormals.empty())
				{
					if(_pFace->Index[p][N] != INVALID_INDEX)
						pOBJ->Normals.push_back(tempNormals[_pFace->Index[p][N]]);
					else
						pOBJ->Normals.push_back(GetDegenerateNormal());
				}

				indices[p] = (unsigned int)(pOBJ->Positions.size() - 1);

				vertIndexMap[hash] = indices[p];
			}
			else
			{
				indices[p] = (*it).second;
			}
		}

		static const unsigned int CCWindices[6] = { 0, 2, 1, 2, 0, 3, };
		static const unsigned int CWindices[6] = { 0, 1, 2, 2, 3, 0, };
		const unsigned int* pOrder = _FlipFaces ? CWindices : CCWindices;

		pOBJ->Indices.push_back(indices[pOrder[0]]);
		pOBJ->Indices.push_back(indices[pOrder[1]]);
		pOBJ->Indices.push_back(indices[pOrder[2]]);

		if(isQuad)
		{
			pOBJ->Indices.push_back(indices[pOrder[3]]);
			pOBJ->Indices.push_back(indices[pOrder[4]]);
			pOBJ->Indices.push_back(indices[pOrder[5]]);
		}
	}

	typedef unsigned __int64 uint64;
	typedef std::map<uint64, unsigned int> tVertIndexMap; // @oooii-mike: For some unknown reason, unordered_map takes *forever* to clear, so this is much faster with a standard map.

	inline uint64 FaceToHash(unsigned int p, unsigned int t, unsigned int n)
	{
		return p + tempPositions.size() * t + tempPositions.size() * tempTexCoords.size() * n;
	}

	oOBJ* pOBJ;

	tVertIndexMap vertIndexMap;
	std::vector<float3> tempPositions;
	std::vector<float3> tempNormals;
	std::vector<float2> tempTexCoords;

	size_t nDegenerateNormals;
};

bool oOBJLoad(const char* _OBJPath, const char* _OBJString, bool _FlipFaces, oOBJ* _pOBJ)
{
	ObjLoader(_OBJPath, _OBJString, _FlipFaces, _pOBJ);

	oTRACE("OBJ %s loaded %u verts, %u tris", oSAFESTRN(_OBJPath), _pOBJ->Positions.size(), _pOBJ->Indices.size() / 3);
	return true;
}
