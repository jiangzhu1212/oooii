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
#include <oBasis/oRef.h>
#include <oBasis/oString.h>
#include <oPlatform/oD3D11.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oGDI.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oWindow.h>
#include <oPlatform/oWindowUI.h>
#include <oPlatform/oWinCursor.h>
#include <oPlatform/oWinDialog.h>
#include <oPlatform/oWinMenu.h>
#include <oPlatform/oX11KeyboardSymbols.h>

class oScopedGPUCompositing
{	bool Prior;
public:
	oScopedGPUCompositing(bool _Enabled) : Prior(oSystemGUIUsesGPUCompositing()) { oSystemGUIEnableGPUCompositing(_Enabled); }
	~oScopedGPUCompositing() { oSystemGUIEnableGPUCompositing(Prior); }
};

struct TEST_RESIZE_CONTEXT
{
	int2 OldPos;
	int2 OldDimensions;
	oGUI_WINDOW_STATE OldState;
	bool Resizing;
};

// When windows is captured with its frame, a few pixels on the edge can come 
// from the background, not the window itself. So for image-compare consistency, 
// create a known blank window to guarantee those pixel colors.

bool CreateBlotterWindow(threadsafe oWindow** _ppBlotter)
{
	oWindow::DESC d;
	d.Style = oGUI_WINDOW_BORDERLESS;
	d.State = oGUI_WINDOW_MAXIMIZED;
	return oWindowCreate(d, nullptr, oWindow::USE_DEFAULT, _ppBlotter);
}

bool TESTOnEvent(oWindow::EVENT _Event, const float3& _Position, int _SuperSampleScale, threadsafe oWindow* _pWindow, TEST_RESIZE_CONTEXT* _pTestContext)
{
	oWindow::DESC desc;
	_pWindow->GetDesc(&desc);

	int2 pos = desc.ClientPosition;
	int2 dim = desc.ClientSize;
	
	switch (_Event)
	{
		case oWindow::RESIZING:
			oTRACE("Resizing: Client code should release all device-dependent resources such as back-buffers and HDCs etc. because they're about to be resized/recreated");
			break;

		case oWindow::RESIZED:
			oTRACE("Resized %s %dx%d -> %s %dx%d", oAsString(_pTestContext->OldState), _pTestContext->OldDimensions.x, _pTestContext->OldDimensions.y, oAsString(desc.State), dim.x, dim.y);
			if (!_pTestContext->Resizing)
			{
				_pTestContext->OldState = desc.State;
				_pTestContext->OldDimensions = dim;
			}
			break;

		case oWindow::DRAW_FRONTBUFFER:
		{
			HWND hWnd = (HWND)_pWindow->GetNativeHandle();
			oGDIScopedGetDC hDC(hWnd);
			RECT rClient;
			oVB(GetClientRect(hWnd, &rClient));
			oVERIFY(oGDIDrawText(hDC, rClient, oGUI_ALIGNMENT_MIDDLE_RIGHT, std::Aqua, 0, true, "Hello World 2"));
			break;
		}

		case oWindow::KEY_DOWN:
		{
			oTRACE("Got key down: %s", oAsString((oKEYBOARD_KEY)_SuperSampleScale));
			break;
		}

		case oWindow::KEY_UP:
		{
			oTRACE("Got key up: %s", oAsString((oKEYBOARD_KEY)_SuperSampleScale));
			break;
		}
		case oWindow::POINTER_MOVE:
		{
			oTRACE("MOUSE MOVE: %f %f, Wheel Move %f and BUTTON DOWN: %s", _Position.x, _Position.y, _Position.z, oAsString((oKEYBOARD_KEY)_SuperSampleScale));
		}
		break;
	}

	return true;
}

struct TESTWindowBase : public oTest
{
	struct UI_ELEMENTS
	{
		oRef<threadsafe oWindowUIBox> BoxTopLeft;
		oRef<threadsafe oWindowUIBox> BoxTopRight;
		oRef<threadsafe oWindowUIBox> BoxBottomLeft;
		oRef<threadsafe oWindowUIBox> BoxBottomRight;
		oRef<threadsafe oWindowUIFont> Font;
		oRef<threadsafe oWindowUIText> Text;
		oRef<threadsafe oWindowUILine> Line;
		oRef<threadsafe oWindowUIBox> Box;
		oRef<threadsafe oWindowUIPicture> Picture;
		unsigned int ExtraHook;
		TEST_RESIZE_CONTEXT ResizeContext;

		UI_ELEMENTS() : ExtraHook(oInvalid) {}
		~UI_ELEMENTS() { Clear(); }

		void Clear()
		{
			if (ExtraHook != oInvalid)
			{
				oRef<threadsafe oWindow> Window;
				Text->GetWindow(&Window);
				Window->Unhook(ExtraHook);
			}

			BoxTopLeft = BoxTopRight = BoxBottomLeft = BoxBottomRight = nullptr;
			Picture = nullptr;
			Box = nullptr;
			Line = nullptr;
			Text = nullptr;
			Font = nullptr;
		}
	};

	bool InitTestUI(threadsafe oWindow* _pWindow, UI_ELEMENTS& _Elements)
	{
		{
			oWindowUILine::DESC d;
			d.P1 = int2(200, 25);
			d.P2 = int2(300, 100);
			d.Thickness = 6;
			d.Color = std::RoyalBlue;
			if (!oWindowUILineCreate(d, _pWindow, &_Elements.Line))
				return false;
		}

		{
			oWindowUIBox::DESC d;
			d.Size = int2(140, 70);
			d.Anchor = oGUI_ALIGNMENT_TOP_LEFT;
			d.Color = std::Red;
			d.BorderColor = std::White;
			d.Roundness = 100.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.BoxTopLeft))
				return false;
		}
		{
			oWindowUIBox::DESC d;
			d.Size = int2(100, 70);
			d.Anchor = oGUI_ALIGNMENT_TOP_RIGHT;
			d.Color = std::Black;
			d.BorderColor = std::White;
			d.Roundness = 0.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.BoxTopRight))
				return false;
		}

		{
			oWindowUIBox::DESC d;
			d.Size = int2(70, 70);
			d.Anchor = oGUI_ALIGNMENT_BOTTOM_LEFT;
			d.Color = std::Blue;
			d.BorderColor = std::SkyBlue;
			d.Roundness = 10.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.BoxBottomLeft))
				return false;
		}

		{
			oWindowUIBox::DESC d;
			d.Size = int2(70, 70);
			d.Anchor = oGUI_ALIGNMENT_BOTTOM_RIGHT;
			d.Color = std::SlateGray;
			d.BorderColor = std::White;
			d.Roundness = 100.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.BoxBottomRight))
				return false;
		}

		{
			oWindowUIBox::DESC d;
			d.Position = int2(0, 0);
			d.Size = int2(400, 280);
			d.Anchor = oGUI_ALIGNMENT_MIDDLE_CENTER;
			d.Color = std::BurlyWood;
			d.BorderColor = std::Sienna;
			d.Roundness = 10.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.Box))
				return false;
		}

		{
			oWindowUIFont::DESC d;
			d.FontDesc.FontName = "Tahoma";
			d.FontDesc.Italic = true;
			d.FontDesc.PointSize = 24.0f;
			d.ShadowOffset = 1.0f;
			if (!oWindowUIFontCreate(d, _pWindow, &_Elements.Font))
				return false;
		}

		{
			oWindowUIText::DESC d;
			d.Position = int2(oDEFAULT, oDEFAULT);
			d.Size = int2(400, 280);
			d.Anchor = oGUI_ALIGNMENT_MIDDLE_CENTER;
			d.Alignment = oGUI_ALIGNMENT_MIDDLE_CENTER;
			d.Color = std::White;
			d.ShadowColor = std::Black;
			d.MultiLine = true;
			if (!oWindowUITextCreate(d, _pWindow, &_Elements.Text))
				return false;
			_Elements.Text->SetFont(_Elements.Font);
			_Elements.Text->SetText("Hello World!");
		}

		{
			void* pBuffer = 0;
			size_t size = 0;

			oStringPath imgPath;
			if (!FindInputFile(imgPath, "oooii.ico"))
				return oErrorSetLast(oERROR_NOT_FOUND, "Could not find %s", "oooii.ico");

			if (!oFileLoad(&pBuffer, &size, malloc, imgPath))
				return false;

			oRef<oImage> Image;
			if (!oImageCreate(imgPath, pBuffer, size, &Image))
				return false;
			free(pBuffer);

			oWindowUIPicture::DESC d;
			d.Position = int2(0, 0);
			d.Size = int2(100, 100);
			d.Anchor = oGUI_ALIGNMENT_TOP_CENTER;
			Image->GetDesc(&d.ImageDesc);

			if (!oWindowUIPictureCreate(d, _pWindow, &_Elements.Picture))
				return false;
			_Elements.Picture->Copy(Image->GetData(), d.ImageDesc.RowPitch, false, true);
		}

		memset(&_Elements.ResizeContext, 0, sizeof(_Elements.ResizeContext));
		_Elements.ExtraHook = _pWindow->Hook(oBIND(TESTOnEvent, oBIND1, oBIND2, oBIND3, _pWindow, &_Elements.ResizeContext));
		
		_pWindow->Refresh(false);
		return true;
	}

	RESULT RunTest(char* _StrStatus, size_t _SizeofStrStatus, oWindow::DRAW_MODE _DrawMode)
	{
		// NOTE: There's a lot of transitions that don't seem to settle correctly 
		// which may more be an issue with properly testing to wait for windows to 
		// settle than it is a flaw in the transition code, so currently a bunch of 
		// screen compares fail if ExhaustiveTest is enabled, but look carefully at 
		// each to confirm sanity. Also it seems that even with the black backdrop
		// for some of these test that sometimes a pixel can sneak in, so check the
		// upper left and upper right corner pixels - they ought to be black.
		const bool InteractiveTest = false;	// Interactive test will print all keys presses, mouse moves and mouse buttons presses to the output
		const bool ExhaustiveTest = false; 
		const bool DevMode = false;

		if (_DrawMode == oWindow::USE_D2D && oGetWindowsVersion() < oWINDOWS_7)
		{
			sprintf_s(_StrStatus, _SizeofStrStatus, "D2D is not supported on Windows versions prior to Windows7");
			return SKIPPED;
		}

		if (DevMode || !ExhaustiveTest)
		{
			oWindow::DESC d;
			d.ClientPosition = int2(1100, oDEFAULT);
			d.SupportTouchEvents = InteractiveTest; // Support touch panels
			d.AllowUserFullscreenToggle = DevMode;
			oRef<threadsafe oWindow> Window;
			oTESTB(oWindowCreate(d, nullptr, _DrawMode, &Window), "");
			Window->SetTitle("DevMode");
			UI_ELEMENTS El;
			InitTestUI(Window, El);
			unsigned int i = 0;
			oStringPath path;
			oSystemGetPath(path.c_str(), path.capacity(), oSYSPATH_DESKTOP);
			strcat_s(path, "/test.bmp");

			// Interactive test is for testing keyboard
			if (InteractiveTest)
			{
				oTRACE("Keyboard Test Mode: Enter Key...");
				while (Window->IsOpen())
				{
					oSleep(100);
				}
			}

			if (DevMode)
			{
				while (Window->IsOpen())
				{
					oStringS n;
					sprintf_s(n, "%u", i++);

					oSleep(500);
					if (El.Text)
						El.Text->SetText(n);

					if (i > 10)
					{
						oRef<oImage> image;
						Window->CreateSnapshot(&image);
						oImageSave(image, path);
					}
				}

				return SKIPPED;
			}

			else
			{
				oSleep(200);
				oRef<oImage> snapshot;
				Window->CreateSnapshot(&snapshot);
				oTESTB(TestImage(snapshot, 1337), "Snapshot compare failed");
				return SUCCESS;
			}
		}

		std::vector<unsigned int> ImageFailures;
		RESULT result = SUCCESS;

		// _____________________________________________________________________________
		// First test initial startup in various configurations

		oGUI_WINDOW_STYLE Styles[] = 
		{
			oGUI_WINDOW_BORDERLESS,
			oGUI_WINDOW_FIXED,
			oGUI_WINDOW_DIALOG,
			oGUI_WINDOW_SIZEABLE,
		};

		oGUI_WINDOW_STATE States[] =
		{
			oGUI_WINDOW_HIDDEN,
			oGUI_WINDOW_RESTORED,
			oGUI_WINDOW_MINIMIZED,
			oGUI_WINDOW_MAXIMIZED,
			oGUI_WINDOW_FULLSCREEN,
		};

		oWindow::DESC d;
		d.ClientSize = int2(640, 480);
		d.CursorState = oGUI_CURSOR_WAIT_FOREGROUND;
		d.UseAntialiasing = true;
		d.Enabled = true;
		d.HasFocus = true;
		d.AlwaysOnTop = true;

		oStringL Title;

		UI_ELEMENTS Elements;
		oRef<threadsafe oWindow> Window;

		oStringPath SnapshotPath;
		oRef<oImage> Snapshot;
		unsigned int ImageIndex = 0;

		// Testing styles, which means how the frame looks. It's hard to do snapshot
		// compares with Aero since it blends in the arbitrary background. So 
		// disable it for this part of the test. Also we're going to test "maximized"
		// so hard-code a screen resolution so this test can be run on any PC 
		// capable of this lowest-common-denominator res
		if (ExhaustiveTest)
		{
			oScopedGPUCompositing saveCompositingState(false);

			oRef<threadsafe oWindow> Blotter;
			oTESTB(CreateBlotterWindow(&Blotter), "");
			Blotter->Refresh();
			for (size_t style = 0; style < oCOUNTOF(Styles); style++)
			{
				Elements.Clear();
				Window = nullptr;

				d.Style = static_cast<oGUI_WINDOW_STYLE>(style);
				sprintf_s(Title, "Startup: %s/%s", oAsString(d.State), oAsString(d.Style));

				oTESTB(oWindowCreate(d, nullptr, _DrawMode, &Window), "oWindowCreate failed");
				Window->SetTitle(Title);
				oTESTB(InitTestUI(Window, Elements), "InitTestUI failed");
				Elements.Text->SetText(Title);

				Blotter->SetFocus();
				oSleep(200);
				oTESTB(Window->CreateSnapshot(&Snapshot, true), "Snapshot failed");
			
				if (!TestImage(Snapshot, ImageIndex))
					ImageFailures.push_back(ImageIndex);
				ImageIndex++;
			}

			// _____________________________________________________________________________
			// Test style transitions

			Elements.Clear();
			Window = nullptr;

			d = oWindow::DESC();
			d.ClientSize = int2(640, 480);

			oTESTB(oWindowCreate(d, nullptr, _DrawMode, &Window), "oWindowCreate failed");
			oTESTB(InitTestUI(Window, Elements), "");

			Blotter->SetFocus();
			for (size_t style = 0; style < oCOUNTOF(Styles); style++)
			{
				for (size_t style2 = 0; style2 < oCOUNTOF(Styles); style2++)
				{
					{
						Window->GetDesc(&d);
						sprintf_s(Title, "%s->%s", oAsString(d.Style), oAsString(Styles[style]));
						Window->SetTitle(Title);
						Elements.Text->SetText(Title);
						oTRACE("STYLE TRANSITION: %s", Title.c_str());

						oWindow::DESC* pDesc = Window->Map();
						pDesc->Style = Styles[style];
						Window->Unmap();

						oSleep(200);
						oTESTB(Window->CreateSnapshot(&Snapshot, true), "Snapshot failed");
						if (!TestImage(Snapshot, ImageIndex))
							ImageFailures.push_back(ImageIndex);
						ImageIndex++;
					}

					{
						Window->GetDesc(&d);
						sprintf_s(Title, "%s->%s", oAsString(d.Style), oAsString(Styles[style2]));
						Window->SetTitle(Title);
						Elements.Text->SetText(Title);
						oTRACE("STYLE TRANSITION: %s", Title.c_str());

						oWindow::DESC* pDesc = Window->Map();
						pDesc->Style = Styles[style2];
						Window->Unmap();

						oSleep(200);
						oTESTB(Window->CreateSnapshot(&Snapshot, true), "Snapshot failed");
						if (!TestImage(Snapshot, ImageIndex))
							ImageFailures.push_back(ImageIndex);
						ImageIndex++;
					}
				}
			}
			
			// _____________________________________________________________________________
			// Test state transitions

			oDISPLAY_MODE FixedDisplayMode;
			FixedDisplayMode.Size = int2(1024, 768);
			oScopedDisplayMode saveDisplayMode(oDisplayGetPrimaryIndex(), FixedDisplayMode);
			oSleep(500);

			oDISPLAY_DESC dd;
			oDisplayEnum(oDisplayGetPrimaryIndex(), &dd);
			int2 Position = dd.WorkareaPosition + int2(30, 30);

			Blotter->SetFocus();

			for (size_t state = 0; state < oCOUNTOF(States); state++)
			{
				for (size_t state2 = 0; state2 < oCOUNTOF(States); state2++)
				{
					{
						Window->GetDesc(&d);
						sprintf_s(Title, "%s->%s", oAsString(d.State), oAsString(States[state]));
						Window->SetTitle(Title);
						Elements.Text->SetText(Title);
						oTRACE("STATE TRANSITION: %s", Title.c_str());

						oWindow::DESC* pDesc = Window->Map();
						pDesc->ClientPosition = Position;
						pDesc->State = States[state];
						Window->Unmap();

						oSleep(1000); // HACK

						if (States[state] == oGUI_WINDOW_FULLSCREEN)
							oSleep(500);
						else
							oSleep(200);

						// Hidden/Minimized destination states won't have any visuals to capture
						if (States[state] != oGUI_WINDOW_HIDDEN && States[state] != oGUI_WINDOW_MINIMIZED)
						{
							oTESTB(Window->CreateSnapshot(&Snapshot, true), "Snapshot failed");
							if (!TestImage(Snapshot, ImageIndex))
								ImageFailures.push_back(ImageIndex);
							ImageIndex++;
						}
					}

					{
						Window->GetDesc(&d);
						sprintf_s(Title, "%s->%s", oAsString(d.State), oAsString(States[state2]));
						Window->SetTitle(Title);
						Elements.Text->SetText(Title);
						oTRACE("STATE TRANSITION: %s", Title.c_str());

						oWindow::DESC* pDesc = Window->Map();
						pDesc->ClientPosition = Position;
						pDesc->State = States[state2];
						Window->Unmap();

						oSleep(1000); // HACK

						if (States[state2] == oGUI_WINDOW_FULLSCREEN)
							oSleep(500);
						else
							oSleep(200);

						// Hidden/Minimized destination states won't have any visuals to capture
						if (States[state2] != oGUI_WINDOW_HIDDEN && States[state2] != oGUI_WINDOW_MINIMIZED)
						{
							oTESTB(Window->CreateSnapshot(&Snapshot, true), "Snapshot failed");
							if (!TestImage(Snapshot, ImageIndex))
								ImageFailures.push_back(ImageIndex);
							ImageIndex++;
						}
					}
				}
			}
		}

		*_StrStatus = 0;
		for (auto it = ImageFailures.begin(); it != ImageFailures.end(); ++it)
		{
			if (!*_StrStatus)
				oStrAppend(_StrStatus, _SizeofStrStatus, "Image compares failed: %u", *it);
			else
				oStrAppend(_StrStatus, _SizeofStrStatus, ",%u", *it);
		}

		return ImageFailures.empty() ? SUCCESS : FAILURE;
	}
};

struct TESTWindowD2D : public TESTWindowBase
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		return RunTest(_StrStatus, _SizeofStrStatus, oWindow::USE_DEFAULT);
	}
};

struct TESTWindowGDI : public TESTWindowBase
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		return RunTest(_StrStatus, _SizeofStrStatus, oWindow::USE_GDI);
	}
};

oTEST_REGISTER(TESTWindowD2D);
oTEST_REGISTER(TESTWindowGDI);
