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
#include "pch.h"
#include <oooii/oWindows.h>
#include <oooii/oAssert.h>
#include <oooii/oRef.h>
#include <oooii/oImage.h>
#include <oooii/oKeyboard.h>
#include <oooii/oMouse.h>
#include <oooii/oPath.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>
#include <oooii/oWindow.h>

struct TEST_RESIZE_CONTEXT
{
	unsigned int OldWidth;
	unsigned int OldHeight;
	oWindow::STATE OldState;
	bool Resizing;
};

void TESTResizeHandler(oWindow::RESIZE_EVENT _Event, oWindow::STATE _State, unsigned int _Width, unsigned int _Height, void* _pUserData)
{
	TEST_RESIZE_CONTEXT* pContext = static_cast<TEST_RESIZE_CONTEXT*>(_pUserData);
	switch (_Event)
	{
	case oWindow::RESIZE_BEGIN:
		oTRACE("Entering Resize %ux%u -> %ux%u", pContext->OldWidth, pContext->OldHeight, _Width, _Height);
		pContext->Resizing = true;
		pContext->OldWidth = _Width;
		pContext->OldHeight = _Height;
		break;

	case oWindow::RESIZE_CHANGE:
		oTRACE("Resizing %s %ux%u -> %s %ux%u", oAsString(pContext->OldState), pContext->OldWidth, pContext->OldHeight, oAsString(_State), _Width, _Height);
		if (!pContext->Resizing)
		{
			pContext->OldState = _State;
			pContext->OldWidth = _Width;
			pContext->OldHeight = _Height;
		}

		break;

	case oWindow::RESIZE_END:
		oTRACE("Exiting Resize %ux%u -> %ux%u", pContext->OldWidth, pContext->OldHeight, _Width, _Height);
		pContext->Resizing = false;
		break;

	default: oASSUME(0);
	}
}

struct TESTWindowBase : public oTest
{
	RESULT RunTest(char* _StrStatus, size_t _SizeofStrStatus, int _DrawAPIFourCC)
	{
		if (!_DrawAPIFourCC && oGetWindowsVersion() < oWINDOWS_7)
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "D2D is not supported on Windows versions prior to Windows7");
			return SKIPPED;
		}

		oRef<oWindow> Window;
		{
			oWindow::DESC desc;
			desc.ClientX = oWindow::DEFAULT;
			desc.ClientY = oWindow::DEFAULT;
			desc.ClientWidth = 640;
			desc.ClientHeight = 480;
			desc.State = oWindow::HIDDEN;
			desc.Style = oWindow::SIZEABLE;
			desc.UseAntialiasing = true;
			desc.Enabled = true; // control is handled by this test and a timeout
			desc.HasFocus = true;
			desc.AlwaysOnTop = false;
			desc.EnableCloseButton = false;
			oTESTB(oWindow::Create(&desc, "OOOii oWindow", _DrawAPIFourCC, &Window), "Failed to create window");

			#if defined(_WIN32) || defined (_WIN64)
				// Load/test OOOii lib icon:
				extern void GetDescoooii_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);
				const char* BufferName = 0;
				const void* pBuffer = 0;
				size_t bufferSize = 0;
				GetDescoooii_ico(&BufferName, &pBuffer, &bufferSize);

				oRef<oImage> ico;
				oTESTB(oImage::Create(pBuffer, bufferSize, &ico), "Failed to load icon");

				HICON hIcon = ico->AsIco();
				oTESTB(Window->SetProperty("Icon", &hIcon), "Failed to set icon");
				DeleteObject(hIcon);
			#endif
		}

		threadsafe oRef<oWindow::Resizer> Resizer;
		TEST_RESIZE_CONTEXT resizeContext;
		memset(&resizeContext, 0, sizeof(resizeContext));
		oTESTB(Window->CreateResizer(TESTResizeHandler, &resizeContext, &Resizer), "Failed to create resizer");

		threadsafe oRef<oWindow::Font> Font;
		{
			oWindow::Font::DESC desc;
			strcpy_s(desc.FontName, "Tahoma");
			desc.Style = oWindow::Font::ITALIC;
			desc.PointSize = 24.0f;
			desc.ShadowOffset = 1.0f;
			oTESTB(Window->CreateFont(&desc, &Font), "Failed to create font");
		}

		threadsafe oRef<oWindow::Text> Text;
		{
			oWindow::Text::DESC desc;
			desc.X = oWindow::DEFAULT;
			desc.Y = oWindow::DEFAULT;
			desc.Width = 320;
			desc.Height = 240;
			desc.Anchor = oWindow::MIDDLE_CENTER;
			desc.Alignment = oWindow::MIDDLE_CENTER;
			desc.Color = std::White;
			desc.ShadowColor = std::Black;
			strcpy_s(desc.String, "Hello World!");
			desc.MultiLine = true;
			oTESTB(Window->CreateText(&desc, Font, &Text), "Failed to create text");
		}

		threadsafe oRef<oWindow::Line> Line;
		{
			oWindow::Line::DESC desc;
			desc.X1 = 200;
			desc.Y1 = 25;
			desc.X2 = 300;
			desc.Y2 = 100;
			desc.Thickness = 6;
			desc.Color = std::RoyalBlue;
			oTESTB(Window->CreateLine(&desc, &Line), "Failed to create line");
		}

		threadsafe oRef<oWindow::RoundedBox> RoundedBox;
		{
			oWindow::RoundedBox::DESC desc;
			desc.X = 0;
			desc.Y = 0;
			desc.Width = 320;
			desc.Height = 240;
			desc.Anchor = oWindow::MIDDLE_CENTER;
			desc.Color = std::BurlyWood;
			desc.BorderColor = std::Sienna;
			desc.Roundness = 10.0f;
			oTESTB(Window->CreateRoundedBox(&desc, &RoundedBox), "Failed to create rounded box");
		}

		threadsafe oRef<oWindow::Picture> Picture;
		{
			void* pBuffer = 0;
			size_t size = 0;

			char imgPath[_MAX_PATH];
			oTESTB(oTestManager::Singleton()->FindFullPath(imgPath, "oooii.ico"), "Failed to find oooii.ico");
			oTESTB(oLoadBuffer(&pBuffer, &size, malloc, imgPath, false), "Failed to load test image %s", imgPath);

			oRef<oImage> Image;
			oTESTB(oImage::Create(pBuffer, size, &Image), "Failed to create image");
			free(pBuffer);
			oImage::DESC iDesc;
			Image->GetDesc(&iDesc);

			oWindow::Picture::DESC desc;
			desc.X = 100;
			desc.Y = 25;
			desc.Width = 100;
			desc.Height = 100;
			desc.Anchor = oWindow::TOP_LEFT;
			desc.SurfaceDesc.Width = iDesc.Width;
			desc.SurfaceDesc.Height = iDesc.Height;
			desc.SurfaceDesc.RowPitch = iDesc.Pitch;
			desc.SurfaceDesc.Format = iDesc.Format;

			oTESTB(Window->CreatePicture(&desc, &Picture), "Failed to create picture");
			Picture->Copy(Image->GetData(), iDesc.Pitch, false, true);
		}

		threadsafe oRef<oKeyboard> Keyboard;
		oTESTB(oKeyboard::Create(Window->GetNativeHandle(), true, &Keyboard), "Failed to create keyboard");

		threadsafe oRef<oMouse> Mouse;
		oTESTB(oMouse::Create(Window->GetNativeHandle(), false, &Mouse), "Failed to create mouse");

		bool bShouldPrintUp[oKeyboard::NUM_KEYS];
		for (size_t i = 0; i < oCOUNTOF(bShouldPrintUp); i++)
			bShouldPrintUp[i] = Keyboard && Keyboard->IsDown((oKeyboard::KEY)i);

		bool bMouseShouldPrintUp[oMouse::NUM_BUTTONS];
		for (size_t i = 0; i < oCOUNTOF(bMouseShouldPrintUp); i++)
			bMouseShouldPrintUp[i] = Mouse && Mouse->IsDown((oMouse::BUTTON)i);

		// Now that setup is done, show the window and go into the main loop
		{
			oWindow::DESC desc;
			Window->GetDesc(&desc);
			desc.State = oWindow::RESTORED;
			Window->SetDesc(&desc);
		}

		double start = oTimer();
		RESULT result = SUCCESS;

		oMouse::ScopedWait wait = oMouse::ScopedWait(Mouse);

		while (Window->IsOpen())
		{
			if (Keyboard)
				Keyboard->Update();

			if (Mouse)
				Mouse->Update();

			if (Window->Begin())
			{
				// TRACE keyboard state
				if (Keyboard)
				{
					for (size_t i = 0; i < oKeyboard::NUM_KEYS; i++)
					{
						oKeyboard::KEY k = (oKeyboard::KEY)i;

						if (Keyboard->IsPressed(k))
							oTRACE("%s pressed", oAsString(k));
						else if (Keyboard->IsDown(k))
							oTRACE("%s down", oAsString(k));
						else if (Keyboard->IsReleased(k))
						{
							oTRACE("%s released", oAsString(k));
							bShouldPrintUp[i] = true;
						}

						else if (bShouldPrintUp[i] && Keyboard->IsUp(k))
						{
							oTRACE("%s up", oAsString(k));
							bShouldPrintUp[i] = false;
						}
					}
				}

				// TRACE mouse state
				{
					int x, y, v = 0, h = 0;
					Mouse->GetPosition(&x, &y, &v, &h);

					if (v) oTRACE("mouse vwheel: %d", v);
					if (h) oTRACE("mouse hwheel: %d", h);

					for (size_t i = 0; i < oMouse::NUM_BUTTONS; i++)
					{
						oMouse::BUTTON b = (oMouse::BUTTON)i;

						if (Mouse->IsPressed(b))
							oTRACE("%s pressed", oAsString(b));
						else if (Mouse->IsDown(b))
							oTRACE("%s down", oAsString(b));
						else if (Mouse->IsReleased(b))
						{
							oTRACE("%s released", oAsString(b));
							bMouseShouldPrintUp[i] = true;
						}

						else if (bMouseShouldPrintUp[i] && Mouse->IsUp(b))
						{
							oTRACE("%s up", oAsString(b));
							bMouseShouldPrintUp[i] = false;
						}
					}
				}

				Window->End();

				HDC hDC = GetDC((HWND)Window->GetNativeHandle());
				RECT rClient;
				GetClientRect((HWND)Window->GetNativeHandle(), &rClient);
				oGDIDrawText(hDC, &rClient, std::Aqua, 0, "rm", "Hello World 2");
				ReleaseDC((HWND)Window->GetNativeHandle(), hDC);

				if ((oTimer() - start) > 1.0)
				{
					oRef<oImage> snapshot;
					oTESTB(Window->CreateSnapshot(&snapshot), "Failed to create snapshot");

					if (!this->TestImage(snapshot))
					{
						result = FAILURE;
						sprintf_s(_StrStatus, _SizeofStrStatus, "Image compare failed");
					}

					Window->Close();
				}
			}
		}

		// Ensure anything attached to the window is taken down before Window goes out of scope.
		Keyboard = 0;
		Mouse = 0;

		return result;
	}
};

struct TESTWindowD2D : public TESTWindowBase
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		return RunTest(_StrStatus, _SizeofStrStatus, 0);
	}
};

struct TESTWindowGDI : public TESTWindowBase
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		return RunTest(_StrStatus, _SizeofStrStatus, 'GDI ');
	}
};

TESTWindowD2D TestWindowD2D;
TESTWindowGDI TestWindowGDI;
