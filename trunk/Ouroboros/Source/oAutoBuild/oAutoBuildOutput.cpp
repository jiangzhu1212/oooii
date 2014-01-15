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
#include "oAutoBuildOutput.h"
#include "oMSBuild.h"
#include <oPlatform/oEMail.h>
#include <oGUI/msgbox.h>

using namespace ouro;

// TODO: Where to put this?
oRTTI_ATOM_DECLARATION(oRTTI_CAPS_ARRAY, oNetAddr)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oAutoBuildEmailSettings)
	oRTTI_COMPOUND_ABSTRACT(oAutoBuildEmailSettings)
	oRTTI_COMPOUND_VERSION(oAutoBuildEmailSettings, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oAutoBuildEmailSettings)
		oRTTI_COMPOUND_ATTR(oAutoBuildEmailSettings, EmailServer, oRTTI_OF(oNetAddr), "EmailServer", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuildEmailSettings, FromAddress, oRTTI_OF(ouro_sstring), "FromAddress", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuildEmailSettings, FromPasswordBase64, oRTTI_OF(ouro_sstring), "FromPasswordBase64", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuildEmailSettings, AdminEmails, oRTTI_OF(std_vector_ouro_sstring), "AdminEmails", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oAutoBuildEmailSettings, UserEmails, oRTTI_OF(std_vector_ouro_sstring), "UserEmails", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oAutoBuildEmailSettings)
oRTTI_COMPOUND_END_DESCRIPTION(oAutoBuildEmailSettings)


const char* oBUILD_TOOL_STANDARD_SUBJECT = "oAutoBuild:";

inline void oHTMLLink(std::string& _HTML, const char* _pLink, const char* _pLinkText)
{
	_HTML.append("<a href=\"");
	_HTML.append(_pLink);
	_HTML.append("\">");
	_HTML.append(_pLinkText);
	_HTML.append("</a>");
}

static void FormatMSBuildResults(const oMSBuildResults& _Results, std::string& _LogHTML)
{
	_LogHTML.append("<p><b>Building stage...</b></p>");
	if (!_Results.CleanSucceeded)
	{
		_LogHTML.append("<p>Build failed due to clean failing.</p>");
	}

	mstring BuildStatus;
	if (_Results.BuildSucceeded)
		BuildStatus = "succeeded";
	else
		BuildStatus = _Results.BuildTimedOut ? "timed out" : "failed";

	// Note that this string gets parsed to determine build times and status
	// so preferably don't change it.
	mstring BuildTimeString;
	snprintf(BuildTimeString, "<p>Build %s, taking %.2f seconds.  Here are the logs:</p>", BuildStatus.c_str(), _Results.BuildTimeSeconds);
	_LogHTML.append(BuildTimeString);

	for (auto& BuildLogfile : _Results.BuildLogfiles)
	{
		sstring BuildName = path(BuildLogfile).basename().c_str();
		_LogHTML.append("<p>");
		oHTMLLink(_LogHTML, BuildName, BuildName);
		if (_Results.BuildSucceeded)
			_LogHTML.append("</p>");
		else
		{
			bool FirstWarningOrError = true;
			oMSBuildParseLogfile(BuildLogfile, true, [&](o_msbuild_stdout_t _WarningOrError)->bool
			{
				if (FirstWarningOrError)
					_LogHTML.append(" failed with the following warnings and/or errors:</p>");
				FirstWarningOrError = false;

				o_msbuild_stdout_t encodedWarningOrError;
				ampersand_encode(encodedWarningOrError.c_str(), encodedWarningOrError.capacity(), _WarningOrError.c_str());

				_LogHTML.append("<p>");
				_LogHTML.append(encodedWarningOrError);
				_LogHTML.append("</p>");
				return true;
			});

			if (FirstWarningOrError)
				_LogHTML.append(" succeeded</p>");
		}
	}
}

static void FormatUnitTestResults(const oUnitTestResults& results, std::string& _LogHTML)
{
	_LogHTML.append("<p><b>Testing stage...</b></p>");

	mstring TestingStatus;
	if (results.TestingSucceeded)
		TestingStatus = "succeeded";
	else
		TestingStatus = results.HasTimedOut ? "timed out" : "failed";

	// Note that this string gets parsed to determine build times and status
	// so preferably don't change it.
	mstring TestTimeString;
	snprintf(TestTimeString, "<p>Testing %s, taking %.2f seconds.  Here are the logs:</p>", TestingStatus.c_str(), results.TimePassedSeconds);
	_LogHTML.append(TestTimeString);
	
	_LogHTML.append("<p>");
	path stdoutLogfile = path(results.StdoutLogfile).filename();
	oHTMLLink(_LogHTML, stdoutLogfile, stdoutLogfile);
	_LogHTML.append("</p>");

	_LogHTML.append("<p>");
	path stderrLogfile = path(results.StderrLogfile).filename();
	oHTMLLink(_LogHTML, stderrLogfile, stderrLogfile.c_str());
	_LogHTML.append("</p>");

	if (!results.TestingSucceeded)
	{
		if (results.FailedTests.size())
			_LogHTML.append("<p>Testing failed the following tests:</p><p><table border=\"1\" cellpadding=\"2\"><tr><th>TEST NAME</th><th>STATUS</th><th>STATUS MESSAGE</th></tr>");

		for (auto& item : results.FailedTests)
		{
			lstring message;
			ampersand_encode(message.c_str(), message.capacity(), item.Message.c_str());

			_LogHTML.append("<tr><td>");
			_LogHTML.append(item.Name);
			_LogHTML.append("</td><td>");
			_LogHTML.append(item.Status);
			_LogHTML.append("</td><td>");
			_LogHTML.append(message);
			_LogHTML.append("</td></tr>");
		}

		_LogHTML.append("</table></p>");

		bool FoundAFailedImageCompare = false;
		ouro::filesystem::enumerate(path(results.FailedImagePath), 
			[&](const path& _FullPath, const ouro::filesystem::file_status& _Status, unsigned long long _Size)->bool
		{
			if (_FullPath.has_extension(".png"))
			{
				path_string filebase = _FullPath.basename();

				// Skip _diff and _golden images, we'll add them in the table
				// paired with the outputs
				if (filebase.size() > 5 && 0 == strcmp(&filebase[filebase.size()-5], "_diff")) return true;
				if (filebase.size() > 7 && 0 == strcmp(&filebase[filebase.size()-7], "_golden")) return true;

				path ImagePath(_FullPath);
				ImagePath.make_relative(results.FailedImagePath);
				if (!FoundAFailedImageCompare)
				{
					_LogHTML.append("<p>Here are the failed images:</p><table border=\"1\" cellpadding=\"2\" style=\"width:90%;\"><tr><th style=\"width:30%;\">TEST OUPUT</th><th style=\"width:30%;\">DIFF</th><th style=\"width:30%;\">GOLDEN IMAGE</th></tr>");
					FoundAFailedImageCompare = true;
				}
				_LogHTML.append("<tr><td style=\"width:30%;\"><a href=\"");
				_LogHTML.append(&ImagePath[1]); // Skip leading '/' to make image relative to FailedImagePath
				_LogHTML.append("\"><img src=\"");
				_LogHTML.append(&ImagePath[1]); // Skip leading '/' to make image relative to FailedImagePath
				_LogHTML.append("\" style=\"width:100%;\"/></a></td><td style=\"width:30%;\"><a href=\"");
				ouro::path ImagePathDiff(ImagePath);
				ImagePathDiff.replace_extension_with_suffix("_diff.png");
				_LogHTML.append(&ImagePathDiff[1]); // Skip leading '/' to make image relative to FailedImagePath
				_LogHTML.append("\"><img src=\"");
				_LogHTML.append(&ImagePathDiff[1]); // Skip leading '/' to make image relative to FailedImagePath
				_LogHTML.append("\" style=\"width:100%;\"/></a></td><td style=\"width:30%;\"><a href=\"");
				ouro::path ImagePathGolden(ImagePath);
				ImagePathGolden.replace_extension_with_suffix("_golden.png");
				_LogHTML.append(&ImagePathGolden[1]); // Skip leading '/' to make image relative to FailedImagePath
				_LogHTML.append("\"><img src=\"");
				_LogHTML.append(&ImagePathGolden[1]); // Skip leading '/' to make image relative to FailedImagePath
				_LogHTML.append("\" style=\"width:100%;\"/></a></td></tr>");
			}

			return true;
		});

		if (FoundAFailedImageCompare)
		{
			_LogHTML.append("</table>");
		}
	}
}

static void FormatPackagingResults(const oPackagingResults& _Results, std::string& _LogHTML)
{
	_LogHTML.append("<p><b>Packaging stage...</b></p>");

	mstring PackageString;
	snprintf(PackageString, "<p>Packaging finished, taking %f seconds.</p>", _Results.PackagingTimeSeconds);
	_LogHTML.append(PackageString);
}

void oAutoBuildOutputResults(const oAutoBuildEmailSettings& _EmailSettings, int _WebServerPort, const oAutoBuildResults& _Results)
{
	bool BuildSuccess = _Results.BuildResults.BuildSucceeded && _Results.TestResults.TestingSucceeded;

	std::string LogHTML;
	LogHTML.append("<html><title>");
	LogHTML.append(_Results.BuildName);
	LogHTML.append(BuildSuccess ? oBUILD_TOOL_PASS_TITLE : oBUILD_TOOL_FAIL_TITLE);
	LogHTML.append("</title><body>");
	LogHTML.reserve(oKB(4));

	// Add a description of the build
	sstring User;
	oP4GetChangelistUser(User, _Results.ChangeList);
	xxlstring Description;
	oP4GetChangelistDescription(Description, _Results.ChangeList);

	trim(Description, Description.c_str());

	xxlstring EncodedDescription;
	ampersand_encode(EncodedDescription.c_str(), EncodedDescription.capacity(), Description.c_str());

	mstring DescriptionHeader;
	snprintf(DescriptionHeader, "<p><b>Changelist %d by %s:</b></p><p>", _Results.ChangeList, User);
	LogHTML.append(DescriptionHeader);
	LogHTML.append(EncodedDescription);
	LogHTML.append("</p>");

	FormatMSBuildResults(_Results.BuildResults, LogHTML);
	
	if (_Results.BuildResults.BuildSucceeded)
	{
		FormatUnitTestResults(_Results.TestResults, LogHTML);
		FormatPackagingResults(_Results.PackagingResults, LogHTML);
	}

	LogHTML.append("</body>");

	if (!BuildSuccess)
	{
		std::string EmailHTML;
		EmailHTML.reserve(LogHTML.size() + oKB(1));

		// Get the server IP address by creating a socket to an external IP
		// (using the EmailServer) and then the host name of the socket.
		// This should guarantee that the server IP address is on the adapter
		// that is connected to the internet and thus highly likely the LAN.
		sstring serverIP;
		{
			intrusive_ptr<threadsafe oSocket> Socket;
			oSocket::DESC Desc;
			Desc.Addr = _EmailSettings.EmailServer;
			Desc.Protocol = oSocket::TCP;
			Desc.Style = oSocket::BLOCKING;
			if (oSocketCreate( "Server Connection", Desc, &Socket))
			{
				uri_string serverHostname;
				sstring port;
				Socket->GetHostname(serverHostname.c_str(), serverIP.c_str(), port.c_str());
			}
		}

		// Patch all links to go to the server
		uri_string ServerAddress;
		snprintf(ServerAddress, "http://%s:%i/logs/", serverIP.c_str(), _WebServerPort);
		for(size_t i = 6; i < LogHTML.size(); ++i) // 6 is to skip past <html> which is implicit in the mime type
		{
			const char* LinkStrings[] = { "<a href=\"", "<img src=\""};

			bool FoundLink = false;
			for(int j = 0; j < oCOUNTOF(LinkStrings); ++j)
			{
				const char* pLink = LinkStrings[j];
				size_t LinkLen = strlen(pLink);
				if ( 0 == strncmp(&LogHTML[i], pLink, LinkLen))
				{
					// Found a link+
					EmailHTML.append(pLink);
					EmailHTML.append(ServerAddress);
					EmailHTML.append(_Results.BuildName);
					EmailHTML.append("/");
					i+=LinkLen - 1;
					FoundLink = true;
					break;
				}
			}
			if (!FoundLink)
				EmailHTML.push_back(LogHTML[i]);
		}
		if (std::errc::invalid_argument == oErrorGetLast())
		{
			// Build and Test failed because of build problem
			oEmailAdminAndStop(_EmailSettings, EmailHTML.c_str(), _Results.ChangeList, true);
			return;
		}

		intrusive_ptr<oEMail> Mail;
		oEMailCreate(oEMail::USE_TLS, _EmailSettings.EmailServer, &Mail);

		if (Mail)
		{
			lstring EmailSubject;
			if (_Results.IsDailyBuild)
				if (_Results.BuildResults.BuildSucceeded)
					snprintf(EmailSubject, "%s Daily Build failed to pass unit tests with CL %d", oBUILD_TOOL_STANDARD_SUBJECT, _Results.ChangeList);
				else
					snprintf(EmailSubject, "%s Daily Build failed to build with CL %d", oBUILD_TOOL_STANDARD_SUBJECT, _Results.ChangeList);
			else
				if (_Results.BuildResults.BuildSucceeded)
					snprintf(EmailSubject, "%s %s failed to pass unit tests with CL %d", oBUILD_TOOL_STANDARD_SUBJECT, User, _Results.ChangeList);
				else
					snprintf(EmailSubject, "%s %s broke the build with CL %d", oBUILD_TOOL_STANDARD_SUBJECT, User, _Results.ChangeList);
			auto EmailList = _EmailSettings.UserEmails;
			for (auto Email : EmailList)
			{
				Mail->Send(Email, _EmailSettings.FromAddress, _EmailSettings.FromPasswordBase64, EmailSubject, (const char*)&EmailHTML[0], true, true);
			}
		}
	}

	// Terminate HTML
	LogHTML.append("</html>");

	// Save HTML to the output folder
	path HTMLFilename = _Results.OutputFolder;
	HTMLFilename /= "index.html";
	ouro::filesystem::save(HTMLFilename, &LogHTML[0], LogHTML.size(), ouro::filesystem::save_option::text_write);
}

void oEmailAdminAndStop(const oAutoBuildEmailSettings& _EmailSettings, const char* _pMessage, int _CL, bool _HTML)
{
	sstring Subject;
	snprintf(Subject, "%s Fatal Error. All builds halted. Stopped on %d", oBUILD_TOOL_STANDARD_SUBJECT, _CL);

	intrusive_ptr<oEMail> Mail;
	oEMailCreate(oEMail::USE_TLS, _EmailSettings.EmailServer, &Mail);

	if (Mail)
	{
		auto EmailList = _EmailSettings.AdminEmails;
		for (auto Email : EmailList)
		{
			Mail->Send(Email, _EmailSettings.FromAddress, _EmailSettings.FromPasswordBase64, Subject, _pMessage, true, _HTML);
		}
	}

	ouro::msgbox(ouro::msg_type::warn, nullptr, nullptr, Subject);
}
