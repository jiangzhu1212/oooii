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
// The input mapper maps simple key combinations to an enum of inputs to be used 
// by the greater system. Basically this supports the idea of mapping 
// oGUI_KEY symbols to some logical action, and that mapping can be changed 
// without changing how the logical actions behave in the application. Also 
// several combinations can be mapped to a single logical action.
#pragma once
#ifndef oInputMapper_h
#define oInputMapper_h

#include <oBasis/oGUI.h>
#include <oBasis/oInterface.h>
#include <oBasis/oRTTI.h>
#include <oBase/xml.h>

interface oInputSet : oInterface
{
	virtual const oRTTI& GetRTTI() const threadsafe = 0;
	virtual const char* GetName() const threadsafe = 0;
};

interface oInputMapper : oInterface
{
	virtual void SetInputSet(threadsafe oInputSet* _pInputSet) threadsafe = 0;

	virtual int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe = 0;
	virtual void UnhookActions(int _HookID) threadsafe = 0;

	// Call this from the key event handler to record the event.
	virtual void OnAction(const oGUI_ACTION_DESC& _Action) threadsafe = 0;
	virtual void OnLostCapture() threadsafe = 0;
};

// Parses an XML list of oAirKeySets to create a keyset based on the specified 
// keyset name. The expected XML format is:
/*
	<oInputSetList>
		<oInputSet id="oMEDIA_INPUT" version="1.0">
			<oInput id="oMEDIA_INPUT_PLAY_PAUSE" Keys="oGUI_KEY_SPACE OR oGUI_KEY_P" />
			<oInput id="oMEDIA_INPUT_TRACK_NEXT_1" Keys="oGUI_KEY_1" />
			<oInput id="oMEDIA_INPUT_TRACK_NEXT_2" Keys="oGUI_KEY_2" />

			<oInputSequence
				id="oMEDIA_INPUT_TRACK_NEXT"
				MinTimeMS="100"
				MaxTimeMS="500"
				Seq="!oMEDIA_INPUT_TRACK_NEXT_1 oMEDIA_INPUT_TRACK_NEXT_2"
			/>
		</oInputSet>
	</oInputSetList>
*/
bool oParseInputSetList(const ouro::xml& _XML, ouro::xml::node _hInputSetList, const oRTTI& _InputEnum, threadsafe oInputSet** _ppInputSet);

// Parses an XML file to build an InputSet object. The expected XML format
// of the node is:
/*
	<oInputSet id="oMEDIA_INPUT" version="1.0">
		<oInput id="oMEDIA_INPUT_PLAY_PAUSE" Keys="oGUI_KEY_SPACE OR oGUI_KEY_P" />
		<oInput id="oMEDIA_INPUT_TRACK_NEXT_1" Keys="oGUI_KEY_1" />
		<oInput id="oMEDIA_INPUT_TRACK_NEXT_2" Keys="oGUI_KEY_2" />

		<oInputSequence
		id="oMEDIA_INPUT_TRACK_NEXT"
		MinTimeMS="100"
		MaxTimeMS="500"
		Seq="!oMEDIA_INPUT_TRACK_NEXT_1 oMEDIA_INPUT_TRACK_NEXT_2"
		/>
	</oInputSet>
*/
// oInputSet Attributes
//	id: the string name of an oRTTI-reflected enum that will be the "inputs" 
//      supported by the set.
//  version: the version of the formatting for oInputSet
//
// oInput Attributes
//  id: A string name of an enum value from the id specified in oInputSet.
//  Keys: a string of multiple oGUI_KEYs arranged in the following way:
//        < oGUI_KEY [oGUI_KEY...] [OR oGUI_KEY [oGUI_KEY...] ...] >
//        Up to 4 oGUI_KEYs specified as space-delimited means "must all be down
//        at same time" to trigger the input. There is no rule restriction so
//        defining Ctrl-S and S-Ctrl would be the same. Up to 4 sets of these
//        can be specified with OR connectors for multiple input sources. For
//        example to support 1 and 2 at the same time or both mouse buttons or
//        two joystick buttons, specify: 
//        "oGUI_KEY1 oGUI_KEY2 OR oGUI_KEY_MOUSE_LEFT oGUI_KEY_MOUSE_RIGHT OR oGUI_KEY_JOYSTICK_RDOWN oGUI_KEY_JOYSTICK_RRIGHT"
//
// oInputSequence Attributes
//  id: A string name of an enum value from the id specified in oInputSet.
//  MinTimeMS: if the entire Seq occurs in less than this time, do not trigger
//             it as an input.
//  MaxTimeMS: if the entire Seq occurs in more than this time, do not trigger
//             it as an input.
// Seq: A space-delimited list of string names of enum values from the enum 
//      described by oInputSet's id that describes the sequence for oInputs that
//      will be reinterpreted as a single new combo input. Specifying the name
//      alone means "key down". Putting an exclamation in front of the name 
//      specifies "key up".
bool oInputSetCreate(const ouro::xml& _XML, ouro::xml::node _hInputSet, const oRTTI& _IDEnum, oInputSet** _ppInputSet);
inline bool oInputSetCreate(const ouro::xml& _XML, ouro::xml::node _hInputSet, const oRTTI& _IDEnum, threadsafe oInputSet** _ppInputSet) { return oInputSetCreate(_XML, _hInputSet, _IDEnum, thread_cast<oInputSet**>(_ppInputSet)); }

bool oInputMapperCreate(threadsafe oInputMapper** _ppInputMapper);

#endif
