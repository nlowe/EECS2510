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

// A weighted, non-directed graph of a fixed size
//
// The maximum number of vertices is stored in VertexCount
//
struct WeightedGraph
{
	explicit WeightedGraph(size_t size)
	{
		VertexCount = size;
		Vertices = new std::string[VertexCount];
		Weights = new double[VertexCount * VertexCount];
	}

	explicit WeightedGraph(std::ifstream& reader)
	{
		std::string line;
		getline(reader, line);
		VertexCount = stoull(line);

		Vertices = new std::string[VertexCount];
		Weights  = new double[VertexCount * VertexCount];

		// Load the vertex names
		for(auto i = 0; i < VertexCount; i++)
		{
			getline(reader, Vertices[i]);
			if (reader.bad()) throw std::runtime_error("Incomplete or corrupt graph file");
		}

		// Read the adajacency matrix
		for(auto i=0; i <= VertexCount; i++)
		{
			getline(reader, line);
			size_t prev = 0;
			size_t pos;
			size_t count = 0;
			while ((pos = line.find_first_of(" \t,", prev)) != std::string::npos)
			{
				if (pos > prev) SetWeight(i, count++, stod(line.substr(prev, pos - prev), nullptr));
				prev = pos + 1;

				if(count >= VertexCount || reader.bad() && !reader.eof()) throw std::runtime_error("Incomplete or corrupt graph file");
			}
			if (prev < line.length()) SetWeight(i, count, stod(line.substr(prev, std::string::npos), nullptr));
		}
	}

	~WeightedGraph()
	{
		delete[] Vertices;
		delete[] Weights;
	}

	// The number of vertices in this graph
	size_t VertexCount;

	// Actually an array of strings containing the vertex names
	std::string* Vertices;

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
private:
	// Actually an array of doubles containing the weights from a vertex to another vertex
	// If the weight is zero, the vertex is not connected. This is a flattened 2d array
	double*     Weights;
};
