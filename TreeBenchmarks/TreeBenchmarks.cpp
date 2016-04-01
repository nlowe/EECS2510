/*
 * TreeBenchmarks.cpp - The main benchmarking harness
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
#include <chrono>

#include "AVL.h"
#include "RBT.h"
#include "Options.h"

using namespace std;

BST* binarySearchTree;
AVL* avlTree;
RBT* redBlackTree;

// When benchmarking random strings, they will be made up of these characters
const string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// Forward-declare the functions so main can be at the top of the file as required
void printHelp();
inline string generateRandomString(size_t len);
int runFileBenchmarks(Options options);
int runRandomBenchmarks(Options options);
double benchmarkFile(BST* tree, string path);
double benchmarkRandom(BST* tree, size_t count, size_t itemLength);

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
	else if(opts.RandomCount > 0 && opts.RandomSize > 0)
	{
		return runRandomBenchmarks(opts);
	}
	else if(opts.TestFilePath != "")
	{
		return runFileBenchmarks(opts);
	}
	else
	{
		printHelp();
	}

    return 0;
}

void printHelp()
{
	cout << "TreeBenchmarks <-f path || <-r count <-s size>> [-c [-n]]" << endl;
	cout << "Parameters:" << endl;
	cout << "\t-f, --file\t\tThe input file to test" << endl;
	cout << "\t-r, --random-count\tThe number of random strings to insert" << endl;
	cout << "\t-s, --random-size\tThe size of the random strings to insert" << endl;
	cout << "\t-c, --csv\t\tOutput data in CSV Format" << endl;
	cout << "\t-n, --no-headers\tDon't include headers in CSV. Implies -c" << endl;

	cout << endl;

	cout << "In file mode, the file will be processed line by line and each word is inserted into" << endl;
	cout << "each of the binary trees under test. Words that occur more than once in the file will" << endl;
	cout << "have their count incremented. Stats pertaining to the tree are recorded for each tree." << endl;

	cout << endl;

	cout << "In random mode, the specified number of randomly generated strings are inserted into" << endl;
	cout << "each tree under test. Multiple occurrences of each word is recorded. Stats pertaining" << endl;
	cout << "the tree are recorded." << endl;

	cout << endl;

	cout << "If CSV mode is not specified, an in-order traversal will also be performed on each" << endl;
	cout << "tree implementation, listing the words and the number of times they each occur" << endl;
}

// Run a file benchmark with the specified options
int runFileBenchmarks(Options options)
{
	string path = options.TestFilePath;

	ifstream reader;

	reader.open(path);

	// Ensure the file exists
	if(!reader.good())
	{
		cerr << "Unable to open " << path << " for read" << endl;
		return -1;
	}

	reader.close();

	// initialize the trees
	binarySearchTree = new BST();
	avlTree = new AVL();
	redBlackTree = new RBT();

	// Run the benchmarks, recording the time
	auto overhead = benchmarkFile(nullptr, path);
	auto bstTime = benchmarkFile(binarySearchTree, path);
	auto avlTime = benchmarkFile(avlTree, path);
	auto rbtTime = benchmarkFile(redBlackTree, path);

	// Print the results
	if (options.csvMode)
	{
		if (!options.noHeaders)
		{
			cout << "File,Overhead,BTime,BHeight,BDist,BTotal,BComp,BRef,ATime,AHeight,ADist,ATotal,AComp,ARef,ABal,RTime,RHeight,RDist,RTotal,RComp,RRef,RRec" << endl;
		}
		cout << '"' << path << "\"," << overhead << ',';
		cout << bstTime << ',' << binarySearchTree->height() << ',' << binarySearchTree->totalNodes() << ',' << binarySearchTree->totalWords() << ',' << binarySearchTree->getComparisonCount() << ',' << binarySearchTree->getReferenceChanges() << ',';
		cout << avlTime << ',' << avlTree->height() << ',' << avlTree->totalNodes() << ',' << avlTree->totalWords() << ',' << avlTree->getComparisonCount() << ',' << avlTree->getReferenceChanges() << ',' << avlTree->getBalanceFactorChangeCount() << ',';
		cout << rbtTime << ',' << redBlackTree->height() << ',' << redBlackTree->totalNodes() << ',' << redBlackTree->totalWords() << ',' << redBlackTree->getComparisonCount() << ',' << redBlackTree->getReferenceChanges() << ',' << redBlackTree->getRecolorCount() << endl;
	}
	else
	{
		cout << "Total Runtime for file \"" << path << "\": " << (overhead + bstTime + avlTime + rbtTime) << "ms" << endl;
		cout << "Overhead: " << overhead << "ms" << endl;
		cout << "BST: Height=" << binarySearchTree->height() << ", DistinctWords=" << binarySearchTree->totalNodes() << ", TotalWords=" << binarySearchTree->totalWords() << ", Time=" << bstTime << "ms, Comparisons=" << binarySearchTree->getComparisonCount() << ", ReferenceChanges=" << binarySearchTree->getReferenceChanges() << endl;
		cout << "AVL: Height=" << avlTree->height() << ", DistinctWords=" << avlTree->totalNodes() << ", TotalWords=" << avlTree->totalWords() << ", Time=" << avlTime << "ms, Comparisons=" << avlTree->getComparisonCount() << ", ReferenceChanges=" << avlTree->getReferenceChanges() << ", BalanceFactorChanges=" << avlTree->getBalanceFactorChangeCount() << endl;
		cout << "RBT: Height=" << redBlackTree->height() << ", DistinctWords=" << redBlackTree->totalNodes() << ", TotalWords=" << redBlackTree->totalWords() << ", Time=" << rbtTime << "ms, Comparisons=" << redBlackTree->getComparisonCount() << ", ReferenceChanges=" << redBlackTree->getReferenceChanges() << ", ReColors=" << redBlackTree->getRecolorCount() << endl;

		cout << "BST In Order:" << endl;
		binarySearchTree->inOrderPrint();
		cout << "--------------------------" << endl << endl;
		cout << "AVL In Order:" << endl;
		avlTree->inOrderPrint();
		cout << "--------------------------" << endl << endl;
		cout << "RBT In Order:" << endl;
		redBlackTree->BST::inOrderPrint();
		cout << "--------------------------" << endl;
	}

	// Free the trees
	delete binarySearchTree;
	delete avlTree;
	delete redBlackTree;

	return 0;
}

// Run a random benchmark with the specified options
int runRandomBenchmarks(Options options)
{
	// Initialize the trees
	binarySearchTree = new BST();
	avlTree = new AVL();
	redBlackTree = new RBT();

	// Run the benchmarks and record the times
	auto bstTime = benchmarkRandom(binarySearchTree, options.RandomCount, options.RandomSize);
	auto avlTime = benchmarkRandom(avlTree, options.RandomCount, options.RandomSize);
	auto rbtTime = benchmarkRandom(redBlackTree, options.RandomCount, options.RandomSize);

	// Print the results
	if(options.csvMode)
	{
		if(!options.noHeaders)
		{
			cout << "Count,Size,BTime,BHeight,BDist,BTotal,BComp,BRef,ATime,AHeight,ADist,ATotal,AComp,ARef,ABal,RTime,RHeight,RDist,RTotal,RComp,RRef,RRec" << endl;
		}
		cout << options.RandomCount << ',' << options.RandomSize << ',';
		cout << bstTime << ',' << binarySearchTree->height() << ',' << binarySearchTree->totalNodes() << ',' << binarySearchTree->totalWords() << ',' << binarySearchTree->getComparisonCount() << ',' << binarySearchTree->getReferenceChanges() << ',';
		cout << avlTime << ',' << avlTree->height() << ',' << avlTree->totalNodes() << ',' << avlTree->totalWords() << ',' << avlTree->getComparisonCount() << ',' << avlTree->getReferenceChanges() << ',' << avlTree->getBalanceFactorChangeCount() << ',';
		cout << rbtTime << ',' << redBlackTree->height() << ',' << redBlackTree->totalNodes() << ',' << redBlackTree->totalWords() << ',' << redBlackTree->getComparisonCount() << ',' << redBlackTree->getReferenceChanges() << ',' << redBlackTree->getRecolorCount() << endl;
	}
	else
	{
		cout << "Total Runtime for " << options.RandomCount << " random strings of length " << options.RandomSize << ": " << (bstTime + avlTime + rbtTime) << "ms" << endl;
		cout << "BST: Height=" << binarySearchTree->height() << ", DistinctWords=" << binarySearchTree->totalNodes() << ", TotalWords=" << binarySearchTree->totalWords() << ", Time=" << bstTime << "ms, Comparisons=" << binarySearchTree->getComparisonCount() << ", ReferenceChanges=" << binarySearchTree->getReferenceChanges() << endl;
		cout << "AVL: Height=" << avlTree->height() << ", DistinctWords=" << avlTree->totalNodes() << ", TotalWords=" << avlTree->totalWords() << ", Time=" << avlTime << "ms, Comparisons=" << avlTree->getComparisonCount() << ", ReferenceChanges=" << avlTree->getReferenceChanges() << ", BalanceFactorChanges=" << avlTree->getBalanceFactorChangeCount() << endl;
		cout << "RBT: Height=" << redBlackTree->height() << ", DistinctWords=" << redBlackTree->totalNodes() << ", TotalWords=" << redBlackTree->totalWords() << ", Time=" << rbtTime << "ms, Comparisons=" << redBlackTree->getComparisonCount() << ", ReferenceChanges=" << redBlackTree->getReferenceChanges() << ", ReColors=" << redBlackTree->getRecolorCount() << endl;

		cout << "BST In Order:" << endl;
		binarySearchTree->inOrderPrint();
		cout << "--------------------------" << endl << endl;
		cout << "AVL In Order:" << endl;
		avlTree->inOrderPrint();
		cout << "--------------------------" << endl << endl;
		cout << "RBT In Order:" << endl;
		redBlackTree->BST::inOrderPrint();
		cout << "--------------------------" << endl;
	}

	// Free the trees
	delete binarySearchTree;
	delete avlTree;
	delete redBlackTree;

	return 0;
}

// Run a file benchmark against the specified tree implementation and file
double benchmarkFile(BST* tree, string path)
{
	auto start = std::chrono::high_resolution_clock::now();

	ifstream reader;

	reader.open(path);

	// Read the file line by line and then word by word
	// We assume we can read the file, as this is tested in the function that calls this
	string line;
	while(getline(reader, line))
	{
		size_t prev = 0;
		size_t pos;
		while ((pos = line.find_first_of(" \t-'\";:,.!?()[]", prev)) != string::npos)
		{
			if (pos > prev && tree != nullptr) tree->add(line.substr(prev, pos - prev));
			prev = pos + 1;
		}
		if (prev < line.length() && tree != nullptr) tree->add(line.substr(prev, string::npos));
	}

	reader.close();
	auto end = chrono::high_resolution_clock::now();

	// Convert to floating point milliseconds
	chrono::duration<double, milli> duration = end - start;
	return duration.count();
}

// Run a random benchmark against the specified tree, generating "count" random
// alphanumeric strings of length "itemLength". Returns the time in milliseconds
// it took to run
double benchmarkRandom(BST* tree, size_t count, size_t itemLength)
{
	auto start = chrono::high_resolution_clock::now();

	for (auto i = 0; i < count; i++)
	{
		tree->add(generateRandomString(itemLength));
	}

	auto end = chrono::high_resolution_clock::now();

	chrono::duration<double, milli> duration = end - start;
	return duration.count();
}

// Generate a random string of the specified length
inline string generateRandomString(size_t len)
{
	string result;

	for (auto i = 0; i < len; i++) result += alphabet[rand() % alphabet.length()];

	return result;
}
