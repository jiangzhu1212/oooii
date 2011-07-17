// $(header)
#include <oGfx/oGfx.h>
#include <oooii/oWindows.h>

#include <GEVSByteCode.h>
#include <GEPSOpaqueByteCode.h>
#include <GEPSTestByteCode.h>
#include <GEPSBlendByteCode.h>
#include <LIPSNonShadowingByteCode.h>
#include <LIPSShadowingByteCode.h>
#include <MAPSOpaqueByteCode.h>
#include <MAPSTestByteCode.h>
#include <MAPSBlendByteCode.h>
#include <LNVSByteCode.h>
#include <LNPSByteCode.h>
#include <PIPSLineByteCode.h>
#include <PIPSOpaqueByteCode.h>
#include <PIPSTestByteCode.h>
#include <PIPSBlendByteCode.h>
#include <DBVSByteCode.h>
#include <DBGPSDepthByteCode.h>
#include <DBGPSNormalsByteCode.h>
#include <DBGPSNormalsXByteCode.h>
#include <DBGPSNormalsYByteCode.h>
#include <DBGPSNormalsZByteCode.h>
#include <DBGPSContinuityByteCode.h>
#include <DBGPSInstanceByteCode.h>
#include <DBGPSShininessByteCode.h>
#include <DBLPSDiffuseByteCode.h>
#include <DBLPSSpecularByteCode.h>

#define GEOMETRY_LAYOUT \
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, \
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, \
	{ "BASIS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, \
	{ "CONTINUITY", 0, DXGI_FORMAT_R8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }

static const D3D11_INPUT_ELEMENT_DESC sIAGeometry[] = 
{
	GEOMETRY_LAYOUT
};

static const D3D11_INPUT_ELEMENT_DESC sIALines[] = 
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

const oGfxPipeline::DESC* GetPipelineDescs()
{
	static const oGfxPipeline::DESC PipelineSource[] = 
	{
		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, GEPSOpaqueByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, GEPSTestByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, GEPSBlendByteCode },

		{ sIAGeometry, oCOUNTOF(sIAGeometry), LIVSByteCode, 0, 0, 0, LIPSNonShadowingByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), LIVSByteCode, 0, 0, 0, LIPSShadowingByteCode },

		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, MAPSOpaqueByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, MAPSTestByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, MAPSBlendByteCode },

		{ sIALines, oCOUNTOF(sIALines), LNVSByteCode, 0, 0, 0, LNPSByteCode },
			
		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, PIPSOpaqueByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, PIPSTestByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), GEVSByteCode, 0, 0, 0, PIPSBlendByteCode },

		{ sIALines, oCOUNTOF(sIALines), LNVSByteCode, 0, 0, 0, PIPSLineByteCode },
	};
	oSTATICASSERT(oCOUNTOF(PipelineSource) == oGfxScene::NUM_PIPELINE_STATES);
	
	return PipelineSource;
}

const oGfxPipeline::DESC* GetDebugPipelineDescs()
{
	static const oGfxPipeline::DESC DebugPipelineSource[] = 
	{
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBGPSDepthByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBGPSNormalsByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBGPSNormalsXByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBGPSNormalsYByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBGPSNormalsZByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBGPSContinuityByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBGPSInstanceByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBGPSShininessByteCode },

		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBLPSDiffuseByteCode },
		{ sIAGeometry, oCOUNTOF(sIAGeometry), DBVSByteCode, 0, 0, 0, DBLPSSpecularByteCode },
	};
	oSTATICSASERT(oCOUNTOF(DebugPipelineSource) == oGfxScene::NUM_DEBUG_VISUALIZATIONS);

	return DebugPipelineSource;
}
