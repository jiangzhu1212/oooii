// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/assert.h>
#include <oConcurrency/concurrent_stack.h>
#include <oConcurrency/concurrency.h>
#include <vector>
#include "../../test_services.h"

namespace ouro { namespace tests {

struct Node
{
	Node() { memset(this, 0xaa, sizeof(Node)); }
	Node* next;
	size_t value;
};

static void test_intrusive_basics(test_services& services)
{
	Node* values = (Node*)default_allocate(sizeof(Node) * 5, memory_alignment::align8);
	test_services::finally dealloc([&] { default_deallocate(values); });
	concurrent_stack<Node> s;
	oTEST(s.empty(), "Stack should be empty (init)");

	for (int i = 0; i < 5; i++)
	{
		values[i].value = i;
		s.push(&values[i]);
	}

 	oTEST(s.size() == 5, "Stack size is not correct");

	Node* v = s.peek();
	oTEST(v && v->value == 4, "Stack value is not correct (peek)");

	v = s.pop();
	oTEST(v && v->value == 4, "Stack value is not correct (pop 1)");

	v = s.pop();
	oTEST(v && v->value == 3, "Stack value is not correct (pop 2)");

	for (size_t i = 2; i < 5; i++)
		s.pop();

	oTEST(s.empty(), "Stack should be empty (after pops)");

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
		oTEST(v->value == --i, "Stack value is not correct (pop_all)");
		v = v->next;
	}
}

static void test_intrusive_concurrency(test_services& services)
{
	std::vector<char> buf(sizeof(Node) * 40);
	Node* nodes = (Node*)buf.data();
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
		oTEST(nodes[i].value != 0xaaaaaaa, "Node %d was never processed by task system", i);
		oTEST(nodes[i].value == 0xc001c0de, "Node %d was never inserted into stack", i);
	}
	
	ouro::parallel_for(0, 40, [&](size_t _Index)
	{
		Node* popped = s.pop();
		popped->value = 0xdeaddead;
	});
	
	oTEST(s.empty(), "Stack should be empty");

	for (int i = 0; i < 40; i++)
		oTEST(nodes[i].value == 0xdeaddead, "Node %d was not popped correctly", i);
}

void TESTconcurrent_stack(test_services& services)
{
	test_intrusive_basics(services);
	test_intrusive_concurrency(services);
}

}}
