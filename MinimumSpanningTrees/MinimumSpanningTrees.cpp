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
#include <iostream>
#include <fstream>

#include "Options.h"
#include "SpanningTree.h"
#include "WeightedGraph.h"

using namespace std;

void printHelp();
SpanningTree* Kruskal(WeightedGraph& graph);
SpanningTree* Prim(WeightedGraph& graph);

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

	auto k = Kruskal(g);
	auto p = Prim(g);

	// TODO: Print results

	delete k;
	delete p;

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

SpanningTree* Kruskal(WeightedGraph& graph)
{
	// TODO: Implement
	return new SpanningTree();
}

SpanningTree* Prim(WeightedGraph& graph)
{
	// TODO: Implement
	return new SpanningTree();
}
