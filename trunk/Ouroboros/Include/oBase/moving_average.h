// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// A small class to encapsulate calculating a cumulative moving average

#pragma once

namespace ouro {

template<typename T>
class moving_average
{
public:
	moving_average()
		: CA(T(0))
		, Count(T(0))
	{}

	// Given the latest sample value, this returns the moving average for all 
	// values up to the latest specified.
	T calculate(const T& value)
	{
		Count += T(1);
		CA = CA + ((value - CA) / Count);
		return CA;
	}

private:
	T CA;
	T Count;
};

}
