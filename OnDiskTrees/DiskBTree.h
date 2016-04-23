/*
 * DiskBTree.h - Interface for a BTree in which nodes are stored on disk
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
#include <memory>
#include <iostream>

#include "IWordCounter.h"
#include "Word.h"
#include "Utils.h"

// A structure for a B-Tree node stored on disk
//
// A B-Tree Node has at most 2*T - 1 keys, and 2*T children where T is the degree of the tree
//
// File Format:
//
// For all 2*T - 1 nodes:
//     MaximumKeyLength bytes: The key
//     4 bytes: The occurrance count of the key
// 4 bytes * (2*T): array of unsigned integers containing the ID of the child nodes
struct BTreeNode
{
	// The degree of the tree this node resides in
	const unsigned short TFactor;
	// The maximum allowed length of keys for this tree. When written to disk
	// the keys are padded with 0x00 up this length
	const unsigned short MaxKeyLen;

	// The ID of this node
	const unsigned int ID;
	// Whether or not this node is considered a leaf node
	bool isLeaf;

	// N-1 in the slides, but we're zero-indexed so we should be fine
	uint32_t KeyCount = 0;
	// Actually an array of Word pointers
	Word** Keys;
	// Actually an array of uint32_t
	uint32_t* Children;

	BTreeNode(unsigned int id, unsigned short factor, unsigned short maxlen) :
		TFactor(factor), MaxKeyLen(maxlen), ID(id), isLeaf(true), KeyCount(0)
	{
		Keys = new Word*[MaxNumKeys()]{nullptr};
		Children = new uint32_t[MaxNumKeys() + 1]{ 0 };
	}

	BTreeNode(unsigned int id, unsigned short factor, unsigned short maxlen, std::fstream& f) :
		BTreeNode(id, factor, maxlen)
	{
		// Read the keys
		for (auto i = 0; i < MaxNumKeys(); i++)
		{
			auto buff = new char[MaxKeyLen + 1]{ 0 };
			f.read(buff, maxlen);

			uint32_t count;
			utils::read_binary(f, count);

			// Check to see if we even have a valid string (unused keys are just filled with null bytes)
			if(buff[0] != 0)
			{
				Keys[i] = new Word(std::string(buff), count);
				KeyCount++;
			}

			delete[] buff;
		}

		// Now, read the child ID's
		for (auto i = 0; i <= MaxNumKeys(); i++)
		{
			uint32_t c;
			utils::read_binary(f, c);

			Children[i] = c;
			if (c != 0) isLeaf = false; // Remember if we have at least one child
		}
	}

	~BTreeNode()
	{
		// Free all allocated keys
		for(auto i = 0; i < KeyCount; i++)
		{
			if(Keys[i] != nullptr)
			{
				delete Keys[i];
				Keys[i] = nullptr;
			}
		}

		delete[] Keys;
		delete[] Children;
	}

	// Writes this node to the specified stream. The stream should already be
	// in the correct offset for writing this node.
	void write(std::fstream& f) const
	{
		for (auto i = 0; i < MaxNumKeys(); i++)
		{
			if (Keys[i] == nullptr)
			{
				// The key is empty, just write a bunch of zeroes
				auto dummy = new char[MaxKeyLen]{0};
				f.write(dummy, MaxKeyLen);

				uint32_t dummyCount = 0;
				utils::write_binary(f, dummyCount);
			}
			else
			{
				// Write the key...
				auto k = Keys[i];
				auto buff = k->key;
				buff.resize(MaxKeyLen, 0x00);
				f.write(const_cast<char*>(buff.c_str()), MaxKeyLen);

				// ...And it's occurrance count
				utils::write_binary(f, k->count);
			}
		}

		// Write the child pointers
		for (auto i = 0; i <= MaxNumKeys(); i++)
		{
			utils::write_binary(f, Children[i]);
		}
	}

	// Whether or not this node is full
	bool isFull() const { return KeyCount == MaxNumKeys(); }
	// Whether or not this node is empty
	bool isEmpty() const { return KeyCount == 0; }

	// The maximum number of keys this node can hold
	size_t MaxNumKeys() const { return 2 * TFactor - 1; }
};

// A B-Tree in which nodes are kept in clusters on disk
//
// This tree only supports inserts, searches, and an in-order debug treversal
//
// On-Disk Format:
// 
// 4 bytes: unsigned int containing the next node id
// 4 bytes: the ID of the root node, 0 if none
// 2 bytes: unsigned short containing the T-factor of the tree
// 2 bytes: unsigned short containing the maximum length in bytes of all keys
// variable: for each node in the tree
//     maximum key length * (2*T-factor - 1) bytes: all keys in the node. If a key is not used, it is filled with 0x00
//     4 bytes * (2*T-Factor): array of unsigned integers containing the ID of the child nodes
//
// This structure is not thread-safe for inserts / writes
class DiskBTree : public IWordCounter
{
public:
	DiskBTree(std::string path, uint16_t branchingFactor, uint16_t maxKeySize);
	~DiskBTree();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	void add(std::string key) override;

	// Find the given key in the tree
	// Returns:
	//     A unique pointer to a word represented by the specified key, or a nullptr if not found
	std::unique_ptr<Word> find(std::string key) override { return findFrom(RootID, key); }

	// Get statistics about the tree, such as its height and the number of total and distinct words
	// Returns:
	//     A unique pointer to a DocumentStatistics object
	std::unique_ptr<DocumentStatistics> getDocumentStatistics() override { return documentStatsFrom(RootID); }

	size_t GetFileSize() override
	{
		FileHandle.seekg(0, std::ios::end);
		size_t ret = FileHandle.tellg();
		FileHandle.seekg(0, std::ios::beg);

		return ret;
	}

	// Performs an in-order traversal on the tree, printing out the keys and their occurrance counts
	void inOrderPrint() override { return inOrderPrintFrom(RootID); }

	// Check to see if the tree is empty (the root is null)
	bool isEmpty() const { return RootID == 0; }

private:
	// The path to the tree
	std::string TreePath = "";
	// The file handle used for accessing the tree
	std::fstream FileHandle;

	// The ID of the next node to be allocated
	uint32_t NextNode = 1;
	// The ID of the root node
	uint32_t RootID = 0;
	// The degree of the tree nodes
	uint16_t TFactor;
	// The maximum size of any key in this particular tree
	uint16_t MaxKeySize;

	// The maximum number of keys any particular node in this tree can hold
	size_t MaxNumKeys() const { return 2 * TFactor - 1; }

	// Allocate a new node. Returns the next available ID and updates the tree metadata
	unsigned int AllocateNode()
	{
		return NextNode++;
	}

	// Performs an in-order traversal on the sub-tree from the specified node,
	// printing out the keys and their occurrance counts
	void inOrderPrintFrom(uint32_t node);

	// Search the specified subtree for the specified key
	std::unique_ptr<Word> findFrom(uint32_t node, std::string key);

	// Attempt to load the specified node from disk. A runtime
	// exception is thrown if the node could not be read
	std::shared_ptr<BTreeNode> load(uint32_t id);

	// Commit the specified node (and optionally the tree metadata) to disk
	void commit(std::shared_ptr<BTreeNode> node, bool includeBase = false);
	
	// Write the tree metadata to disk
	void commitBase(bool append=false);

	// Get statistics about the sub-tree from the specified node, such as its
	// height and the number of total and distinct words
	// Returns:
	//     A unique pointer to a DocumentStatistics object
	std::unique_ptr<DocumentStatistics> documentStatsFrom(uint32_t id)
	{
		if (id == 0) return std::make_unique<DocumentStatistics>(0, 0, 0);

		auto n = load(id);

		size_t total = 0;
		size_t distinct = n->KeyCount;

		// Get the total number of words in all keys of this node
		for(auto i = 0; i < n->KeyCount; i++)
		{
			total += n->Keys[i]->count;
		}

		// And process all sub-children
		size_t subtreeHeight = 0;
		size_t subtreeNodes = 0;
		for(auto i = 0; i <= n->KeyCount; i++)
		{
			auto subStats = documentStatsFrom(n->Children[i]);
			subtreeHeight = max(subtreeHeight, subStats->TreeHeight); // New maximum height?
			total += subStats->TotalWords;
			distinct += subStats->DistinctWords;
			subtreeNodes += subStats->TotalNodes;
		}

		return std::make_unique<DocumentStatistics>(1 + subtreeHeight, total, distinct, 1 + subtreeNodes);
	}

	// Insert the specified key k into the guaranteed non-full node x
	// Note that one or more children of x may be full. They will be
	// split if needed.
	void insertNonFull(std::shared_ptr<BTreeNode> x, std::string k);

	// Split the child of x at the specified index, promoting the median into x
	void split(std::shared_ptr<BTreeNode> x, uint16_t idx);
	// Split the child of x at the specified index (that is already loaded and available at y)
	void split(std::shared_ptr<BTreeNode> x, uint16_t idx, std::shared_ptr<BTreeNode> y);

};

