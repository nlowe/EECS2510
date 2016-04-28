/*
 * MinPriorityQueue.h
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
#include <stdexcept>
#include <functional>

#include "utils.h"

// A minimum priority queue implemented with a minimum binary heap
//
// The user is responsible for freeing the inserted elements (only pointers to them
// are stored)
template <typename T>
class MinPriorityQueue
{
public:
	MinPriorityQueue(std::function<double(T*,T*)> comparator, size_t initialCapacity)
		: comparator(comparator), size(0), capacity(initialCapacity)
	{
		elements = new T*[capacity + 1]{ nullptr };
	}

	~MinPriorityQueue()
	{
		delete[] elements;
	}

	// Add the specified element to the queue
	void enqueue(T* element)
	{
		if (size == capacity-1) grow(2 * capacity);

		elements[++size] = element;
		auto i = size;
		while(i > 1 && comparator(elements[parentOf(i)], elements[i]) > 0)
		{
			utils::swap(elements[i], elements[parentOf(i)]);
			i = parentOf(i);
		}
	}

	// Remove and return the minimum element from the queue
	T* dequeue()
	{
		if (size == 0) throw std::underflow_error("Nothing in the heap");

		auto min = elements[1];
		
		elements[1] = elements[size];
		elements[size--] = nullptr;
		minHeapify(1);

		return min;
	}

	// Return the minimum element from the queue without modifying the queue
	T* peek()
	{
		if (size == 0) throw std::underflow_error("Nothing in the heap");
		return elements[1];
	}

	// Call this when the priority of a node is changed externally
	void notifyPriorityUpdated(T* k)
	{
		auto index = 0;
		for(auto i = 1; i <= size; i++)
		{
			if(elements[i] == k)
			{
				index = i;
				break;
			}
		}

		if (index == 0) throw std::domain_error("Element not found");
		while(index > 1 && comparator(elements[parentOf(index)], elements[index]) > 0)
		{
			utils::swap(elements[index], elements[parentOf(index)]);
			index = parentOf(index);
		}
	}

	// Returns true iff the specified element is in the queue
	bool contains(T* k) const
	{
		for(auto i = 1; i <= size; i++)
		{
			if (elements[i] == k) return true;
		}
		return false;
	}

	// Increases the capacity of the queue. The new capacity must be greater than the previous capacity
	void grow(size_t s)
	{
		if (s <= capacity) throw std::runtime_error("New capacity must be greater than current capacity");

		T** newElements = new T*[s];
		for(auto i = 0; i <= capacity; i++)
		{
			newElements[i] = elements[i];
			elements[i] = nullptr;
		}

		delete[] elements;
		elements = newElements;
		capacity = s;
	}

	// Apply the specified function to all elements in the queue
	void each(std::function<void(T*)> action)
	{
		for(size_t i = 1; i <= size; i++)
		{
			action(elements[i]);
		}
	}

	// Returns true iff the queue contains no elements
	bool isEmpty() const { return size == 0; }

	// Returns the number of elements in the queue
	size_t Size() const	{ return size; }
	// Returns the maximum number of elements in the queue
	size_t Capacity() const	{ return capacity; }
private:
	std::function<double(T*,T*)> comparator;

	size_t size;
	size_t capacity;
	T** elements;

	// Returns the index of the parent of the element at the specified index
	static size_t parentOf(size_t index) { return floor(index / 2); }
	// Returns the index of the left child of the element at the specified index
	static size_t leftOf(size_t index) { return 2 * index; }
	// Returns the index of the right child of the element at the specified index
	static size_t rightOf(size_t index) { return 2 * index + 1; }

	// Ensures that the subtree at the specified index is a min-heap
	void minHeapify(size_t index)
	{
		auto l = leftOf(index);
		auto r = rightOf(index);
		size_t smallest = 0;

		if ( l <= size && comparator(elements[l], elements[index]) < 0)
		{
			smallest = l;
		}
		else
		{
			smallest = index;
		}

		if ( r <= size && comparator(elements[r], elements[smallest]) < 0)
		{
			smallest = r;
		}

		if (smallest != index)
		{
			utils::swap(elements[index], elements[smallest]);
			minHeapify(smallest);
		}
	}
};
