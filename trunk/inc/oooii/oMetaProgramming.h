#pragma once
#ifndef oMetaProgramming_h
#define oMetaProgramming_h

template<bool condition, class Then, class Else>
struct IF
{ 
	typedef Then RET;
};

template<class Then, class Else>
struct IF<false,Then,Else>
{
	typedef Else RET;
};

#endif