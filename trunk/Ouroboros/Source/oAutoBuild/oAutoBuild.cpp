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
#include "oP4ChangelistBuilder.h"
#include <oPlatform/oWebAppWindow.h>
#include <oPlatform/Windows/oWindows.h>
#include <oBasis/oINISerialize.h>
#include <oBasis/oJSONSerialize.h>
#include <oBasis/oURIQuerySerialize.h>
#include <oPlatform/oHTTPHandler.h>
#include <oPlatform/oWebServer.h>
#include <oPlatform/oStandards.h>

// @oooii-jeffrey: If other code needs these, move them to a better place
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, oNetHost)
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, oNetAddr)
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, oNetHost, oNetHost, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, oNetAddr, oNetAddr, 1)

struct oBUILD_TOOL_SERVER_SETTINGS
{
	oBUILD_TOOL_SERVER_SETTINGS()
		: Port(80)
		, StaticBaseURI("file://DATA/Apps/AutoBuild/")
		, DailyBuildHour(oInvalid)
		, NewBuildCheckSeconds(60)
	{}
	unsigned short Port;
	oNetHost PublicIP;
	oStd::uri_string StaticBaseURI; //path to all static html, js, and css files.;
	oStd::uri_string BuildLogsURI;
	int DailyBuildHour;
	uint NewBuildCheckSeconds;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oBUILD_TOOL_SERVER_SETTINGS)
	
oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oBUILD_TOOL_SERVER_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oBUILD_TOOL_SERVER_SETTINGS)
	oRTTI_COMPOUND_VERSION(oBUILD_TOOL_SERVER_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oBUILD_TOOL_SERVER_SETTINGS)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_SERVER_SETTINGS, Port, oRTTI_OF(ushort), "Port", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_SERVER_SETTINGS, PublicIP, oRTTI_OF(oNetHost), "PublicIP", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_SERVER_SETTINGS, StaticBaseURI, oRTTI_OF(ostd_uri_string), "StaticBaseURI", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_SERVER_SETTINGS, BuildLogsURI, oRTTI_OF(ostd_uri_string), "BuildLogsURI", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_SERVER_SETTINGS, DailyBuildHour, oRTTI_OF(int), "DailyBuildHour", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_SERVER_SETTINGS, NewBuildCheckSeconds, oRTTI_OF(uint), "NewBuildCheckSeconds", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oBUILD_TOOL_SERVER_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oBUILD_TOOL_SERVER_SETTINGS)

class oAutoBuildHandler : public oHTTPHandler
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oAutoBuildHandler(const oP4ChangelistBuilder* _pCLBuilder)
		: CLBuilder(_pCLBuilder)
	{}

	void OnPost(CommonParams& _CommonParams, const oHTTP_CONTENT_BODY& _Content) const override { oHTTPHandlerMethodNotAllowed(_CommonParams); }
	void OnPut(CommonParams& _CommonParams, const oHTTP_CONTENT_BODY& _Content) const override { oHTTPHandlerMethodNotAllowed(_CommonParams); }
	void OnDelete(CommonParams& _CommonParams) const override { oHTTPHandlerMethodNotAllowed(_CommonParams); }

protected:
	const oStd::intrusive_ptr<oP4ChangelistBuilder> CLBuilder;
	oRefCount RefCount;
};

class oAutoBuildPendingHandler : public oAutoBuildHandler
{
public:
	oAutoBuildPendingHandler(oP4ChangelistBuilder* _pCLBuilder)
		: oAutoBuildHandler(_pCLBuilder)
	{}
	oStd::lstring HandlesPath() const override { return "pending"; }

	void OnGet(CommonParams& _CommonParams) const override;
};

struct oAutoBuild_Pending
{
	int changelist;
	oStd::sstring user;
	oStd::sstring date;
	int remainingms;
	int progress;
	oStd::sstring stage;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_ARRAY, oAutoBuild_Pending)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oAutoBuild_Pending)
	oRTTI_COMPOUND_ABSTRACT(oAutoBuild_Pending)
	oRTTI_COMPOUND_VERSION(oAutoBuild_Pending, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oAutoBuild_Pending)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Pending, changelist, oRTTI_OF(int), "changelist", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Pending, user, oRTTI_OF(ostd_sstring), "user", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Pending, date, oRTTI_OF(ostd_sstring), "date", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Pending, remainingms, oRTTI_OF(int), "remainingms", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Pending, progress, oRTTI_OF(int), "progress", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Pending, stage, oRTTI_OF(ostd_sstring), "stage", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oAutoBuild_Pending)
oRTTI_COMPOUND_END_DESCRIPTION(oAutoBuild_Pending)

void oAutoBuildPendingHandler::OnGet(CommonParams& _CommonParams) const 
{
	std::vector<oAutoBuild_Pending> PendingTasks;

	CLBuilder->ReportWorking([&](const oP4ChangelistBuilder::ChangeInfo& _Change, int _RemainingMS, int _PercentageDone)
	{
		oAutoBuild_Pending PendingTask;
		PendingTask.changelist =  _Change.CL;
		PendingTask.user = _Change.UserName;
		PendingTask.date = _Change.Date;
		PendingTask.remainingms = _RemainingMS > 0 ? _RemainingMS : 0;
		PendingTask.progress = _PercentageDone;
		PendingTask.stage = _Change.Stage;
			
		PendingTasks.push_back(PendingTask);
	});

	oStd::xxlstring PendingTasksJSON;
	oJSONWriteContainer(PendingTasksJSON.c_str(), PendingTasksJSON.capacity(), &PendingTasks, sizeof(PendingTasks), oRTTI_OF(std_vector_oAutoBuild_Pending));
	
	_CommonParams.AllocateResponse(PendingTasksJSON.length());
	memcpy(_CommonParams.pResponse->Content.pData, PendingTasksJSON.c_str(), PendingTasksJSON.length());
	_CommonParams.pResponse->StatusLine.StatusCode = oHTTP_OK;
	_CommonParams.pResponse->Content.Type = oMIME_APPLICATION_JSON;
}

class oAutoBuildCompletedHandler : public oAutoBuildHandler
{
public:
	oAutoBuildCompletedHandler(oP4ChangelistBuilder* _pCLBuilder)
		: oAutoBuildHandler(_pCLBuilder)
	{}
	oStd::lstring HandlesPath() const override { return "completed"; }

	void OnGet(CommonParams& _CommonParams) const override;
};

struct oAutoBuild_Completed
{
	int changelist;
	bool success;
	oStd::sstring user;
	oStd::sstring date;
	oStd::uri_string HTML;
	int build_time;
	int test_time;
	int pack_time;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_ARRAY, oAutoBuild_Completed)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oAutoBuild_Completed)
	oRTTI_COMPOUND_ABSTRACT(oAutoBuild_Completed)
	oRTTI_COMPOUND_VERSION(oAutoBuild_Completed, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oAutoBuild_Completed)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Completed, changelist, oRTTI_OF(int), "changelist", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Completed, success, oRTTI_OF(bool), "success", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Completed, user, oRTTI_OF(ostd_sstring), "user", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Completed, date, oRTTI_OF(ostd_sstring), "date", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Completed, HTML, oRTTI_OF(ostd_uri_string), "HTML", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Completed, build_time, oRTTI_OF(int), "build_time", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Completed, test_time, oRTTI_OF(int), "test_time", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Completed, pack_time, oRTTI_OF(int), "pack_time", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oAutoBuild_Completed)
oRTTI_COMPOUND_END_DESCRIPTION(oAutoBuild_Completed)

void oAutoBuildCompletedHandler::OnGet(CommonParams& _CommonParams) const 
{
	std::vector<oAutoBuild_Completed> CompletedTasks;

	CLBuilder->ReportBuilt([&](const std::list<oP4ChangelistBuilder::ChangeInfo> & _FinishedBuildInfos)
	{
		int count = 1;
		for(auto FinishedBuildInfo = _FinishedBuildInfos.begin(); FinishedBuildInfo != _FinishedBuildInfos.end(); ++FinishedBuildInfo)
		{
			oAutoBuild_Completed CompletedTask;
			CompletedTask.changelist = FinishedBuildInfo->CL;
			CompletedTask.user = FinishedBuildInfo->UserName;
			CompletedTask.date = FinishedBuildInfo->Date;
			CompletedTask.success = FinishedBuildInfo->Succeeded;
			CompletedTask.build_time = FinishedBuildInfo->BuildTime;
			CompletedTask.test_time = FinishedBuildInfo->TestTime;
			CompletedTask.pack_time = FinishedBuildInfo->PackTime;
			oPrintf(CompletedTask.HTML, "/logs/%d/index.html", FinishedBuildInfo->CL);
			CompletedTasks.push_back(CompletedTask);
			if (count++ == 10) break;
		}
	});

	oStd::xxlstring CompletedTasksJSON;
	oJSONWriteContainer(CompletedTasksJSON.c_str(), CompletedTasksJSON.capacity(), &CompletedTasks, sizeof(CompletedTasks), oRTTI_OF(std_vector_oAutoBuild_Completed));

	_CommonParams.AllocateResponse(CompletedTasksJSON.length());
	memcpy(_CommonParams.pResponse->Content.pData, CompletedTasksJSON.c_str(), CompletedTasksJSON.length());
	_CommonParams.pResponse->StatusLine.StatusCode = oHTTP_OK;
	_CommonParams.pResponse->Content.Type = oMIME_APPLICATION_JSON;
}

class oAutoBuildDailyHandler : public oAutoBuildHandler
{
public:
	oAutoBuildDailyHandler(oP4ChangelistBuilder* _pCLBuilder)
		: oAutoBuildHandler(_pCLBuilder)
	{}
	oStd::lstring HandlesPath() const override { return "daily"; }

	void OnGet(CommonParams& _CommonParams) const override;
};
struct DailyQuery
{
	DailyQuery()
		: last(false)
	{}
	bool last;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, DailyQuery)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, DailyQuery)
	oRTTI_COMPOUND_ABSTRACT(DailyQuery)
	oRTTI_COMPOUND_VERSION(DailyQuery, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(DailyQuery)
		oRTTI_COMPOUND_ATTR(DailyQuery, last, oRTTI_OF(bool), "last", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(DailyQuery)
oRTTI_COMPOUND_END_DESCRIPTION(DailyQuery)

struct oAutoBuild_Daily
{
	bool success;
	oStd::sstring name;
	oStd::sstring HTML;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oAutoBuild_Daily)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oAutoBuild_Daily)
	oRTTI_COMPOUND_ABSTRACT(oAutoBuild_Daily)
	oRTTI_COMPOUND_VERSION(oAutoBuild_Daily, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oAutoBuild_Daily)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Daily, success, oRTTI_OF(bool), "success", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Daily, name, oRTTI_OF(ostd_sstring), "name", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuild_Daily, HTML, oRTTI_OF(ostd_sstring), "HTML", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oAutoBuild_Daily)
oRTTI_COMPOUND_END_DESCRIPTION(oAutoBuild_Daily)

void oAutoBuildDailyHandler::OnGet(CommonParams& _CommonParams) const 
{
	DailyQuery queryParams;
	oURIQueryReadCompound(&queryParams, oRTTI_OF(DailyQuery), _CommonParams.Query, false);

	oAutoBuild_Daily Daily;

	CLBuilder->ReportLastSpecialBuild([&](const char* _pName, bool _Success, const char* _pLastSuccesful)
	{
		const char* pName = nullptr;
		bool Success = true;

		if(queryParams.last)
		{
			pName = _pLastSuccesful;
		}
		else 
		{
			pName = _pName;
			Success = _Success;
		}

		if(pName)
		{
			Daily.success =  Success;
			Daily.name = pName;
			oPrintf(Daily.HTML, "/logs/%s/index.html", pName);
		}
	});

	oStd::xlstring DailyJSON;
	oJSONWriteCompound(DailyJSON.c_str(), DailyJSON.capacity(), &Daily, oRTTI_OF(oAutoBuild_Daily));

	_CommonParams.AllocateResponse(DailyJSON.length());
	memcpy(_CommonParams.pResponse->Content.pData, DailyJSON.c_str(), DailyJSON.length());
	_CommonParams.pResponse->StatusLine.StatusCode = oHTTP_OK;
	_CommonParams.pResponse->Content.Type = oMIME_APPLICATION_JSON;
}

class oAutoBuildLogHandler : public oAutoBuildHandler
{
public:
	oAutoBuildLogHandler(oP4ChangelistBuilder* _pCLBuilder, oStd::uri_string _BuildLogsURI)
		: oAutoBuildHandler(_pCLBuilder)
		, BuildLogsURI(_BuildLogsURI)
	{ }

	// Because the BuildLogsURI can be outside of the WebRoot, we add a virtual
	// path handler and serve content from that folder that way.
	oStd::lstring HandlesPath() const override { return "logs/?remaining"; }

	void OnGet(CommonParams& _CommonParams) const override;

	oStd::uri_string BuildLogsURI;
};

void oAutoBuildLogHandler::OnGet(CommonParams& _CommonParams) const
{
	oStd::uri_string filepath;
	_CommonParams.GetCaptured(&filepath);

	oStd::uri_string filepathAbsolute = BuildLogsURI;
	oEnsureSeparator(filepathAbsolute);
	oStrcat(filepathAbsolute, filepath.c_str());

	// TODO: Look into using the FileCache for this as well (oWebServer is also
	// using it, so perhaps there is a way to share that)
	oStd::intrusive_ptr<oBuffer> cacheBuffer;
	if (!oBufferLoad(filepathAbsolute, &cacheBuffer))
		return;

	oStd::path P(filepathAbsolute);

	oMIMEFromExtension(&_CommonParams.pResponse->Content.Type, P.extension());
	_CommonParams.pResponse->Content.Length = oInt(cacheBuffer->GetSize());
	_CommonParams.pResponse->StatusLine.StatusCode = oHTTP_OK;
 	_CommonParams.AllocateResponse(cacheBuffer->GetSize());
 	memcpy(_CommonParams.pResponse->Content.pData, cacheBuffer->GetData(), cacheBuffer->GetSize());
}

class oAutoBuildCaptureRemaining : public oHTTPURICapture
{
public:
	oAutoBuildCaptureRemaining() { }
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	// TODO: This seems like an over engineered way of handling URI paths to 
	// determine who handles what (and parsing it at the same time). 
	oStd::sstring GetCaptureName() const override { return "remaining"; }
	bool AttemptCapture(const char* _URI, const char** _Remaining) const override;

private:
	oRefCount RefCount;
};	

bool oAutoBuildCaptureRemaining::AttemptCapture(const char* _URI, const char** _Remaining) const 
{
	oStd::uri_string capture;
	oStd::trim_right(capture, _URI, "/");
	*_Remaining = _URI + capture.length();

	return true;
}

void OnNewVersion(oSTREAM_EVENT _Event, const oStd::uri_string& _ChangedURI, oWebAppWindow* _pAppWindow)
{
	static bool NewVersionFound = false;
	switch (_Event)
	{
	case oSTREAM_ACCESSIBLE:
		{
			if (!NewVersionFound && oVUIsUpdateInstallerValid(_ChangedURI))
			{
				NewVersionFound = true;

				if (oVUUnzip(_ChangedURI))
				{
					if (!oVULaunchLauncher(60000))
						oTRACE("%s", oErrorGetLastString());

					oTRACE("Exiting current version...");
					_pAppWindow->Close();
				}
				else
					oTRACE("%s", oErrorGetLastString());

				NewVersionFound = false;
			}
			else
				oTRACE("%s", oErrorGetLastString());
		}

	default:
		break;
	}
}

oMAINA()
{
	if (!oP4IsAvailable()) 
	{
		oMSGBOX_DESC mb;
		mb.Type = oMSGBOX_WARN;
		oMsgBox(mb, "SCC is not available.  Shutting down.");
		return -1;
	}

	oStd::path_string IniPath;
	if (!oINIFindPath(IniPath, "oAutoBuild.ini"))
		return false; // Pass error

	std::shared_ptr<oStd::ini> INI = oINILoad(IniPath);
	if (!INI)
		return false; // PassError

	oBUILD_TOOL_SERVER_SETTINGS Settings;
	oStd::ini::section section = INI->first_section();
	while (section)
	{
		const char* pSectionName = INI->section_name(section);
		if (0 == oStricmp(pSectionName, "Server"))
		{
			oINIReadCompound(&Settings, oRTTI_OF(oBUILD_TOOL_SERVER_SETTINGS), *INI, section, false);
			break;
		}
		section = INI->next_section(section);
	}

	oStd::intrusive_ptr<oWebAppWindow> Window;
	if (!oWebAppWindowCreate("oAutoBuild", Settings.Port, &Window))
		return -1;

	oStd::path logroot = oStd::uri(Settings.BuildLogsURI).path();
	oCore::filesystem::create_directories(logroot.parent_path());
	
	oStd::intrusive_ptr<oP4ChangelistBuilder> CLManager;
	if (!oChangelistManagerCreate(*INI, logroot.c_str(), Settings.Port, &CLManager))
	{
		oMSGBOX_DESC mb;
		mb.Type = oMSGBOX_WARN;
		oMsgBox(mb, "Invalid settings: %s", oErrorGetLastString());
		return -1;
	}

	oStd::intrusive_ptr<oHTTPHandler> Handlers[4];
	Handlers[0] = oStd::intrusive_ptr<oHTTPHandler>(new oAutoBuildCompletedHandler(CLManager), false);
	Handlers[1] = oStd::intrusive_ptr<oHTTPHandler>(new oAutoBuildPendingHandler(CLManager), false);
	Handlers[2] = oStd::intrusive_ptr<oHTTPHandler>(new oAutoBuildDailyHandler(CLManager), false);
	Handlers[3] = oStd::intrusive_ptr<oHTTPHandler>(new oAutoBuildLogHandler(CLManager, Settings.BuildLogsURI), false);

	oStd::intrusive_ptr<oHTTPURICapture> Captures[1];
	Captures[0] = oStd::intrusive_ptr<oHTTPURICapture>(new oAutoBuildCaptureRemaining(), false);

	oStd::intrusive_ptr<threadsafe oWebServer> WebServer;
	oStd::intrusive_ptr<oHTTPServer> BuildServer;

	oWebServer::DESC ServerDesc;
	ServerDesc.AllocBufferCallback = [](size_t _BufferSize) { return new char[_BufferSize]; };
	ServerDesc.DefaultURIReference = "index.html";
	ServerDesc.URIBase = Settings.StaticBaseURI;

	// 1) Create the web server that will handle all app specific requests
	if (!oWebServerCreate(ServerDesc, &WebServer))
	{
		oMSGBOX_DESC mb;
		mb.Type = oMSGBOX_WARN;
		oMsgBox(mb, "Couldn't start build server: %s", oErrorGetLastString());
		return -1;
	}

	// 2) Hook up our handlers
	for(int i = 0; i < oCOUNTOF(Handlers); ++i)
	{
		auto Handler = Handlers[i];
		WebServer->AddHTTPHandler(Handler);
	}
	for(int i = 0; i < oCOUNTOF(Captures); ++i)
	{
		auto Capturer = Captures[i];
		WebServer->AddURICaptureHandler(Capturer);
	}

	// 3) Start the server
	{
		oHTTPServer::DESC HTTPServerDesc;
		HTTPServerDesc.Port = Settings.Port;
		HTTPServerDesc.SupportedMethods = oHTTP_GET;
		HTTPServerDesc.StartResponse = [&](const oHTTP_REQUEST& _Request, const oNetHost& _Client, oHTTP_RESPONSE* _pResponse)
		{
			WebServer->Retrieve(_Request, _pResponse);
		};

		HTTPServerDesc.FinishResponse = [=](const void* _pContentBody)
		{
			delete _pContentBody;
		};

		if (!oHTTPServerCreate(HTTPServerDesc, &BuildServer))
		{
			oMSGBOX_DESC mb;
			mb.Type = oMSGBOX_WARN;
			oMsgBox(mb, "Couldn't start build server: %s", oErrorGetLastString());
			return -1;
		}
	}

	// 4) Auto updater
	oStd::path MonitorPath = oCore::filesystem::app_path(true);
	MonitorPath.replace_filename("../*.exe");

	oSTREAM_MONITOR_DESC md;
	md.Monitor = MonitorPath;
	md.TraceEvents = true;
	md.WatchSubtree = false;

	oStd::intrusive_ptr<threadsafe oStreamMonitor> NewVersionMonitor;
	if (!oStreamMonitorCreate(md, oBIND(OnNewVersion, oBIND1, oBIND2, Window), &NewVersionMonitor))
	{
		oMSGBOX_DESC mb;
		mb.Type = oMSGBOX_WARN;
		oMsgBox(mb, "Error starting OnNewVersion monitor.\n%s", oErrorGetLastString());
	}


	int WorkCountUILast = -1;
	uint LastP4CheckMS = 0;
	Window->Show();
	while(Window->IsRunning())
	{
		Window->FlushMessages();

		// Update UI
		int WorkCount = CLManager->GetCount();
		if (WorkCountUILast != WorkCount)
		{
			WorkCountUILast = WorkCount;
			Window->SetCurrentJobCount(WorkCountUILast);
		}

		// After the specified timeout try to build the next build
		uint CurrentTimeMS = oTimerMS();
		if ((CurrentTimeMS - LastP4CheckMS) > (Settings.NewBuildCheckSeconds * 1000))
		{
			LastP4CheckMS = CurrentTimeMS;

			CLManager->TryNextBuild(Settings.DailyBuildHour);
		}

		CLManager->MainThreadYield(500);
	}
	CLManager->Cancel();
	return 0;
} 
