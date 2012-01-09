// $(header)
// Approximation of the upcoming C++11 std::thread interface. There has been no
// attention paid to proper exception behavior.
#pragma once
#ifndef oStdThread_h
#define oStdThread_h

#include <oBasis/oCallable.h>
#include <oBasis/oStdChrono.h>
#include <functional>
#include <vector>

namespace oStd {

	class thread
	{
		void* hThread;
		void initialize(oCALLABLE _ThreadProc);
		#ifdef oHAS_MOVE_CTOR
			thread(const thread&);
			const thread& operator=(const thread&);
		#endif
	public:
		class id;
		typedef void* native_handle_type;
		thread();
		#ifndef oHAS_VARIADIC_TEMPLATES
			oCALLABLE_TEMPLATE0 explicit thread(oCALLABLE_PARAMS0) { initialize(oCALLABLE_BIND0); }
			oCALLABLE_TEMPLATE1 explicit thread(oCALLABLE_PARAMS1) { initialize(oCALLABLE_BIND1); }
			oCALLABLE_TEMPLATE2 explicit thread(oCALLABLE_PARAMS2) { initialize(oCALLABLE_BIND2); }
			oCALLABLE_TEMPLATE3 explicit thread(oCALLABLE_PARAMS3) { initialize(oCALLABLE_BIND3); }
			oCALLABLE_TEMPLATE4 explicit thread(oCALLABLE_PARAMS4) { initialize(oCALLABLE_BIND4); }
			oCALLABLE_TEMPLATE5 explicit thread(oCALLABLE_PARAMS5) { initialize(oCALLABLE_BIND5); }
			oCALLABLE_TEMPLATE6 explicit thread(oCALLABLE_PARAMS6) { initialize(oCALLABLE_BIND6); }
			oCALLABLE_TEMPLATE7 explicit thread(oCALLABLE_PARAMS7) { initialize(oCALLABLE_BIND7); }
			oCALLABLE_TEMPLATE8 explicit thread(oCALLABLE_PARAMS8) { initialize(oCALLABLE_BIND8); }
			oCALLABLE_TEMPLATE9 explicit thread(oCALLABLE_PARAMS9) { initialize(oCALLABLE_BIND9); }
			oCALLABLE_TEMPLATE10 explicit thread(oCALLABLE_PARAMS10) { initialize(oCALLABLE_BIND10); }
		#endif
		
		// !!! NOTE !!!	All threads must be join()'ed before allowing the thread 
		// to exit or, according to the C++11 standard, std::terminate will be 
		// called thus exiting the application.
		~thread();

		#ifdef oHAS_MOVE_CTOR
			thread(thread&& _That);
			thread& operator=(thread&& _That);
			void move_ctor(thread& _This, thread& _That);
			thread& move_operator_eq(thread& _That);
		#else
			void move_ctor(thread& _This, thread& _That);
			thread& move_operator_eq(thread& _That);
		#endif

		// Uses move operator to swap this and the specified thread
		void swap(thread& _That);

		// Block the calling thread until this thread's callable function exits
		// (basically a wait() call)
		void join();

		// Set this thread's ID to identity and no longer allow control of the 
		// thread from this interface
		void detach();

		// Returns true if join can still be called (i.e. if the thread isn't 
		// detached and hasn't yet exited)
		bool joinable() const;

		id get_id() const;
		native_handle_type native_handle();
		static unsigned int hardware_concurrency();
	};

	class thread::id
	{	unsigned int ID;
	public:
		id();
	};

	namespace this_thread
	{
		thread::id get_id();
		void yield();
		void __sleep_for(unsigned int _Milliseconds);

		template<typename Rep, typename Period> void sleep_for(const chrono::duration<Rep, Period>& _Duration)
		{
			chrono::milliseconds s = chrono::duration_cast<chrono::milliseconds>(_Duration);
			__sleep_for(static_cast<unsigned int>(s.count()));
		}

	} // namespace this_thread

} // namespace oStd

namespace std
{
	void swap(oStd::thread& _This, oStd::thread& _That);

	// default arguments not allowed on a partial specialization
	// So until this is supported, ask client code to be explicit about 
	// std::allocator<oStd::thread> in type decl
	template<typename Alloc /*= std::allocator<oStd::thread>*/ > class vector<oStd::thread, Alloc>
	{
		vector(vector const&);
		vector& operator=(vector const&);
	public:
		typedef Alloc allocator_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef oStd::thread value_type;
		typedef const value_type* const_iterator;
		typedef value_type* iterator;
		typedef typename Alloc::pointer pointer;
		typedef typename Alloc::reference reference;
		typedef typename Alloc::const_reference const_reference;
		typedef typename Alloc::const_pointer const_pointer;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		explicit vector(const allocator_type& _Allocator = allocator_type())
			: Allocator(_Allocator)
			, Buffer(nullptr)
			, Size(0)
			, Capacity(0)
		{}

		explicit vector(size_type size)
			: Allocator(allocator_type())
			, Buffer(nullptr)
			, Size(0)
			, Capacity(0)
		{
			resize(size);
		}

		~vector()
		{
			call_dtor(begin(), end());
			Allocator.deallocate(begin(), Capacity);
		}

		void reserve(size_type size)
		{
			if (size > Capacity)
			{
				void* newBuffer = Allocator.allocate(size);
				if (Buffer)
				{
					memcpy(newBuffer, Buffer, sizeof(size_type) * Size);
					Allocator.deallocate(begin(), Capacity);
				}
				Buffer = newBuffer;
				Capacity = size;
			}
		}

		void resize(size_type size)
		{
			if (size < Size)
			{
				call_dtor(begin()+size, end());
				Size = size;
			}

			else if (size > Size)
			{
				reserve(size);
				for (size_type i = Size; i < size; i++)
					call_ctor(&at(i));
				Size = size;
			}
		}

		void clear()
		{
			resize(0);
		}

		iterator insert(iterator pos, value_type& new_value)
		{
			if (Size == Capacity)
			{
				size_t dist = std::distance(pos, end());
				reserve(2 * Capacity);
				pos = begin() + dist;
			}

			if (pos != end())
				memmove(pos+1, pos, std::distance(pos, end()));

			// *pos = new_value;
			(*pos).move_operator_eq(new_value);
			Size++;
			return pos;
		}

		void push_back(value_type& new_value)
		{
			insert(end(), new_value);
		}

		void pop_back()
		{
			Allocator.destroy(&begin()[--Size]);
		}

		iterator erase(iterator pos)
		{
			Allocator.destroy(pos);
			memmove(pos, pos+1, std::distance(pos+1, end()));
			Size--;
		}

		iterator erase(iterator first, iterator last)
		{
			call_dtor(first, last+1);
			Size -= std::distance(first, last);
		}

		void swap(vector& other)
		{
			size_type sz = __min(size(), other.size());
			for (size_type i = 0; i < sz; i++)
				at(i).swap(other.at[i]);

			if (other.size() > size())
			{
				for (size_type i = size(); i < other.size(); i++)
					push_back(other[i]);
				other.resize(sz);
			}

			else if (other.size() < size())
			{
				for (size_type i = other.size(); i > size(); i++)
					other.push_back(at(i));
				resize(sz);
			}
		}

		bool empty() const
		{
			return !Size;
		}

		size_type capacity() const
		{
			return Capacity;
		}

		size_type size() const
		{
			return Size;
		}

		size_type max_size() const
		{
			return ~((size_type)0);
		}

		allocator_type get_allocator() const
		{
			return Allocator;
		}

		reference operator[](size_type n) { return begin()[n]; }
		const_reference operator[](size_type n) const { return begin()[n]; }
		reference at(size_type n) { return begin()[n]; }
		const_reference at(size_type n) const { return begin()[n]; }
		reference front() { return begin()[0]; }
		const_reference front() const { return begin()[n]; }
		reference back() { return begin()[Size-1]; }
		const_reference back() const { return begin()[Size-1]; }
		iterator begin() { return (iterator)Buffer; }
		iterator end() { return begin() + Size; }
		const_iterator begin() const { return (const_iterator)Buffer; }
		const_iterator end() const { return begin() + Size; }
		reverse_iterator rbegin() { return std::reverse_iterator<iterator>(end()-1); }
		reverse_iterator rend() { return std::reverse_iterator<iterator>(begin()-1); }
		const_reverse_iterator rbegin() const { return std::reverse_iterator<iterator>(end()-1); }
		const_reverse_iterator rend() const { return std::reverse_iterator<iterator>(begin()-1); }
	private:
		void call_dtor(iterator begin, iterator end)
		{
			for (; begin != end; ++begin)
				Allocator.destroy(begin);
		}

		void call_ctor(iterator it)
		{
			new (it) value_type();
		}

		void* Buffer;
		size_type Size;
		size_type Capacity;
		allocator_type Allocator;
	};

	bool operator==(oStd::thread::id x, oStd::thread::id y);
	bool operator!=(oStd::thread::id x, oStd::thread::id y);
	bool operator<(oStd::thread::id x, oStd::thread::id y);
	bool operator<=(oStd::thread::id x, oStd::thread::id y);
	bool operator>(oStd::thread::id x, oStd::thread::id y);
	bool operator>=(oStd::thread::id x, oStd::thread::id y);

} // namespace std

inline bool operator==(oStd::thread::id x, oStd::thread::id y) { return std::operator==(x, y); }
inline bool operator!=(oStd::thread::id x, oStd::thread::id y) { return std::operator!=(x, y); }
inline bool operator<(oStd::thread::id x, oStd::thread::id y) { return std::operator<(x, y); }
inline bool operator<=(oStd::thread::id x, oStd::thread::id y) { return std::operator<=(x, y); }
inline bool operator>(oStd::thread::id x, oStd::thread::id y) { return std::operator>(x, y); }
inline bool operator>=(oStd::thread::id x, oStd::thread::id y) { return std::operator>=(x, y); }

#endif
