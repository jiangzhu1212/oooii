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
#include <oPlatform/oTest.h>
#include <oBasis/oEvent.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oStandards.h>

//static const char* FolderToMonitor = "file://DATA/Apps/PlayerMediaEncoder";
//static const char* FolderToMonitor = "file://abyss/Media/oUnitTests";
static const char* FolderToMonitor = "file://DATA/Test";
static const char* TestFile = "file://DATA/Test/monitor_test.txt";

struct TESTStreamMonitorEvents
{
	double LastEventTimestamp;

	oEvent FileModified;
	oEvent FileAdded;
	oEvent FileRemoved;

	oEvent NZFileAccessible;
	oEvent ZFileAccessible;
};

struct TESTStreamMonitor : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		auto Events = std::make_shared<TESTStreamMonitorEvents>();

		oStringPath NonZeroFile = TestFile;
		*oGetFileExtension(NonZeroFile) = 0;
		oStrAppendf(NonZeroFile, "-NZ.txt");

		oStringPath ZeroFile = TestFile;
		*oGetFileExtension(ZeroFile) = 0;
		oStrAppendf(ZeroFile, "-Z.txt");

		oStreamDelete(TestFile); // should fail, but try anyway in case it was left over from a previous canceled or failed run.
		oStreamDelete(NonZeroFile);
		oStreamDelete(ZeroFile);

		double startTime = oTimer();

		oRef<threadsafe oStreamMonitor> Monitor;
		oTESTB0(oStreamMonitorCreate(FolderToMonitor,
			// It is not normally a good practice to capture the events by reference, 
			// but for simplicity doing it here. counting on not getting further 
			// callbacks once the test is finished.
			[startTime, Events](oSTREAM_EVENT _Event, const oStringURI& _ChangedURI)
			{
				int timeMS = static_cast<int>((oTimer() - startTime) * 1000);
				switch (_Event)
				{
					case oSTREAM_ADDED:
						oTRACE("file %s was added at time %dms from start", _ChangedURI.c_str(), timeMS);
						Events->FileAdded.Set();
						break;
					case oSTREAM_REMOVED:
						oTRACE("file %s was removed at time %dms from start", _ChangedURI.c_str(), timeMS);
						Events->FileRemoved.Set();
						break;
					case oSTREAM_MODIFIED:
					{
						oTRACE("file %s was modified at time %dms from start", _ChangedURI.c_str(), timeMS);
						Events->FileModified.Set();
						break;
					}
					case oSTREAM_ACCESSIBLE:
					{
						oTRACE("file %s is accessible at time %dms from start", _ChangedURI.c_str(), timeMS);
						if (strstr(_ChangedURI, "-NZ.txt"))
							Events->NZFileAccessible.Set();
						else if (strstr(_ChangedURI, "-Z.txt"))
							Events->ZFileAccessible.Set();
						break;
					}
				}

				Events->LastEventTimestamp = oTimer();
				oTRACE("Events->LastEventTimestamp = %f", Events->LastEventTimestamp);
			},
		&Monitor));

		oSTREAM_DESC sd;
		Monitor->GetDesc(&sd);
		oTESTB0(sd.Directory);

		{
			oRef<threadsafe oStreamWriter> writer;
			oTESTB(oStreamWriterCreate(TestFile, &writer), "couldn't create test file"); // should generate an added event.
			char testData[64];
			oSTREAM_WRITE w;
			w.pData = testData;
			w.Range.Offset = 0;
			w.Range.Size = 64;
			writer->Write(w); //should generate a modified event, windows a little iffy on that though, may get one from this, or it may get delayed until the file is closed if its sits in a cache.
		}

		oTESTB0(oStreamCopy(TestFile, NonZeroFile));

		// write a 0-len file
		{
			oRef<threadsafe oStreamWriter> writer;
			oTESTB(oStreamWriterCreate(TestFile, &writer), "couldn't create test file"); // should generate an added event.
		}

		oTESTB0(oStreamCopy(TestFile, ZeroFile));

		static const unsigned int kTimeout = 1500;

		oTESTB(Events->FileAdded.Wait(kTimeout), "timed out waiting for the added event");
		oTESTB(Events->FileModified.Wait(kTimeout), "timed out waiting for the modified event");

		oTESTB(Events->NZFileAccessible.Wait(kTimeout), "timed out waiting for the non-zero-sized file accessible event");
		oTESTB(Events->ZFileAccessible.Wait(kTimeout), "timed out waiting for the zero-sized file accessible event");

		oTESTB0(oStreamDelete(NonZeroFile));
		oTESTB0(oStreamDelete(ZeroFile));

		oTESTB0(oStreamDelete(TestFile)); //should generate a removed event
		oTESTB(Events->FileRemoved.Wait(kTimeout), "timed out waiting for the removed event");

		//Monitor = nullptr;

		// @oooii-tony: HACK wait to ensure the shared ref to Events and any internals are 
		// idle before exiting.
		//while ((Events->LastEventTimestamp + 1.5) < oTimer());

		double dNow = oTimer();

		//oSleep(5000);

		oTRACE("dNow = %f", dNow);
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTStreamMonitor);
