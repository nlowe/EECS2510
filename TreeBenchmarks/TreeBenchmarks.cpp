// TreeBenchmarks.cpp : Defines the entry point for the console application.
//

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

const string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

int runFileBenchmarks(Options options);
int runRandomBenchmarks(Options options);
double benchmarkFile(BST* tree, string path);
double benchmarkRandom(BST* tree, size_t count, size_t itemLength);

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
}

inline string generateRandomString(size_t len)
{
	string result;

	for (auto i = 0; i < len; i++) result += alphabet[rand() % alphabet.length()];

	return result;
}

int main(int argc, char* argv[])
{
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

int runFileBenchmarks(Options options)
{
	string path = options.TestFilePath;

	std::ifstream reader;

	reader.open(path);

	if(!reader.good())
	{
		cerr << "Unable to open " << path << " for read" << endl;
		return -1;
	}

	reader.close();

	binarySearchTree = new BST();
	avlTree = new AVL();
	redBlackTree = new RBT();

	auto overhead = benchmarkFile(nullptr, path);
	auto bstTime = benchmarkFile(binarySearchTree, path);
	auto avlTime = benchmarkFile(avlTree, path);
	auto rbtTime = benchmarkFile(redBlackTree, path);

	if (options.csvMode)
	{
		if (!options.noHeaders)
		{
			cout << "File,Overhead,BTime,BHeight,ATime,AHeight,RTime,RHeight" << endl;
		}
		cout << '"' << path << "\"," << overhead << ',' << bstTime << ',' << binarySearchTree->height() << ',' << avlTime << ',' << avlTree->height() << ',' << rbtTime << ',' << redBlackTree->height() << endl;
	}
	else
	{
		cout << "Total Runtime for file \"" << path << "\": " << (overhead + bstTime + avlTime + rbtTime) << "ms" << endl;
		cout << "Overhead: " << overhead << "ms" << endl;
		cout << "BST: Height=" << binarySearchTree->height() << ", Time=" << bstTime << "ms" << endl;
		cout << "AVL: Height=" << avlTree->height() << ", Time=" << avlTime << "ms" << endl;
		cout << "RBT: Height=" << redBlackTree->height() << ", Time=" << rbtTime << "ms" << endl;
	}

	delete binarySearchTree;
	delete avlTree;
	delete redBlackTree;

	return 0;
}

int runRandomBenchmarks(Options options)
{
	binarySearchTree = new BST();
	avlTree = new AVL();
	redBlackTree = new RBT();

	auto bstTime = benchmarkRandom(binarySearchTree, options.RandomCount, options.RandomSize);
	auto avlTime = benchmarkRandom(avlTree, options.RandomCount, options.RandomSize);
	auto rbtTime = benchmarkRandom(redBlackTree, options.RandomCount, options.RandomSize);

	if(options.csvMode)
	{
		if(!options.noHeaders)
		{
			cout << "Count,Size,BTime,BHeight,ATime,AHeight,RTime,RHeight" << endl;
		}
		cout << options.RandomCount << ',' << options.RandomSize << ',' << bstTime << ',' << binarySearchTree->height() << ',' << avlTime << ',' << avlTree->height() << ',' << rbtTime << ',' << redBlackTree->height() << endl;
	}
	else
	{
		cout << "Total Runtime for " << options.RandomCount << " random strings of length " << options.RandomSize << ": " << (bstTime + avlTime + rbtTime) << "ms" << endl;
		cout << "BST: Height=" << binarySearchTree->height() << ", Time=" << bstTime << "ms" << endl;
		cout << "AVL: Height=" << avlTree->height() << ", Time=" << avlTime << "ms" << endl;
		cout << "RBT: Height=" << redBlackTree->height() << ", Time=" << rbtTime << "ms" << endl;
	}

	delete binarySearchTree;
	delete avlTree;
	delete redBlackTree;

	return 0;
}

double benchmarkFile(BST* tree, string path)
{
	auto start = std::chrono::high_resolution_clock::now();


	ifstream reader;

	reader.open(path);

	string line;
	while(getline(reader, line))
	{
		size_t prev = 0;
		size_t pos;
		while ((pos = line.find_first_of(" \t-'\";:,.!?()", prev)) != string::npos)
		{
			if (pos > prev && tree != nullptr) tree->add(line.substr(prev, pos - prev));
			prev = pos + 1;
		}
		if (prev < line.length() && tree != nullptr) tree->add(line.substr(prev, string::npos));
	}

	reader.close();
	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> duration = end - start;
	return duration.count();
}

// Run a random benchmark against the specified tree, generating "count" random
// alphanumeric strings of length "itemLength". Returns the time in milliseconds
// it took to run
double benchmarkRandom(BST* tree, size_t count, size_t itemLength)
{
	auto start = std::chrono::high_resolution_clock::now();

	for (auto i = 0; i < count; i++)
	{
		tree->add(generateRandomString(itemLength));
	}

	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> duration = end - start;
	return duration.count();
}