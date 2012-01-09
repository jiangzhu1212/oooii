// $(header)
#include <oPlatform/oHLSL.h>

void main(in uint vertexID : SV_VertexID, out float4 position : SV_Position, out float2 texcoord : TEXCOORD0)
{
	oExtractQuadInfoFromVertexID(vertexID, position, texcoord);
}