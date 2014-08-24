/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oBase/assert.h>
#include <oBase/allocate.h>
#include <oBase/concurrent_object_pool.h>
#include <oBase/concurrent_stack.h>
#include <oBase/concurrency.h>
#include <oBase/finally.h>
#include <oBase/macros.h>
#include <oBase/throw.h>
#include <vector>

namespace ouro {
	namespace tests {

struct Node
{
	Node() { memset(this, 0xaa, sizeof(Node)); }
	Node* next;
	size_t value;
};

static void test_intrusive_basics()
{
	Node* values = (Node*)default_allocate(sizeof(Node) * 5, memory_alignment::align_to_8);
	finally dealloc([&] { default_deallocate(values); });
	concurrent_stack<Node> s;
	oCHECK(s.empty(), "Stack should be empty (init)");

	for (int i = 0; i < 5; i++)
	{
		values[i].value = i;
		s.push(&values[i]);
	}

 	oCHECK(s.size() == 5, "Stack size is not correct");

	Node* v = s.peek();
	oCHECK(v && v->value == 4, "Stack value is not correct (peek)");

	v = s.pop();
	oCHECK(v && v->value == 4, "Stack value is not correct (pop 1)");

	v = s.pop();
	oCHECK(v && v->value == 3, "Stack value is not correct (pop 2)");

	for (size_t i = 2; i < 5; i++)
		s.pop();

	oCHECK(s.empty(), "Stack should be empty (after pops)");

	// reinitialize
	for (int i = 0; i < 5; i++)
	{
		values[i].value = i;
		s.push(&values[i]);
	}

	v = s.pop_all();

	size_t i = 5;
	while (v)
	{
		oCHECK(v->value == --i, "Stack value is not correct (pop_all)");
		v = v->next;
	}
}

static void test_intrusive_concurrency()
{
	Node* nodes = (Node*)default_allocate(sizeof(Node) * 40, memory_alignment::align_to_8);
	finally dealloc([&] { default_deallocate(nodes); });

	memset(nodes, 0xaa, sizeof(nodes));

	concurrent_stack<Node> s;

	ouro::parallel_for(0, 40, [&](size_t _Index)
	{
		nodes[_Index].value = _Index;
		s.push(&nodes[_Index]);
	});

	Node* n = s.peek();
	while (n)
	{
		n->value = 0xc001c0de;
		n = n->next;
	}

	for (int i = 0; i < 40; i++)
	{
		oCHECK(nodes[i].value != 0xaaaaaaa, "Node %d was never processed by task system", i);
		oCHECK(nodes[i].value == 0xc001c0de, "Node %d was never inserted into stack", i);
	}
	
	ouro::parallel_for(0, 40, [&](size_t _Index)
	{
		Node* popped = s.pop();
		popped->value = 0xdeaddead;
	});
	
	oCHECK(s.empty(), "Stack should be empty");

	for (int i = 0; i < 40; i++)
		oCHECK(nodes[i].value == 0xdeaddead, "Node %d was not popped correctly", i);
}

void TESTconcurrent_stack()
{
	test_intrusive_basics();
	test_intrusive_concurrency();
}

	} // namespace tests
} // namespace ouro
