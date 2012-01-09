// $(header)
// This header contains utility for working with vertex meshes, most often in a 
// tools/recalcuation/synthesis capacity.
#pragma once
#ifndef oMeshUtil_h
#define oMeshUtil_h

#include <oBasis/oMathTypes.h>
#include <oBasis/oPlatformFeatures.h>

void oTransformPoints(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumPoints);
void oTransformVectors(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumVectors);

// Removes indices for degenerate triangles. After calling this function, use
// oPruneUnindexedVertices() to clean up extra vertices.
// _pPositions: list of XYZ positions indexed by the index array
// _NumberOfVertices: The number of vertices in the _pPositions array
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pNewNumIndices: The new number of indices as a result of removed degenerages
template<typename T> bool oRemoveDegenerates(const TVEC3<T>* _pPositions, size_t _NumberOfPositions, unsigned int* _pIndices, size_t _NumberOfIndices, size_t* _pNewNumIndices);

// Calculates the face normals from the following inputs:
// _pFaceNormals: output, array to fill with normals. This should be at least as
//                large as the number of faces in the specified mesh (_NumberOfIndices/3)
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index 
//              array
// _NumberOfPositions: The number of vertices in the _pPositions array
// _CCW: If true, triangles are assumed to have their front-face be specified by
//       the counter-clockwise order of vertices in a triangle. This affects 
//       which way a normal points.
//
// This can return EINVAL if a parameters isn't something that can be used.
template<typename T> bool oCalculateFaceNormals(TVEC3<T>* _pFaceNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfPositions, bool _CCW = false);

// Calculates the vertex normals by averaging face normals from the following 
// inputs:
// _pVertexNormals: output, array to fill with normals. This should be at least 
//                  as large as the number of vertices in the specified mesh
//                  (_NumberOfVertices).
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index 
//              array
// _NumberOfPositions: The number of vertices in the _pPositions array
// _CCW: If true, triangles are assumed to have their front-face be specified by
//       the counter-clockwise order of vertices in a triangle. This affects 
//       which way a normal points.
// _OverwriteAll: Overwrites any pre-existing data in the array. If this is 
// false, any zero-length vector will be overwritten. Any length-having vector
// will not be touched.
// This can return EINVAL if a parameters isn't something that can be used.
template<typename T> bool oCalculateVertexNormals(TVEC3<T>* _pVertexNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfPositions, bool _CCW = false, bool _OverwriteAll = true);

// Calculates the tangent space vector and its handedness from the following
// inputs:
// _pTangents: output, array to fill with tangents. This should be at least 
//             as large as the number of certices in the specified mesh 
//             (_NumberOfVertices)
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index 
//              array
// _pNormals: list of normalized normals for the mesh that are indexed by the 
//            index array
// _pTexcoords: list of texture coordinates for the mesh that are indexed by the 
//              index array
// _NumberOfVertices: The number of vertices in the _pPositions, _pNormals, and 
//                    _pTexCoords arrays
void oCalculateTangents(float4* _pTangents, const unsigned int* _pIndices, size_t _NumberOfIndices, const float3* _pPositions, const float3* _pNormals, const float2* _pTexcoords, size_t _NumberOfVertices);

// Allocates and fills an edge list for the mesh described by the specified 
// indices:
// _NumberOfVertices: The number of vertices the index array indexes
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _ppEdges: a pointer to receive an allocation and be filled with index pairs 
//           describing an edge. Use oFreeEdgeList() to free memory the edge 
//           list allocation. So every two uints in *_ppEdges represents an edge.
// _pNumberOfEdges: a pointer to receive the number of edge pairs returned
void oCalculateEdges(size_t _NumberOfVertices, const unsigned int* _pIndices, size_t _NumberOfIndices, unsigned int** _ppEdges, size_t* _pNumberOfEdges);
void oFreeEdgeList(unsigned int* _pEdges);

// If some process modified indices, go through and compact the specified vertex
// streams. 
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// pointers to attributes: any can be NULL if not available
// _NumberOfVertices: The number of vertices the index array indexes
// _pNewNumVertices: receives the new count of each of the specified streams of 
// data
//
// All attribute streams will be modified and _pIndices values will be updated
// to reflect new vertices.
template<typename T> void oPruneUnindexedVertices(unsigned int* _pIndices, size_t _NumberOfIndices, TVEC3<T>* _pPositions, TVEC3<T>* _pNormals, TVEC4<T>* _pTangents, TVEC2<T>* _pTexcoords0, TVEC2<T>* _pTexcoords1, unsigned int* _pColors, size_t _NumberOfVertices, size_t *_pNewNumVertices);

// Given an array of points, compute the minimize and maximum axis-aligned 
// corners of the set. (Useful for calculating corners of an axis-aligned 
// bounding box.
template<typename T> void oCalculateMinMaxPoints(const TVEC3<T>* oRESTRICT _pPoints, size_t _NumberOfPoints, TVEC3<T>* oRESTRICT _pMinPoint, TVEC3<T>* oRESTRICT _pMaxPoint);

#endif
