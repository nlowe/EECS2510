/*
 * DiskAVL.h - Interface for an AVL Tree in which nodes are stored in clusters on disk
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

#include "Word.h"
#include "Utils.h"
#include "IWordCounter.h"
#include <cassert>

// An AVL Tree node that is stored on disk
//
// MaxKeyLength: node key
// 4 bytes:  unsigned integer containing the number of times the word appears
// 1 byte:   signed char containing the balance factor of the node
// 4 bytes:  unsigned integer containing the ID of the left child, 0 if none
// 4 bytes:  unsigned integer containing the ID of the right child, 0 if none
struct AVLDiskNode
{
	// The maximum allowed length of keys for this tree. When written to disk
	// the keys are padded with 0x00 up this length
	const unsigned short MaxKeyLen;

	// The ID of the node
	unsigned int ID;
	// The word payload the node contains
	Word* Payload;
	// The balance factor of the node (-1, 0, or 1)
	char BalanceFactor;
	// The ID of the left child
	uint32_t LeftID;
	// The ID of the right child
	uint32_t RightID;

	explicit AVLDiskNode(int id, unsigned short maxKeyLen, Word* payload) : MaxKeyLen(maxKeyLen), ID(id), Payload(payload), BalanceFactor(0), LeftID(0), RightID(0) {}

	explicit AVLDiskNode(int id, unsigned short maxKeyLen, std::fstream& f) : MaxKeyLen(maxKeyLen), ID(id), BalanceFactor(0), LeftID(0), RightID(0)
	{
		auto buff = new char[MaxKeyLen + 1]{ 0 };
		f.read(buff, MaxKeyLen);

		uint32_t count;
		utils::read_binary(f, count);

		// Check to see if we even have a valid string (unused keys are just filled with null bytes)
		assert(buff[0] != 0x00);
		Payload = new Word(std::string(buff), count);

		delete[] buff;

		utils::read_binary(f, BalanceFactor);
		utils::read_binary(f, LeftID);
		utils::read_binary(f, RightID);
	}

	~AVLDiskNode()
	{
		delete Payload;
	}

	void write(std::fstream& f) const
	{
		// Write the key...
		auto buff = Payload->key;
		buff.resize(MaxKeyLen, 0x00);
		f.write(const_cast<char*>(buff.c_str()), MaxKeyLen);

		// ...And it's occurrance count
		utils::write_binary(f, Payload->count);

		utils::write_binary(f, BalanceFactor);
		utils::write_binary(f, LeftID);
		utils::write_binary(f, RightID);
	}
};

// An AVL Tree in which nodes are kept in clusters on disk
//
// This tree keeps its height balanced by keeping track
// of the "Balance Factors" of each node (the height difference between the left and right sub-trees)
//
// When a node's height is different by more than two nodes between its left and right sub-trees,
// rotations are performed to return the tree to an acceptably balanced state.
//
// This tree only supports inserts, searches, and an in-order debug treversal
//
// On-Disk Format:
// 
// 4 bytes: unsigned int containing the next node id
// 4 bytes: the ID of the root node, 0 if none
// variable: For each node in the tree
//     MaxKeyLen: node key
//     4 bytes:  unsigned integer containing the number of times the word appears
//     1 byte:   signed char containing the balance factor of the node
//     4 bytes:  unsigned integer containing the ID of the left child, 0 if none
//     4 bytes:  unsigned integer containing the ID of the right child, 0 if none
//
// This structure is not thread-safe for inserts / writes
class DiskAVL : public IWordCounter
{
public:
	explicit DiskAVL(std::string path, unsigned short maxKeyLen);
	~DiskAVL();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	void add(std::string key) override;

	// Find the given key in the tree
	// Returns:
	//     A unique pointer to a word represented by the specified key, or a nullptr if not found
	std::unique_ptr<Word> find(std::string key) override;

	// Check to see if the tree is empty (the root is null)
	bool isEmpty() const { return RootID == 0; }

	// Returns: The number of times the balance factor of any node was updated
	size_t getBalanceFactorChangeCount() const { return balanceFactorChanges;  }

	// Get statistics about the tree, such as its height and the number of total and distinct words
	// Returns:
	//     A unique pointer to a DocumentStatistics object
	std::unique_ptr<DocumentStatistics> getDocumentStatistics() override
	{
		return documentStatsFrom(RootID);
	}
	
	size_t GetFileSize() override
	{
		FileHandle.seekg(0, std::ios::end);
		size_t ret = FileHandle.tellg();
		FileHandle.seekg(0, std::ios::beg);

		return ret;
	}

	// Performs an in-order traversal on the tree, printing out the keys and their occurrance counts
	void inOrderPrint() override { inOrderPrintFrom(RootID); }
private:
	size_t balanceFactorChanges = 0;

	// The path on disk where the tree resides.
	std::string    TreePath = "";
	// The maximum length of a key
	unsigned short MaxKeyLen;
	// The file handle to the tree on disk
	std::fstream   FileHandle;
	// The ID of the next node to be allocated
	unsigned int   NextNodeID = 1;
	// The ID of the root node
	unsigned int   RootID     = 0;

	// Allocate a new node. Returns the next available ID and updates the tree metadata
	unsigned int AllocateNode()
	{
		return NextNodeID++;
	}

	// Attempt to load the specified node from disk. A runtime
	// exception is thrown if the node could not be read
	std::shared_ptr<AVLDiskNode> load(unsigned int id);

	// Write the specified node (and optinally the tree metadata) to disk
	void commit(std::shared_ptr<AVLDiskNode> node, bool includeBase = false);
	// Write the tree metadata to disk
	void commitBase(bool append=false);

	// Performs an in-order traversal on the sub-tree from the specified node,
	// printing out the keys and their occurrance counts
	void inOrderPrintFrom(unsigned int id)
	{
		if (id == 0) return;

		auto node = load(id);
		inOrderPrintFrom(node->LeftID);
		std::cout << node->Payload->key << ": " << node->Payload->count << " (node " << node->ID << ")" << std::endl;
		inOrderPrintFrom(node->RightID);
	}

	// Get statistics about the sub-tree from the specified node, such as its
	// height and the number of total and distinct words
	// Returns:
	//     A unique pointer to a DocumentStatistics object
	std::unique_ptr<DocumentStatistics> documentStatsFrom(unsigned int id)
	{
		if (id == 0) return std::make_unique<DocumentStatistics>(0, 0, 0);

		auto n = load(id);
		auto leftStats = documentStatsFrom(n->LeftID);
		auto rightStats = documentStatsFrom(n->RightID);

		return std::make_unique<DocumentStatistics>(
			1 + max(leftStats->TreeHeight, rightStats->TreeHeight),
			n->Payload->count + leftStats->TotalWords + rightStats->TotalWords,
			1 + leftStats->DistinctWords + rightStats->DistinctWords
		);
	}

	// Perform tree rotations at the specified rotation candidate according to its balance factor and the specified delta
	// This is required to keep the tree acceptably balanced.
	inline std::shared_ptr<AVLDiskNode> doRotations(std::shared_ptr<AVLDiskNode> lastRotationCandidate, std::shared_ptr<AVLDiskNode> nextAfterRotationCandidate, char delta);

	// Performs a rotation to handle the Left-Left case at the specified rotation candidate
	inline void rotateLeftLeft(std::shared_ptr<AVLDiskNode> lastRotationCandidate, std::shared_ptr<AVLDiskNode> nextAfterRotationCandidate);
	// Performs a rotation to handle the Left-Right case at the specified rotation candidate
	inline std::shared_ptr<AVLDiskNode> rotateLeftRight(std::shared_ptr<AVLDiskNode> lastRotationCandidate, std::shared_ptr<AVLDiskNode> nextAfterRotationCandidate);
	// Performs a rotation to handle the Right-Right case at the specified rotation candidate
	inline void rotateRightRight(std::shared_ptr<AVLDiskNode> lastRotationCandidate, std::shared_ptr<AVLDiskNode> nextAfterRotationCandidate);
	// Performs a rotation to handle the Right-Left case at the specified rotation candidate
	inline std::shared_ptr<AVLDiskNode> rotateRightLeft(std::shared_ptr<AVLDiskNode> lastRotationCandidate, std::shared_ptr<AVLDiskNode> nextAfterRotationCandidate);
};
