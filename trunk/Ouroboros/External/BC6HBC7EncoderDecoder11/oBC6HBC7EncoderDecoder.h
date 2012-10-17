#pragma once
#ifndef oBC6HBC7EncoderDecoder_h
#define oBC6HBC7EncoderDecoder_h

// These functions wrap the BC6HBC7EncoderDecoder sample as found in the 
// June 2010 DirectX SDK with a Microsoft patch applied to it. All to-file 
// operations have been redirected to create a new texture in 
// _ppCompressedTexture/_ppUncompressedTexture filled with the results. This is 
// the common function. Inlines are provided that basically pass flags to this 
// function.
bool oD3D11EncodeBC7(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11EncodeBC6HS(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11EncodeBC6HU(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppCompressedTexture);
bool oD3D11DecodeBC6orBC7(ID3D11Texture2D* _pSourceTexture, bool _UseGPU, ID3D11Texture2D** _ppUncompressedTexture);

static const DXGI_FORMAT oD3D11BC7RequiredSourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
static const DXGI_FORMAT oD3D11BC6HRequiredSourceFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

#endif
