/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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

#include <oPlatform/oWindow.h>
#include <oPlatform/oWindows.h>
#include <oPlatform/oWindowUI.h>
#include <oPlatform/oD3D11.h>

#include <oGfx/oGfx.h>

#include "oGfxTestPipeline.h"

class oRenderTest
{
public:

	oRenderTest()
	{
		oGfxDevice::DESC DevDesc;
		DevDesc.Version = oVersion(10,0);
		DevDesc.UseSoftwareEmulation = false;
		DevDesc.EnableDebugReporting = true;
		oVERIFY(oGfxDeviceCreate(DevDesc, &GfxDevice));

		oRef<ID3D11Device> D3D11Device;
		oVERIFY(GfxDevice->QueryInterface((const oGUID&)__uuidof(ID3D11Device), &D3D11Device));

		oWindow::DESC WinDesc;
		WinDesc.Style = oWindow::SIZEABLE;
		WinDesc.AutoClear = false;
		WinDesc.EnableUIDrawing = true;
		WinDesc.UseAntialiasing = false;
		WinDesc.AllowUserFullscreenToggle = true;
		oVERIFY(oWindowCreate(WinDesc, D3D11Device, oWindow::USE_GDI, &Window));
		Window->SetTitle("User-Specified D3D11 Test");

		oGfxCommandList::DESC CmdListDesc;
		CmdListDesc.DrawOrder = 0;

		oVERIFY(GfxDevice->CreateCommandList("Test CommandList", CmdListDesc, &GfxCommandList));

		oGfxPipeline::DESC PLDesc;
		oVERIFY(oD3D11GetPipelineDesc(oGFX_FOWARD_COLOR, &PLDesc));
		oVERIFY(GfxDevice->CreatePipeline("oGfxTest.Forward.Color", PLDesc, &PLForwardColor));

		oRef<oGeometryFactory> GeoFactory;
		oVERIFY(oGeometryFactoryCreate(&GeoFactory));

		oGeometry::LAYOUT BoxLayout;
		memset(&BoxLayout, 0, sizeof(oGeometry::LAYOUT));
		BoxLayout.Positions = true;
		BoxLayout.Texcoords = true;
		BoxLayout.Normals = true;
		BoxLayout.Colors = true;

		oGeometryFactory::BOX_DESC BoxDesc;
		BoxDesc.FaceType = oGeometry::FRONT_CCW;
		BoxDesc.Bounds = oAABoxf(float3(-0.5f), float3(0.5f));
		BoxDesc.Divide = 4;
		BoxDesc.Color = std::White;
		BoxDesc.FlipTexcoordV = false;

		oRef<oGeometry> BoxGeo;
		oVERIFY(GeoFactory->Create(BoxDesc, BoxLayout, &BoxGeo));

		oGeometry::DESC GeoDesc;
		BoxGeo->GetDesc(&GeoDesc);

		oGfxMesh::DESC MeshDesc;
		MeshDesc.NumIndices = GeoDesc.NumIndices;
		MeshDesc.NumVertices = GeoDesc.NumVertices;
		MeshDesc.NumRanges = 1;
		MeshDesc.LocalSpaceBounds = GeoDesc.Bounds;
		MeshDesc.FrequentIndexUpdate = false;
		MeshDesc.FrequentVertexUpdate[0] = false;
		MeshDesc.pElements = PLDesc.pElements;
		MeshDesc.NumElements = PLDesc.NumElements;
		oVERIFY(GfxDevice->CreateMesh("oGfxTest Mesh", MeshDesc, &GfxMesh));

		oGeometry::MAPPED GeoMapped;
		BoxGeo->Map(&GeoMapped);

		oGfxCommandList::MAPPED mappedR, mappedI, mappedV;
		GfxCommandList->Map(GfxMesh, oGfxMesh::RANGES, &mappedR);
		GfxCommandList->Map(GfxMesh, oGfxMesh::INDICES, &mappedI);
		GfxCommandList->Map(GfxMesh, oGfxMesh::VERTICES0, &mappedV);

		oGfxMesh::RANGE r = *(oGfxMesh::RANGE*)mappedR.pData;
		r.StartTriangle = 0;
		r.NumTriangles = MeshDesc.NumIndices / 3;
		r.MinVertex = 0;
		r.MaxVertex = MeshDesc.NumVertices;

		memcpy(mappedI.pData, GeoMapped.pIndices, sizeof(uint) * MeshDesc.NumIndices);

		size_t VertexStride = oGfxCalcInterleavedVertexSize(MeshDesc.pElements, MeshDesc.NumElements, 0);
		oMemcpyAsym(mappedV.pData, VertexStride, GeoMapped.pPositions, sizeof(float3), MeshDesc.NumVertices);
		oMemcpyAsym(oByteAdd(mappedV.pData, sizeof(float3)), VertexStride, GeoMapped.pTexcoords, sizeof(float2), MeshDesc.NumVertices);

		GfxCommandList->Unmap(GfxMesh, oGfxMesh::VERTICES0);
		GfxCommandList->Unmap(GfxMesh, oGfxMesh::INDICES);
		GfxCommandList->Unmap(GfxMesh, oGfxMesh::RANGES);

		WinHook = Window->Hook(oBIND(&oRenderTest::Render, this, oBIND1, oBIND2, oBIND3));
	}

	~oRenderTest()
	{
		Window->Unhook(WinHook);
	}

	bool Render(oWindow::EVENT _Event, const float3& _Position, int _Value)
	{
		switch (_Event)
		{
			case oWindow::RESIZING:
				GfxRenderTarget = nullptr;
				break;
			case oWindow::RESIZED:
				oVERIFY(GfxDevice->CreateRenderTarget("Test RenderTarget", Window, oSURFACE_D24_UNORM_S8_UINT, &GfxRenderTarget));
				break;

			case oWindow::DRAW_BACKBUFFER:
			{
				static int counter = 0;

				oGfxRenderTarget::CLEAR_DESC cd;
				cd.ClearColor[0] = (counter++ & 0x1) ? std::White : std::Blue;
				GfxRenderTarget->SetClearDesc(cd);

				GfxCommandList->Begin(float4x4::Identity, float4x4::Identity, PLForwardColor, GfxRenderTarget, 0, 0, nullptr);
				
				GfxCommandList->Clear(oGfxCommandList::COLOR_DEPTH_STENCIL);

				GfxCommandList->End();

				GfxDevice->Submit();

				break;
			}

		default:
			break;
		}

		return true;
	}

	inline threadsafe oWindow* GetWindow() threadsafe { return Window; }

private:

	oRef<threadsafe oWindow> Window;
	oRef<threadsafe oGfxDevice> GfxDevice;
	oRef<oGfxRenderTarget> GfxRenderTarget;
	oRef<oGfxCommandList> GfxCommandList;
	oRef<oGfxPipeline> PLForwardColor;
	oRef<oGfxMesh> GfxMesh;
	unsigned int WinHook;
};

int main(int argc, char* argv[])
{
	oRenderTest RenderTest;

	// _____________________________________________________________________________
	// Set up a UI to ensure rendering doesn't stomp on its compositing

	oWindowUIBox::DESC BoxDesc;
	BoxDesc.Position = int2(-20,-20);
	BoxDesc.Size = int2(130,30);
	BoxDesc.Anchor = oBOTTOMRIGHT;
	BoxDesc.Color = std::OOOiiGreen;
	BoxDesc.BorderColor = std::White;
	BoxDesc.Roundness = 10.0f;

	oRef<threadsafe oWindowUIBox> Box;
	oVERIFY(oWindowUIBoxCreate(BoxDesc, RenderTest.GetWindow(), &Box));

	oWindowUIFont::DESC FontDesc;
	FontDesc.FontName = "Tahoma";
	FontDesc.PointSize = 10;

	oRef<threadsafe oWindowUIFont> Font;
	oVERIFY(oWindowUIFontCreate(FontDesc, RenderTest.GetWindow(), &Font));

	oWindowUIText::DESC TextDesc;
	TextDesc.Position = BoxDesc.Position;
	TextDesc.Size = BoxDesc.Size;
	TextDesc.Anchor = BoxDesc.Anchor;
	TextDesc.Alignment = oMIDDLECENTER;

	oRef<threadsafe oWindowUIText> Text;
	oVERIFY(oWindowUITextCreate(TextDesc, RenderTest.GetWindow(), &Text));
	Text->SetFont(Font);
	Text->SetText("D3D11 Test");

	// _____________________________________________________________________________
	// Main loop

	while (RenderTest.GetWindow()->IsOpen())
	{
		oSleep(200);

		RenderTest.GetWindow()->Refresh(false);
	}

	return 0;
}
