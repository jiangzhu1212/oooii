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
#pragma once
#ifndef oKeyState_h
#define oKeyState_h

// Useful threadsafe storage for an array of keys/buttons like for keyboards
// and mice.

#include <oStd/oStdAtomic.h>

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
		oFORI(i, State)
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
