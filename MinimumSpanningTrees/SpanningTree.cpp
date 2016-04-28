/*
 * SpanningTree.cpp - Implementation of a minimum spanning tree
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "stdafx.h"
#include <iostream>

#include "SpanningTree.h"


SpanningTree::SpanningTree()
{
	// Store the edges as a minimum priority queue sorted first by the first vertex alphabetically,
	// then by the second vertex alphabetically. This lets us print the tree without having to sort
	// the edges later
	edges = new MinPriorityQueue<VertexPair>([](VertexPair* lhs, VertexPair* rhs)
	{
		auto firstKey = lhs->A->Name.compare(rhs->A->Name);
		return firstKey == 0 ? lhs->B->Name.compare(rhs->B->Name) : firstKey;
	}, 16);
}


SpanningTree::~SpanningTree()
{
	delete edges;
}

// Add the specified edge to the tree. Does not perform any validation
void SpanningTree::accept(VertexPair* edge) const
{
	edges->enqueue(edge);
}

// Print the total weight and all edges taken to standard output
void SpanningTree::print() const
{
	auto totalWeight = 0.0;
	edges->each([&](VertexPair* e) { totalWeight += e->EdgeWeight; });

	std::cout << totalWeight << std::endl;
	
	while(edges->Size() > 0)
	{
		auto edge = edges->dequeue();
		std::cout << edge->A->Name << "-" << edge->B->Name << ": " << edge->EdgeWeight << std::endl;
	}
}
