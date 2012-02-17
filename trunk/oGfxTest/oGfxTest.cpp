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

bool Render(oWindow::EVENT _Event, const float3& _Position, int _Value, threadsafe oWindow* _pWindow)
{
	switch (_Event)
	{
		case oWindow::DRAW_BACKBUFFER:
		{
			static int counter = 0;

			const FLOAT RGBA[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			const FLOAT RGBA2[] = { 0.0f, 0.0f, 1.0f, 1.0f };

			oRef<ID3D11Device> D3D11Device;
			oVERIFY(_pWindow->QueryInterface((const oGUID&)__uuidof(ID3D11Device), &D3D11Device));

			oRef<ID3D11DeviceContext> DevContext;
			D3D11Device->GetImmediateContext(&DevContext);
			
			oRef<ID3D11RenderTargetView> RTV;
			oVERIFY(_pWindow->QueryInterface((const oGUID&)__uuidof(ID3D11RenderTargetView), &RTV));

			DevContext->ClearRenderTargetView(RTV, (counter++ & 0x1)?RGBA:RGBA2);

			DevContext->Flush();
			break;
		}

		default:
			break;
	}

	return true;
}

int main(int argc, char* argv[])
{
	oGfxDevice::DESC DevDesc;
	DevDesc.Version = oVersion(10,0);
	DevDesc.UseSoftwareEmulation = false;
	DevDesc.EnableDebugReporting = true;

	oRef<threadsafe oGfxDevice> GfxDevice;
	oVERIFY(oGfxDeviceCreate(DevDesc, &GfxDevice));

	oRef<ID3D11Device> D3D11Device;
	oVERIFY(GfxDevice->QueryInterface((const oGUID&)__uuidof(ID3D11Device), &D3D11Device));

	oWindow::DESC WinDesc;
	WinDesc.Style = oWindow::FIXED;
	WinDesc.AutoClear = false;
	WinDesc.EnableUIDrawing = true;
	WinDesc.UseAntialiasing = false;
	WinDesc.AllowUserFullscreenToggle = true;

	oRef<threadsafe oWindow> Window;
	oVERIFY(oWindowCreate(WinDesc, D3D11Device, oWindow::USE_GDI, &Window));
	Window->SetTitle("User-Specified D3D11 Test");

	// Set up a UI to ensure rendering doesn't stomp on its compositing
	oWindowUIBox::DESC BoxDesc;
	BoxDesc.Position = int2(-20,-20);
	BoxDesc.Size = int2(130,30);
	BoxDesc.Anchor = oBOTTOMRIGHT;
	BoxDesc.Color = std::OOOiiGreen;
	BoxDesc.BorderColor = std::White;
	BoxDesc.Roundness = 10.0f;

	oRef<threadsafe oWindowUIBox> Box;
	oVERIFY(oWindowUIBoxCreate(BoxDesc, Window, &Box));

	oWindowUIFont::DESC FontDesc;
	FontDesc.FontName = "Tahoma";
	FontDesc.PointSize = 10;

	oRef<threadsafe oWindowUIFont> Font;
	oVERIFY(oWindowUIFontCreate(FontDesc, Window, &Font));

	oWindowUIText::DESC TextDesc;
	TextDesc.Position = BoxDesc.Position;
	TextDesc.Size = BoxDesc.Size;
	TextDesc.Anchor = BoxDesc.Anchor;
	TextDesc.Alignment = oMIDDLECENTER;

	oRef<threadsafe oWindowUIText> Text;
	oVERIFY(oWindowUITextCreate(TextDesc, Window, &Text));
	Text->SetFont(Font);
	Text->SetText("D3D11 Test");

	Window->Hook(oBIND(Render, oBIND1, oBIND2, oBIND3, Window.c_ptr()));

	while (Window->IsOpen())
	{
		oSleep(200);

		Window->Refresh(false);
	}

	return 0;
}
