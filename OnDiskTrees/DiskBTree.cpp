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


DiskBTree::DiskBTree(std::string path, uint16_t branchingFactor, uint16_t maxKeySize)
	: TreePath(path), BranchingFactor(branchingFactor), MaxKeySize(maxKeySize)
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
		utils::read_binary(reader, BranchingFactor);
		utils::read_binary(reader, MaxKeySize);
	}

	reader.close();
}


DiskBTree::~DiskBTree()
{
}


void DiskBTree::add(std::string key)
{
}

std::unique_ptr<Word> DiskBTree::find(std::string key)
{
	throw std::runtime_error("Not Implemented");
}

std::shared_ptr<BTreeNode> DiskBTree::load(uint32_t parent, uint32_t id)
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
	f.seekg(sizeof(NextNode) + sizeof(RootID) + sizeof(BranchingFactor) + sizeof(MaxKeySize), std::ios::beg);

	// Skip nodes until we get to the node we're looking for
	for (auto i = 0; i < id - 1; i++)
	{
		skipReadNode(f);
	}

	auto node = std::make_shared<BTreeNode>(parent, id, BranchingFactor, MaxKeySize, f);

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
		utils::write_binary(f, BranchingFactor);
		utils::write_binary(f, MaxKeySize);
		f.flush();
	}
	else
	{
		// We're noot writing the base metadata, skip over it
		f.seekp(sizeof(NextNode) + sizeof(RootID) + sizeof(BranchingFactor) + sizeof(MaxKeySize), std::ios::beg);
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
	utils::write_binary(f, BranchingFactor);
	utils::write_binary(f, MaxKeySize);

	f.close();
}
