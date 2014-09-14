// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oInputMapper.h>
#include <oBasis/oError.h>
#include <oBasis/oRefCount.h>
#include <oConcurrency/mutex.h>
#include <array>

using namespace ouro;
using namespace std;

class oActionHookHelper
{
public:
	oActionHookHelper()
	{
		Hooks.reserve(8);
	}

	inline int Hook(const input::action_hook& _Hook) threadsafe
	{
		lock_t lockB(oThreadsafe(Mutex));
		size_t index = sparse_set(oThreadsafe(Hooks), _Hook);
		return as_int(index);
	}

	inline void Unhook(int _HookID) threadsafe
	{
		lock_t lock(oThreadsafe(Mutex));
		ranged_set(oThreadsafe(Hooks), _HookID, nullptr);
	}

	void Call(const input::action& _Action) threadsafe
	{
		lock_shared_t lock(oThreadsafe(Mutex));
		for (const auto& h : oThreadsafe(Hooks))
			h(_Action);
	}

private:
	typedef ouro::shared_mutex mutex_t;
	typedef ouro::lock_guard<mutex_t> lock_t;
	typedef ouro::shared_lock<mutex_t> lock_shared_t;
	mutex_t Mutex;
	std::vector<input::action_hook> Hooks;
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
		for (auto& k : Keys)
			k.fill(input::none);
	}

	// Returns true if InputIsDown should be respected. Several keys can make up
	// an input result, so it could take several key events to get the state of 
	// the input. It can either be down or up based on the value of InputIsDown.
	bool OnKey(input::key _Key, bool _IsDown, bool* _pInputIsDown);

private:
	// Up to 4 keys can be pressed at the same time to trigger a Input.
	// Up to 7 different combinations can trigger a Input.
	std::array<std::array<input::key, 4>, 4> Keys;
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
		lstring temp(_InputMapping);

		char* ctx = nullptr;
		char* tok = strtok_s(temp, oWHITESPACE, &ctx);
		int KeySetIndex = 0, KeyIndex = 0;
		while (tok)
		{
			if (!_stricmp("OR", tok))
			{
				if (++KeySetIndex >= as_int(Keys.size()))
					oTHROW(no_buffer_space, "Only %u sets of keys OR'ed together are allowed.", Keys.size());
				KeyIndex = 0;
			}

			else
			{
				if (KeyIndex >= as_int(Keys[KeySetIndex].size()))
					oTHROW(no_buffer_space, "Only up to %u simultaneous keys supported", Keys[KeySetIndex].size());

				input::key Key = input::none;
				if (!oRTTI_OF(ouro_input_key).FromString(tok, &Key, sizeof(Key)))
					oTHROW(protocol_error, "unrecognized input::key \"%s\"", tok);

				Keys[KeySetIndex][KeyIndex] = Key;

				const int Bit = (KeySetIndex * 4) + KeyIndex;
				StateValidMask |= (1<<Bit);

				KeyIndex++;
			}

			tok = strtok_s(nullptr, oWHITESPACE, &ctx);
		}
	}
}

bool oInput::OnKey(input::key _Key, bool _IsDown, bool* _pInputIsDown)
{
	const bool WasDown = IsDown();
	int StateMask = 1;
	for (const auto& K : Keys)
	{
		for (int i = 0; i < 4; i++)
		{
			if (K[i] != input::none && K[i] == _Key)
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
	oInputSequence(const xml& _XML, xml::node _hInputSequence, const oRTTI& _IDEnum, bool* _pSuccess);

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

oInputSequence::oInputSequence(const xml& _XML, xml::node _hInputSequence, const oRTTI& _IDEnum, bool* _pSuccess)
	: MinTimeMS(invalid)
	, MaxTimeMS(invalid)
	, InputID(invalid)
{
	*_pSuccess = false;

	const char* IDStr = _XML.find_attr_value(_hInputSequence, "ID");
	if (!_IDEnum.FromString(IDStr, &InputID, sizeof(int)))
	{
		sstring name;
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
		lstring temp(_SeqMapping);

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
				sstring name;
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

	oInputSetImpl(const xml& _XML, xml::node _InputSet, const oRTTI& _IDEnum, bool* _pSuccess);

	const oRTTI& GetRTTI() const threadsafe { return oThreadsafe(this)->IDRTTI; }
	const char* GetName() const threadsafe override { return oThreadsafe(this)->Name; }

	const oRTTI& IDRTTI;
	mstring Name;
	std::vector<oInput> Inputs;
	std::vector<oInputSequence> InputSequences;
	unsigned int MaxTimeMS;
	oRefCount RefCount;
};

oInputSetImpl::oInputSetImpl(const xml& _XML, xml::node _InputSet, const oRTTI& _IDEnum, bool* _pSuccess)
	: IDRTTI(_IDEnum)
{
	*_pSuccess = false;

	MaxTimeMS = invalid;
	_XML.find_attr_value(_InputSet, "id", &Name);
	for (xml::node hChild = _XML.first_child(_InputSet); hChild; hChild = _XML.next_sibling(hChild))
	{
		if (!_stricmp("oInput", _XML.node_name(hChild)))
		{
			const char* IDString = _XML.find_attr_value(hChild, "ID");
			if (!oSTRVALID(IDString))
			{
				oErrorSetLast(std::errc::protocol_error, "an ID must be specified for each oInput in oInputSet(%s) in %s", Name.c_str(), _XML.name());
				return;
			}

			int EnumValue = invalid;
			if (!_IDEnum.FromString(IDString, &EnumValue, sizeof(int)))
			{
				mstring temp;
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
			safe_set(Inputs, EnumValue, Input);
		}

		else if (!_stricmp("oInputSequence", _XML.node_name(hChild)))
		{
			bool success = false;
			oInputSequence Seq(_XML, hChild, _IDEnum, &success);
			if (!success)
				return; // pass through error

			InputSequences.push_back(std::move(Seq));
			MaxTimeMS = __max(MaxTimeMS, static_cast<unsigned int>(Seq.MaxTimeMS));
		}
	}

	*_pSuccess = true;
}

bool oInputSetCreate(const xml& _XML, xml::node _InputSet, const oRTTI& _IDEnum, oInputSet** _ppInputSet)
{
	bool success = false;
	oCONSTRUCT(_ppInputSet, oInputSetImpl(_XML, _InputSet, _IDEnum, &success));
	return success;
}

bool oParseInputSetList(const xml& _XML, xml::node _InputSetList, const oRTTI& _InputEnum, threadsafe oInputSet** _ppInputSet)
{
	const char* Name = _XML.node_name(_InputSetList);
	if (_stricmp(Name, "oInputSetList"))
		return oErrorSetLast(std::errc::invalid_argument, "The specified node is not an \"oInputSetList\" in %s", _XML.name());

	mstring EnumName;
	_InputEnum.GetName(EnumName);
	xml::node hInputSet = _XML.first_child(_InputSetList, "oInputSet");
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
		: Latest(invalid)
	{
		History.resize(_HistorySize);
		fill(History, oINPUT_LOG(0, invalid, false));
	}

	void Add(unsigned int _TimestampMS, int _InputID, bool _IsDown)
	{
		Latest = (Latest + 1) % History.size();
		History[Latest] = oINPUT_LOG(_TimestampMS, _InputID, _IsDown);
	}

	bool SequenceTriggered(const oInputSequence& _InputSequence, unsigned int _TriggerTimeMS) const
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
		const unsigned int LastTimestamp = _InputSequence.MaxTimeMS == invalid ? invalid : (_TriggerTimeMS - _InputSequence.MaxTimeMS);

		while (Current != Latest)
		{
			if (History[Current].TimestampMS < LastTimestamp)
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
		oINPUT_LOG(unsigned int _TimestampMS = 0, int _InputID = invalid, bool _IsDown = false)
			: TimestampMS(_TimestampMS)
			, InputID(_InputID)
			, IsDown(_IsDown)
		{}

		unsigned int TimestampMS;
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

	int HookActions(const input::action_hook& _Hook) threadsafe override { return Hooks.Hook(_Hook); }
	void UnhookActions(int _HookID) threadsafe override { Hooks.Unhook(_HookID); }

	// Call this from the key event handler to record the event.
	void OnAction(const input::action& _Action) threadsafe override;
	void OnLostCapture() threadsafe override;

	intrusive_ptr<threadsafe oInputSet> InputSet;
	oInputHistory InputHistory;
	typedef ouro::shared_mutex mutex_t;
	typedef ouro::lock_guard<mutex_t> lock_t;
	typedef ouro::shared_lock<mutex_t> lock_shared_t;
	mutex_t Mutex;
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
	lock_t lock(oThreadsafe(Mutex));
	InputSet = _pInputSet;
};

void oInputMapperImpl::OnAction(const input::action& _Action) threadsafe
{
	switch (_Action.action_type)
	{
		case input::action_type::key_down:
		case input::action_type::key_up:
		{
			lock_shared_t lock(oThreadsafe(Mutex));
			if (InputSet)
			{
				oInputSetImpl* pInputSet = oThreadsafe(static_cast<threadsafe oInputSetImpl*>(InputSet.c_ptr()));
				for (int i = 0; i < as_int(pInputSet->Inputs.size()); i++)
				{
					bool InputDown = false;
					if (pInputSet->Inputs[i].OnKey(_Action.key, _Action.action_type == input::action_type::key_down, &InputDown))
					{
						// this is not really safe. It is only safe if OnAction is only ever 
						// called from one thread for one instance, which tends to be the
						// case, so more wasn't done here to ensure concurrency. The 
						// threadsafe marker of this API refers more to the idea that 
						// changing the input set is safe in one thread while updating it in 
						// another.
						thread_cast<oInputHistory&>(InputHistory).Add(_Action.timestamp_ms, i, InputDown);

						input::action a(_Action);
						a.action_type = input::action_type::control_activated;
						a.action_code = i; // at least trigger this input
						
						// Check to see if it gets overridden by a sequence
						for (int s = 0; s < as_int(pInputSet->InputSequences.size()); s++)
						{
							if (thread_cast<oInputHistory&>(InputHistory).SequenceTriggered(pInputSet->InputSequences[s], _Action.timestamp_ms))
							{
								a.action_code = pInputSet->InputSequences[s].InputID;
								break;
							}
						}

						oTRACE("Input %d %s", a.action_code, (_Action.action_type == input::action_type::key_down) ? "down" : "up");
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
	lock_shared_t lock(oThreadsafe(Mutex));
	if (InputSet)
	{
		oInputSetImpl* pInputSet = oThreadsafe(static_cast<threadsafe oInputSetImpl*>(InputSet.c_ptr()));
		for (auto& Input : pInputSet->Inputs)
			Input.Clear();
	}
}
