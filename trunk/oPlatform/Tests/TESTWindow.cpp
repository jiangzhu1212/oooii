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
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oWindow.h>
#include <oPlatform/oWindowUI.h>
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
	oWindow::STATE OldState;
	bool Resizing;
};

// When windows is captured with its frame, a few pixels on the edge can come 
// from the background, not the window itself. So for image-compare consistency, 
// create a known blank window to guarantee those pixel colors.

bool CreateBlotterWindow(threadsafe oWindow** _ppBlotter)
{
	oWindow::DESC d;
	d.Style = oWindow::BORDERLESS;
	d.State = oWindow::MAXIMIZED;
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
			oVERIFY(oGDIDrawText(hDC, rClient, oMIDDLERIGHT, std::Aqua, 0, true, "Hello World 2"));
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
			d.Anchor = oTOPLEFT;
			d.Color = std::Red;
			d.BorderColor = std::White;
			d.Roundness = 100.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.BoxTopLeft))
				return false;
		}
		{
			oWindowUIBox::DESC d;
			d.Size = int2(100, 70);
			d.Anchor = oTOPRIGHT;
			d.Color = std::Black;
			d.BorderColor = std::White;
			d.Roundness = 0.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.BoxTopRight))
				return false;
		}

		{
			oWindowUIBox::DESC d;
			d.Size = int2(70, 70);
			d.Anchor = oBOTTOMLEFT;
			d.Color = std::Blue;
			d.BorderColor = std::SkyBlue;
			d.Roundness = 10.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.BoxBottomLeft))
				return false;
		}

		{
			oWindowUIBox::DESC d;
			d.Size = int2(70, 70);
			d.Anchor = oBOTTOMRIGHT;
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
			d.Anchor = oMIDDLECENTER;
			d.Color = std::BurlyWood;
			d.BorderColor = std::Sienna;
			d.Roundness = 10.0f;
			if (!oWindowUIBoxCreate(d, _pWindow, &_Elements.Box))
				return false;
		}

		{
			oWindowUIFont::DESC d;
			d.FontName = "Tahoma";
			d.Style = oWindowUIFont::ITALIC;
			d.PointSize = 24.0f;
			d.ShadowOffset = 1.0f;
			if (!oWindowUIFontCreate(d, _pWindow, &_Elements.Font))
				return false;
		}

		{
			oWindowUIText::DESC d;
			d.Position = int2(oDEFAULT, oDEFAULT);
			d.Size = int2(400, 280);
			d.Anchor = oMIDDLECENTER;
			d.Alignment = oMIDDLECENTER;
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
			d.Anchor = oTOPCENTER;
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
			d.SupportDoubleClicks = InteractiveTest; // Support double clicks when running interact test to make sure double click events work correctly
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

		oWindow::STYLE Styles[] = 
		{
			oWindow::BORDERLESS,
			oWindow::FIXED,
			oWindow::DIALOG,
			oWindow::SIZEABLE,
		};

		oWindow::STATE States[] =
		{
			oWindow::HIDDEN,
			oWindow::RESTORED,
			oWindow::MINIMIZED,
			oWindow::MAXIMIZED,
			oWindow::FULLSCREEN,
		};

		oWindow::DESC d;
		d.ClientSize = int2(640, 480);
		d.CursorState = oWindow::WAIT_FOREGROUND;
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

				d.Style = static_cast<oWindow::STYLE>(style);
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

						if (States[state] == oWindow::FULLSCREEN)
							oSleep(500);
						else
							oSleep(200);

						// Hidden/Minimized destination states won't have any visuals to capture
						if (States[state] != oWindow::HIDDEN && States[state] != oWindow::MINIMIZED)
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

						if (States[state2] == oWindow::FULLSCREEN)
							oSleep(500);
						else
							oSleep(200);

						// Hidden/Minimized destination states won't have any visuals to capture
						if (States[state2] != oWindow::HIDDEN && States[state2] != oWindow::MINIMIZED)
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

struct TESTWindowD3D11 : public TESTWindowBase
{
	// @oooii-tony: NOTE: This is more of a bring-up test than a unit test at the 
	// moment. Enable this if there's a problem creating the oWindow with a user-
	// specified ID3D11Device

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

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		oD3D11_DEVICE_DESC DevDesc("TestDevice");
		DevDesc.MinimumAPIFeatureLevel = oVersion(10,0);
		DevDesc.Debug = true;

		oRef<ID3D11Device> D3D11Device;
		oTESTB0(oD3D11CreateDevice(DevDesc, &D3D11Device));

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

		Window->Hook(oBIND(&TESTWindowD3D11::Render, this, oBIND1, oBIND2, oBIND3, Window.c_ptr()));

		while (Window->IsOpen())
		{
			static int ctr = 0;

			oSleep(200);

			Window->Refresh(false);
		}

		return SUCCESS;
	}
};

#include <oPlatform/oWinWindowing.h>
#include <oPlatform/oWinAsString.h>
#include <oPlatform/oWinRect.h>

class oWindowUITest
{
public:

	oWindowUITest(bool* _pSuccess);
	~oWindowUITest();

	void Run();

	oDECLARE_WNDPROC(, WndProc);
	oDECLARE_WNDPROC(static, StaticWndProc);

private:

	enum
	{
		MENU_FILE_OPEN,
		MENU_FILE_EXIT,
		MENU_EDIT_CUT,
		MENU_EDIT_COPY,
		MENU_EDIT_PASTE,
		MENU_VIEW_SOLID,
		MENU_VIEW_WIREFRAME,
		MENU_HELP_ABOUT,
	};

	HWND hTopLevel;
	HMENU hMenu;
	HMENU hFileMenu;
	HMENU hEditMenu;
	HMENU hViewMenu;
	HMENU hHelpMenu;
	bool Running;
};

oWindowUITest::oWindowUITest(bool* _pSuccess)
	: hTopLevel(nullptr)
	, Running(true)
	, hMenu(oWinMenuCreate(true))
	, hFileMenu(oWinMenuCreate())
	, hEditMenu(oWinMenuCreate())
	, hViewMenu(oWinMenuCreate())
	, hHelpMenu(oWinMenuCreate())
{
	*_pSuccess = false;

	if (!oWinCreate(&hTopLevel, StaticWndProc, this, true))
		return; // pass through error

	oWINDOW_CONTROL_DESC ButtonDesc;
	ButtonDesc.hParent = hTopLevel;
	ButtonDesc.Type = oWINDOW_CONTROL_BUTTON;
	ButtonDesc.Text = "&Push Me";
	ButtonDesc.Position = int2(10,10);
	ButtonDesc.Dimensions = int2(100,20);
	ButtonDesc.ID = 0;
	ButtonDesc.StartsNewGroup = false;

	oWinControlCreate(ButtonDesc);

	oWinMenuAddSubmenu(hMenu, hFileMenu, "&File");
	oWinMenuAddMenuItem(hFileMenu, MENU_FILE_OPEN, "&Open...");
	oWinMenuAddSeparator(hFileMenu);
	oWinMenuAddMenuItem(hFileMenu, MENU_FILE_EXIT, "E&xit");
	oWinMenuAddSubmenu(hMenu, hEditMenu, "&Edit");
	oWinMenuAddMenuItem(hEditMenu, MENU_EDIT_CUT, "Cu&t");
	oWinMenuAddMenuItem(hEditMenu, MENU_EDIT_COPY, "&Copy");
	oWinMenuAddMenuItem(hEditMenu, MENU_EDIT_PASTE, "&Paste");
	oWinMenuAddSubmenu(hMenu, hViewMenu, "&View");
	oWinMenuAddMenuItem(hViewMenu, MENU_VIEW_SOLID, "&Solid");
	oWinMenuCheck(hViewMenu, MENU_VIEW_SOLID, true);
	oWinMenuAddMenuItem(hViewMenu, MENU_VIEW_WIREFRAME, "&Wireframe");
	oWinMenuAddSubmenu(hMenu, hHelpMenu, "&Help");
	oWinMenuAddMenuItem(hHelpMenu, MENU_HELP_ABOUT, "&About...");
	oWinMenuSet(hTopLevel, hMenu);

	RECT rDesktop;
	GetClientRect(GetDesktopWindow(), &rDesktop);
	RECT r = oWinRectResolve(rDesktop, int2(0,0), int2(640,480), oMIDDLECENTER, true);

	if (!oWinSetStyle(hTopLevel, oWINDOW_SIZEABLE, &r))
		return;

	*_pSuccess = true;
}

oWindowUITest::~oWindowUITest()
{
	if (hTopLevel)
		DestroyWindow(hTopLevel);
}

void oWindowUITest::Run()
{
	oWinSetState(hTopLevel, oWINDOW_RESTORED);

	bool More = false;
	while (Running || More)
		More = oWinProcessSingleMessage(hTopLevel, true);
}

//#define DEBUGGING_WINDOWS_MESSAGES
#ifdef DEBUGGING_WINDOWS_MESSAGES
	oDEFINE_WNDPROC_DEBUG(oWindowUITest, StaticWndProc);
#else
	oDEFINE_WNDPROC(oWindowUITest, StaticWndProc);
#endif

LRESULT oWindowUITest::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
		case WM_CLOSE:
			Running = false;
			PostQuitMessage(0);
			return 0;

		case WM_COMMAND:
			if (HIWORD(_wParam) == 0)
			{
				switch (LOWORD(_wParam))
				{
					case MENU_FILE_OPEN:
						break;
					case MENU_FILE_EXIT:
						PostMessage(_hWnd, WM_CLOSE, 0, 0);
						break;
					case MENU_EDIT_CUT:
						break;
					case MENU_EDIT_COPY:
						break;
					case MENU_EDIT_PASTE:
						break;
					case MENU_VIEW_SOLID:
						oWinMenuCheck(hViewMenu, MENU_VIEW_SOLID, true);
						oWinMenuCheck(hViewMenu, MENU_VIEW_WIREFRAME, false);
						oWinMenuEnable(hFileMenu, MENU_FILE_EXIT);
						break;
					case MENU_VIEW_WIREFRAME:
						oWinMenuCheck(hViewMenu, MENU_VIEW_SOLID, false);
						oWinMenuCheck(hViewMenu, MENU_VIEW_WIREFRAME, true);
						oWinMenuEnable(hFileMenu, MENU_FILE_EXIT, false);
						break;
					case MENU_HELP_ABOUT:
						break;
					default:
						break;
				}

				if (oWinMenuIsChecked(hViewMenu, MENU_VIEW_SOLID))
					oTRACE("View Solid");
				else if (oWinMenuIsChecked(hViewMenu, MENU_VIEW_WIREFRAME))
					oTRACE("View Wireframe");
				else
					oTRACE("View Nothing");

				oTRACE("Exit is %sabled", oWinMenuIsEnabled(hFileMenu, MENU_FILE_EXIT) ? "en" : "dis");
			}

			return 0;

		default:
			break;
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

struct TESTWindowUI : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		bool success = false;
		oWindowUITest test(&success);
		oTESTB0(success);
		test.Run();
		return SUCCESS;
	}
};

class oSystemProperties
{
	enum Controls
	{
		TAB,

		BT_OK,
		BT_CANCEL,
		BT_APPLY,

		AD_GB_PERF,
		AD_GB_USER,
		AD_GB_STARTUP,
		AD_LB_ADMIN,
		AD_LB_PERF,
		AD_LB_USER,
		AD_LB_STARTUP,
		AD_BT_PERF,
		AD_BT_USER,
		AD_BT_STARTUP,
		AD_BT_ENV,
	};

public:
	oSystemProperties(bool* _pSuccess);
	~oSystemProperties();

	oDECLARE_WNDPROC(, WndProc);
	oDECLARE_WNDPROC(static, StaticWndProc);

	void Run();

private:
	HWND hTopLevel;
	HWND hTab;
};

oSystemProperties::oSystemProperties(bool* _pSuccess)
{
	*_pSuccess = false;
	if (!oWinCreate(&hTopLevel, StaticWndProc, this, true))
		return;

	if (!oWinSetTitle(hTopLevel, "System Properties"))
		return;

	RECT r;
	r.left = 100;
	r.top = 100;
	r.right = 510;
	r.bottom = 540;
	if (!oWinSetStyle(hTopLevel, oWINDOW_DIALOG, &r))
		return;

	if (!oWinSetState(hTopLevel, oWINDOW_RESTORED))
		return;
	{
		oWINDOW_CONTROL_DESC ctl;
		ctl.hParent = hTopLevel;
		ctl.Type = oWINDOW_CONTROL_BUTTON_DEFAULT;
		ctl.Text = "OK";
		ctl.Position = int2(168,410);
		ctl.Dimensions = int2(74,23);
		ctl.ID = BT_OK;
		ctl.StartsNewGroup = false;
		oVERIFY(oWinControlCreate(ctl));

		ctl.Type = oWINDOW_CONTROL_BUTTON;
		ctl.Text = "Cancel";
		ctl.Position.x += ctl.Dimensions.x + 7;
		ctl.ID = BT_CANCEL;
		oVERIFY(oWinControlCreate(ctl));

		ctl.Text = "&Apply";
		ctl.Position.x += ctl.Dimensions.x + 7;
		ctl.ID = BT_APPLY;
		HWND hApply = oWinControlCreate(ctl);
		oVERIFY(oWinSetEnabled(hApply, false));
	}

	{
		oWINDOW_CONTROL_DESC ctl;
		ctl.hParent = hTopLevel;
		ctl.Type = oWINDOW_CONTROL_TAB;
		ctl.Text = "SysPropTabs";
		ctl.Position = int2(5,4);
		ctl.Dimensions = int2(400,400);
		ctl.ID = TAB;
		ctl.StartsNewGroup = false;
		hTab = oWinControlCreate(ctl);
		oVERIFY(oWinControlAddListItem(hTab, "Computer Name"));
		oVERIFY(oWinControlAddListItem(hTab, "Hardware"));
		oVERIFY(oWinControlAddListItem(hTab, "Advanced"));
		oVERIFY(oWinControlAddListItem(hTab, "System Protection"));
		oVERIFY(oWinControlAddListItem(hTab, "Remote"));

		oVERIFY(oWinControlSelect(hTab, oWinControlFindListItem(hTab, "Advanced")));
	}

	RECT rTab;
	GetClientRect(hTab, &rTab);

	oWINDOW_CONTROL_DESC ctl;
	ctl.hParent = hTab;
	ctl.Type = oWINDOW_CONTROL_GROUPBOX;
	ctl.Text = "Performance";
	ctl.Position = int2(rTab.left + 21,63);
	ctl.Dimensions = int2(oWinRectW(rTab) - 42, 85);
	ctl.ID = AD_GB_PERF;
	ctl.StartsNewGroup = false;
	oVERIFY(oWinControlCreate(ctl));

	ctl.Text = "User Profiles";
	ctl.Position.y += ctl.Dimensions.y + 8;
	ctl.ID = AD_GB_USER;
	oVERIFY(oWinControlCreate(ctl));

	ctl.Text = "Startup and Recovery";
	ctl.Position.y += ctl.Dimensions.y + 8;
	ctl.ID = AD_GB_STARTUP;
	oVERIFY(oWinControlCreate(ctl));

	ctl.Type = oWINDOW_CONTROL_LABEL;
	ctl.Position = int2(rTab.left + 21,33);
	ctl.Dimensions = int2(oWinRectW(rTab) - 42, 20);
	ctl.Text = "You must be logged on as an Administrator to make most of these changes.";
	ctl.ID = AD_LB_ADMIN;
	oVERIFY(oWinControlCreate(ctl));

	*_pSuccess = true;
}

oSystemProperties::~oSystemProperties()
{
	if (hTopLevel)
		DestroyWindow(hTopLevel);
}

void oSystemProperties::Run()
{
	while (oWinProcessSingleMessage(hTopLevel));
}

//#define DEBUGGING_WINDOWS_MESSAGES
#ifdef DEBUGGING_WINDOWS_MESSAGES
oDEFINE_WNDPROC_DEBUG(oSystemProperties, StaticWndProc);
#else
oDEFINE_WNDPROC(oSystemProperties, StaticWndProc);
#endif

LRESULT oSystemProperties::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		case WM_NOTIFY:
		{
			const NMHDR& n = *(NMHDR*)_lParam;
			if (n.hwndFrom == hTab && n.idFrom == TAB)
			{
				oTRACE("TCN Code = %s", oWinAsStringTCN(n.code));
			}

			break;
		}

		case WM_COMMAND:
		{
 			switch (LOWORD(_wParam))
			{
				case BT_OK: PostQuitMessage(0); break;
				case BT_CANCEL: PostQuitMessage(0); break;
				case BT_APPLY: oWinSetEnabled((HWND)_lParam, false); break;
				case AD_GB_PERF: break;
				case AD_GB_USER: break;
				case AD_GB_STARTUP: break;
				case AD_LB_ADMIN: break;
				case AD_LB_PERF: break;
				case AD_LB_USER: break;
				case AD_LB_STARTUP: break;
				case AD_BT_PERF: break;
				case AD_BT_USER: break;
				case AD_BT_STARTUP: break;
				case AD_BT_ENV: break;
				default:
					break;
			}

			if (LOWORD(_wParam) != BT_APPLY)
				oWinSetEnabled((HWND)_lParam, true);
			break;
		}

	default:
		break;
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

struct TESTWindowUISystemProperties : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		bool success = false;
		oSystemProperties test(&success);
		oTESTB0(success);
		test.Run();
		return SUCCESS;
	}
};

class WindowInWindow
{
public:
	WindowInWindow(bool* _pSuccess)
		: hTopLevel(nullptr)
		, Running(true)
	{
		*_pSuccess = false;

		if (!oWinCreate(&hTopLevel, StaticWndProc, this, true))
			return; // pass through error

		RECT rDesktop;
		GetClientRect(GetDesktopWindow(), &rDesktop);
		RECT r = oWinRectResolve(rDesktop, int2(0,0), int2(640,480), oMIDDLECENTER, true);

		oWinSetTitle(hTopLevel, "Window In Window Test");

		if (!oWinSetStyle(hTopLevel, oWINDOW_SIZEABLE, &r))
			return;

		oD3D11_DEVICE_DESC DevDesc("TestDevice");
		DevDesc.MinimumAPIFeatureLevel = oVersion(10,0);
		DevDesc.Debug = true;

		oRef<ID3D11Device> D3D11Device;
		oVERIFY(oD3D11CreateDevice(DevDesc, &D3D11Device));

		oWindow::DESC WinDesc;
		WinDesc.Style = oWindow::BORDERLESS;
		WinDesc.ClientPosition = int2(20,20);
		WinDesc.ClientSize = int2(100,100);

		if (!oWindowCreate(WinDesc, D3D11Device, oWindow::USE_GDI, &MediaWindow))
			return;

		MediaWindow->Hook(oBIND(&WindowInWindow::Render, this, oBIND1, oBIND2, oBIND3, MediaWindow.c_ptr()));

		//SetWindowLongPtr((HWND)MediaWindow->GetNativeHandle(), GWLP_HWNDPARENT, (LONG_PTR)hTopLevel.c_ptr());

		SetParent((HWND)MediaWindow->GetNativeHandle(), hTopLevel);

		oWINDOW_CONTROL_DESC ButtonDesc;
		ButtonDesc.hParent = hTopLevel;
		ButtonDesc.Type = oWINDOW_CONTROL_BUTTON;
		ButtonDesc.Text = "&Push Me";
		ButtonDesc.Position = int2(120,20);
		ButtonDesc.Dimensions = int2(100,25);
		ButtonDesc.ID = 0;
		ButtonDesc.StartsNewGroup = false;

		oWinControlCreate(ButtonDesc);

		*_pSuccess = true;
	}

	~WindowInWindow()
	{
		SetParent((HWND)MediaWindow->GetNativeHandle(), nullptr);
	}

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

	void Run()
	{
		oWinSetState(hTopLevel, oWINDOW_RESTORED);
		bool More = false;
		double time = oTimer();
		while (Running || More)
		{
			More = oWinProcessSingleMessage(hTopLevel, true);

			if (time < (oTimer() + 0.2))
				MediaWindow->Refresh(false);
		}
	}

	oDECLARE_WNDPROC(, WndProc);
	oDECLARE_WNDPROC(static, StaticWndProc);

private:
	oRef<oWindow> MediaWindow;
	oScopedHWND hTopLevel;
	bool Running;
};

//#define DEBUGGING_WINDOWS_MESSAGES
#ifdef DEBUGGING_WINDOWS_MESSAGES
	oDEFINE_WNDPROC_DEBUG(WindowInWindow, StaticWndProc);
#else
	oDEFINE_WNDPROC(WindowInWindow, StaticWndProc);
#endif

LRESULT WindowInWindow::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
		case WM_SIZE:
			if (MediaWindow)
				MediaWindow->Refresh();
			break;

		case WM_CLOSE:
			Running = false;
			PostQuitMessage(0);
			return 0;
		default:
			break;
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

struct TESTWindowInWindow : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		bool success = false;
		WindowInWindow test(&success);
		oTESTB0(success);
		test.Run();
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTWindowD2D);
oTEST_REGISTER(TESTWindowGDI);
//oTEST_REGISTER(TESTWindowD3D11);
//oTEST_REGISTER(TESTWindowUI);
//oTEST_REGISTER(TESTWindowUISystemProperties);
//oTEST_REGISTER(TESTWindowInWindow);
