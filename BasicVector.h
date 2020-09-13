#ifndef BASIC_VECTOR_H
#define BASIC_VECTOR_H

#include <iostream>
#include <algorithm>
#include <memory.h>
#include <memory>
#include <cassert>

#define INITIAL_VEC_SIZE 2

//if modern c++ (c++11 and up), make everything constexpr
namespace basic {

	template<typename T /*should add typename Allocator = std::allocator*/>
	class Vector {
	private:
		T* m_Begin;	//start of data
		T* m_End;	//one after last element
		T* m_Capacity;	//one after last possible element

	private:
		void expand();

	public:
		Vector(size_t size = INITIAL_VEC_SIZE);
		Vector(std::initializer_list<T>);

		Vector(Vector const&) = delete;	//no copies allowed
		Vector& operator=(Vector const&) = delete;

		Vector(Vector&&) = delete;	//no moves allowed
		Vector& operator=(Vector&&) = delete;

		~Vector();

		void push_back(const T& arg);
		void push_back(T&& arg);

		template<typename... Args>
		void emplace_back(Args&&... args);

		void pop_back();


		T* begin() { return m_Begin; }
		T* end() { return m_End; }
		const T* begin() const { return m_Begin; }
		const T* end() const { return m_End; }
		
		const T* cbegin() const { return m_Begin; }
		const T* cend() const { return m_End; }

		T& front() { assert(m_Begin != m_End); return *m_Begin; }
		T& back() { assert(m_Begin != m_End); return *(m_End - 1); }
		const T& front() const { assert(m_Begin != m_End); return *m_Begin; }
		const T& back() const { assert(m_Begin != m_End); return *(m_End - 1); }

		void clear();
		size_t size() const { return static_cast<size_t>(m_End - m_Begin); }

		T& operator[](size_t i) { return *(m_Begin + i); }
		const T& operator[](size_t i) const { return *(m_Begin + i); }
	};

	//Template definitions

	template<typename T>
	Vector<T>::Vector(size_t size)  {
		assert(size != 0);
		m_Begin = static_cast<T*>(::operator new(sizeof(T) * size)); //reserve new space //new invokes default ctor, and T might not have it, so ANSII C it is
		m_End = m_Begin;

		m_Capacity = m_Begin + size;
	}

	//Requires data to be copyable, if not, gives the same error as std::vector, so i'll leave it here
	//could be by copy,but ill leave it as move, with hopes that in the future initialiser list is not const, so called forwards compatibility
	template<typename T>
	Vector<T>::Vector(std::initializer_list<T> l) : Vector(l.size()) {
		const T* first = l.begin();
		T* dest = m_Begin;
		for (; first != l.end(); ++first, ++dest) {
			push_back(std::move(*first));
		}
	}

	template<typename T>
	void Vector<T>::push_back(const T& arg) {
		if (m_End >= m_Capacity) expand();	// >= instead of ==, because im paranoid

		::new (m_End) T{ arg };	//construct in already defined space
		++m_End;
	}
	template<typename T>
	void Vector<T>::push_back(T&& arg) {
		if (m_End >= m_Capacity) expand();	// >= instead of ==, because im paranoid

		::new (m_End) T{ std::move(arg) };	//construct in already defined space
		++m_End;
	}

	template<typename T>
	template<typename... Args>
	void Vector<T>::emplace_back(Args&&... args) {
		if (m_End >= m_Capacity) expand();	// >= instead of ==, because im paranoid

		::new (m_End) T{ std::forward<Args>(args)... };	//construct in already defined space
		++m_End;
	}

	template<typename T>
	void Vector<T>::pop_back() {
		if (m_End > m_Begin) {
			--m_End;
			m_End->~T();	//constructed in place, so we call dtor explicitly
		}
	}

	template<typename T>
	Vector<T>::~Vector() {
		clear();
		::operator delete (m_Begin);
	}

	template<typename T>
	void Vector<T>::clear() {
		size_t count = size();
		for (size_t i = 0; i < count; ++i) {
			pop_back();
		}
	}

	template<typename T>
	void Vector<T>::expand() {
		size_t length = size();
		T* newSpace = static_cast<T*>(::operator new(sizeof(T) * length * 2));	//reserve new space
		assert(newSpace && "ERROR::Expand(), malloc failed.");
		//std::move(m_begin, m_end, newSpace);	//doesnt work, memory needs to be initialised
		//memcpy(newSpace, m_begin, m_size * sizeof(T));	//gives warnings about non-copyable object, destrucotrs go crazy

		//equal to std::uninitialised_movein c++17 and up
		T* first = m_Begin;
		T* dest = newSpace;
		for (; first != m_End; ++first, ++dest) {
			::new (static_cast<void*>(dest)) T(std::move(*first));
		}

		clear();	
		::operator delete (m_Begin);

		m_Begin = newSpace;
		m_End = m_Begin + length;

		m_Capacity = m_Begin + length*2;
	}
}


#endif


