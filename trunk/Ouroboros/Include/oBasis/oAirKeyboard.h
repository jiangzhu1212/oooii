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
// This uses ouro::tracking_skeleton input to collide specific bones against 
// specific skeleton-space (often camera-space) boxes to broadcast 
// ouro::gui_action::key_down/ouro::gui_action::key_up events for an ouro::input_key::value. The 
// pathological case would be a full reproduction of a typical keyboard using 
// space and gesture but more often a very few keys are used similar to the 
// keyboard mapping for a joystick in modern and classic video games. This way 
// gesture-based input can be authored in a uniform way that can be confirmed 
// without custom hardware/playspace requirements - just enter the same keyboard 
// commands to get the same results.
#pragma once
#ifndef oAirKeyboard_h
#define oAirKeyboard_h

#include <oBasis/oGUI.h>
#include <oBasis/oRTTI.h>
#include <oBasis/oInterface.h>
#include <oBase/types.h>
#include <oBase/xml.h>

struct oAIR_KEY
{
	// The air key emulates a keyboard's key. If the trigger bone is in the 
	// volume, then the key is down. If the trigger bone is outside the volume
	// then the key is up. Volumes are relative to a bone, or if an invalid bone
	// is specified then they are in the same coordinate system as all bones.

	// Bounds is in the same space as the skeleton, so by default view space with
	// the view at the camera in a left-handed coordinate system going out from
	// the camera. This means in practice from the user's point of view +Y is up, 
	// -Y is down, +X is right, -X is left, -Z is towards the camera, +Z is away
	// from the camera.

	oAIR_KEY()
		: Origin(ouro::skeleton_bone::invalid)
		, Trigger(ouro::skeleton_bone::hand_right)
		, Key(ouro::input_key::none)
	{}

	oAABoxf Bounds;
	ouro::skeleton_bone::value Origin; // bounds coords are relative to this
	ouro::skeleton_bone::value Trigger; // only this bone can trigger events for this box
	ouro::input_key::value Key;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_ARRAY, oAIR_KEY)

typedef std::function<void(const oAIR_KEY& _Key, ouro::gui_action::value _LastAction)> oAIR_KEY_VISITOR;

interface oAirKeySet : oInterface
{
	// The air key set is a collection of air keys. Like some gamer keyboards 
	// that allow the key buttons to be popped off the hardware, this system 
	// allows a subset of keys to be defined. A model to think of is MAME. Its 
	// emulation of an arcade stick and controllers is done by mapping the 
	// hardware to keyboard keys, then mapping keyboard inputs to MAME inputs.
	
	virtual const char* GetName() const threadsafe = 0;
};

// Parses an XML list of oAirKeySets to create a keyset based on the specified 
// keyset name. The expected XML format is:
/*
	<oAirKeySetList>
		<oAirKeySet Name="MediaKeySet" version="1.0">
			<oAirKey Key="oKB_Media_Play_Pause" Origin="lshoulder" Trigger="lhand" Bounds="-0.3 -0.2 -1.0 0.0 0.3 -0.2" />
		</oAirKeySet>

		<oAirKeySet Name="OtherKeySet" version="1.0">
			<oAirKey Key="oKB_1" Origin="rshoulder" Trigger="rankle" Bounds="-0.3 -0.2 -1.0 0.0 0.3 -0.2" />
		</oAirKeySet>
	</oAirKeySetList>
*/
bool oParseAirKeySetsList(const ouro::xml& _XML, ouro::xml::node _AirSetList, const char* _AirKeySetName, threadsafe oAirKeySet** _ppAirKeySet);

// Parses an XML file to build an oAirKeyboard object. The expected XML format
// of the node is:
/*
	<oAirKeyboard version="1.0">
		<oAirKey Key="oKB_Media_Play_Pause" Origin="lshoulder" Trigger="lhand" Bounds="-0.3 -0.2 -1.0 0.0 0.3 -0.2" />
		<oAirKey Key="oKB_Media_Play_Pause" Origin="rshoulder" Trigger="Rhand" Bounds="0.0 -0.2 -1.0 0.3 0.3 -0.2" />
	</oAirKeyboard>
*/
bool oAirKeySetCreate(const ouro::xml& _pXML, ouro::xml::node _AirSet, threadsafe oAirKeySet** _ppAirKeySet);

interface oAirKeyboard : oInterface
{
	// This does the logic that maps a skeleton and a keyset to actions. Create
	// one of these, set a keyset (only one at a time) and add/remove skeletons
	// and observers to be used. Call update from an ouro::gui_action::skeleton action 
	// to analyze updated data as it comes in.

public:

	// Returns false if there is already a box with the specified ID.
	virtual void SetKeySet(threadsafe oAirKeySet* _pAirKeySet) threadsafe;

	// This is intended for visualization. Visit each currently set key.
	virtual void VisitKeys(const oAIR_KEY_VISITOR& _Visitor) threadsafe;

	virtual bool AddSkeleton(int _ID) threadsafe;
	virtual void RemoveSkeleton(int _ID) threadsafe;

	virtual int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe;
	virtual void UnhookActions(int _HookID) threadsafe;

	// Analyze the specified skeleton for interaction with an oAirKeySet and 
	// trigger oGUI_ACTION_DESCs appropriately. The specified timestamp will be
	// passed through to any triggered actions.
	virtual void Update(const ouro::tracking_skeleton& _Skeleton, unsigned int _TimestampMS) threadsafe;

	// Manually trigger an action
	virtual void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe;
};

bool oAirKeyboardCreate(threadsafe oAirKeyboard** _ppAirKeyboard);

#endif
