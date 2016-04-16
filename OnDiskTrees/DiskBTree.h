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
	const unsigned short BranchingFactor;
	const unsigned short MaxKeyLen;

	const unsigned int ID;
	unsigned int ParentID;
	bool isLeaf;

	uint32_t NextFreeKey = 0;
	Word** Keys;
	uint32_t* Children;

	BTreeNode(unsigned int parent, unsigned int id, unsigned short factor, unsigned short maxlen) :
		BranchingFactor(factor), MaxKeyLen(maxlen), ID(id), ParentID(parent), isLeaf(true), NextFreeKey(0)
	{
		Keys = new Word*[factor-1];
		Children = new uint32_t[factor];
	}

	BTreeNode(unsigned int parent, unsigned int id, unsigned short factor, unsigned short maxlen, std::fstream& f) :
		BTreeNode(parent, id, factor, maxlen)
	{
		utils::read_binary(f, NextFreeKey);

		for (auto i = 0; i < factor - 1; i++)
		{
			std::string buff;
			buff.resize(maxlen);
			f.read(const_cast<char*>(buff.c_str()), maxlen);

			uint32_t count;
			utils::read_binary(f, count);

			Keys[i] = i >= NextFreeKey ? nullptr : new Word(std::string(buff), count);
		}

		for (auto i = 0; i < factor; i++)
		{
			uint32_t c;
			utils::read_binary(f, c);

			Children[i] = c;
			if (c != 0) isLeaf = false;
		}
	}

	~BTreeNode()
	{
		for (auto i = 0; i < BranchingFactor - 1; i++)
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

	void write(std::fstream& f) const
	{
		utils::write_binary(f, NextFreeKey);

		for (auto i = 0; i < BranchingFactor - 1; i++)
		{
			if (Keys[i] == nullptr)
			{
				std::string dummy;
				dummy.resize(MaxKeyLen);
				f.write(dummy.c_str(), MaxKeyLen);

				unsigned short dummyCount = 0;
				utils::write_binary(f, dummyCount);
			}
			else
			{
				Keys[i]->key.resize(MaxKeyLen);
				auto buff = Keys[i]->key.c_str();
				f.write(buff, MaxKeyLen);

				utils::write_binary(f, Keys[i]->count);
			}
		}

		for (auto i = 0; i < BranchingFactor; i++)
		{
			utils::write_binary(f, Children[i]);
		}
	}
};

// A B-Tree in which nodes are kept in clusters on disk
//
// This tree only supports inserts, searches, and an in-order debug treversal
//
// On-Disk Format:
// 
// 4 bytes: unsigned int containing the next node id
// 4 bytes: the ID of the root node, 0 if none
// 2 bytes: unsigned short containing the branching factor of the tree
// 2 bytes: unsigned short containing the maximum length in bytes of all keys
// variable: for each node in the tree
//     maximum key length * (branching factor - 1) bytes: all keys in the node. If a key is not used, it is filled with 0x00
//     4 bytes * branching factor: array of unsigned integers containing the ID of the child nodes
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
	std::unique_ptr<Word> find(std::string key) override;

	// Check to see if the tree is empty (the root is null)
	bool isEmpty() const { return RootID == 0; }

private:
	std::string TreePath = "";

	uint32_t NextNode = 1;
	uint32_t RootID = 0;
	uint16_t BranchingFactor;
	uint16_t MaxKeySize;

	std::shared_ptr<BTreeNode> load(uint32_t parent, uint32_t id);

	void commit(std::shared_ptr<BTreeNode> node, bool includeBase = false);
	
	// Write the tree metadata to disk
	void commitBase(bool append=false);

	// Skip over the next node in the specified stream
	void skipReadNode(std::fstream& f) const
	{
		f.seekg(MaxKeySize * (BranchingFactor - 1) + 4 * BranchingFactor, std::ios::cur);
	}

};

