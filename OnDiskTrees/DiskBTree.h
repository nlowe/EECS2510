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

struct BTreeNode
{
	const unsigned short TFactor;
	const unsigned short MaxKeyLen;

	const unsigned int ID;
	bool isLeaf;

	// N-1 in the slides, but we're zero-indexed so we should be fine
	uint32_t NextFreeKey = 0;
	// Actually an array of Word pointers
	Word** Keys;
	// Actually an array of uint32_t
	uint32_t* Children;

	BTreeNode(unsigned int id, unsigned short factor, unsigned short maxlen) :
		TFactor(factor), MaxKeyLen(maxlen), ID(id), isLeaf(true), NextFreeKey(0)
	{
		Keys = new Word*[MaxNumKeys()]{ nullptr };
		Children = new uint32_t[MaxNumKeys() + 1]{ 0 };
	}

	BTreeNode(unsigned int id, unsigned short factor, unsigned short maxlen, std::fstream& f) :
		BTreeNode(id, factor, maxlen)
	{
		utils::read_binary(f, NextFreeKey);

		for (auto i = 0; i < MaxNumKeys(); i++)
		{
			auto buff = new char[MaxKeyLen];
			f.read(buff, maxlen);

			uint32_t count;
			utils::read_binary(f, count);

			Keys[i] = i >= NextFreeKey ? nullptr : new Word(std::string(buff), count);

			delete[] buff;
		}

		for (auto i = 0; i <= MaxNumKeys(); i++)
		{
			uint32_t c;
			utils::read_binary(f, c);

			Children[i] = c;
			if (c != 0) isLeaf = false;
		}
	}

	~BTreeNode()
	{
		for (auto i = 0; i < MaxNumKeys(); i++)
		{
			if(i < NextFreeKey)
			{
				delete Keys[i];
				Keys[i] = nullptr;
			}
		}
		delete[] Keys;
		delete[] Children;
	}

	void write(std::fstream& f) const
	{
		utils::write_binary(f, NextFreeKey);

		for (auto i = 0; i < MaxNumKeys(); i++)
		{
			if (Keys[i] == nullptr)
			{
				std::string dummy;
				dummy.resize(MaxKeyLen, 0x00);
				f.write(dummy.c_str(), MaxKeyLen);

				unsigned short dummyCount = 0;
				utils::write_binary(f, dummyCount);
			}
			else
			{
				auto k = Keys[i];
				auto buff = k->key;
				buff.resize(MaxKeyLen, 0x00);
				f.write(const_cast<char*>(buff.c_str()), MaxKeyLen);

				utils::write_binary(f, k->count);
			}
		}

		for (auto i = 0; i <= MaxNumKeys(); i++)
		{
			utils::write_binary(f, Children[i]);
		}
	}

	bool isFull() const { return NextFreeKey == MaxNumKeys(); }

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
//     maximum key length * (2*T-factor - 2) bytes: all keys in the node. If a key is not used, it is filled with 0x00
//     4 bytes * (2*T-Factor - 1): array of unsigned integers containing the ID of the child nodes
//
// This structure is not thread-safe for inserts / writes
class DiskBTree : public IWordCounter
{
public:
	DiskBTree(std::string path, uint16_t branchingFactor, uint16_t maxKeySize);
	~DiskBTree();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	void add(std::string key) override;
	std::unique_ptr<Word> find(std::string key) override { return findFrom(RootID, key); }

	std::unique_ptr<DocumentStatistics> getDocumentStatistics() override { return documentStatsFrom(RootID); }

	void inOrderPrint() override;

	// Check to see if the tree is empty (the root is null)
	bool isEmpty() const { return RootID == 0; }

private:
	std::string TreePath = "";

	uint32_t NextNode = 1;
	uint32_t RootID = 0;
	uint16_t TFactor;
	uint16_t MaxKeySize;

	size_t MaxNumKeys() const { return 2 * TFactor - 1; }

	// Allocate a new node. Returns the next available ID and updates the tree metadata
	unsigned int AllocateNode()
	{
		return NextNode++;
	}

	std::unique_ptr<Word> findFrom(uint32_t id, std::string key);

	std::shared_ptr<BTreeNode> load(uint32_t id);

	void commit(std::shared_ptr<BTreeNode> node, bool includeBase = false);
	
	// Write the tree metadata to disk
	void commitBase(bool append=false);

	// Skip over the next node in the specified stream
	void skipReadNode(std::fstream& f) const
	{
		f.seekg(MaxKeySize * MaxNumKeys() + 4 * (MaxNumKeys() + 1), std::ios::cur);
	}

	std::unique_ptr<DocumentStatistics> documentStatsFrom(unsigned int id)
	{
		if (id == 0) return std::make_unique<DocumentStatistics>(0, 0, 0);

		auto n = load(id);

		size_t total = 0;
		size_t distinct = 0;

		for(auto i = 0; i < n->NextFreeKey; i++)
		{
			total += n->Keys[i]->count;
			distinct++;
		}

		size_t subtreeHeight = 0;
		for(auto i = 0; i <= n->NextFreeKey; i++)
		{
			auto subStats = documentStatsFrom(n->Children[i]);
			subtreeHeight = max(subtreeHeight, subStats->TreeHeight);
			total += subStats->TotalWords;
			distinct += subStats->DistinctWords;
		}

		return std::make_unique<DocumentStatistics>(1 + subtreeHeight, total, distinct);
	}

	void insertNonFull(std::shared_ptr<BTreeNode> x, std::string k);
	void split(std::shared_ptr<BTreeNode> x, uint16_t idx);
	void split(std::shared_ptr<BTreeNode> x, uint16_t idx, std::shared_ptr<BTreeNode> y);

};

