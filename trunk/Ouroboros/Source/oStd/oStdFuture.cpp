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
#include <oStd/oStdFuture.h>

namespace oStd
{
	namespace future_detail
	{
		class future_category_impl : public std::error_category
		{
		public:
			const char* name() const override { return "future"; }
			std::string message(value_type _ErrCode) const override
			{
				switch (_ErrCode)
				{
					case future_errc::broken_promise: return "broken_promise: This means the promise has been destroyed before it has set a value or error code and the future may wait forever for a value that isn't going to come.";
					case future_errc::future_already_retrieved: return "future_already_retrieved: get_future can only be called once per promise. Use shared_future for multiple futures.";
					case future_errc::promise_already_satisfied: return "promise_already_satisfied: this promise has already had a value or error set on it. Thus the future waiting on the value may have already unblocked itself as a result.";
					case future_errc::no_state: return "no_state: The future/promise is not initialized.";
					default: return "unrecognized future state";
				}
			}
		};

		void* oCommitmentAllocate(size_t _SizeofCommitment)
		{
			return ::new char[_SizeofCommitment];
		}

		void oCommitmentDeallocate(void* _pPointer)
		{
			::delete [] _pPointer;
		}

	} // namespace future_detail

	const std::error_category& future_category()
	{
		static future_detail::future_category_impl sSingleton;
		return sSingleton;
	}

} // namespace oStd