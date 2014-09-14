// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oAirKeyboard.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oXMLSerialize.h>
#include <oConcurrency/mutex.h>

using namespace ouro;
using namespace std;

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oAIR_KEY)
	oRTTI_COMPOUND_ABSTRACT(oAIR_KEY)
	oRTTI_COMPOUND_VERSION(oAIR_KEY, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oAIR_KEY)
	oRTTI_COMPOUND_ATTR(oAIR_KEY, Bounds, oRTTI_OF(aaboxf), "Bounds", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTR(oAIR_KEY, Origin, oRTTI_OF(ouro_input_skeleton_bone), "Origin", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTR(oAIR_KEY, Trigger, oRTTI_OF(ouro_input_skeleton_bone), "Trigger", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTR(oAIR_KEY, Key, oRTTI_OF(ouro_input_key), "Key", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oAIR_KEY)
oRTTI_COMPOUND_END_DESCRIPTION(oAIR_KEY)

bool oParseAirKeySetsList(const xml& _XML, xml::node _AirSetList, const char* _AirKeySetName, threadsafe oAirKeySet** _ppAirKeySet)
{
	const char* Name = _XML.node_name(_AirSetList);
	if (!oSTRVALID(Name) && _stricmp(Name, "oAirKeySetList"))
		return oErrorSetLast(std::errc::invalid_argument, "The specified node is not an \"oAirKeySetList\" in %s", _XML.name());

	xml::node AirKeySet = _XML.first_child(_AirSetList, "oAirKeySet");
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

	oAirKeySetImpl(const xml& _XML, xml::node _AirSet, bool* _pSuccess);

	const char* GetName() const threadsafe override { return oThreadsafe(this)->Name; }

	std::vector<oAIR_KEY> Keys;
	mstring Name;
	oRefCount RefCount;
};

oAirKeySetImpl::oAirKeySetImpl(const xml& _XML, xml::node _AirSet, bool* _pSuccess)
{
	*_pSuccess = false;

	if (_stricmp("oAirKeySet", _XML.node_name(_AirSet)))
	{
		oErrorSetLast(std::errc::protocol_error, "not an oAirKeySet node");
		return;
	}

	version v(1,0);
	_XML.find_attr_value(_AirSet, "version", &v);
	if (v == version(1,0))
	{
		_XML.find_attr_value(_AirSet, "name", &Name);
		oXMLReadContainer(&Keys, sizeof(Keys), oRTTI_OF(std_vector_oAIR_KEY), "oAirKey", false, _XML, _AirSet, true);
	}

	else
	{
		sstring strVer;
		oErrorSetLast(std::errc::protocol_error, "unsupported version %s", to_string(strVer, v));
		return;
	}

	*_pSuccess = true;
}

bool oAirKeySetCreate(const xml& _XML, xml::node _AirSetNode, threadsafe oAirKeySet** _ppAirKeySet)
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
	int HookActions(const ouro::input::action_hook& _Hook) threadsafe override;
	void UnhookActions(int _HookID) threadsafe override;
	void Update(const ouro::input::tracking_skeleton& _Skeleton, unsigned int _TimestampMS) threadsafe override;
	void Trigger(const ouro::input::action& _Action) threadsafe override;

private:
	ouro::shared_mutex KeySetMutex;
	intrusive_ptr<threadsafe oAirKeySet> KeySet;
	std::vector<ouro::input::action_type> KeyAction;

	ouro::shared_mutex SkeletonsMutex;
	std::map<int, ouro::input::tracking_skeleton> Skeletons;

	ouro::shared_mutex HooksMutex;
	std::vector<ouro::input::action_hook> Hooks;

	oRefCount RefCount;

	void UpdateInternal(const ouro::input::tracking_skeleton& _Skeleton, unsigned int _TimestampMS);
	void TriggerInternal(const ouro::input::action& _Action);
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
	ouro::lock_guard<ouro::shared_mutex> lock(oThreadsafe(KeySetMutex));
	KeySet = _pKeySet;
	if (KeySet)
	{
		const auto& Keys = thread_cast<oAirKeySetImpl*>(static_cast<threadsafe oAirKeySetImpl*>(KeySet.c_ptr()))->Keys;
		auto& Actions = oThreadsafe(this)->KeyAction;
		Actions.resize(Keys.size());
		fill(Actions, ouro::input::action_type::key_up);
	}
}

void oAirKeyboardImpl::VisitKeys(const oAIR_KEY_VISITOR& _Visitor) threadsafe
{
	ouro::lock_guard<ouro::shared_mutex> lock(oThreadsafe(KeySetMutex));
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
	ouro::lock_guard<ouro::shared_mutex> lock(oThreadsafe(SkeletonsMutex));
	return unique_set(oThreadsafe(Skeletons), _ID, ouro::input::tracking_skeleton());
}

void oAirKeyboardImpl::RemoveSkeleton(int _ID) threadsafe
{
	ouro::lock_guard<ouro::shared_mutex> lock(oThreadsafe(SkeletonsMutex));
	find_and_erase(oThreadsafe(Skeletons), _ID);
}

int oAirKeyboardImpl::HookActions(const ouro::input::action_hook& _Hook) threadsafe
{
	ouro::lock_guard<ouro::shared_mutex> lockB(oThreadsafe(HooksMutex));
	size_t index = sparse_set(oThreadsafe(Hooks), _Hook);
	return static_cast<int>(index);
}

void oAirKeyboardImpl::UnhookActions(int _HookID) threadsafe
{
	ouro::lock_guard<ouro::shared_mutex> lock(oThreadsafe(HooksMutex));
	ranged_set(oThreadsafe(Hooks), _HookID, nullptr);
}

void oAirKeyboardImpl::Update(const ouro::input::tracking_skeleton& _Skeleton, unsigned int _TimestampMS) threadsafe
{
	ouro::lock_guard<ouro::shared_mutex> lockB(oThreadsafe(KeySetMutex));
	ouro::lock_guard<ouro::shared_mutex> lockS(oThreadsafe(SkeletonsMutex));
	oThreadsafe(this)->UpdateInternal(_Skeleton, _TimestampMS);
}

void oAirKeyboardImpl::UpdateInternal(const ouro::input::tracking_skeleton& _Skeleton, unsigned int _TimestampMS)
{
	if (!KeySet)
		return;

	auto it = Skeletons.find(_Skeleton.source_id);
	if (it == Skeletons.end())
		return;
	ouro::input::tracking_skeleton& OldSkeleton = it->second;

	ouro::input::action a;
	a.device_type = ouro::input::type::skeleton;
	a.device_id = _Skeleton.source_id;
	a.timestamp_ms = _TimestampMS;
	
	const auto& Keys = thread_cast<oAirKeySetImpl*>(static_cast<threadsafe oAirKeySetImpl*>(KeySet.c_ptr()))->Keys;
	for (size_t k = 0; k < Keys.size(); k++)
	{
		const auto& Key = Keys[k];
		a.key = Key.Key;
		aaboxf NewBounds(Key.Bounds), OldBounds(Key.Bounds);
		if (Key.Origin != ouro::input::skeleton_bone::invalid_bone)
		{
			ouro::translate(NewBounds, _Skeleton.positions[(int)Key.Origin].xyz());
			ouro::translate(OldBounds, OldSkeleton.positions[(int)Key.Origin].xyz());
		}

		for (int i = 0; i < oCOUNTOF(_Skeleton.positions); i++)
		{
			if (Key.Trigger == (ouro::input::skeleton_bone)i)
			{
				const float4& New = _Skeleton.positions[i];
				const float4& Old = OldSkeleton.positions[i];

				if (New.w >= 0.0f && Old.w >= 0.0f)
				{
					const int NewInside = ouro::contains(NewBounds, New.xyz());
					const int OldInside = ouro::contains(OldBounds, Old.xyz());

					if (NewInside != OldInside)
					{
						a.action_type = KeyAction[k] = NewInside ? ouro::input::action_type::key_down : ouro::input::action_type::key_up; 
						a.action_code = i;
						a.position(New);
						TriggerInternal(a);
					}
				}
			}
		}
	}

	it->second = _Skeleton;
}

void oAirKeyboardImpl::TriggerInternal(const ouro::input::action& _Action)
{
	for (const auto& Hook : Hooks)
		Hook(_Action);
}

void oAirKeyboardImpl::Trigger(const ouro::input::action& _Action) threadsafe
{
	ouro::lock_guard<ouro::shared_mutex> lock(oThreadsafe(HooksMutex));
	oThreadsafe(this)->TriggerInternal(_Action);
}
