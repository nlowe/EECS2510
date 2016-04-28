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

// A node in a generic linked list
template <typename T>
struct Node
{
	explicit Node(T* p) : Payload(p), Next(nullptr){}
	~Node()
	{
		if (Next != nullptr) delete Next;
	}

	// The payload of the node
	const T* Payload;
	// The next node in the list
	Node<T>* Next;
};

// A generic linked list
template <typename T>
class LinkedList
{
public:
	explicit LinkedList(){}
	~LinkedList()
	{
		if (root != nullptr) delete root;
	}

	// Add the specified element to the list
	void add(T* key)
	{
		auto n = new Node<T>(key);
		n->Next = root;
		root = n;
		count++;
	}

	// Add all elements in the specified linked list to this list
	void addAll(LinkedList<T> other)
	{
		other.each([&](T* t) { add(t); });
	}

	// Returns true iff the specified element is in this list
	bool Contains(T* element)
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

	// Applys the specified function to every element in this list
	void each(std::function<void(T*)> func)
	{
		Node<T>* candidate = root;

		while(candidate != nullptr)
		{
			func(const_cast<T*>(candidate->Payload));
			candidate = candidate->Next;
		}
	}

	// Returns the number of elements this list contains
	size_t Count() const { return count; }
private:
	Node<T>* root = nullptr;

	size_t count = 0;
};
