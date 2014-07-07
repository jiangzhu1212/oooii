/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
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
