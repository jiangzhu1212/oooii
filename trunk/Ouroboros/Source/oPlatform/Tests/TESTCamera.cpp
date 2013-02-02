/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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

// @oooii-tony: I'm not sure how to automatically test a camera since it could
// be looking at anything and that's even if there's a camera attached to the
// computer. So include this function that enumerates all attached cameras and
// opens an oWindow for each with its video stream for verification/iteration.

#include <oPlatform/oCamera.h>
#include <oBasis/oRef.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oWindow.h>

int ShowAllCameras()
{
	struct CONTEXT
	{
		CONTEXT(oRef<threadsafe oCamera> _pCamera)
			: Camera(_pCamera)
			, LastFrame(oInvalid)
		{}

		oRef<threadsafe oCamera> Camera;
		oRef<threadsafe oWindow> Window;
		unsigned int LastFrame;
	};

	std::vector<CONTEXT> Contexts;
	unsigned int index = 0;

	while (1)
	{
		oRef<threadsafe oCamera> Camera;
		if (oCameraEnum(index++, &Camera))
		{
			oCamera::MODE mode;
			mode.Dimensions = int2(640, 480);
			mode.Format = oSURFACE_R8G8B8_UNORM;
			mode.BitRate = ~0u;

			oCamera::MODE closest;
			if (!Camera->FindClosestMatchingMode(mode, &closest))
				oASSERT(false, "");

			if (!Camera->SetMode(closest))
			{
				oMSGBOX_DESC d;
				d.Type = oMSGBOX_ERR;
				d.Title = "oCamera Test";
				oMsgBox(d, "Camera %s does not support mode %s %dx%d", Camera->GetName(), oAsString(mode.Format), mode.Dimensions.x, mode.Dimensions.y);
				continue;
			}

			Contexts.push_back(Camera);
		}

		else if (oErrorGetLast() == oERROR_NOT_FOUND)
			break;
	}

//	if (oErrorGetLast() == oERROR_NOT_FOUND && !Contexts.empty())
//	{
//		oMSGBOX_DESC d;
//		d.Type = oMSGBOX_ERR;
//		d.Title = "oCamera Test";
//#if o64BIT
//		oMsgBox(d, "Enumerating system cameras is not supported on 64-bit systems.");
//#else
//		oMsgBox(d, "Failed to enumerate system cameras because an internal interface could not be found.");
//#endif
//		return -1;
//	}

	if (Contexts.empty())
	{
		oMSGBOX_DESC d;
		d.Type = oMSGBOX_INFO;
		d.Title = "oCamera Test";
		oMsgBox(d, "No cameras were found, so no windows will open");
	}

	for (size_t i = 0; i < Contexts.size(); i++)
	{
		oCamera::DESC cd;
		Contexts[i].Camera->GetDesc(&cd);

		oStringL Title;
		oPrintf(Title, "%s (%dx%d %s)", Contexts[i].Camera->GetName(), cd.Mode.Dimensions.x, cd.Mode.Dimensions.y, oAsString(cd.Mode.Format));

		oWINDOW_INIT init;
		init.WinDesc.ClientSize = cd.Mode.Dimensions;
		init.WinDesc.ClientPosition = int2(30, 30) * int2(oUInt(i + 1), oUInt(i + 1));
		init.WindowTitle = Title;
		oVERIFY(oWindowCreate(init, &Contexts[i].Window));
	}

	bool aWindowIsOpen = false;
	for (size_t i = 0; i < Contexts.size(); i++)
		if (Contexts[i].Window->IsOpen())
			aWindowIsOpen = true;

	while (aWindowIsOpen)
	{
		for (size_t i = 0; i < Contexts.size(); i++)
		{
			oCamera::DESC cd;
			Contexts[i].Camera->GetDesc(&cd);

			oCamera::MAPPED mapped;
			if (Contexts[i].Camera->Map(&mapped))
			{
				if (mapped.Frame != Contexts[i].LastFrame)
				{
					HWND hWnd = nullptr;
					Contexts[i].Window->QueryInterface(oGetGUID<oGUI_WINDOW>(), &hWnd);

					oVERIFY(oGDIStretchBits(hWnd, cd.Mode.Dimensions, cd.Mode.Format, mapped.pData, mapped.RowPitch));
					Contexts[i].LastFrame = mapped.Frame;

					float fps = Contexts[i].Camera->GetFPS();
					oStringS sFPS;
					oPrintf(sFPS, "FPS: %.01f", fps + (rand() %100) / 100.0f);
					
					RECT rClient;
					GetClientRect(hWnd, &rClient);

					oGUI_TEXT_DESC td;
					td.Foreground = std::OOOiiGreen;
					td.Position = int2(0, 0);
					td.Size = oWinRectSize(rClient);
					td.Shadow = std::Black;
					td.ShadowOffset = int2(1,1);
					td.Alignment = oGUI_ALIGNMENT_MIDDLE_LEFT;

					oGDIScopedGetDC hDC(hWnd);
					oGDIDrawText(hDC, td, sFPS);
				}

				Contexts[i].Camera->Unmap();
			}
		}

		aWindowIsOpen = false;
		for (size_t i = 0; i < Contexts.size(); i++)
			if (Contexts[i].Window->IsOpen())
				aWindowIsOpen = true;
	}

	return 0;
}

struct PLATFORM_oCamera : oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		return 0 == ShowAllCameras() ? oTest::SUCCESS : oTest::FAILURE;
	}
};

//oTEST_REGISTER(PLATFORM_oCamera);
