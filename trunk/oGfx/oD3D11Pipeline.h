// $(header)
#pragma once
#ifndef oD3D11Pipline_h
#define oD3D11Pipline_h

#include "oGfxCommon.h"
#include <oooii/oD3D11.h>

oDECLARE_GFXDEVICECHILD_IMPLEMENTATION(oD3D11, Pipeline)
{
	oDEFINE_GFXDEVICECHILD_INTERFACE();
	oDECLARE_GFXDEVICECHILD_CTOR(oD3D11, Pipeline);
	~oD3D11Pipeline();

	void GetDesc(DESC* _pDesc) const threadsafe override;

	oRef<ID3D11InputLayout> InputLayout;
	oRef<ID3D11VertexShader> VertexShader;
	oRef<ID3D11HullShader> HullShader;
	oRef<ID3D11DomainShader> DomainShader;
	oRef<ID3D11GeometryShader> GeometryShader;
	oRef<ID3D11PixelShader> PixelShader;

	oIAELEMENT* pElements;
	unsigned int NumElements;
};

#endif
