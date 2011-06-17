// $(header)
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
