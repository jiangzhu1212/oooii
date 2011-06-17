// $(header)
#include <oooii/oOBJLoader.h>
#include <oooii/oAssert.h>
#include <oooii/oHash.h>
#include <unordered_map>

struct TOPOLOGY
{
	unsigned int NumPositions;
	unsigned int NumNormals;
	unsigned int NumTexcoords;
	unsigned int NumFaces; // # quads or tris both count as 1 face in this count
	unsigned int NumGroups;
};

struct VERTEX { int P, N, T; };
struct FACE { VERTEX v[4]; };

struct VERTEX_STORE
{
	std::vector<float3>* P;
	std::vector<float3>* N;
	std::vector<float2>* T;
};

inline bool operator==(const VERTEX& v0, const VERTEX& v1) { return v0.P == v1.P && v0.N == v1.N && v0.T == v1.T; }
struct hash_vertex { size_t operator()(const VERTEX& _Vertex) const { return oHash_superfast(&_Vertex, sizeof(_Vertex)); } };
typedef std::tr1::unordered_map<VERTEX, unsigned int, hash_vertex> vertex_hash;

inline void MovePastWhitespace(const char** _CurPos) { *_CurPos += strspn(*_CurPos, " \t\n\r\v"); }
inline void MoveToEndOfLine(const char** _CurPos) { *_CurPos += strcspn(*_CurPos, "\n"); }

inline void ParseVertexAttributeLine(const char* _BeginningOfVertexAttributeLine, VERTEX_STORE* _pVertexStore)
{
	float3 v;
	sscanf_s(_BeginningOfVertexAttributeLine, "v%*[nt ] %f %f %f\n", &v.x, &v.y, &v.z);
	switch (_BeginningOfVertexAttributeLine[1])
	{
		case ' ': _pVertexStore->P->push_back(v); break;
		case 'n': _pVertexStore->N->push_back(v); break;
		case 't': _pVertexStore->T->push_back(v.XY()); break;
		default: oASSUME(0);
	}
}

inline unsigned int ParseFaceLine(const char* _BeginningOfFaceLine, FACE* _pFace)
{
	bool isQuad;
	char buf[256];
	sscanf_s(_BeginningOfFaceLine+1, "%[^\n]", buf, oCOUNTOF(buf));
	memset(_pFace, 0, sizeof(FACE));
	if (strstr(buf, "//"))
		isQuad = 8 == sscanf_s(buf, "%d//%d %d//%d %d//%d %d//%d", &_pFace->v[0].P, &_pFace->v[0].N, &_pFace->v[1].P, &_pFace->v[1].N, &_pFace->v[2].P, &_pFace->v[2].N, &_pFace->v[3].P, &_pFace->v[3].N);
	else if (strstr(buf, "/"))
	{
		size_t nEntries = sscanf_s(buf, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &_pFace->v[0].P, &_pFace->v[0].T, &_pFace->v[0].N, &_pFace->v[1].P, &_pFace->v[1].T, &_pFace->v[1].N, &_pFace->v[2].P, &_pFace->v[2].T, &_pFace->v[2].N, &_pFace->v[3].P, &_pFace->v[3].T, &_pFace->v[3].N);
		if (nEntries != 9 && nEntries != 12)
			isQuad = 8 == sscanf_s(buf, "%d/%d %d/%d %d/%d %d/%d", &_pFace->v[0].P, &_pFace->v[0].T, &_pFace->v[1].P, &_pFace->v[1].T, &_pFace->v[2].P, &_pFace->v[2].T, &_pFace->v[3].P, &_pFace->v[3].T);
		else
			isQuad = 12 == nEntries;
	}
	else
		isQuad = 4 == sscanf_s(buf, "%d %d %d %d", &_pFace->v[0].P, &_pFace->v[1].P, &_pFace->v[2].P, &_pFace->v[3].P);

	return isQuad;
}

inline void AddFaceVerticesAndUpdateIndices(FACE& _Face, unsigned int _IsQuads, const VERTEX_STORE* _pFileVertices, VERTEX_STORE* _pUniqueVertices, vertex_hash* _pVHash, size_t* _pNumDegenerateNormals)
{
	std::vector<float3>& Ps = *_pFileVertices->P;
	std::vector<float3>& Ns = *_pFileVertices->N;
	std::vector<float2>& Ts = *_pFileVertices->T;
	std::vector<float3>& Pd = *_pUniqueVertices->P;
	std::vector<float3>& Nd = *_pUniqueVertices->N;
	std::vector<float2>& Td = *_pUniqueVertices->T;

	const unsigned int vertCount = _IsQuads ? 4 : 3;
	for (size_t i = 0; i < vertCount; i++)
	{
		VERTEX& v = _Face.v[i];

		// Negative indices are from the back of the list so far, so adjust this 
		// before hashing so that forward and backward faces that arrive at the same
		// place will evaluate to the same in the hash.
		if (v.P < 0) v.P = static_cast<int>(Ps.size()) + v.P + 1;
		if (v.N < 0) v.N = static_cast<int>(Ns.size()) + v.N + 1;
		if (v.T < 0) v.T = static_cast<int>(Ts.size()) + v.T + 1;
	
		vertex_hash::iterator it = _pVHash->find(v);
		if (_pVHash->end() == it)
		{
			// @oooii-tony: NOTE: OBJ Files don't require ALL faces to have the same
			// format, meaning some faces may have normals for example, and some 
			// don't. This makes indexing very difficult because you don't want to 
			// add normals we don't have

			_pVHash->insert(vertex_hash::value_type(v, static_cast<unsigned int>(Pd.size())+1));
			Pd.push_back(Ps[v.P-1]);

			// To preserve indexing, if there is no normal for a face push a degenerate
			// in its place for now and recalculate later.

			if (v.N)
				Nd.push_back(Ns[v.N-1]);

			else
			{
				(*_pNumDegenerateNormals)++;
				Nd.push_back(float3(0.0f, 0.0f, 0.0f));
			}

			Td.push_back(v.T ? Ts[v.T-1] : float2(0.0f, 0.0f));

			v.P = static_cast<int>(Pd.size());
			v.T = v.N = v.P;
		}

		else
		{
			v.P = it->second;
		}
	}
}

inline void AddFaceIndices(FACE& _Face, unsigned int _IsQuad, bool _FlipFaces, std::vector<unsigned int>* _pIndices)
{
	std::vector<unsigned int>& I = *_pIndices;

	if (_IsQuad)
	{
		// build 2 tris for the quad
		static const unsigned int CCWindices[6] = { 0, 2, 1, 2, 0, 3, };
		static const unsigned int CWindices[6] = { 0, 1, 2, 2, 3, 0, };
		const unsigned int* indices = _FlipFaces ? CWindices : CCWindices;
		for (size_t i = 0; i < 6; i++)
			I.push_back(_Face.v[indices[i]].P - 1);
	}

	else
	{
		size_t baseIndex = _pIndices->size();
		for (size_t i = 0; i < 3; i++)
			I.push_back(_Face.v[i].P - 1);
		
		if (!_FlipFaces)
			std::swap(I[baseIndex], I[baseIndex+1]);
	}
}

void ScanTopology(const char* _OBJString, TOPOLOGY* _pOBJTopology)
{
	unsigned int& p = _pOBJTopology->NumPositions;
	unsigned int& n = _pOBJTopology->NumNormals;
	unsigned int& t = _pOBJTopology->NumTexcoords;
	unsigned int& f = _pOBJTopology->NumFaces;
	unsigned int& g = _pOBJTopology->NumGroups;
	p = n = t = f = g = 0;
	const char* r = _OBJString;
	while (*r)
	{
		MovePastWhitespace(&r);
		switch (*r)
		{
			case 'v':
				switch (r[1])
				{
					case ' ': p++; break;
					case 'n': n++; break;
					case 't': t++; break;
					default: oASSUME(0);
				}
				break;
			case 'f': f += 2; break;
			case 'g': g++; break;
			default: break;
		}
		MoveToEndOfLine(&r);
	}
}

bool oOBJLoad(const char* _OBJPath, const char* _OBJString, bool _FlipFaces, oOBJ* _pOBJ)
{
	TOPOLOGY ot;
	ScanTopology(_OBJString, &ot);

	if (!ot.NumPositions)
	{
		oSetLastError(EINVAL, "No positions found");
		return false;
	}

	std::vector<float3> P, N;
	std::vector<float2> T;
	VERTEX_STORE VSFile = { &P, &N, &T }, VSUnique = { &_pOBJ->Positions, &_pOBJ->Normals, &_pOBJ->Texcoords };
	vertex_hash vhash;
	size_t nDegenerateNormals = 0;

	_pOBJ->Positions.clear(); _pOBJ->Positions.reserve(ot.NumPositions);
	_pOBJ->Normals.clear(); _pOBJ->Normals.reserve(ot.NumNormals);
	_pOBJ->Texcoords.clear(); _pOBJ->Texcoords.reserve(ot.NumTexcoords);
	_pOBJ->Indices.clear(); _pOBJ->Indices.reserve(ot.NumFaces * 3);
	_pOBJ->Groups.clear(); _pOBJ->Groups.reserve(ot.NumGroups);

	P.reserve(ot.NumPositions); N.reserve(ot.NumNormals); T.reserve(ot.NumTexcoords);

	_pOBJ->OBJPath = _OBJPath;

	oOBJ::GROUP g;
	FACE f;
	unsigned int s = 0, isQuad = 0;
	char buf[512];
	bool parsedAGroup = false;

	const char* r = _OBJString;
	while (*r)
	{
		MovePastWhitespace(&r);
		switch (*r)
		{
			case 'v':
				ParseVertexAttributeLine(r, &VSFile);
				break;
			case 'f':
				isQuad = ParseFaceLine(r, &f);
				AddFaceVerticesAndUpdateIndices(f, !!isQuad, &VSFile, &VSUnique, &vhash, &nDegenerateNormals);
				AddFaceIndices(f, !!isQuad, _FlipFaces, &_pOBJ->Indices);
				break;
			case 's':
				sscanf_s(++r, "%u", &s);
				break;
			case 'g':
				if (parsedAGroup) { g.NumIndices = static_cast<unsigned int>(_pOBJ->Indices.size() - g.StartIndex); _pOBJ->Groups.push_back(g); }
				parsedAGroup = true;
				++r;
				MovePastWhitespace(&r);
				sscanf_s(r, "%[^\n]", buf, oCOUNTOF(buf));
				g.GroupName = buf;
				g.StartIndex = static_cast<unsigned int>(_pOBJ->Indices.size());
				break;
			case 'u':
				r += 6; // move past "usemtl"
				MovePastWhitespace(&r);
				sscanf_s(r, "%[^\n]", buf, oCOUNTOF(buf));
				g.MaterialName = buf;
				break;
			case 'm':
				r += 6; // move past "mtllib"
				MovePastWhitespace(&r);
				sscanf_s(r, "%[^\n]", buf, oCOUNTOF(buf));
				_pOBJ->MaterialLibraryPath = buf;
			default:
				break;
		}
		MoveToEndOfLine(&r);
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

	oTRACE("OBJ %s loaded %u verts, %u tris", oSAFESTRN(_OBJPath), _pOBJ->Positions.size(), _pOBJ->Indices.size() / 3);
	return true;
}

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
	}

	return true;
}