/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oInputMapper.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oConcurrency/mutex.h>
#include <array>

using namespace oConcurrency;

class oActionHookHelper
{
public:
	oActionHookHelper()
	{
		Hooks.reserve(8);
	}

	inline int Hook(const oGUI_ACTION_HOOK& _Hook) threadsafe
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lockB(Mutex);
		return oInt(oStd::sparse_set(oThreadsafe(Hooks), _Hook));
	}

	inline void Unhook(int _HookID) threadsafe
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		oStd::ranged_set(oThreadsafe(Hooks), _HookID, nullptr);
	}

	void Call(const oGUI_ACTION_DESC& _Action) threadsafe
	{
		oConcurrency::shared_lock<oConcurrency::shared_mutex> lock(Mutex);
		oFOR(const auto& h, oThreadsafe(Hooks))
			h(_Action);
	}

private:
	oConcurrency::shared_mutex Mutex;
	std::vector<oGUI_ACTION_HOOK> Hooks;
};

class oInput
{
public:
	oInput(const char* _InputMapping = nullptr)
	{
		Clear();
		Parse(_InputMapping);
	}

	inline void Clear()
	{
		StateValidMask = State = 0;
		oFOR(auto& k, Keys)
			k.fill(oGUI_KEY_NONE);
	}

	// Returns true if InputIsDown should be respected. Several keys can make up
	// an input result, so it could take several key events to get the state of 
	// the input. It can either be down or up based on the value of InputIsDown.
	bool OnKey(oGUI_KEY _Key, bool _IsDown, bool* _pInputIsDown);

private:
	// Up to 4 keys can be pressed at the same time to trigger a Input.
	// Up to 7 different combinations can trigger a Input.
	std::array<std::array<oGUI_KEY, 4>, 4> Keys;
	short StateValidMask;
	short State;

	void Parse(const char* _InputMapping);

	inline bool IsDown() const
	{
		#define SETEQ(_Mask) (StateValidMask & _Mask) && ((State & _Mask) == (StateValidMask & _Mask))
		return SETEQ(0xf) || SETEQ(0xf0) || SETEQ(0xf00) || SETEQ(0xf000);
		#undef SETEQ
	}
};

void oInput::Parse(const char* _InputMapping)
{
	if (_InputMapping)
	{
		oStd::lstring temp(_InputMapping);

		char* ctx = nullptr;
		char* tok = strtok_s(temp, oWHITESPACE, &ctx);
		int KeySetIndex = 0, KeyIndex = 0;
		while (tok)
		{
			if (!_stricmp("OR", tok))
			{
				if (++KeySetIndex >= oInt(Keys.size()))
					oTHROW(no_buffer_space, "Only %u sets of keys OR'ed together are allowed.", Keys.size());
				KeyIndex = 0;
			}

			else
			{
				if (KeyIndex >= oInt(Keys[KeySetIndex].size()))
					oTHROW(no_buffer_space, "Only up to %u simultaneous keys supported", Keys[KeySetIndex].size());

				oGUI_KEY Key = oGUI_KEY_NONE;
				if (!oRTTI_OF(oGUI_KEY).FromString(tok, &Key, sizeof(Key)))
					oTHROW(protocol_error, "unrecognized oGUI_KEY \"%s\"", tok);

				Keys[KeySetIndex][KeyIndex] = Key;

				const int Bit = (KeySetIndex * 4) + KeyIndex;
				StateValidMask |= (1<<Bit);

				KeyIndex++;
			}

			tok = strtok_s(nullptr, oWHITESPACE, &ctx);
		}
	}
}

bool oInput::OnKey(oGUI_KEY _Key, bool _IsDown, bool* _pInputIsDown)
{
	const bool WasDown = IsDown();
	int StateMask = 1;
	oFOR(const auto& K, Keys)
	{
		for (int i = 0; i < 4; i++)
		{
			if (K[i] != oGUI_KEY_NONE && K[i] == _Key)
			{
				if (_IsDown)
					State |= StateMask;
				else
					State &=~ StateMask;
			}

			StateMask <<= 1;
		}
	}
	*_pInputIsDown = IsDown();

	return WasDown != *_pInputIsDown;
}

struct oINPUT_STATE
{
	int InputID;
	bool IsDown;
	bool operator==(const oINPUT_STATE& _That) const { return InputID == _That.InputID && IsDown == _That.IsDown; }
};

class oInputSequence
{
public:
	oInputSequence(const oStd::xml& _XML, oStd::xml::node _hInputSequence, const oRTTI& _IDEnum, bool* _pSuccess);

	oInputSequence(oInputSequence&& _That) { operator=(std::move(_That)); }
	oInputSequence& operator=(oInputSequence&& _That)
	{
		if (this != &_That)
		{
			InputID = std::move(_That.InputID);
			MinTimeMS = std::move(_That.MinTimeMS);
			MaxTimeMS = std::move(_That.MaxTimeMS);
			Sequence = std::move(_That.Sequence);
		}

		return *this;
	}

	int InputID;
	unsigned short MinTimeMS;
	unsigned short MaxTimeMS;
	
	std::vector<oINPUT_STATE> Sequence;

private:
	oInputSequence(const oInputSequence&);
	const oInputSequence& operator=(const oInputSequence&);

	bool Parse(const char* _SeqMapping, const oRTTI& _IDEnum);
};

oInputSequence::oInputSequence(const oStd::xml& _XML, oStd::xml::node _hInputSequence, const oRTTI& _IDEnum, bool* _pSuccess)
	: MinTimeMS(oInvalid)
	, MaxTimeMS(oInvalid)
	, InputID(oInvalid)
{
	*_pSuccess = false;

	const char* IDStr = _XML.find_attr_value(_hInputSequence, "ID");
	if (!_IDEnum.FromString(IDStr, &InputID, sizeof(int)))
	{
		oStd::sstring name;
		oErrorSetLast(std::errc::protocol_error, "ID %s is not a valid %s (%s)", IDStr, _IDEnum.GetName(name), _XML.name());
		return;
	}

	_XML.find_attr_value(_hInputSequence, "MinTimeMS", &MinTimeMS);
	_XML.find_attr_value(_hInputSequence, "MaxTimeMS", &MaxTimeMS);

	const char* SeqMapping = _XML.find_attr_value(_hInputSequence, "Seq");
	if (!oSTRVALID(SeqMapping))
	{
		oErrorSetLast(std::errc::protocol_error, "A Seq attr must be specified for oInputSequence ID=%s (%s)", IDStr, _XML.name());
		return;
	}

	if (!Parse(SeqMapping, _IDEnum))
		return; // pass through error

	*_pSuccess = true;
}

bool oInputSequence::Parse(const char* _SeqMapping, const oRTTI& _IDEnum)
{
	if (_SeqMapping)
	{
		oStd::lstring temp(_SeqMapping);

		char* ctx = nullptr;
		char* tok = strtok_s(temp, oWHITESPACE, &ctx);
		oINPUT_STATE s;
		while (tok)
		{
			s.IsDown = (*tok != '!');
			if (!s.IsDown)
				tok++;

			if (!_IDEnum.FromString(tok, &s.InputID, sizeof(s.InputID)))
			{
				oStd::sstring name;
				return oErrorSetLast(std::errc::protocol_error, "unrecognized %s \"%s\"", _IDEnum.GetName(name), tok);
			}

			Sequence.push_back(s);

			tok = strtok_s(nullptr, oWHITESPACE, &ctx);
		}
	}

	return true;
}

struct oInputSetImpl : oInputSet
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oInputSetImpl(const oStd::xml& _XML, oStd::xml::node _InputSet, const oRTTI& _IDEnum, bool* _pSuccess);

	const oRTTI& GetRTTI() const threadsafe { return oThreadsafe(this)->IDRTTI; }
	const char* GetName() const threadsafe override { return oThreadsafe(this)->Name; }

	const oRTTI& IDRTTI;
	oStd::mstring Name;
	std::vector<oInput> Inputs;
	std::vector<oInputSequence> InputSequences;
	int MaxTimeMS;
	oRefCount RefCount;
};

oInputSetImpl::oInputSetImpl(const oStd::xml& _XML, oStd::xml::node _InputSet, const oRTTI& _IDEnum, bool* _pSuccess)
	: IDRTTI(_IDEnum)
{
	*_pSuccess = false;

	MaxTimeMS = oInvalid;
	_XML.find_attr_value(_InputSet, "id", &Name);
	for (oStd::xml::node hChild = _XML.first_child(_InputSet); hChild; hChild = _XML.next_sibling(hChild))
	{
		if (!_stricmp("oInput", _XML.node_name(hChild)))
		{
			const char* IDString = _XML.find_attr_value(hChild, "ID");
			if (!oSTRVALID(IDString))
			{
				oErrorSetLast(std::errc::protocol_error, "an ID must be specified for each oInput in oInputSet(%s) in %s", Name.c_str(), _XML.name());
				return;
			}

			int EnumValue = oInvalid;
			if (!_IDEnum.FromString(IDString, &EnumValue, sizeof(int)))
			{
				oStd::mstring temp;
				oErrorSetLast(std::errc::protocol_error, "ID(%s) not recognized as a %s for a oInput in oInputSet(%s) in %s", IDString, _IDEnum.GetName(temp), Name.c_str(), _XML.name());
				return;
			}

			const char* InputMapping = _XML.find_attr_value(hChild, "Keys");
			if (!oSTRVALID(IDString))
			{
				oErrorSetLast(std::errc::protocol_error, "Keys must be specified for each oInput in oInputSet(%s) in %s", Name.c_str(), _XML.name());
				return;
			}

			oInput Input(InputMapping);
			oStd::safe_set(Inputs, EnumValue, Input);
		}

		else if (!_stricmp("oInputSequence", _XML.node_name(hChild)))
		{
			bool success = false;
			oInputSequence Seq(_XML, hChild, _IDEnum, &success);
			if (!success)
				return; // pass through error

			InputSequences.push_back(std::move(Seq));
			MaxTimeMS = __max(MaxTimeMS, oInt(Seq.MaxTimeMS));
		}
	}

	*_pSuccess = true;
}

bool oInputSetCreate(const oStd::xml& _XML, oStd::xml::node _InputSet, const oRTTI& _IDEnum, oInputSet** _ppInputSet)
{
	bool success = false;
	oCONSTRUCT(_ppInputSet, oInputSetImpl(_XML, _InputSet, _IDEnum, &success));
	return success;
}

bool oParseInputSetList(const oStd::xml& _XML, oStd::xml::node _InputSetList, const oRTTI& _InputEnum, threadsafe oInputSet** _ppInputSet)
{
	const char* Name = _XML.node_name(_InputSetList);
	if (_stricmp(Name, "oInputSetList"))
		return oErrorSetLast(std::errc::invalid_argument, "The specified node is not an \"oInputSetList\" in %s", _XML.name());

	oStd::mstring EnumName;
	_InputEnum.GetName(EnumName);
	oStd::xml::node hInputSet = _XML.first_child(_InputSetList, "oInputSet");
	if (!hInputSet)
		return oErrorSetLast(std::errc::protocol_error, "Could not find \"oInputSet\" named \"%s\" in %s", EnumName.c_str(), _XML.name());

	do 
	{
		const char* id = _XML.find_attr_value(hInputSet, "id");
		if (!id)
			return oErrorSetLast(std::errc::protocol_error, "oInputSet \"%s\" malformed (missing Name attribute)", EnumName.c_str(), _XML.name());

		if (!_stricmp(EnumName, id))
			return oInputSetCreate(_XML, hInputSet, _InputEnum, _ppInputSet);

		hInputSet = _XML.next_sibling(hInputSet);

	} while (hInputSet);

	return oErrorSetLast(std::errc::no_such_file_or_directory, "oInputSet \"%s\" not found in %s", EnumName.c_str(), _XML.name());
}

class oInputHistory
{
public:
	oInputHistory(size_t _HistorySize = 1500)
		: Latest(oInvalid)
	{
		History.resize(_HistorySize);
		oStd::fill(History, oINPUT_LOG(0.0, oInvalid, false));
	}

	void Add(double _Timestamp, int _InputID, bool _IsDown)
	{
		Latest = (Latest + 1) % History.size();
		History[Latest] = oINPUT_LOG(_Timestamp, _InputID, _IsDown);
	}

	bool SequenceTriggered(const oInputSequence& _InputSequence, double _TriggerTime) const
	{
		// Is the latest value the last item in the sequence? If not, then the 
		// sequence isn't triggered at this time. 
		auto itSeq = _InputSequence.Sequence.rbegin();
		if (History[Latest] != *itSeq)
			return false;

		// If there's only one item and it matched, then that's a match.
		if (++itSeq == _InputSequence.Sequence.rend())
			return true;

		// start looking for the rest

		// Keep checking backwards until there's a timeout or no more history.
		size_t Current = (Latest - 1) % History.size();
		const double LastTimestamp = _InputSequence.MaxTimeMS == oInvalid ? 1e100 : (_TriggerTime - (_InputSequence.MaxTimeMS / 1000.0));

		while (Current != Latest)
		{
			if (History[Current].Timestamp < LastTimestamp)
				return false; // any history is older than the sequence max timeout

			if (History[Current] == *itSeq)
			{
				if (++itSeq == _InputSequence.Sequence.rend())
					return true;
			}

			Current = (Current - 1) % History.size();
		}
		
		return false;
	}
	
	struct oINPUT_LOG
	{
		oINPUT_LOG(double _Timestamp = 0.0, int _InputID = oInvalid, bool _IsDown = false)
			: Timestamp(_Timestamp)
			, InputID(_InputID)
			, IsDown(_IsDown)
		{}

		double Timestamp;
		int InputID;
		bool IsDown;
		bool operator==(const oINPUT_STATE& _That) const { return InputID == _That.InputID && IsDown == _That.IsDown; }
		bool operator!=(const oINPUT_STATE& _That) const { return !(*this == _That); }
	};
	
	std::vector<oINPUT_LOG> History;
	size_t Latest;
};

struct oInputMapperImpl : oInputMapper
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oInputMapperImpl(bool* _pSuccess);

	void SetInputSet(threadsafe oInputSet* _pInputSet) threadsafe override;

	int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe override { return Hooks.Hook(_Hook); }
	void UnhookActions(int _HookID) threadsafe override { Hooks.Unhook(_HookID); }

	// Call this from the key event handler to record the event.
	void OnAction(const oGUI_ACTION_DESC& _Action) threadsafe override;
	void OnLostCapture() threadsafe override;

	oRef<threadsafe oInputSet> InputSet;
	oInputHistory InputHistory;
	shared_mutex Mutex;
	oRefCount RefCount;
	oActionHookHelper Hooks;
};

oInputMapperImpl::oInputMapperImpl(bool* _pSuccess)
	: InputHistory(1500)
{
	*_pSuccess = true;
}

bool oInputMapperCreate(threadsafe oInputMapper** _ppInputMapper)
{
	bool success = false;
	oCONSTRUCT(_ppInputMapper, oInputMapperImpl(&success));
	return success;
}

void oInputMapperImpl::SetInputSet(threadsafe oInputSet* _pInputSet) threadsafe
{
	lock_guard<shared_mutex> lock(Mutex);
	InputSet = _pInputSet;
};

void oInputMapperImpl::OnAction(const oGUI_ACTION_DESC& _Action) threadsafe
{
	switch (_Action.Action)
	{
		case oGUI_ACTION_KEY_DOWN:
		case oGUI_ACTION_KEY_UP:
		{
			shared_lock<shared_mutex> lock(Mutex);
			if (InputSet)
			{
				oInputSetImpl* pInputSet = oThreadsafe(static_cast<threadsafe oInputSetImpl*>(InputSet.c_ptr()));
				for (int i = 0; i < oInt(pInputSet->Inputs.size()); i++)
				{
					bool InputDown = false;
					if (pInputSet->Inputs[i].OnKey(_Action.Key, _Action.Action == oGUI_ACTION_KEY_DOWN, &InputDown))
					{
						// this is not really safe. It is only safe if OnAction is only ever 
						// called from one thread for one instance, which tends to be the
						// case, so more wasn't done here to ensure concurrency. The 
						// threadsafe marker of this API refers more to the idea that 
						// changing the input set is safe in one thread while updating it in 
						// another.
						thread_cast<oInputHistory&>(InputHistory).Add(_Action.Timestamp, i, InputDown);

						oGUI_ACTION_DESC a(_Action);
						a.Action = oGUI_ACTION_CONTROL_ACTIVATED;
						a.ActionCode = i; // at least trigger this input
						
						// Check to see if it gets overridden by a sequence
						for (int s = 0; s < oInt(pInputSet->InputSequences.size()); s++)
						{
							if (thread_cast<oInputHistory&>(InputHistory).SequenceTriggered(pInputSet->InputSequences[s], _Action.Timestamp))
							{
								a.ActionCode = pInputSet->InputSequences[s].InputID;
								break;
							}
						}

						oTRACE("Input %d %s", a.ActionCode, (_Action.Action == oGUI_ACTION_KEY_DOWN) ? "down" : "up");
						Hooks.Call(a);
					}
				}
			}
			break;
		}

		default:
			break;
	}
}

void oInputMapperImpl::OnLostCapture() threadsafe
{
	shared_lock<shared_mutex> lock(Mutex);
	if (InputSet)
	{
		oInputSetImpl* pInputSet = oThreadsafe(static_cast<threadsafe oInputSetImpl*>(InputSet.c_ptr()));
		oFOR(auto& Input, pInputSet->Inputs)
			Input.Clear();
	}
}
