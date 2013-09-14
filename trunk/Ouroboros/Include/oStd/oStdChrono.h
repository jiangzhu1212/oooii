/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
// Approximation of the upcoming C++11 std::chrono interface
#pragma once
#ifndef oStdChrono_h
#define oStdChrono_h

#include <oStd/equal.h>
#include <oStd/oStdRatio.h>
#include <oStd/config.h>

namespace oStd {

#ifdef oHAS_BAD_DOUBLE_TO_ULLONG_CONVERSION
	unsigned long long dtoull(double n);
#endif

	namespace chrono {

// double -> ullong is broken in VS2010, so wrap it to a custom implementation
template<typename T, typename U> T chrono_static_cast(const U& _Value) { return static_cast<T>(_Value); }

#ifdef oHAS_BAD_DOUBLE_TO_ULLONG_CONVERSION
		template<> inline unsigned long long chrono_static_cast(const double& _Value) { return dtoull(_Value); }
#endif

template<typename Rep> struct treat_as_floating_point : std::tr1::is_floating_point<Rep> {};

template<class Rep, class Period=ratio<1> > class duration
{
	Rep Value;

public:
	typedef Rep rep;
	typedef Period period;

	duration() : Value(zero().count()) {}
	duration(const duration& _Duration) : Value(_Duration.Value) {}

	// VS2010 doesn't support this yet, so do the check as a static_assert below
	//template<typename _Rep2, typename = typename std::enable_if<std::is_convertible<_Rep2, rep>::value && (treat_as_floating_point<rep>::value || !treat_as_floating_point<_Rep2>::value)>::type>
	template<typename Rep2>
	explicit duration(const Rep2& _Duration)
		: Value(chrono_static_cast<Rep>(_Duration))
	{
		static_assert((std::tr1::is_convertible<Rep2, rep>::value) && (treat_as_floating_point<rep>::value || !treat_as_floating_point<Rep2>::value), "Incompatible duration type conversion (if you're trying to use oStd::chrono::seconds, consider using oSeconds instead)");
	}

	//template<typename Rep2, typename Period2, typename = typename enable_if<treat_as_floating_point<rep>::value || (ratio_divide<Period2, period>::type::den == 1 && !treat_as_floating_point<Rep2>::value)>::type>
	template<typename Rep2, typename Period2>
	duration(const duration<Rep2, Period2>& _Duration) 
		: Value(duration_cast<duration>(_Duration).count())
	{
		static_assert((std::tr1::is_convertible<Rep2, rep>::value) && (treat_as_floating_point<rep>::value || !treat_as_floating_point<Rep2>::value), "Incompatible duration type conversion (if you're trying to use oStd::chrono::seconds, consider using oSeconds instead)");
	}
	
	Rep count() const { return Value; }
	duration operator+() const { return *this; }
	duration operator-() const { return duration(-Value); }
	duration& operator++() { Value += chrono_static_cast<rep>(1); return *this; }
	duration operator++(int) { duration d(*this); Value += chrono_static_cast<rep>(1); return d; }
	duration& operator--() { Value -= chrono_static_cast<rep>(1); return *this; }
	duration operator--(int) { duration d(*this); Value -= chrono_static_cast<rep>(1); return d; }
	duration& operator+=(const duration& _That) { Value += _That.Value; return *this; }
	duration& operator-=(const duration& _That) { Value -= _That.Value; return *this; }
	duration& operator*=(const rep& _That) { Value *= _That.Value; return *this; }
	duration& operator/=(const rep& _That) { Value /= _That.Value; return *this; }

	static /*constexpr*/ duration zero() { return duration(chrono_static_cast<rep>(0)); }
	//static /*constexpr*/ duration min();
	//static /*constexpr*/ duration max();
};

template<typename ToDuration, typename Rep, typename Period> ToDuration duration_cast(const duration<Rep, Period>& _Duration)
{
	//always casting to double for now. look at gcc for the gcd method to get around this if needed for performance.
	double Seconds = (_Duration.count() * (double)Period::num) / (double)Period::den;
	double newVal = (Seconds * (double)ToDuration::period::den) / (double)ToDuration::period::num;
	return ToDuration(chrono_static_cast<ToDuration::rep>(newVal));
}

template<typename Rep1, typename Period1, typename Rep2, typename Period2> duration<typename std::common_type<Rep1, Rep2>::type, typename std::common_type<Period1, Period2>::type > operator+(const duration<Rep1, Period1>& x, const duration<Rep2, Period2>& y)
 {
	 typedef duration<typename std::common_type<Rep1, Rep2>::type, typename std::common_type<Period1, Period2>::type > ret;
	 ret d(x);
	 d += y;
	 return d;
 }

template<typename Rep, typename Period> bool operator==(const duration<Rep, Period>& x, const duration<Rep, Period>& y) { return x.count() == y.count(); }
template<typename Period> bool operator==(const duration<float, Period>& x, const duration<float, Period>& y) { return oStd::equal(x.count(), y.count()); }
template<typename Period> bool operator==(const duration<double, Period>& x, const duration<double, Period>& y) { return oStd::equal(x.count(), y.count()); }
template<typename Rep, typename Period> bool operator!=(const duration<Rep, Period>& x, const duration<Rep, Period>& y) { return !(x == y); }
template<typename Rep, typename Period> bool operator<(const duration<Rep, Period>& x, const duration<Rep, Period>& y) { return x.count() < y.count(); }
template<typename Rep, typename Period> bool operator>=(const duration<Rep, Period>& x, const duration<Rep, Period>& y) { return !(x < y); }
template<typename Rep, typename Period> bool operator<=(const duration<Rep, Period>& x, const duration<Rep, Period>& y) { return x < y || x == y; }
template<typename Rep, typename Period> bool operator>(const duration<Rep, Period>& x, const duration<Rep, Period>& y) { return !(x <= y); }
template<typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator==(const duration<Rep1, Period1>& x, const duration<Rep2, Period2>& y) { return x == duration_cast<duration<Rep1, Period1>(y); }
template<typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator!=(const duration<Rep1, Period1>& x, const duration<Rep2, Period2>& y) { return !(x == y); }
template<typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator<(const duration<Rep1, Period1>& x, const duration<Rep2, Period2>& y) { return x < duration_cast<duration<Rep1, Period1> >(y); }
template<typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator>=(const duration<Rep1, Period1>& x, const duration<Rep2, Period2>& y) { return !(x < y); }
template<typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator<=(const duration<Rep1, Period1>& x, const duration<Rep2, Period2>& y) { return x <= duration_cast<duration<Rep1, Period1> >(y); }
template<typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator>(const duration<Rep1, Period1>& x, const duration<Rep2, Period2>& y) { return !(x <= y); }

typedef duration<int, ratio<604800> > weeks;
typedef duration<int, ratio<86400> > days;
typedef duration<int, ratio<3600> > hours;
typedef duration<int, ratio<60> > minutes;
typedef duration<long long> seconds;
typedef duration<long long, milli> milliseconds;
typedef duration<long long, micro> microseconds;
typedef duration<long long, nano> nanoseconds;

template<class Clock, class Duration = typename Clock::duration> class time_point
{
	Duration D;
public:
	typedef Clock clock;
	typedef Duration duration;
	typedef typename duration::rep rep;
	typedef typename duration::period period;
	
	time_point() : D(duration::zero()) {}
	explicit time_point(const duration& _Duration) : D(_Duration) {}
	//template <class Duration2> time_point(const time_point<Clock, Duration2>& t);

	duration time_since_epoch() const { return D; }

	// TODO: Assert if operation pushes us over or under the epoch
	time_point& operator+=(const duration& d) { D += d; return *this; }
	time_point& operator-=(const duration& d) { D -= d; return *this; }

	static /*constexpr*/ time_point min() { return time_point(duration::min()); }
	static /*constexpr*/ time_point max() { return time_point(duration::max()); }
};

template<typename ToDuration, typename Clock, typename Duration> time_point<Clock, ToDuration> time_point_cast(const time_point<Clock, Duration>& _TimePoint) { return time_point<Clock, ToDuration>(duration_cast<ToDuration>(_TimePoint.time_since_epoch())); }
template<typename Clock, typename Duration1, typename Duration2> typename std::common_type<Duration1, Duration2>::type operator+(const time_point<Clock, Duration1>& x, const time_point<Clock, Duration2>& y) { return x.time_since_epoch() + y.time_since_epoch(); }
template<typename Clock, typename Duration1, typename Duration2> typename std::common_type<Duration1, Duration2>::type operator-(const time_point<Clock, Duration1>& x, const time_point<Clock, Duration2>& y) { return std::common_type<Duration1, Duration2>::type(x.time_since_epoch().count() - y.time_since_epoch().count()); }
template<typename Clock, typename Rep1, typename Period1, typename Rep2, typename Period2> time_point<Clock, duration<typename std::common_type<Rep1, Rep2>::type, typename std::common_type<Period1, Period2>::type> > operator+(const time_point<Clock, duration<Rep1, Period1> >& x, const duration<Rep2, Period2>& y) { return time_point<Clock, duration<typename std::common_type<Rep1, Rep2>::type, typename std::common_type<Period1, Period2>::type> >(x.time_since_epoch() + duration_cast<duration<Rep1, Period1> >(y)); }
template<typename Clock, typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator==(const time_point<Clock, duration<Rep1, Period1> >& x, const time_point<Clock, duration<Rep2, Period2> >& y) { return x.time_since_epoch() == time_point_cast<duration<Rep1, Period1> >(y).time_since_epoch(); }
template<typename Clock, typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator!=(const time_point<Clock, duration<Rep1, Period1> >& x, const time_point<Clock, duration<Rep2, Period2> >& y) { return !(x == y); }
template<typename Clock, typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator<(const time_point<Clock, duration<Rep1, Period1> >& x, const time_point<Clock, duration<Rep2, Period2> >& y) { return x.time_since_epoch() < time_point_cast<duration<Rep1, Period1> >(y).time_since_epoch(); }
template<typename Clock, typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator>=(const time_point<Clock, duration<Rep1, Period1> >& x, const time_point<Clock, duration<Rep2, Period2> >& y) { return !(x < y); }
template<typename Clock, typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator<=(const time_point<Clock, duration<Rep1, Period1> >& x, const time_point<Clock, duration<Rep2, Period2> >& y) { return (x < y) || (x == y); }
template<typename Clock, typename Rep1, typename Period1, typename Rep2, typename Period2> bool operator>(const time_point<Clock, duration<Rep1, Period1> >& x, const time_point<Clock, duration<Rep2, Period2> >& y) { return !(x <= y); }

class high_resolution_clock
{
public:
	typedef double rep;
	typedef oStd::ratio<1,1> period;
	typedef oStd::chrono::duration<rep, period> duration;
	typedef oStd::chrono::time_point<high_resolution_clock, duration> time_point;

	static const bool is_steady = true;
	static const bool is_monotonic = true;

	static time_point now();
};

class system_clock
{
public:
	typedef time_t rep;
	typedef oStd::ratio<1,1> period;
	typedef oStd::chrono::duration<rep,period> duration;
	typedef oStd::chrono::time_point<system_clock> time_point;
	
	static const bool is_steady = true;
	static const bool is_monotonic = true;

	static time_point now();

	static time_t to_time_t(const time_point& _TimePoint);
	static time_point from_time_t(time_t _TimePoint);
};

	} // namespace chrono
} // namespace oStd

namespace std {
	template <class Rep1, class Period1, class Rep2, class Period2>
	struct common_type<oStd::chrono::duration<Rep1, Period1>, oStd::chrono::duration<Rep2, Period2>>
	{
		typedef oStd::chrono::duration<
			typename std::common_type<Rep1, Rep2>::type,
			typename std::common_type<Period1, Period2>::type
		> type;
	};
}

// The C++11 standard ignores conversion with rounding but defines seconds as 
// unsigned long long, so you can't really work with milli and microseconds in 
// terms of fractions of a second. Anyway those in the pre- and final proposals 
// recommending spinning your own duration type based on double, so here it is. 
// What is really lame is the errors given don't indicate at all where your 
// error comes from if you use chrono::seconds with a double.
typedef oStd::chrono::duration<double> oSeconds;

#endif
