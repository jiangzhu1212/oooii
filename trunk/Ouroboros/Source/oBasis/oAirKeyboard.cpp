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
#include <oBasis/oAirKeyboard.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oXMLSerialize.h>
#include <oConcurrency/mutex.h>
#include <oConcurrency/thread_safe.h>

using namespace oConcurrency;

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oAIR_KEY)
	oRTTI_COMPOUND_ABSTRACT(oAIR_KEY)
	oRTTI_COMPOUND_VERSION(oAIR_KEY, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oAIR_KEY)
	oRTTI_COMPOUND_ATTR(oAIR_KEY, Bounds, oRTTI_OF(oAABoxf), "Bounds", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTR(oAIR_KEY, Origin, oRTTI_OF(oGUI_BONE), "Origin", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTR(oAIR_KEY, Trigger, oRTTI_OF(oGUI_BONE), "Trigger", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTR(oAIR_KEY, Key, oRTTI_OF(oGUI_KEY), "Key", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oAIR_KEY)
oRTTI_COMPOUND_END_DESCRIPTION(oAIR_KEY)

bool oParseAirKeySetsList(const oStd::xml& _XML, oStd::xml::node _AirSetList, const char* _AirKeySetName, threadsafe oAirKeySet** _ppAirKeySet)
{
	const char* Name = _XML.node_name(_AirSetList);
	if (!oSTRVALID(Name) && _stricmp(Name, "oAirKeySetList"))
		return oErrorSetLast(std::errc::invalid_argument, "The specified node is not an \"oAirKeySetList\" in %s", _XML.name());

	oStd::xml::node AirKeySet = _XML.first_child(_AirSetList, "oAirKeySet");
	if (!AirKeySet)
		return oErrorSetLast(std::errc::protocol_error, "Could not find \"oAirKeySet\" named \"%s\" in %s", oSAFESTRN(_AirKeySetName), _XML.name());

	do 
	{
		const char* Name = _XML.find_attr_value(AirKeySet, "id");
		if (!Name)
			return oErrorSetLast(std::errc::protocol_error, "oAirKeySet \"%s\" malformed (missing Name attribute)", oSAFESTRN(_AirKeySetName), _XML.name());

		if (!_stricmp(_AirKeySetName, Name))
			return oAirKeySetCreate(_XML, AirKeySet, _ppAirKeySet);

		AirKeySet = _XML.next_sibling(AirKeySet);

	} while (AirKeySet);

	return oErrorSetLast(std::errc::no_such_file_or_directory, "oAirKeySet \"%s\" not found in %s", oSAFESTRN(_AirKeySetName), _XML.name());
}

struct oAirKeySetImpl : oAirKeySet
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oAirKeySetImpl(const oStd::xml& _XML, oStd::xml::node _AirSet, bool* _pSuccess);

	const char* GetName() const threadsafe override { return oThreadsafe(this)->Name; }

	std::vector<oAIR_KEY> Keys;
	oStd::mstring Name;
	oRefCount RefCount;
};

oAirKeySetImpl::oAirKeySetImpl(const oStd::xml& _XML, oStd::xml::node _AirSet, bool* _pSuccess)
{
	*_pSuccess = false;

	if (_stricmp("oAirKeySet", _XML.node_name(_AirSet)))
	{
		oErrorSetLast(std::errc::protocol_error, "not an oAirKeySet node");
		return;
	}

	oStd::version v(1,0);
	_XML.find_attr_value(_AirSet, "version", &v);
	if (v == oStd::version(1,0))
	{
		_XML.find_attr_value(_AirSet, "name", &Name);
		oXMLReadContainer(&Keys, sizeof(Keys), oRTTI_OF(std_vector_oAIR_KEY), "oAirKey", false, _XML, _AirSet, true);
	}

	else
	{
		oStd::sstring strVer;
		oErrorSetLast(std::errc::protocol_error, "unsupported version %s", oStd::to_string(strVer, v));
		return;
	}

	*_pSuccess = true;
}

bool oAirKeySetCreate(const oStd::xml& _XML, oStd::xml::node _AirSetNode, threadsafe oAirKeySet** _ppAirKeySet)
{
	bool success = false;
	oCONSTRUCT(_ppAirKeySet, oAirKeySetImpl(_XML, _AirSetNode, &success));
	return success;
}

struct oAirKeyboardImpl : oAirKeyboard
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oAirKeyboardImpl(bool* _pSuccess);

	void SetKeySet(threadsafe oAirKeySet* _pAirKeySet) threadsafe override;
	void VisitKeys(const oAIR_KEY_VISITOR& _Visitor) threadsafe override;
	bool AddSkeleton(int _ID) threadsafe override;
	void RemoveSkeleton(int _ID) threadsafe override;
	int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe override;
	void UnhookActions(int _HookID) threadsafe override;
	void Update(const oGUI_BONE_DESC& _Skeleton, unsigned int _TimestampMS) threadsafe override;
	void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe override;

private:
	oConcurrency::shared_mutex KeySetMutex;
	oStd::ref<threadsafe oAirKeySet> KeySet;
	std::vector<oGUI_ACTION> KeyAction;

	oConcurrency::shared_mutex SkeletonsMutex;
	std::map<int, oGUI_BONE_DESC> Skeletons;

	oConcurrency::shared_mutex HooksMutex;
	std::vector<oGUI_ACTION_HOOK> Hooks;

	oRefCount RefCount;

	void UpdateInternal(const oGUI_BONE_DESC& _Skeleton, unsigned int _TimestampMS);
	void TriggerInternal(const oGUI_ACTION_DESC& _Action);
};

oAirKeyboardImpl::oAirKeyboardImpl(bool* _pSuccess)
{
	Hooks.reserve(8);
	*_pSuccess = true;
}

bool oAirKeyboardCreate(threadsafe oAirKeyboard** _ppAirKeyboard)
{
	bool success = false;
	oCONSTRUCT(_ppAirKeyboard, oAirKeyboardImpl(&success));
	return success;
}

void oAirKeyboardImpl::SetKeySet(threadsafe oAirKeySet* _pKeySet) threadsafe
{
	lock_guard<shared_mutex> lock(KeySetMutex);
	KeySet = _pKeySet;
	if (KeySet)
	{
		const auto& Keys = thread_cast<oAirKeySetImpl*>(static_cast<threadsafe oAirKeySetImpl*>(KeySet.c_ptr()))->Keys;
		auto& Actions = oThreadsafe(this)->KeyAction;
		Actions.resize(Keys.size());
		oStd::fill(Actions, oGUI_ACTION_KEY_UP);
	}
}

void oAirKeyboardImpl::VisitKeys(const oAIR_KEY_VISITOR& _Visitor) threadsafe
{
	lock_guard<shared_mutex> lock(KeySetMutex);
	if (KeySet)
	{
		auto& Keys = oThreadsafe(static_cast<threadsafe oAirKeySetImpl*>(KeySet.c_ptr())->Keys);
		auto& Actions = oThreadsafe(this)->KeyAction;
		for (size_t i = 0; i < Keys.size(); i++)
			_Visitor(Keys[i], Actions[i]);
	}
}

bool oAirKeyboardImpl::AddSkeleton(int _ID) threadsafe
{
	lock_guard<shared_mutex> lock(SkeletonsMutex);
	return oStd::unique_set(oThreadsafe(Skeletons), _ID, oGUI_BONE_DESC());
}

void oAirKeyboardImpl::RemoveSkeleton(int _ID) threadsafe
{
	lock_guard<shared_mutex> lock(SkeletonsMutex);
	oStd::find_and_erase(oThreadsafe(Skeletons), _ID);
}

int oAirKeyboardImpl::HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe
{
	lock_guard<shared_mutex> lockB(HooksMutex);
	return oInt(oStd::sparse_set(oThreadsafe(Hooks), _Hook));
}

void oAirKeyboardImpl::UnhookActions(int _HookID) threadsafe
{
	lock_guard<shared_mutex> lock(HooksMutex);
	oStd::ranged_set(oThreadsafe(Hooks), _HookID, nullptr);
}

void oAirKeyboardImpl::Update(const oGUI_BONE_DESC& _Skeleton, unsigned int _TimestampMS) threadsafe
{
	lock_guard<shared_mutex> lockB(KeySetMutex);
	lock_guard<shared_mutex> lockS(SkeletonsMutex);
	oThreadsafe(this)->UpdateInternal(_Skeleton, _TimestampMS);
}

void oAirKeyboardImpl::UpdateInternal(const oGUI_BONE_DESC& _Skeleton, unsigned int _TimestampMS)
{
	if (!KeySet)
		return;

	auto it = Skeletons.find(_Skeleton.SourceID);
	if (it == Skeletons.end())
		return;
	oGUI_BONE_DESC& OldSkeleton = it->second;

	oGUI_ACTION_DESC a;
	a.DeviceType = oGUI_INPUT_DEVICE_SKELETON;
	a.DeviceID = _Skeleton.SourceID;
	a.TimestampMS = _TimestampMS;
	//a.hSource = ?;
	
	const auto& Keys = thread_cast<oAirKeySetImpl*>(static_cast<threadsafe oAirKeySetImpl*>(KeySet.c_ptr()))->Keys;
	for (size_t k = 0; k < Keys.size(); k++)
	{
		const auto& Key = Keys[k];
		a.Key = Key.Key;
		oAABoxf NewBounds(Key.Bounds), OldBounds(Key.Bounds);
		if (Key.Origin != oGUI_BONE_INVALID)
		{
			oTranslate(NewBounds, _Skeleton.Positions[Key.Origin].xyz());
			oTranslate(OldBounds, OldSkeleton.Positions[Key.Origin].xyz());
		}

		for (int i = 0; i < oCOUNTOF(_Skeleton.Positions); i++)
		{
			if (Key.Trigger == i)
			{
				const float4& New = _Skeleton.Positions[i];
				const float4& Old = OldSkeleton.Positions[i];

				if (New.w >= 0.0f && Old.w >= 0.0f)
				{
					const bool NewInside = oContains(NewBounds, New.xyz());
					const bool OldInside = oContains(OldBounds, Old.xyz());

					if (NewInside != OldInside)
					{
						a.Action = KeyAction[k] = NewInside ? oGUI_ACTION_KEY_DOWN : oGUI_ACTION_KEY_UP; 
						a.ActionCode = i;
						a.Position = New;
						TriggerInternal(a);
					}
				}
			}
		}
	}

	it->second = _Skeleton;
}

void oAirKeyboardImpl::TriggerInternal(const oGUI_ACTION_DESC& _Action)
{
	oFOR(const auto& Hook, Hooks)
		Hook(_Action);
}

void oAirKeyboardImpl::Trigger(const oGUI_ACTION_DESC& _Action) threadsafe
{
	lock_guard<shared_mutex> lock(HooksMutex);
	oThreadsafe(this)->TriggerInternal(_Action);
}
