// $(header)
// This include-all-headers-in-oooii.lib header is intended
// for use only in the precompiled headers of dependent libs.
// Using this as a short-cut for includes can cause precompiled-
// header-related build dependency problems, so still be 
// disciplined in the use of specific and minimal include files.
#pragma once
#ifndef oooii_h
#define oooii_h
#include <oPlatform/oConsole.h>
#include <oPlatform/oCPU.h>
#include <oPlatform/oCRTHeap.h>
#include <oPlatform/oDateTime.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oDispatchQueueConcurrentWTP.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oEvent.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oGPU.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oKeyboard.h>
#include <oPlatform/oMirroredArena.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oMouse.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oP4.h>
#include <oPlatform/oPageAllocator.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oProgressBar.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oSocket.h>
#include <oPlatform/oStddef.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oThreadX.h>
#include <oPlatform/oWindow.h>
#include <oPlatform/oWindowUI.h>
#endif
