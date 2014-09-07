// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oString_text_document_h
#define oString_text_document_h

// A text document is basically a text file format considered solely as a string 
// without any file I/O. This head includes a common base on which to build such 
// parsers. The intent of an ouro::text_document is fast load speed, read-only
// access and a small memory footprint. To facilitate that the assumption is 
// that a nul-terminated string that is compliant with the text document's 
// format can be parsed inline into an indexable state.

#include <system_error>
#include <oMemory/allocate.h>
#include <oBase/fixed_string.h>

namespace ouro {

enum class text_document_errc { generic_parse_error, unclosed_scope, unclosed_comment };

const std::error_category& text_document_category();

/*constexpr*/ inline std::error_code make_error_code(text_document_errc err_code) { return std::error_code(static_cast<int>(err_code), text_document_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(text_document_errc err_code) { return std::error_condition(static_cast<int>(err_code), text_document_category()); }

class text_document_error : public std::logic_error
{
public:
	text_document_error(text_document_errc err_code) 
		: logic_error(text_document_category().message(static_cast<int>(err_code))) {}
};

	namespace detail {
		class text_buffer
		{
			// A buffer assumed to be a nul-terminated string with a uri label
			// and a deallocate function for the string buffer.
		
			text_buffer(const text_buffer&);
			const text_buffer& operator=(const text_buffer&);
		
		public:
			text_buffer() : data(nullptr), deallocate(nullptr) {}

			text_buffer(const char* _uri, char* _data, const deallocate_fn& _deallocate)
				: data(_data)
				, deallocate(_deallocate)
				, uri(_uri)
			{
				if (!_data)
					throw std::invalid_argument("[text_buffer] invalid data buffer");
				if (!_deallocate)
					throw std::invalid_argument("[text_buffer] invalid deallocate function");
			}

			text_buffer(const char* _uri, const char* data_src, const allocator& alloc, const char* label)
				: data(nullptr)
				, deallocate(alloc.deallocate)
				, uri(_uri)
			{
				size_t size = strlen(data_src) + 1;
				data = (char*)alloc.allocate(size, allocate_options(), label);
				strlcpy(data, data_src, size);
			}

			~text_buffer()
			{
				delete_buffer();
			}

			text_buffer(text_buffer&& that)
			{ 
				data = that.data; data = nullptr;
				deallocate = that.deallocate; that.deallocate = nullptr;
				uri = std::move(that.uri); uri.clear();
			}
			
			text_buffer& operator=(text_buffer&& that)
			{
				if (this != &that)
				{
					delete_buffer();
					data = std::move(that.data); that.data = nullptr;
					deallocate = std::move(that.deallocate);
					uri = std::move(that.uri);
				}
				return *this;
			}

			operator bool() const { return !!data; }

			char* data;
			deallocate_fn deallocate;
			uri_string uri;

		private:
			void delete_buffer()
			{
				if (data)
				{
					if (deallocate)
					{
						deallocate(data);
						deallocate = nullptr;
					}

					data = nullptr;
				}
			}
		};
	}
}

#endif
