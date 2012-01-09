// $(header)
// Prefer inheriting from this base class over defining a private copy 
// constructor and assignment operator because this is more self-documenting.
#pragma once
#ifndef oNoncopyable_h
#define oNoncopyable_h

class oNoncopyable
{
	oNoncopyable(const oNoncopyable&);
	const oNoncopyable& operator=(const oNoncopyable&);
protected:
	oNoncopyable() {}
	~oNoncopyable() {}
};

#endif
