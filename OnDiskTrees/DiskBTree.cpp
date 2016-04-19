/*
 * DiskBTree.cpp - Implementation of a disk-backed BTree
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

#include "DiskBTree.h"


DiskBTree::DiskBTree(std::string path, uint16_t branchingFactor, uint16_t maxKeySize)
	: TreePath(path), TFactor(branchingFactor), MaxKeySize(maxKeySize)
{
	std::fstream reader;
	reader.open(path, std::ios::binary | std::ios::in);

	readCount++;

	if(!reader.good())
	{
		// Ensure the directory the tree should be placed in exists
		utils::createDirectories(utils::parent(path));

		// We have to commit the base first instead of with the first node
		// since there is no way to distinguish between the file not existing
		// and an IO error
		commitBase();
	
		// Probably a new tree, try to commit the empty metadata
		auto x = new BTreeNode(AllocateNode(), TFactor, MaxKeySize);
		x->isLeaf = true;
		RootID = x->ID;
		commit(x, true);

		delete x;
	}
	else
	{
		// Existing tree
		utils::read_binary(reader, NextNode);
		utils::read_binary(reader, RootID);
		utils::read_binary(reader, TFactor);
		utils::read_binary(reader, MaxKeySize);
	}

	reader.close();
}


DiskBTree::~DiskBTree()
{
}

// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
void DiskBTree::add(std::string key)
{
	if (key.length() > MaxKeySize) throw std::runtime_error("Key to large. Try again with a larger max key size");

	auto r = load(RootID);
	assert(r != nullptr);

	if(r->isFull())
	{
		// If the root node is full, we have to push a new root out to the top
		auto s = new BTreeNode(AllocateNode(), TFactor, MaxKeySize);

		RootID = s->ID;
		s->isLeaf = false;
		s->Children[s->KeyCount] = r->ID;
		commit(s, true);
		split(s, 0, r);
		insertNonFull(s, key);

		delete s;
	}
	else
	{
		insertNonFull(r, key);
	}

	delete r;
}

// Performs an in-order traversal on the sub-tree from the specified node,
// printing out the keys and their occurrance counts
void DiskBTree::inOrderPrintFrom(uint32_t id)
{
	if (id == 0) return;

	auto node = load(id);

	for(auto i=0; i < node->KeyCount; i++)
	{
		inOrderPrintFrom(node->Children[i]);
		std::cout << node->Keys[i]->key << ": " << node->Keys[i]->count << std::endl;
	}
	inOrderPrintFrom(node->Children[node->KeyCount]);

	delete node;
}

// Search the specified subtree for the specified key
std::unique_ptr<Word> DiskBTree::findFrom(uint32_t id, std::string key)
{
	if (id == 0) return nullptr;

	auto x = load(id);
	auto i = 0;

	while (i < x->KeyCount && key.compare(x->Keys[i]->key) > 0)
	{
		i++;
		comparisons++;
	}

	std::unique_ptr<Word> res;

	if (i < x->KeyCount && key.compare(x->Keys[i]->key) == 0)
	{
		comparisons += 2; // One for the last check of the while loop, and one for the if statement
		res = std::make_unique<Word>(x->Keys[i]->key, x->Keys[i]->count);
	}
	else if(!x->isLeaf)
	{
		res = findFrom(x->Children[i-1], key);
	}
	
	delete x;
	
	return res;
}

// Attempt to load the specified node from disk. A runtime
// exception is thrown if the node could not be read
BTreeNode* DiskBTree::load(uint32_t id)
{
	if (id == 0) return nullptr;

	std::fstream f(TreePath, std::ios::binary | std::ios::in);

	if (!f.good())
	{
		f.close();
		throw std::runtime_error("Unable to open tree for read: " + TreePath);
	}
	readCount++;

	// Skip the metadata
	f.seekg(sizeof(NextNode) + sizeof(RootID) + sizeof(TFactor) + sizeof(MaxKeySize), std::ios::beg);

	// Skip nodes until we get to the node we're looking for
	for (auto i = 0; i < id - 1; i++)
	{
		skipReadNode(f);
	}

	auto node = new BTreeNode(id, TFactor, MaxKeySize, f);

	f.close();

	return node;
}

// Commit the specified node (and optionally the tree metadata) to disk
void DiskBTree::commit(BTreeNode* node, bool includeBase)
{
	std::fstream f(TreePath, std::ios::binary | std::ios::in | std::ios::out);

	if (!f.good() && !includeBase)
	{
		f.close();
		throw std::runtime_error("Unable to open tree for read or create: " + TreePath);
	}

	writeCount++;
	readCount++;

	if(includeBase)
	{
		utils::write_binary(f, NextNode);
		utils::write_binary(f, RootID);
		utils::write_binary(f, TFactor);
		utils::write_binary(f, MaxKeySize);
		f.flush();
	}
	else
	{
		// We're noot writing the base metadata, skip over it
		f.seekp(sizeof(NextNode) + sizeof(RootID) + sizeof(TFactor) + sizeof(MaxKeySize), std::ios::beg);
	}

	// Skip any nodes before this node and seek the writer
	for (unsigned short i = 0; i < node->ID - 1; i++)
	{
		skipReadNode(f);
	}

	node->write(f);

	f.close();
}

// Write the tree metadata to disk
void DiskBTree::commitBase(bool append)
{
	auto flags = std::ios::binary | std::ios::out;
	if (append) flags |= std::ios::in;

	std::fstream f(TreePath, flags);

	if (!f.good())
	{
		f.close();
		throw std::runtime_error("Unable to open tree for write or create: " + TreePath);
	}

	writeCount++;

	utils::write_binary(f, NextNode);
	utils::write_binary(f, RootID);
	utils::write_binary(f, TFactor);
	utils::write_binary(f, MaxKeySize);

	f.close();
}

// Insert the specified key k into the guaranteed non-full node x
// Note that one or more children of x may be full. They will be
// split if needed.
void DiskBTree::insertNonFull(BTreeNode* x, std::string k)
{
	// Are we inserting the first element?
	if(x->isEmpty())
	{
		x->Keys[x->KeyCount++] = new Word(k);
		commit(x);
		return;
	}
	
	int64_t i = x->KeyCount - 1;

	// See if we're inserting a duplicate
	for(auto j = 0; j <= i; j++)
	{
		auto res = k.compare(x->Keys[j]->key);
		if(res == 0)
		{
			x->Keys[j]->count++;
			commit(x);
			return;
		}

		if (res < 0) break;
	}

	if(x->isLeaf)
	{
		// X is a leaf. Find where to insert the new key at
		while(i >= 0 && k.compare(x->Keys[i]->key) < 0)
		{
			x->Keys[i + 1] = x->Keys[i--];
			comparisons++;
		}

		x->Keys[i + 1] = new Word(k);
		x->KeyCount++;

		commit(x, true);
	}
	else
	{
		// X has child pointers, and the key is not in this node
		// So find the child we need to search next
		while(i >= 0 && k.compare(x->Keys[i]->key) < 0)
		{
			i--;
			comparisons++;
		}
		i++; // We're not 1-indexed...

		auto y = load(x->Children[i]);

		if (y->isFull())
		{
			// See if we're inserting a duplicate first, since we don't have to split then
			for(auto j = 0; j <= y->KeyCount - 1; j++)
			{
				auto res = k.compare(y->Keys[j]->key);
				if(res == 0)
				{
					y->Keys[j]->count++;
					commit(y);
					delete y;
					return;
				}
				
				if (res < 0) break;
			}

			// Split the node so we have room to insert the new key
			split(x, i, y);
			comparisons++;
			if(k.compare(x->Keys[i]->key) > 0)
			{
				i++;
			}
		}

		// The child index we're looking at may have changed
		// Re-Load it
		delete y;
		y = load(x->Children[i]);

		insertNonFull(y, k);
		delete y;
	}
}

// Split the child of x at the specified index, promoting the median into x
void DiskBTree::split(BTreeNode* x, uint16_t idx)
{
	auto y = load(x->Children[idx]);
	split(x, idx, y);
	delete y;
}

// Split the child of x at the specified index (that is already loaded and available at y)
void DiskBTree::split(BTreeNode* x, uint16_t i, BTreeNode* y)
{
	auto z = new BTreeNode(AllocateNode(), TFactor, MaxKeySize);
	z->isLeaf = y->isLeaf;
	z->KeyCount = y->KeyCount = TFactor - 1;

	// Move the largest t – 1 keys and corresponding t children from y to z
	for(int64_t j = 0; j < TFactor - 1; j++)
	{
		z->Keys[j] = y->Keys[j + TFactor];
		y->Keys[j + TFactor] = nullptr;
	}

	if(!y->isLeaf)
	{
		// Y isn't a leaf. Don't forget to move t child pointers
		for(auto j = 0; j < TFactor; j++)
		{
			z->Children[j] = y->Children[j + TFactor];
			y->Children[j + TFactor] = 0;
		}
	}

	// insert z as a child of x
	for(int64_t j = x->KeyCount; j >= i; j--)
	{
		x->Children[j + 1] = x->Children[j];
	}
	x->Children[i+1] = z->ID;

	// Make room for the median of the split
	for(int64_t j = x->KeyCount; j > i; j--)
	{
		x->Keys[j] = x->Keys[j-1];
	}

	// Promote the median
	x->Keys[i] = y->Keys[TFactor-1];
	y->Keys[TFactor - 1] = nullptr;

	// And update the key count of x
	x->KeyCount++;

	// Commit all the things
	commit(x);
	commit(y);
	commit(z, true);

	// And clean up
	delete z;
}
