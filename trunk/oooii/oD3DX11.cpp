// $(header)
#include <oooii/oD3DX11.h>
#include <oooii/oWindows.h>

static const char* d3dx11_dll_functions[] = 
{
	"D3DX11CreateTextureFromMemory",
	"D3DX11LoadTextureFromTexture",
};

oD3DX11::oD3DX11()
{
	hD3DX11 = oModule::Link("d3dx11_43.dll", d3dx11_dll_functions, (void**)&D3DX11CreateTextureFromMemory, oCOUNTOF(d3dx11_dll_functions));
	oASSERT(hD3DX11, "");
}

oD3DX11::~oD3DX11()
{
	oModule::Unlink(hD3DX11);
}
