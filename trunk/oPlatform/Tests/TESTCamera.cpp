// $(header)

// @oooii-tony: I'm not sure how to automatically test a camera since it could
// be looking at anything and that's even if there's a camera attached to the
// computer. So include this function that enumerates all attached cameras and
// opens an oWindow for each with its video stream for verification/iteration.

#include <oPlatform/oCamera.h>
#include <oBasis/oRef.h>
#include <oPlatform/oMsgBox.h>
#include <oBasis/oSize.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oWindow.h>
#include <oPlatform/oWindowUI.h>

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
		oRef<threadsafe oWindowUIPicture> Picture;
		oRef<threadsafe oWindowUIFont> Font;
		oRef<threadsafe oWindowUIText> Text;
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
			mode.Size = int2(640, 480);
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
				oMsgBox(d, "Camera %s does not support mode %s %dx%d", Camera->GetName(), oAsString(mode.Format), mode.Size.x, mode.Size.y);
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

		oWindow::DESC d;
		d.BackgroundSleepMS = 0;
		d.ClientSize = cd.Mode.Size;
		d.ClientPosition = int2(30, 30) * int2(oSize32(i + 1), oSize32(i + 1));
		oVERIFY(oWindowCreate(d, nullptr, oWindow::USE_DEFAULT, &Contexts[i].Window));

		oStringM Title;
		sprintf_s(Title, "%s (%dx%d %s)", Contexts[i].Camera->GetName(), cd.Mode.Size.x, cd.Mode.Size.y, oAsString(cd.Mode.Format));
		Contexts[i].Window->SetTitle(Title);

		oWindowUIPicture::DESC pd;
		pd.SurfaceDesc.Dimensions.x = cd.Mode.Size.x;
		pd.SurfaceDesc.Dimensions.y = cd.Mode.Size.y;
		pd.SurfaceDesc.Dimensions.z = 1;
		pd.SurfaceDesc.RowPitch = cd.Mode.Size.x * oSurfaceGetSize(cd.Mode.Format); // @oooii-tony: Is this always true? no alignment?
		pd.SurfaceDesc.Format = cd.Mode.Format;
		pd.SurfaceDesc.NumMips = 1;

		oVERIFY(oWindowUIPictureCreate(pd, Contexts[i].Window, &Contexts[i].Picture));

		oWindowUIFont::DESC fd;
		oVERIFY(oWindowUIFontCreate(fd, Contexts[i].Window, &Contexts[i].Font));

		oWindowUIText::DESC td;
		td.Anchor = oTOPLEFT;
		td.Alignment = oMIDDLELEFT;
		td.Color = std::OOOiiGreen;
		td.Position = int2(10,10);
		td.Size = int2(150,0);
		oVERIFY(oWindowUITextCreate(td, Contexts[i].Window, &Contexts[i].Text));
		Contexts[i].Text->SetFont(Contexts[i].Font);
	}

	bool aWindowIsOpen = false;
	for (size_t i = 0; i < Contexts.size(); i++)
		if (Contexts[i].Window->IsOpen())
			aWindowIsOpen = true;

	while (aWindowIsOpen)
	{
		for (size_t i = 0; i < Contexts.size(); i++)
		{
			oCamera::MAPPED mapped;
			if (Contexts[i].Camera->Map(&mapped))
			{
				if (mapped.Frame != Contexts[i].LastFrame)
				{
					Contexts[i].Picture->Copy(mapped.pData, mapped.RowPitch);
					Contexts[i].LastFrame = mapped.Frame;

					float fps = Contexts[i].Camera->GetFPS();
					oStringS sFPS;
					sprintf_s(sFPS, "FPS: %.01f", fps + (rand() %100) / 100.0f);
					Contexts[i].Text->SetText(sFPS);
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

struct TESTCamera : oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		return 0 == ShowAllCameras() ? oTest::SUCCESS : oTest::FAILURE;
	}
};

//oTEST_REGISTER(TESTColorSpaceCPU);
