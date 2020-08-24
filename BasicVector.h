#ifndef BASIC_VECTOR_H
#define BASIC_VECTOR_H

#include <iostream>
#include <algorithm>
#include <memory.h>
#include <memory>
#include <cassert>

#define INITIAL_VEC_SIZE 2

namespace basic_vector {

	template<typename T>
	class Vector {
	private:
		T* m_Begin;	//start of data
		T* m_End;	//one after last element
		T* m_Capacity;	//one after last possible element

		size_t m_Size;
	private:
		void Clear();
		void Expand();
	public:
		Vector(size_t size = INITIAL_VEC_SIZE);
		Vector(std::initializer_list<T>);

		Vector(Vector const&) = delete;	//no copies allowed
		Vector& operator=(Vector const&) = delete;

		Vector(Vector&&) = delete;	//no moves allowed
		Vector& operator=(Vector&&) = delete;

		~Vector();

		//Different type convention, just to easily show that it's interchangeable with std::vector
		template<typename... Args>
		void emplace_back(Args&&... args);

		void pop_back();
		
		T* begin() { return m_Begin; }
		T* end() { return m_End; }

		const T* begin() const { return m_Begin; }
		const T* end() const { return m_End; }

		T& front() { assert(m_Begin != m_End); return *m_Begin; }
		T& back() { assert(m_Begin != m_End); return *(m_End-1); }

		const T& front() const { assert(m_Begin != m_End); return *m_Begin; }
		const T &back() const { assert(m_Begin != m_End); return *(m_End - 1); }

		size_t size() const { return static_cast<size_t>(m_End - m_Begin); }

		T& operator[](size_t i) { return *(m_Begin+i); }
		const T& operator[](size_t i) const { return *(m_Begin+i); }
	};

	//Template definitions

	template<typename T>
	Vector<T>::Vector(size_t size) : m_Size(size) {
		assert(size != 0);
		m_Begin = (T *)malloc(sizeof(T) * m_Size); //reserve new space //new invokes default ctor, and T might not have it, so ANSII C it is
		m_End = m_Begin;

		m_Capacity = m_Begin + m_Size;
	}
	
	//Requires data to be copyable, if not gives the same error as std::vector, so i'll leave it here
	//could be by copy,but ill leave it as move, to be consistent with Expand()
	template<typename T>
	Vector<T>::Vector(std::initializer_list<T> l) : Vector(l.size()) {
		const T *first = l.begin();
		T *dest = m_Begin;
		for (; first != l.end(); ++first, ++dest){
			::new (dest) T(std::move(*first));
		}

		m_End = m_Begin + m_Size;
	}
	
	template<typename T>
	template<typename... Args>
	void Vector<T>::emplace_back(Args&&... args) {
		if (m_End >= m_Capacity) Expand();	// >= instead of ==, because im paranoid

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
		Clear();
		free(m_Begin);
	}

	template<typename T>
	void Vector<T>::Clear() {
		size_t count = size();
		for (size_t i = 0; i < count; ++i) {
			pop_back();
		}
	}

	template<typename T>
	void Vector<T>::Expand() {
		T* newSpace = (T*)malloc(sizeof(T) * m_Size * 2);	//reserve new space
		assert(newSpace && "ERROR::Expand(), malloc failed.")
		//std::move(m_begin, m_end, newSpace);	//doesnt work, memory needs to be initialised
		//memcpy(newSpace, m_begin, m_size * sizeof(T));	//gives warnings about non-copyable object, destrucotrs go crazy

		//std::uninitialised_move, by hand because its in C++17 and we're not quite there yet
		//it's 3:37 in the morning, its working...
		T *first = m_Begin;
		T *dest = newSpace;
		for (; first != m_End; ++first, ++dest){
			::new (static_cast<void *>(dest)) T(std::move(*first));
		}

		Clear();	//need to run destructor on old values
		free(m_Begin);	//m_begin got malloc'ed

		m_Begin = newSpace;
		m_End = m_Begin + m_Size;

		m_Size *= 2;	//with each expand size grows by a factor of 2
		m_Capacity = m_Begin + m_Size;
	}
}


#endif

