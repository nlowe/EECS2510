/*
 * WeightedGraph.h - interface for a Weighted, non-directed graph
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
#pragma once
#include <string>
#include <fstream>

#include "LinkedList.h"
#include "MinPriorityQueue.h"

// A wrapper for a vertex in a Graph
struct Vertex
{
	explicit Vertex(uint32_t id, std::string k) : ID(id), Key(k) {}

	bool operator==(const Vertex& rhs) const
	{
		return Key == rhs.Key;
	}

	const uint32_t ID;
	const std::string Key;
};

struct VertexPair
{
	const Vertex* A;
	const Vertex* B;
	const double EdgeWeight;

	explicit VertexPair(Vertex* a, Vertex* b, double w) : A(a), B(b), EdgeWeight(w) {}

	bool operator==(const VertexPair& rhs) const
	{
		return (A->Key == rhs.A->Key || A->Key == rhs.B->Key) && (B->Key == rhs.B->Key || B->Key == rhs.A->Key) && EdgeWeight == rhs.EdgeWeight;
	}
};

// A weighted, non-directed graph of a fixed size
//
// The maximum number of vertices is stored in VertexCount
//
struct WeightedGraph
{
	explicit WeightedGraph(size_t size)
	{
		VertexCount = size;
		Vertices    = new Vertex*[VertexCount]{ nullptr };
		Weights     = new double[VertexCount * VertexCount]{ 0 };
	}

	explicit WeightedGraph(std::ifstream& reader)
	{
		std::string line;
		getline(reader, line);
		VertexCount = stoull(line);

		Vertices = new Vertex*[VertexCount];
		Weights  = new double[VertexCount * VertexCount];

		// Load the vertex names
		for(auto i = 0; i < VertexCount; i++)
		{
			std::string k;
			getline(reader, k);
			Vertices[i] = new Vertex(i, k);
			if (reader.bad()) throw std::runtime_error("Incomplete or corrupt graph file");
		}

		// Read the adajacency matrix
		size_t idx = 0;
		double w;
		while(reader >> w)
		{
			SetWeight(idx++, w);
		}

		if(idx < VertexCount * VertexCount) throw std::runtime_error("Incomplete or corrupt graph file");
	}

	~WeightedGraph()
	{
		for(auto i = 0; i < VertexCount; i++)
		{
			if(Vertices[i] != nullptr)
			{
				delete Vertices[i];
				Vertices[i] = nullptr;
			}
		}

		delete[] Vertices;
		delete[] Weights;
	}

	// The number of vertices in this graph
	size_t VertexCount;

	// Actually an array of strings containing the vertex names
	Vertex** Vertices;

	// Get the weight between the two specified vertices
	double GetWeight(Vertex*& A, Vertex*& B) const
	{
		return GetWeight(A->ID, B->ID);
	}

	// Get the entry in the weight matrix at the specified row and column
	double GetWeight(size_t r, size_t c) const
	{
		if (r >= VertexCount || c >= VertexCount) throw std::domain_error("Index out of bounds");

		return Weights[r * VertexCount + c];
	}

	// Set the entry in the weight matrix at the specified row and column
	void SetWeight(size_t r, size_t c, double w) const
	{
		if (r >= VertexCount || c >= VertexCount) throw std::domain_error("Index out of bounds");

		Weights[r * VertexCount + c] = w;
	}

	void SetWeight(size_t idx, double w) const
	{
		if (idx > VertexCount * VertexCount) throw std::domain_error("Index out of bounds");
		Weights[idx] = w;
	}

	// Returns a minimum priority queue containing all edges sorted by edge weight
	MinPriorityQueue<VertexPair>* Edges() const
	{
		auto e = new MinPriorityQueue<VertexPair>([](VertexPair* a, VertexPair* b) { return a->EdgeWeight - b->EdgeWeight; }, VertexCount);

		for(auto i = 0; i < VertexCount - 1; i++)
		{
			for(auto j = i+1; j < VertexCount; j++)
			{
				auto w = GetWeight(i, j);

				if(w > 0) e->enqueue(new VertexPair(Vertices[i], Vertices[j], w));
			}
		}

		return e;
	}
private:
	// Actually an array of doubles containing the weights from a vertex to another vertex
	// If the weight is zero, the vertex is not connected. This is a flattened 2d array
	double*     Weights;
};
