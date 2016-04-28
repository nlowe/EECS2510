/*
 * PrimVertex.h - Interface for a PrimVertex
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
#include "WeightedGraph.h"

// A wrapper around a standard vertex that includes a pointer to the parent vertex
// and the weight of the branch taken to get from the parent vertex to here. This
// is used when calculating the minimum spanning tree using Prim's algorithm
struct PrimVertex : Vertex
{
	explicit PrimVertex(Vertex* v) : Vertex(v->ID, v->Name), QKey(INT64_MAX), pi(nullptr) {}

	bool operator==(const PrimVertex& rhs) const { return rhs.Name == Name; }

	// The weight of the branch taken to get to this vertex, used as
	// the key in the minimum priority queue
	int64_t QKey;

	// The parent vertex
	PrimVertex* pi;
};
