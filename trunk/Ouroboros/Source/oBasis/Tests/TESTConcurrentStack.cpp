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
#include <oBasis/oConcurrentStack.h>
#include "oBasisTestCommon.h"
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMemory.h>
#include <oBasis/oRef.h>
#include <oBasis/oTask.h>

struct Node
{
	Node* pNext;
	size_t Value;
};

static bool oBasisTest_oConcurrentStack_Trivial()
{
	Node values[5];
	oConcurrentStack<Node> s;
	oTESTB(s.empty(), "Stack should be empty (init)");

	for (int i = 0; i < oCOUNTOF(values); i++)
	{
		values[i].Value = i;
		s.push(&values[i]);
	}

	oTESTB(s.size() == oCOUNTOF(values), "Stack size is not correct");

	Node* v = s.peek();
	oTESTB(v && v->Value == oCOUNTOF(values)-1, "Stack value is not correct (peek)");

	v = s.pop();
	oTESTB(v && v->Value == oCOUNTOF(values)-1, "Stack value is not correct (pop 1)");

	v = s.pop();
	oTESTB(v && v->Value == oCOUNTOF(values)-2, "Stack value is not correct (pop 2)");

	for (size_t i = 2; i < oCOUNTOF(values); i++)
		s.pop();

	oTESTB(s.empty(), "Stack should be empty (after pops)");

	// reinialize
	for (int i = 0; i < oCOUNTOF(values); i++)
	{
		values[i].Value = i;
		s.push(&values[i]);
	}

	v = s.pop_all();

	size_t i = oCOUNTOF(values);
	while (v)
	{
		oTESTB(v->Value == --i, "Stack value is not correct (pop_all)");
		v = v->pNext;
	}

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

static bool oBasisTest_oConcurrentStack_Concurrency()
{
	Node nodes[4000];
	oMemset4(nodes, 0xbaadc0de, sizeof(nodes));

	oConcurrentStack<Node> s;

	oTaskParallelFor(0, oCOUNTOF(nodes), [&](size_t _Index)
	{
		nodes[_Index].Value = _Index;
		s.push(&nodes[_Index]);
	});

	Node* n = s.peek();
	while (n)
	{
		n->Value = 0xc001c0de;
		n = n->pNext;
	}

	for (int i = 0; i < oCOUNTOF(nodes); i++)
	{
		oTESTB(nodes[i].Value != 0xbaadc0de, "Node %d was never processed by task system", i);
		oTESTB(nodes[i].Value == 0xc001c0de, "Node %d was never inserted into stack", i);
	}
	
	oTaskParallelFor(0, oCOUNTOF(nodes), [&](size_t _Index)
	{
		Node* popped = s.pop();
		popped->Value = 0xdeaddead;
	});
	
	oTESTB(s.empty(), "Stack should be empty");

	for (int i = 0; i < oCOUNTOF(nodes); i++)
		oTESTB(nodes[i].Value == 0xdeaddead, "Node %d was not popped correctly", i);

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

bool oBasisTest_oConcurrentStack()
{
	if (!oBasisTest_oConcurrentStack_Trivial())
		return false;
	if (!oBasisTest_oConcurrentStack_Concurrency())
		return false;
	oErrorSetLast(oERROR_NONE, "");
	return true;
}