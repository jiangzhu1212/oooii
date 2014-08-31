// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Include interface to pass to D3DCompile
#pragma once
#ifndef oGPU_d3d_types_h
#define oGPU_d3d_types_h

struct ID3D11Device;
struct ID3D11DeviceChild;
struct ID3D11DeviceContext;
struct ID3D11CommandList;
struct ID3D11InfoQueue;
struct ID3D11UserDefinedAnnotation;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11GeometryShader;
struct ID3D11PixelShader;
struct ID3D11ComputeShader;
struct ID3D11Resource;
struct ID3D11Buffer;
struct ID3D11Texture1D;
struct ID3D11Texture2D;
struct ID3D11Texture3D;
struct ID3D11Query;
struct ID3D11Asynchronous;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11View;
struct IDXGISwapChain;

namespace ouro { namespace gpu { namespace d3d {

typedef ID3D11Device Device;
typedef ID3D11DeviceChild DeviceChild;
typedef ID3D11DeviceContext DeviceContext;
typedef ID3D11CommandList CommandList;
typedef ID3D11InfoQueue InfoQueue;
typedef ID3D11UserDefinedAnnotation UserDefinedAnnotation;
typedef ID3D11InputLayout InputLayout;
typedef ID3D11VertexShader VertexShader;
typedef ID3D11HullShader HullShader;
typedef ID3D11DomainShader DomainShader;
typedef ID3D11GeometryShader GeometryShader;
typedef ID3D11PixelShader PixelShader;
typedef ID3D11ComputeShader ComputeShader;
typedef ID3D11Resource Resource;
typedef ID3D11Buffer Buffer;
typedef ID3D11Texture1D Texture1D;
typedef ID3D11Texture2D Texture2D;
typedef ID3D11Texture3D Texture3D;
typedef ID3D11Query Query;
typedef ID3D11Asynchronous Asynchronous;
typedef ID3D11SamplerState SamplerState;
typedef ID3D11RasterizerState RasterizerState;
typedef ID3D11BlendState BlendState;
typedef ID3D11DepthStencilState DepthStencilState;
typedef ID3D11DepthStencilView DepthStencilView;
typedef ID3D11RenderTargetView RenderTargetView;
typedef ID3D11ShaderResourceView ShaderResourceView;
typedef ID3D11UnorderedAccessView UnorderedAccessView;
typedef ID3D11View View;
typedef IDXGISwapChain SwapChain;

}}}

#endif
