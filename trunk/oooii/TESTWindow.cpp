// $(header)
#include <oooii/oAssert.h>
#include <oooii/oFile.h>
#include <oooii/oImage.h>
#include <oooii/oKeyboard.h>
#include <oooii/oMouse.h>
#include <oooii/oPath.h>
#include <oooii/oRef.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>
#include <oooii/oWindow.h>

struct TEST_RESIZE_CONTEXT
{
	int2 OldPos;
	int2 OldDimensions;
	oWindow::STATE OldState;
	bool Resizing;
};

void TESTResizeHandler(oWindow::RECT_EVENT _Event, oWindow::STATE _State, oRECT _Rect, void* _pUserData)
{
	TEST_RESIZE_CONTEXT* pContext = static_cast<TEST_RESIZE_CONTEXT*>(_pUserData);

	int2 pos = _Rect.GetMin();
	int2 dim = _Rect.GetDimensions();
	
	switch (_Event)
	{
		case oWindow::RECT_BEGIN:
			oTRACE("Entering Resize %ux%u -> %ux%u", pContext->OldDimensions.x, pContext->OldDimensions.y, dim.x, dim.y);
			pContext->Resizing = true;
			pContext->OldDimensions = dim;
			break;

		case oWindow::RESIZE_OCCURING:
			oTRACE("Resizing %s %ux%u -> %s %ux%u", oAsString(pContext->OldState), pContext->OldDimensions.x, pContext->OldDimensions.y, oAsString(_State), dim.x, dim.y);
			if (!pContext->Resizing)
			{
				pContext->OldState = _State;
				pContext->OldDimensions = dim;
			}

			break;

		case oWindow::RECT_END:
			oTRACE("Exiting Resize %ux%u -> %ux%u", pContext->OldDimensions.x, pContext->OldDimensions.y, dim.x, dim.y);
			pContext->Resizing = false;
			break;

		case oWindow::MOVE_OCCURING:
		{
			oTRACE("Moving %ux%u -> %ux%u", pContext->OldPos.x, pContext->OldPos.y, pos.x, pos.y);
			pContext->OldPos = pos;
			break;
		}

		default: oASSUME(0);
	}
}

struct TESTWindowBase : public oTest
{
	void TraceKeyState(threadsafe oKeyboard* _pKeyboard, bool _bShouldPrintUp[oKeyboard::NUM_KEYS])
	{
		if (_pKeyboard)
		{
			for (size_t i = 0; i < oKeyboard::NUM_KEYS; i++)
			{
				oKeyboard::KEY k = (oKeyboard::KEY)i;

				if (_pKeyboard->IsPressed(k))
					oTRACE("%s pressed", oAsString(k));
				else if (_pKeyboard->IsDown(k))
					oTRACE("%s down", oAsString(k));
				else if (_pKeyboard->IsReleased(k))
				{
					oTRACE("%s released", oAsString(k));
					_bShouldPrintUp[i] = true;
				}

				else if (_bShouldPrintUp[i] && _pKeyboard->IsUp(k))
				{
					oTRACE("%s up", oAsString(k));
					_bShouldPrintUp[i] = false;
				}
			}
		}
	}

	void TraceMouseState(threadsafe oMouse* _pMouse, bool _bMouseShouldPrintUp[oMouse::NUM_BUTTONS])
	{
		int x, y, v = 0, h = 0;
		_pMouse->GetPosition(&x, &y, &v, &h);

		if (v) oTRACE("mouse vwheel: %d", v);
		if (h) oTRACE("mouse hwheel: %d", h);

		for (size_t i = 0; i < oMouse::NUM_BUTTONS; i++)
		{
			oMouse::BUTTON b = (oMouse::BUTTON)i;

			if (_pMouse->IsPressed(b))
				oTRACE("%s pressed", oAsString(b));
			else if (_pMouse->IsDown(b))
				oTRACE("%s down", oAsString(b));
			else if (_pMouse->IsReleased(b))
			{
				oTRACE("%s released", oAsString(b));
				_bMouseShouldPrintUp[i] = true;
			}

			else if (_bMouseShouldPrintUp[i] && _pMouse->IsUp(b))
			{
				oTRACE("%s up", oAsString(b));
				_bMouseShouldPrintUp[i] = false;
			}
		}
	}

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
			desc.MSSleepWhenNoFocus = 0;
			oTESTB(oWindow::Create(&desc, NULL, "OOOii oWindow", _DrawAPIFourCC, &Window), "Failed to create window: %s: %s", oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
		}

		oRef<threadsafe oWindow::Resizer> Resizer;
		TEST_RESIZE_CONTEXT resizeContext;
		memset(&resizeContext, 0, sizeof(resizeContext));
		oTESTB(Window->CreateResizer(oBIND(&TESTResizeHandler, oBIND1, oBIND2, oBIND3, &resizeContext ), &Resizer), "Failed to create resizer");

		oRef<threadsafe oWindow::Font> Font;
		{
			oWindow::Font::DESC desc;
			strcpy_s(desc.FontName, "Tahoma");
			desc.Style = oWindow::Font::ITALIC;
			desc.PointSize = 24.0f;
			desc.ShadowOffset = 1.0f;
			oTESTB(Window->CreateFont(&desc, &Font), "Failed to create font");
		}

		oRef<threadsafe oWindow::Text> Text;
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

		oRef<threadsafe oWindow::Line> Line;
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

		oRef<threadsafe oWindow::RoundedBox> RoundedBox;
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

		oRef<threadsafe oWindow::Picture> Picture;
		{
			void* pBuffer = 0;
			size_t size = 0;

			char imgPath[_MAX_PATH];
			oTESTB(oTestManager::Singleton()->FindFullPath(imgPath, "oooii.ico"), "Failed to find oooii.ico");
			oTESTB(oFile::LoadBuffer(&pBuffer, &size, malloc, imgPath, false), "Failed to load test image %s", imgPath);

			oRef<oImage> Image;
			oTESTB(oImage::Create(pBuffer, size, oSurface::UNKNOWN, &Image), "Failed to create image");
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
			Picture->Copy(Image->Map(), iDesc.Pitch, false, true);
			Image->Unmap();
		}

		oRef<threadsafe oKeyboard> Keyboard;
		oTESTB(oKeyboard::Create(Window->GetNativeHandle(), true, &Keyboard), "Failed to create keyboard");

		oRef<threadsafe oMouse> Mouse;
		oMouse::DESC MouseDesc;
		MouseDesc.ShortCircuitEvents = false;
		oTESTB(oMouse::Create(MouseDesc, Window->GetNativeHandle(), &Mouse), "Failed to create mouse");

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
				TraceKeyState(Keyboard, bShouldPrintUp);
				TraceMouseState(Mouse, bMouseShouldPrintUp);

				Window->End();

				HWND hWnd = (HWND)Window->GetNativeHandle();
				HDC hDC = GetDC(hWnd);
				RECT rClient;
				GetClientRect(hWnd, &rClient);
				oGDIDrawText(hDC, &rClient, std::Aqua, 0, "rm", "Hello World 2");
				ReleaseDC(hWnd, hDC);

				if ((oTimer() - start) > 1.0)
				{
					oRef<oImage> snapshot;
					oTESTB(Window->CreateSnapshot(&snapshot), "Failed to create snapshot");
					oTESTI(snapshot);
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

oTEST_REGISTER(TESTWindowD2D);
oTEST_REGISTER(TESTWindowGDI);
