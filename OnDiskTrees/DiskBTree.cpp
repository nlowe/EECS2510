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
#include "DiskBTree.h"
#include <cassert>


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

		// Probably a new tree, try to commit the empty metadata
		commitBase();
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


void DiskBTree::add(std::string key)
{
	auto r = load(RootID);
	assert(r != nullptr);

	if(r->isFull())
	{
		auto s = std::make_shared<BTreeNode>(AllocateNode(), TFactor, MaxKeySize);
		RootID = s->ID;
		s->isLeaf = false;
		s->Children[s->NextFreeKey++] = r->ID;
		split(s, 0);
		insertNonFull(s, key);
	}
	else
	{
		insertNonFull(r, key);
	}
}

std::unique_ptr<Word> DiskBTree::findFrom(uint32_t id, std::string key)
{
	if (id == 0) return nullptr;

	auto n = load(id);
	auto i = 0;

	while (i <= n->NextFreeKey - 1 && key.compare(n->Keys[i]->key) > 0)
	{
		i++;
		comparisons++;
	}

	comparisons += 2; // One for the last check of the while loop, and one for the next check
	if (i <= n->NextFreeKey - 1 && key.compare(n->Keys[i]->key) == 0)
	{
		return std::make_unique<Word>(n->Keys[i]->key, n->Keys[i]->count);
	}
	
	if(n->isLeaf) return nullptr;
	
	return findFrom(n->Children[i], key);
}

std::shared_ptr<BTreeNode> DiskBTree::load(uint32_t id)
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

	auto node = std::make_shared<BTreeNode>(id, TFactor, MaxKeySize, f);

	f.close();

	return node;
}

void DiskBTree::commit(std::shared_ptr<BTreeNode> node, bool includeBase)
{
	std::fstream f(TreePath, std::ios::binary | std::ios::in | std::ios::out);

	if (!f.good())
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

void DiskBTree::insertNonFull(std::shared_ptr<BTreeNode> x, std::string k)
{
	auto i = x->NextFreeKey - 1;
	if(x->isLeaf)
	{
		while(i >= 0 && k.compare(x->Keys[i]->key) < 0)
		{
			x->Keys[i + 1] = x->Keys[i];
			i--;
			comparisons++;
		}
		x->Keys[i + 1] = new Word(k);
		x->NextFreeKey++;
		commit(x);
	}
	else
	{
		while(i >= 0 && k.compare(x->Keys[i]->key) < 0)
		{
			i--;
			comparisons++;
		}
		i++;
		auto y = load(x->Children[i]);
		if (y->isFull())
		{
			split(x, i, y);
			comparisons++;
			if(k.compare(x->Keys[i]->key) > 0)
			{
				i++;
			}
			insertNonFull(y, k);
		}
	}
}

void DiskBTree::split(std::shared_ptr<BTreeNode> x, uint16_t idx)
{
	split(x, idx, load(x->Children[idx]));
}

void DiskBTree::split(std::shared_ptr<BTreeNode> x, uint16_t idx, std::shared_ptr<BTreeNode> y)
{
	auto z = std::make_shared<BTreeNode>(AllocateNode(), TFactor, MaxKeySize);
	z->isLeaf = y->isLeaf;
	z->NextFreeKey = TFactor - 1;

	for(auto j = 0; j < TFactor; j++)
	{
		z->Keys[j] = y->Keys[j + TFactor];
	}

	if(!y->isLeaf)
	{
		for(auto j = 0; j <= TFactor; j++)
		{
			z->Children[j] = y->Children[j + TFactor];
		}
	}
	y->NextFreeKey = TFactor - 1;

	for(auto j = x->NextFreeKey; j >= idx + 1; j--)
	{
		x->Children[j + 1] = x->Children[j];
	}
	x->Children[idx + 1] = z->ID;

	for(auto j = x->NextFreeKey; j >= idx; j--)
	{
		x->Keys[j + 1] = x->Keys[j];
	}
	x->Keys[idx] = x->Keys[TFactor];
	x->NextFreeKey++;

	commit(y);
	commit(z);
	commit(x);
}
