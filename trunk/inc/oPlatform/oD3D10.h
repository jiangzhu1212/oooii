// $(header)

// Soft-link D3D10 and some utility functions to address common challenges in 
// D3D10.
#pragma once
#ifndef oD3D10_h
#define oD3D10_h

#include <oBasis/oRef.h>
#include <oBasis/oMathTypes.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oWindows.h>
#include <vector>

struct oD3D10 : oModuleSingleton<oD3D10>
{
	oD3D10();
	~oD3D10();

	HRESULT (__stdcall *D3D10CreateDevice1)( IDXGIAdapter *pAdapter, D3D10_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, D3D10_FEATURE_LEVEL1 HardwareLevel, UINT SDKVersion, ID3D10Device1 **ppDevice);

protected:
	oHMODULE hD3D10;
};

// Resolves the specified position to an adapter and creates an ID3D10Device
// interface from that adapter. If 10.1 isn't supported, a 10.0 device is 
// returned.
bool oD3D10CreateDevice(const int2& _VirtualDesktopPosition, ID3D10Device1** _ppDevice);

bool oD3D10CreateSnapshot(ID3D10Texture2D* _pRenderTarget, interface oImage** _ppImage);

interface oD3D10DeviceManager : oInterface
{
	// oD3D10DeviceManager makes use of the fact that on most any sane system
	// the number of IDXGIAdapters (physical GPUs) remains constant, but the
	// number of Monitors can change.
	struct DESC
	{
		DESC()
			: DeviceFlags(0)
		{}
		unsigned int DeviceFlags;
	};

	virtual bool EnumDevices(size_t n, ID3D10Device1** _ppDevice ) = 0;
	virtual bool GetDevice(const RECT& _Rect, ID3D10Device1** _ppDevice) = 0;
};

oAPI bool oD3D10DeviceManagerCreate(const oD3D10DeviceManager::DESC& _Desc, oD3D10DeviceManager** _ppDeviceManager);

struct oD3D10ShaderState
{
	struct STATE
	{
		oRef<ID3D10InputLayout> pInputLayout;
		oRef<ID3D10VertexShader> pVertexShader;
		oRef<ID3D10GeometryShader> pGeometryShader;
		oRef<ID3D10PixelShader> pPixelShader;

		void SetState(ID3D10Device* _pDevice);
	};

	static void CreateShaders(ID3D10Device* _pDevice
		, oD3D10ShaderState::STATE* _pState
		, const BYTE* _pByteCodeVS, size_t _SizeofByteCodeVS
		, const BYTE* _pByteCodeGS, size_t _SizeofByteCodeGS
		, const BYTE* _pByteCodePS, size_t _SizeofByteCodePS);

	// Registers a state so it can be set using SetState()
	void RegisterState(unsigned int _Index, STATE& _State);

	void SetState(ID3D10Device* _pDevice, unsigned int _Index);

protected:
	std::vector<STATE> States;
};

struct oD3D10FullscreenQuad
{
	void Create(ID3D10Device* _pDevice, const BYTE* _pPixelShaderByteCode, size_t _szBiteCode);
	void Draw(ID3D10Device* _pDevice);

protected:
	void CreateStates(ID3D10Device* _pDevice);
	void SetStates(ID3D10Device* _pDevice);

	oD3D10ShaderState::STATE ShaderState;
	oRef<ID3D10RasterizerState> RasterState;
	oRef<ID3D10DepthStencilState> DepthState;
	oRef<ID3D10BlendState> BlendState;
};

struct oD3D10ScreenQuad : private oD3D10FullscreenQuad
{
	void Create(ID3D10Device* _pDevice, const BYTE* _pPixelShaderByteCode, size_t _szBiteCode);
	void Draw(ID3D10Device* _pDevice, oRECTF _Destination, oRECTF _Source);

private:
	struct VSIN
	{
		float2 Position;
		float2 Texcoord;
	};

	oRef<ID3D10InputLayout> VertexLayout;
	oRef<ID3D10Buffer> VertexBuffer;
};

#endif
