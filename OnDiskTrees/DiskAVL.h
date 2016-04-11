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
#include "Verbose.h"
#include "IWordCounter.h"

// An AVL Tree node that is stored on disk
//
// 2 bytes:  unsigned short containing the length of the key
// variable: node key
// 4 bytes:  unsigned integer containing the number of times the word appears
// 1 byte:   signed char containing the balance factor of the node
// 4 bytes:  unsigned integer containing the ID of the left child, 0 if none
// 4 bytes:  unsigned integer containing the ID of the right child, 0 if none
struct AVLDiskNode
{
	unsigned int ID;
	Word* Payload;
	char BalanceFactor;
	uint32_t LeftID;
	uint32_t RightID;

	explicit AVLDiskNode(int id, Word* payload) : ID(id), Payload(payload), BalanceFactor(0), LeftID(0), RightID(0) {}

	explicit AVLDiskNode(int id, std::fstream& f) : ID(id), BalanceFactor(0), LeftID(0), RightID(0)
	{
		unsigned short keylen;
		utils::read_binary(f, keylen);

		std::string buff;
		buff.resize(keylen);
		f.read(const_cast<char*>(buff.c_str()), keylen);

		uint32_t count;
		utils::read_binary(f, count);

		Payload = new Word(std::string(buff), count);

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
		auto buff = Payload->key.c_str();
		unsigned short len = strlen(buff);

		utils::write_binary(f, len);
		f.write(buff, strlen(buff));
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
//     2 bytes:  unsigned short containing the length of the key
//     variable: node key
//     4 bytes:  unsigned integer containing the number of times the word appears
//     1 byte:   signed char containing the balance factor of the node
//     4 bytes:  unsigned integer containing the ID of the left child, 0 if none
//     4 bytes:  unsigned integer containing the ID of the right child, 0 if none
//
// This structure is not thread-safe for inserts / writes
class DiskAVL : public IWordCounter
{
public:
	explicit DiskAVL(std::string path);
	~DiskAVL();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	void add(std::string key) override;
	std::unique_ptr<Word> find(std::string key) override;

	// Check to see if the tree is empty (the root is null)
	bool isEmpty() const { return RootID == 0; }

	// Returns: The number of times the balance factor of any node was updated
	size_t getBalanceFactorChangeCount() const { return balanceFactorChanges;  }

	std::unique_ptr<DocumentStatistics> getDocumentStatistics() override
	{
		return documentStatsFrom(RootID);
	}

	void inOrderPrint() override { inOrderPrintFrom(RootID); }
private:
	size_t balanceFactorChanges = 0;

	std::string    TreePath = "";
	unsigned int   NextNodeID = 1;
	unsigned int   RootID     = 0;

	// Allocate a new node. Returns the next available ID and updates the tree metadata
	unsigned int AllocateNode()
	{
		return NextNodeID++;
	}

	// Attempt to load the specified node from disk. A runtime
	// exception is thrown if the node could not be read
	std::shared_ptr<AVLDiskNode> loadNode(unsigned int id);

	// Write the specified node to a cluster on disk
	void commit(std::shared_ptr<AVLDiskNode> node, bool includeBase = false)
	{
		std::shared_ptr<AVLDiskNode> arr[1] = { node };

		commitMany(1, arr, includeBase);
	}
	
	// Write multiple nodes to a cluster on disk
	void commitMany(size_t count, std::shared_ptr<AVLDiskNode> nodes[], bool includeBase = false);

	// Write the tree metadata to disk
	void commitBase(bool append=false);

	// Skip over the next node in the specified stream
	static void skipReadNode(std::fstream& f)
	{
		unsigned short len;
		utils::read_binary(f, len);

		auto offset = len + 13;

		verbose::write("Asked to skip one node, skipping 2 + " + std::to_string(offset) + " bytes");

		// Skip the key and other data
		f.seekg(offset, std::ios::cur);
	}

	void inOrderPrintFrom(unsigned int id)
	{
		if (id == 0) return;

		auto node = loadNode(id);
		inOrderPrintFrom(node->LeftID);
		std::cout << node->Payload->key << ": " << node->Payload->count << " (node " << node->ID << ")" << std::endl;
		inOrderPrintFrom(node->RightID);
	}

	std::unique_ptr<DocumentStatistics> documentStatsFrom(unsigned int id)
	{
		if (id == 0) return std::make_unique<DocumentStatistics>(0, 0, 0);

		auto n = loadNode(id);
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
