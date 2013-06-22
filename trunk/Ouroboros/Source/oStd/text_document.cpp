/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oStd/text_document.h>
#include <oStd/config.h>

namespace oStd
{
	namespace text_document_detail
	{
		class text_document_category_impl : public std::error_category
		{
		public:
			const char* name() const override { return "text_document"; }
			std::string message(value_type _ErrCode) const override
			{
				switch (_ErrCode)
				{
					case text_document_errc::generic_parse_error: return "generic_parse_error: an unspecified parsing error occurred";
					case text_document_errc::unclosed_scope: return "unclosed_scope: an open brace/tag was not matched by a closing brace/tag";
					case text_document_errc::unclosed_comment: return "unclosed_comment: a block comment was not closed with an ending tag";
					default: return "unrecognized text_document error";
				}
			}
		};

	} // namespace text_document_detail

	const std::error_category& text_document_category()
	{
		static text_document_detail::text_document_category_impl sSingleton;
		return sSingleton;
	}

} // namespace oStd
