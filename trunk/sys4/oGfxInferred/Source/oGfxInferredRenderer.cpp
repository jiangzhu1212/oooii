// $(header)
#include <oGfx/oGfx.h>

const oGfxPipeline::DESC* GetPipelineDescs();
const oGfxPipeline::DESC* GetDebugPipelineDescs();

struct oColor2 : public oColor
{
	oColor2(unsigned int _R, unsigned int _G, unsigned int _B, unsigned int _A)
		: oColor(oComposeColor(_R, _G, _B, _A))
	{}
};

bool CreateGBuffer(threadsafe oGfxDevice* _pDevice, oGfxRenderTarget2** _ppGBuffer)
{
	static const oSurface::FORMAT RTFormats[] = 
	{
		oSurface::R16G16_FLOAT,
		oSurface::R8G8B8A8_UINT,
	};

	static const oColor ClearColors[] = 
	{
		std::Black,
		oColor2(255, 255, 0, 0),
	};

	oGfxRenderTarget2::DESC desc;
	desc.Width = 0;
	desc.Height = 0;
	desc.MRTCount = oCOUNTOF(RTFormats);
	desc.ArraySize = 1;
	memcpy(desc.Format, RTFormats, sizeof(RTFormats));
	desc.DepthStencilFormat = oSurface::R32_TYPELESS;
	memcpy(desc.ClearDesc, ClearColors, sizeof(ClearColors));
	desc.GenerateMips = false;

	return _pDevice->CreateRenderTarget2("GBuffer", desc, _ppGBuffer);
}

bool CreateLBuffer(threadsafe oGfxDevice* _pDevice, oGfxRenderTarget2** _ppLBuffer)
{
	static const oSurface::FORMAT RTFormats[] = 
	{
		oSurface::R16G16B16A16_FLOAT,
	};

	static const oColor ClearColors[] = 
	{
		std::Black,
	};

	oGfxRenderTarget2::DESC desc;
	desc.Width = 0;
	desc.Height = 0;
	desc.MRTCount = oCOUNTOF(RTFormats);
	desc.ArraySize = 1;
	memcpy(desc.Format, RTFormats, sizeof(RTFormats));
	desc.DepthStencilFormat = oSurface::UNKNOWN;
	memcpy(desc.ClearDesc, ClearColors, sizeof(ClearColors));
	desc.GenerateMips = false;

	return _pDevice->CreateRenderTarget2("LBuffer", desc, _ppLBuffer);
}

bool CreateMBuffer(threadsafe oGfxDevice* _pDevice, threadsafe oWindow* _pWindow, oGfxRenderTarget2** _ppMBuffer)
{
	return _pDevice->CreateRenderTarget2("MBuffer", _pWindow, _ppMBuffer);
}

bool CreatePipelines(threadsafe oGfxDevice* _pDevice, oRef<oGfxPipeline> _Pipelines[oGfxScene::NUM_PIPELINE_STATES])
{
	for (size_t i = 0; i < oCOUNTOF(_Pipelines); i++)
		oVERIFY(_pDevice->CreatePipeline(oAsString((oGfxScene::PIPELINE_STATE)i), GetPipelineDescs()[i], &_Pipelines[i]));

	return true;
}

bool CreateDebugPipelines(threadsafe oGfxDevice* _pDevice, oRef<oGfxPipeline> _DebugPipelines[oGfxScene::NUM_DEBUG_VISUALIZATIONS])
{
	for (size_t i = 0; i < oCOUNTOF(_DebugPipelines); i++)
		oVERIFY(_pDevice->CreatePipeline(oAsString((oGfxScene::DEBUG_VISUALIZATION)i), GetDebugPipelineDescs()[i], &_DebugPipelines[i]));

	return true;
}
