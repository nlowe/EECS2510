/*
 * LinkedList.h - Interface for a simple, generic linked list
 *
 * Built for EECS2510 - Nonlinear Data Structures
 *	at The University of Toledo, Spring 2016
 *
 * Copyright (c) 2016 Nathan Lowe
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS ORtemplate <typename T>
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#include <functional>

struct None{};

template <typename T>
struct Node
{
	explicit Node(T* p) : Payload(p), Next(nullptr){}
	~Node()
	{
		if (Next != nullptr) delete Next;
	}

	const T* Payload;
	Node<T>* Next;
};

template <typename T>
class LinkedList
{
public:
	explicit LinkedList(){}
	~LinkedList();

	void add(T* key);
	void addAll(LinkedList<T> other);

	bool Contains(T* element);

	void each(std::function<void(T*)> func);

	size_t Count() const { return count; }
private:
	Node<T>* root = nullptr;

	size_t count = 0;
};

template <typename T>
LinkedList<T>::~LinkedList()
{
	if (root != nullptr) delete root;
}


template <typename T>
void LinkedList<T>::add(T* key)
{
	auto n = new Node<T>(key);
	n->Next = root;
	root = n;
	count++;
}

template <typename T>
void LinkedList<T>::addAll(LinkedList<T> other)
{
	other.each([&](T* t) { add(t); });
}

template <typename T>
bool LinkedList<T>::Contains(T* element)
{
	Node<T>* candidate = root;

	while(candidate != nullptr)
	{
		if(candidate->Payload == element)
		{
			return true;
		}
		candidate = candidate->Next;
	}

	return false;
}

template <typename T>
void LinkedList<T>::each(std::function<void(T*)> func)
{
	Node<T>* candidate = root;

	while(candidate != nullptr)
	{
		func(const_cast<T*>(candidate->Payload));
		candidate = candidate->Next;
	}
}

