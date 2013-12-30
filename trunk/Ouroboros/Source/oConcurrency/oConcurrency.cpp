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
#include <oConcurrency/oConcurrency.h>
#include <oConcurrency/concurrent_queue_base.h>
#include <oBase/config.h>

namespace oConcurrency {
	namespace detail {
		class detail_container_category : public std::error_category
		{
		public:
			const char* name() const override { return "container"; }
			std::string message(int _ErrCode) const override
			{
				switch (_ErrCode)
				{
					case container_errc::not_empty: return "not_empty: a container is being destroyed when there are still elements";
					default: return "unrecognized container error";
				}
			}
		};

		class detail_threadpool_category : public std::error_category
		{
		public:
			const char* name() const override { return "threadpool"; }
			std::string message(int _ErrCode) const override
			{
				switch (_ErrCode)
				{
					case threadpool_errc::call_after_join: return "call_after_join: an api call was made on an objects after its join was called, indicating a logic error in client code";
					default: return "unrecognized oConcurrency error";
				}
			}
		};
	} // namespace detail


	const std::error_category& container_category()
	{
		static detail::detail_container_category sSingleton;
		return sSingleton;
	}

	const std::error_category& threadpool_category()
	{
		static detail::detail_threadpool_category sSingleton;
		return sSingleton;
	}

} // namespace oConcurrency
