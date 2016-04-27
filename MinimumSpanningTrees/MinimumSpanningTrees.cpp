/*
 * MinimumSpanningTrees.cpp - Main entry point for the program
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
#include <cassert>
#include <iostream>
#include <fstream>

#include "LinkedList.h"
#include "Options.h"
#include "WeightedGraph.h"
#include "SpanningTree.h"
#include "Verbose.h"
#include "PrimVertex.h"

using namespace std;

// Since the verbose namespace is used multiple times in different files,
// we need to define an implementation of the enable flag in exactly one
// translation unit so that it is shared between all units
namespace verbose
{
	bool enable = false;
}

void printHelp();
void Kruskal(WeightedGraph& graph);
void Prim(WeightedGraph& graph);

int main(int argc, char* argv[])
{
	// parse the command-line arguments
	auto opts = Options(argc, argv);

	if (opts.help)
	{
		printHelp();
	}
	else if(opts.errors)
	{
		cout << "One or more errors occurred while parsing arguments: " << endl;
		cout << opts.errorMessage;
		cout << endl;
		cout << "Call with --help for help" << endl;

		return -1;
	}

	ifstream reader;
	reader.open(opts.TestFilePath, std::ios::in);

	if(!reader.good())
	{
		reader.close();
		cout << "Unable to open graph for read: '" << opts.TestFilePath << "'" << endl;
		return -1;
	}

	WeightedGraph g(reader);
	reader.close();

	cout << "Finding minimum spanning tree using Kruskal's algorithm..." << endl;
	Kruskal(g);

	cout << endl << "Finding minimum spanning tree using Prim's algorithm..." << endl;
	Prim(g);

    return 0;
}

void printHelp()
{
	cout << "MinimumSpanningTrees <-f path> [-q]" << endl;
	cout << "Parameters:" << endl;
	cout << "\t-f, --file\t\tThe input file to test" << endl;
	cout << "\t-q, --quiet\t\tJust print the weight of the minimum spanning tree" << endl;

	cout << endl;

	cout << "At the end of each algorithm, the generated minimum spanning tree in addition to its total" << endl;
	cout << "weight is printed to standard out. To suppress the printing of the tree, specify the -q flag" << endl;
}

void Kruskal(WeightedGraph& graph)
{
	auto sets = new LinkedList<Vertex>*[graph.VertexCount]{nullptr};

	for(auto i = 0; i < graph.VertexCount; i++)
	{
		sets[i] = new LinkedList<Vertex>();
		sets[i]->add(graph.Vertices[i]);
	}

	verbose::write("[Kruskal] Obtaining edges...");
	auto edges = graph.Edges();
	auto tree = new SpanningTree();

	while (!edges->isEmpty())
	{
		// Find the set containing A and B from this edge
		LinkedList<Vertex> *a = nullptr, *b = nullptr;
		auto edge = edges->dequeue();

		size_t aIndex = 0;
		size_t bIndex = 0;
		for(auto index = 0; index < graph.VertexCount; index++)
		{
			if(sets[index] != nullptr)
			{
				if (sets[index]->Contains(const_cast<Vertex*>(edge->A)))
				{
					a = sets[index];
					aIndex = index;
				}
				if (sets[index]->Contains(const_cast<Vertex*>(edge->B)))
				{
					b = sets[index];
					bIndex = index;
				}

				if (a != nullptr && b != nullptr) break;
			}
		}

		assert(a != nullptr && b != nullptr && edge != nullptr);

		if (aIndex != bIndex)
		{
			verbose::write("[Kruskal] Picking edge " + edge->A->Key + "-" + edge->B->Key + ": " + std::to_string(edge->EdgeWeight));
			a->addAll(*b);
			sets[bIndex] = nullptr;
			tree->accept(edge);
		}
		else
		{
			verbose::write("[Kruskal] Edge " + edge->A->Key + "-" + edge->B->Key + ": " + std::to_string(edge->EdgeWeight) + " is redundant");
		}
	}

	verbose::write("[Kruskal] Done");
	tree->print();
}

void Prim(WeightedGraph& graph)
{
//	MST-Prim(G, w, r)        # r is an arbitrarily chosen vertex
//		for each u ∈ G.V
//			u.key = ∞
//			u.π = NIL
//		r.key = 0
//		Q = G.V                    # Q is a min-priority queue
//		while Q ≠ ∅
//			u = Extract-min(Q)
//			for each v ∈ Adj[u]
//				if v ∈ Q and w(u, v) < v.key
//					v.π = u
//					v.key = w(u, v)
	verbose::write("[Prim] Initializing vertices...");
	
	auto pvs = new PrimVertex*[graph.VertexCount]{ nullptr };
	auto Q = new MinPriorityQueue<PrimVertex>([](PrimVertex* lhs, PrimVertex* rhs) { return lhs->QKey - rhs->QKey; }, graph.VertexCount);

	for(auto i = 0; i < graph.VertexCount; i++)
	{
		auto pv = new PrimVertex(graph.Vertices[i]);
		if (i == 0) pv->QKey = 0;
		pvs[i] = pv;
		Q->enqueue(pv);
	}

	auto tree = new SpanningTree();

	while(!Q->isEmpty())
	{
		auto u = Q->dequeue();
		verbose::write("[Prim] Now analyzing connections from " + u->Key + " (local weight: " + to_string(u->QKey) + ")");

		for(auto c = 0; c < graph.VertexCount; c++)
		{
			auto w = graph.GetWeight(u->ID, c);
			if (w == 0) continue;
			auto v = pvs[c];

			if(Q->contains(v) && w < v->QKey)
			{
				verbose::write(
					"[Prim]     Discovered a better way to get to " + v->Key +
					" (via " + u->Key + " with weight " + to_string(w) +
					(v->QKey == INT64_MAX ? "" : ", previous best was " + to_string(v->QKey) +
					(v->pi == nullptr ? "" : " via " + v->pi->Key)) + ")"
				);

				v->pi = u;
				v->QKey = w;
				Q->notifyPriorityUpdated(v);
			}
		}
	}

	for(int i = graph.VertexCount - 1; i >= 0; i--)
	{
		auto v = pvs[i];
		if (v->pi != nullptr) tree->accept(new VertexPair(v->pi, v, v->QKey));
	}

	verbose::write("[Prim] Done");
	tree->print();
}
