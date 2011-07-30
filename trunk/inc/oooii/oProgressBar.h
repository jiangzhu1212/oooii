// $(header)
// A simple progress bar for very simple applications. NOTE: This is for the 90%
// case. Dialogs are fairly easy to construct and should be tailored to the task,
// so if there's a complex case such as wanting to see individual task v. overall
// progress, or seeing progress of each thread, a different dialog box should be
// created. In the meantime, ALL components of a major task should add up to the
// single completion of the progress bar. I.e. don't have Step1 go to 100% and
// then Step2 resets and goes to 100%, just update the task and have each add up
// to 50% of the overall progress. See GUI suggestions on the net. There's a 
// good starting point at http://msdn.microsoft.com/en-us/library/aa511486.aspx
#pragma once
#ifndef oProgressBar_h
#define oProgressBar_h

#include <oooii/oStddef.h>

void oProgressBarShow(const char* _Title, bool _Show = true, bool _AlwaysOnTop = true, bool _UnknownProgress = false);
void oProgressBarSetText(const char* _Text, const char* _Subtext = nullptr);
void oProgressBarSetStopped(bool _Stopped = true);
void oProgressBarSet(int _Percentage);
void oProgressBarAdd(int _Percentage);
bool oProgressBarWait(unsigned int _TimeoutMS = oINFINITE_WAIT);

#endif
