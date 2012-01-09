// $(header)
#include <oBasis/oConcurrentStack.h>
#include "oBasisTestCommon.h"
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMemory.h>
#include <oBasis/oTask.h>

struct Node
{
	Node* pNext;
	int Value;
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

	int i = oCOUNTOF(values);
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

	oCountdownLatch latch("Sync", oCOUNTOF(nodes));
	for (int i = 0; i < oCOUNTOF(nodes); i++)
		oTaskIssueAsync([&,i] { nodes[i].Value = i; s.push(&nodes[i]); latch.Release(); } );

	latch.Wait();

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

	latch.Reset(oCOUNTOF(nodes));
	for (int i = 0; i < oCOUNTOF(nodes); i++)
		oTaskIssueAsync([&] { Node* popped = s.pop(); popped->Value = 0xdeaddead, latch.Release(); } );

	latch.Wait();

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