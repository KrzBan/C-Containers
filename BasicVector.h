#ifndef BASIC_VECTOR_H
#define BASIC_VECTOR_H

#include <iostream>
#include <algorithm>
#include <memory>
#include <cassert>

namespace basic {

	//no idea whether this works, how it works, how to test, or even how it's supposed to function
	template<typename Alloc>
	std::enable_if_t<std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value> copy_assign_allocator(Alloc& dst, const Alloc& src){
		dst = src;
	}
	template<typename Alloc>
	std::enable_if_t<!std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value> copy_assign_allocator(Alloc& dst, const Alloc& src) {}

	template<typename Alloc>
	std::enable_if_t<std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value> move_assign_allocator(Alloc& dst, const Alloc& src){
		dst = src;
	}
	template<typename Alloc>
	std::enable_if_t<!std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value> move_assign_allocator(Alloc& dst, const Alloc& src) {}

	template<typename Alloc>
	std::enable_if_t<std::allocator_traits<Alloc>::propagate_on_container_swap::value> swap_assign_allocator(Alloc& dst, const Alloc& src){
		dst = src;
	}
	template<typename Alloc>
	std::enable_if_t<!std::allocator_traits<Alloc>::propagate_on_container_swap::value> swap_assign_allocator(Alloc& dst, const Alloc& src) {}

	uint32_t RoundUpPower2(uint32_t arg);	//basic::Vector stores memory only if its a pwoer of 2

	template<typename T, typename Allocator = std::allocator<T>>
	class Vector {
	public:
		typedef T																	value_type;
		typedef Allocator															allocator_type;
		typedef typename T&															reference;
		typedef typename const T&													const_reference;
		typedef T*																	iterator;
		typedef const T*															const_iterator;
		typedef typename std::allocator_traits<allocator_type>::size_type			size_type;
		typedef typename std::allocator_traits<allocator_type>::difference_type		difference_type;
		typedef typename std::allocator_traits<allocator_type>::pointer				pointer;
		typedef typename std::allocator_traits<allocator_type>::const_pointer		const_pointer;
		typedef std::reverse_iterator<iterator>										reverse_iterator;
		typedef std::reverse_iterator<const_iterator>								const_reverse_iterator;
	private:
		T* m_Begin;	//start of data
		T* m_End;	//one after last element
		T* m_Capacity;	//one after last possible element

		allocator_type alloc;

	public:
		Vector();
		Vector(allocator_type a);	//no, imnot overloading every function to take in an optional allocator, do it this way, its for learning anyways
		Vector(const_iterator begin, const_iterator end);
		Vector(std::initializer_list<T>);

		Vector(const Vector&) = delete;	//no copies allowed
		Vector& operator=(const Vector&) = delete;

		Vector(Vector&&)  noexcept;
		Vector& operator=(Vector&&) noexcept;

		~Vector();

		void push_back(const T& arg);
		void push_back(T&& arg);

		template<typename... Args>
		void emplace_back(Args&&... args);

		void pop_back() noexcept;

		iterator		begin()		  noexcept	{ return iterator(m_Begin); }
		const_iterator	begin()	const noexcept	{ return begin(); }
		iterator		end()		  noexcept	{ return iterator(m_End); }
		const_iterator	end()	const noexcept	{ return end(); }

		reverse_iterator		rbegin()	   noexcept	{ return reverse_iterator(end()); }
		const_reverse_iterator	rbegin() const noexcept	{ return rbegin(); }
		reverse_iterator		rend()		   noexcept	{ return reverse_iterator(begin()); }
		const_reverse_iterator	rend()	 const noexcept	{ return rend(); }

		const_iterator cbegin()  const noexcept { return begin(); }
		const_iterator cend()	 const noexcept { return end(); }
		const_iterator crbegin() const noexcept { return rbegin(); }
		const_iterator crend()	 const noexcept { return rend(); }

		reference front()			  { assert(m_Begin != m_End); return reference(*m_Begin); }
		const_reference front() const { front(); }
		reference back()			  { assert(m_Begin != m_End); return reference(*(m_End - 1)); }
		const_reference back()	const { back(); }

		void clear() noexcept;
		void swap(Vector& other) noexcept;
		size_type size() const noexcept { return static_cast<size_type>(m_End - m_Begin); }
		size_type maxSize() const noexcept { return static_cast<size_type>(m_Capacity - m_Begin); }
		size_type capacity() const noexcept { return maxSize(); }

		reference		operator[](size_t i)	   { return *(m_Begin + i); }
		const_reference operator[](size_t i) const { return *(m_Begin + i); }

		allocator_type get_allocator() const noexcept { return alloc; }

	private:
		void resize(size_t newSize);
		void destroy() noexcept;
	};

	//Template definitions

	template<typename T, typename Allocator>
	Vector<T, Allocator>::Vector(): m_Begin(nullptr), m_End(nullptr), m_Capacity(nullptr)  {}

	template<typename T, typename Allocator>
	Vector<T, Allocator>::Vector(allocator_type a) : m_Begin(nullptr), m_End(nullptr), m_Capacity(nullptr), alloc(a) {}

	template<typename T, typename Allocator>
	Vector<T, Allocator>::Vector(const_iterator begin, const_iterator end) {
		size_t size = RoundUpPower2(end - begin);
		resize(size);

		const T* first = begin;
		T* dest = m_Begin;
		for (; first != end; ++first, ++dest) {
			push_back(*first);
		}
	}

	//Requires data to be copyable, if not, gives the same error as std::vector, so i'll leave it here
	//could be by copy,but ill leave it as move, with hopes that in the future initialiser list is not const, so called forwards compatibility
	template<typename T, typename Allocator>
	Vector<T, Allocator>::Vector(std::initializer_list<T> l) : Vector(l.begin(), l.end()) {}

	template<typename T, typename Allocator>
	Vector<T, Allocator>::Vector(Vector&& other)  noexcept {
		m_Begin = std::move(other.m_Begin);
		m_End = std::move(other.m_End);
		m_Capacity = std::move(other.m_Capacity);

		other.m_Begin = other.m_End = other.m_Capacity = nullptr;

		move_assign_allocator(this->alloc, other.alloc);
	}

	template<typename T, typename Allocator>
	Vector<T, Allocator>& Vector<T, Allocator>::operator=(Vector&& other) noexcept{
		destroy();

		m_Begin = std::move(other.m_Begin);
		m_End = std::move(other.m_End);
		m_Capacity = std::move(other.m_Capacity);

		other.m_Begin = other.m_End = other.m_Capacity = nullptr;

		move_assign_allocator(this->alloc, other.alloc);

		return *this;
	}

	template<typename T, typename Allocator>
	void Vector<T, Allocator>::push_back(const T& arg) {
		if (m_End == m_Capacity) resize(size() * 2);

		//::new (m_End) T{ arg };	
		std::allocator_traits<allocator_type>::construct(alloc, m_End, arg);
		++m_End;
	}
	template<typename T, typename Allocator>
	void Vector<T, Allocator>::push_back(T&& arg) {
		if (m_End == m_Capacity) resize(size() * 2);	

		//::new (m_End) T{ std::move(arg) };	
		std::allocator_traits<allocator_type>::construct(alloc, m_End, std::move(arg) );
		++m_End;
	}

	template<typename T, typename Allocator>
	template<typename... Args>
	void Vector<T, Allocator>::emplace_back(Args&&... args) {
		if (m_End == m_Capacity) resize(size() * 2);	

		//::new (m_End) T{ std::forward<Args>(args)... };	
		std::allocator_traits<allocator_type>::construct(alloc, m_End, std::forward<Args>(args)...);
		++m_End;
	}

	template<typename T, typename Allocator>
	void Vector<T, Allocator>::pop_back() noexcept {
		if (m_End > m_Begin) {
			--m_End;
			//m_End->~T();	//constructed in place, so we call dtor explicitly
			std::allocator_traits<allocator_type>::destroy(alloc, m_End);
		}
	}

	template<typename T, typename Allocator>
	Vector<T, Allocator>::~Vector() {
		clear();
		//::operator delete (m_Begin);
		alloc.deallocate(m_Begin, maxSize());
	}

	template<typename T, typename Allocator >
	void Vector<T, Allocator>::clear() noexcept {
		size_t count = size();
		for (size_t i = 0; i < count; ++i) {
			pop_back();
		}
		//Does not release memory, if you want to, use basic::Vector<T>.swap(yourVecHere);
	}

	template<typename T, typename Allocator >
	void Vector<T, Allocator>::destroy() noexcept {
		clear();
		//::operator delete (m_Begin);
		alloc.deallocate(m_Begin, maxSize());

		m_Begin = m_End = m_Capacity = nullptr;
	}

	template<typename T, typename Allocator>
	void Vector<T, Allocator>::resize(size_t newSize) {
		size_t currSize = size();
		if (newSize == 0) newSize = 1;	//resize usually gets called with resize( size() *2 ), but size() couldbe 0, so check for that TODO: find a solution? maeby?
		if (newSize <= currSize) return; // dont support down-sizing
		//T* newSpace = static_cast<T*>(::operator new(sizeof(T) * newSize));
		T* newSpace = static_cast<T*>(alloc.allocate(newSize));

		//equal (somewhat) to std::uninitialised_move in c++17 and up
		T* first = m_Begin;
		T* dest = newSpace;
		for (; first != m_End; ++first, ++dest) {
			//::new (static_cast<void*>(dest)) T(std::move(*first));
			//alloc.construct(dest, std::move(*first));
			std::allocator_traits<allocator_type>::construct(alloc, dest, std::move(*first));
		}

		//::operator delete (m_Begin);
		alloc.deallocate(m_Begin, maxSize());

		m_Begin = newSpace;
		m_End = m_Begin + currSize;

		m_Capacity = m_Begin + newSize;
	}

	

	template<typename T, typename Allocator>
	void Vector<T, Allocator>::swap(Vector& other) noexcept {

		Vector temp{ std::move(other) };
		other = std::move(*this);
		*this = std::move(temp);

		swap_assign_allocator(this->alloc, other.alloc);
	}

	template<typename T, typename Allocator>
	void swap(Vector<T, Allocator>& left, Vector<T, Allocator>& right) {
		left.swap(right);
	}

	//Ideally, make it work with size_t and check with compiler flags which buts to shift
	// or use sime well maintained library function
	uint32_t RoundUpPower2(uint32_t arg) {	//works for 32-bit values
		--arg;
		arg |= arg >> 1;
		arg |= arg >> 2;
		arg |= arg >> 4;
		arg |= arg >> 8;
		arg |= arg >> 16;
		++arg;

		return arg;
	}
}

#endif


