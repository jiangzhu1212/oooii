// $(header)
#include "oD3D11LineList.h"
#include "oD3D11Device.h"

oDEFINE_GFXDEVICE_CREATE(oD3D11, LineList);
oBEGIN_DEFINE_GFXRESOURCE_CTOR(oD3D11, LineList)
{
	Desc.MaxNumLines = 0;
	Desc.NumLines = _Desc.NumLines;
	Resize(_Desc.MaxNumLines);
	*_pSuccess = true;
}

void oD3D11LineList::Resize(uint _MaxNumLines)
{
	oD3D11DEVICE();

	if (!_MaxNumLines)
		Lines = 0;
	else if (_MaxNumLines != Desc.MaxNumLines)
	{
		oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, Name, true, 0, _MaxNumLines, sizeof(LINE), &Lines));
		Desc.MaxNumLines = _MaxNumLines;
	}
}

void oD3D11LineList::SetNumLines(uint _NumLines)
{
	Desc.NumLines = _NumLines;
}
