/*
 * DiskAVL.cpp - Implementation of a disk-backed AVL tree
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

#include "DiskAVL.h"
#include "Utils.h"


DiskAVL::DiskAVL(std::string path) : TreePath(path)
{
	std::ifstream reader;
	reader.open(path, std::ios::binary);

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
		reader >> NextNodeID;
		reader >> RootID;
	}

	reader.close();
}

DiskAVL::~DiskAVL()
{
}

Word* DiskAVL::add(std::string key)
{
}

AVLDiskNode* DiskAVL::loadNode(unsigned int id)
{
	if (id == 0) return nullptr;

	std::ifstream reader;
	reader.open(TreePath, std::ios::binary);

	if (!reader.good())
	{
		reader.close();
		throw std::runtime_error("Unable to open tree for read: " + TreePath);
	}
	
	readCount++;

	// Skip nodes until we get to the node we're looking for
	for (unsigned short i = 0; i < id - 1; i++)
	{
		skipReadNode(reader);
	}

	auto node = new AVLDiskNode(id, reader);

	reader.close();

	return node;
}

void DiskAVL::commitNode(AVLDiskNode* node)
{
	std::ifstream reader;
	std::ofstream writer;

	writer.open(TreePath, std::ios::binary);

	if (!writer.good())
	{
		writer.close();
		throw std::runtime_error("Unable to open cluster for read or create: " + TreePath);
	}

	writeCount++;
	readCount++;

	reader.open(TreePath, std::ios::binary);

	// Skip any nodes before this node and seek the writer
	size_t totalSeek = 0;
	for (unsigned short i = 0; i < node->ID - 1; i++)
	{
		totalSeek += skipReadNode(reader);
	}
	writer.seekp(totalSeek, std::ios::cur);

	node->write(writer);

	writer.close();
	reader.close();
}

void DiskAVL::commitBase()
{
	std::ofstream writer;
	writer.open(TreePath, std::ios::binary);

	if (!writer.good())
	{
		writer.close();
		throw std::runtime_error("Unable to open tree for write or create: " + TreePath);
	}

	writeCount++;

	writer.write(reinterpret_cast<const char*>(&NextNodeID), sizeof(NextNodeID));
	writer.write(reinterpret_cast<const char*>(&RootID), sizeof(RootID));

	writer.close();
}

void DiskAVL::doRotations(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate, char delta)
{
}

void DiskAVL::rotateLeftLeft(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate)
{
}

void DiskAVL::rotateLeftRight(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate)
{
}

void DiskAVL::rotateRightRight(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate)
{
}

void DiskAVL::rotateRightLeft(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate)
{
}
