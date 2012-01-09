// $(header)
// Approximation of the upcoming C++11 std::ratio interface
#pragma once
#ifndef oStdRatio_h
#define oStdRatio_h

namespace oStd {

template<long long N, long long D = 1> class ratio
{
	// @oooii-tony: This is not even close to the standards requirement of 
	// reduction of terms or sign conservation and doesn't support any of the 
	// meta-math yet.

public:
	static const long long num = N;
	static const long long den = D;
};

typedef ratio<1, 1000000000000000000> atto;
typedef ratio<1, 1000000000000000> femto;
typedef ratio<1, 1000000000000> pico;
typedef ratio<1, 1000000000> nano;
typedef ratio<1, 1000000> micro;
typedef ratio<1, 1000> milli;
typedef ratio<1, 100> centi;
typedef ratio<1, 10> deci;
typedef ratio<10, 1> deca;
typedef ratio<100, 1> hecto;
typedef ratio<1000, 1> kilo;
typedef ratio<1000000, 1> mega;
typedef ratio<1000000000, 1> giga;
typedef ratio<1000000000000, 1> tera;
typedef ratio<1000000000000000, 1> peta;
typedef ratio<1000000000000000000, 1> exa;

} // namespace oStd

#endif
