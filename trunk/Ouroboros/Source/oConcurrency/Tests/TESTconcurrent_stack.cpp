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
#include <oConcurrency/concurrent_stack.h>
#include <oStd/algorithm.h>
#include <oStd/macros.h>

namespace oConcurrency {
	namespace tests {

struct Node
{
	Node* pNext;
	size_t Value;
};

static void test_basics()
{
	Node values[5];
	concurrent_stack<Node> s;
	oCHECK(s.empty(), "Stack should be empty (init)");

	oFORI(i, values)
	{
		values[i].Value = i;
		s.push(&values[i]);
	}

	oCHECK(s.size() == oCOUNTOF(values), "Stack size is not correct");

	Node* v = s.peek();
	oCHECK(v && v->Value == oCOUNTOF(values)-1, "Stack value is not correct (peek)");

	v = s.pop();
	oCHECK(v && v->Value == oCOUNTOF(values)-1, "Stack value is not correct (pop 1)");

	v = s.pop();
	oCHECK(v && v->Value == oCOUNTOF(values)-2, "Stack value is not correct (pop 2)");

	for (size_t i = 2; i < oCOUNTOF(values); i++)
		s.pop();

	oCHECK(s.empty(), "Stack should be empty (after pops)");

	// reinitialize
	oFORI(i, values)
	{
		values[i].Value = i;
		s.push(&values[i]);
	}

	v = s.pop_all();

	size_t i = oCOUNTOF(values);
	while (v)
	{
		oCHECK(v->Value == --i, "Stack value is not correct (pop_all)");
		v = v->pNext;
	}
}

static void test_concurrency()
{
	Node nodes[4000];
	memset(nodes, 0xaa, sizeof(nodes));

	concurrent_stack<Node> s;

	parallel_for(0, oCOUNTOF(nodes), [&](size_t _Index)
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

	oFORI(i, nodes)
	{
		oCHECK(nodes[i].Value != 0xaaaaaaa, "Node %d was never processed by task system", i);
		oCHECK(nodes[i].Value == 0xc001c0de, "Node %d was never inserted into stack", i);
	}
	
	parallel_for(0, oCOUNTOF(nodes), [&](size_t _Index)
	{
		Node* popped = s.pop();
		popped->Value = 0xdeaddead;
	});
	
	oCHECK(s.empty(), "Stack should be empty");

	oFORI(i, nodes)
		oCHECK(nodes[i].Value == 0xdeaddead, "Node %d was not popped correctly", i);
}

void TESTconcurrent_stack()
{
	test_basics();
	test_concurrency();
}

	} // namespace tests
} // namespace oConcurrency
