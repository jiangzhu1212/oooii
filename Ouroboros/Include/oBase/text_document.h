// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_text_document_h
#define oBase_text_document_h

// A text document is basically a text file format considered solely as a string 
// without any file I/O. This head includes a common base on which to build such 
// parsers. The intent of an ouro::text_document is fast load speed, read-only
// access and a small memory footprint. To facilitate that the assumption is 
// that a nul-terminated string that is compliant with the text document's 
// format can be parsed inline into an indexable state.

#include <functional>
#include <system_error>
#include <oBase/fixed_string.h>

namespace ouro {

enum class text_document_errc { generic_parse_error, unclosed_scope, unclosed_comment };

const std::error_category& text_document_category();

/*constexpr*/ inline std::error_code make_error_code(text_document_errc err_code) { return std::error_code(static_cast<int>(err_code), text_document_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(text_document_errc err_code) { return std::error_condition(static_cast<int>(err_code), text_document_category()); }

class text_document_error : public std::logic_error
{
public:
	text_document_error(text_document_errc err_code) : logic_error(text_document_category().message(static_cast<int>(err_code))) {}
};

typedef std::function<void(char* _pData)> text_document_deleter_t;

	namespace detail {
		class text_buffer
		{
			// A buffer assumed to be a nul-terminated string that has a uri source
			// name as well as a way to delete the allocated string. If no deleter is
			// specified, then this allocates its own memory and copies the source.
		
			text_buffer(const text_buffer&);
			const text_buffer& operator=(const text_buffer&);
		
		public:
			text_buffer()
				: pData(nullptr)
			{}

			text_buffer(const char* _URI, char* _pData, const text_document_deleter_t& _Delete)
				: URI(_URI)
				, pData(_pData)
				, Delete(_Delete)
			{
				if (!Delete)
				{
					size_t size = strlen(_pData) + 1;
					pData = new char[size];
					strlcpy(pData, _pData, size);
				}
			}

			~text_buffer() { delete_buffer(); }

			text_buffer(text_buffer&& _That) { operator=(std::move(_That)); }
			text_buffer& operator=(text_buffer&& _That)
			{
				if (this != &_That)
				{
					delete_buffer();
					pData = std::move(_That.pData);
					_That.pData = nullptr;
					Delete = std::move(_That.Delete);
					URI = std::move(_That.URI);
				}
				return *this;
			}

			operator bool() const { return !!pData; }

			char* pData;
			text_document_deleter_t Delete;
			uri_string URI;

		private:
			void delete_buffer()
			{
				if (pData)
				{
					if (Delete)
					{
						Delete(pData);
						Delete = nullptr;
					}
					else
						delete [] pData;

					pData = nullptr;
				}
			}
		};
	} // namespace detail
}

#endif
