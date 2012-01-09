// $(header)
#pragma once
#ifndef oKeyState_h
#define oKeyState_h

// Useful threadsafe storage for an array of keys/buttons like for keyboards
// and mice.

#include <oBasis/oStdAtomic.h>

template<size_t size> class oKeyState
{
	int State[size];
public:
	enum STATE
	{
		UP,
		RELEASED,
		PRESSED,
		DOWN,
		REPEATED,
	};

	oKeyState()
	{
		memset(State, 0, sizeof(State));
	}

	void PromoteReleasedPressedToUpDown() threadsafe
	{
		for (int i = 0; i < oCOUNTOF(State); i++)
		{
			int oldState, newState;
			do
			{
				oldState = State[i];
				if (oldState == PRESSED || oldState == REPEATED)
					newState = DOWN;
				else if (oldState == RELEASED)
					newState = UP;
				else
					break;

			} while (!oStd::atomic_compare_exchange(&State[i], newState, oldState));
		}
	}

	void RecordUpDown(int _Index, bool _IsDown) threadsafe
	{
		if (_Index >= oCOUNTOF(State))
			return;

		int oldState, newState;
		do
		{
			oldState = State[_Index];

			if (!_IsDown)
				newState = (State[_Index] >= PRESSED) ? RELEASED : UP;
			else 
				newState = (State[_Index] >= PRESSED) ? REPEATED : PRESSED;
		} while (!oStd::atomic_compare_exchange(&State[_Index], newState, oldState));
	}

	int FindFirstPressed(int _Start = 0) const threadsafe
	{
		for (int i = _Start; i < oCOUNTOF(State); i++)
			if (IsPressed(i))
				return i;
		return -1;
	}

	bool IsUp(int _Index) const threadsafe { return State[_Index] == UP || State[_Index] == RELEASED; }
	bool IsReleased(int _Index) const threadsafe { return State[_Index] == RELEASED; }
	bool IsDown(int _Index) const threadsafe { return State[_Index] >= PRESSED; }
	bool IsPressed(int _Index) const threadsafe { return State[_Index] == PRESSED || State[_Index] == REPEATED; }
};

#endif
