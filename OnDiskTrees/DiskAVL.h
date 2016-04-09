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
#include <iostream>

#include "Word.h"
#include "Utils.h"
#include "IDiskStatisticsTracker.h"

// An AVL Tree node that is stored on disk
//
// Nodes are stored in clusters of multiple nodes, and have the following format:
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
	unsigned int LeftID;
	unsigned int RightID;

	explicit AVLDiskNode(int id, Word* payload) : ID(id), Payload(payload), BalanceFactor(0), LeftID(0), RightID(0) {}

	explicit AVLDiskNode(int id, std::ifstream& reader) : ID(id), BalanceFactor(0), LeftID(0), RightID(0)
	{
		unsigned short keylen;
		reader >> keylen;

		auto buff = new char[keylen];
		reader.read(buff, keylen);

		uint32_t count;
		reader >> count;

		Payload = new Word(std::string(buff), count);

		delete[] buff;

		reader >> BalanceFactor;
		reader >> LeftID;
		reader >> RightID;
	}

	~AVLDiskNode()
	{
		delete Payload;
	}

	void write(std::ofstream& writer) const
	{
		writer << static_cast<unsigned short>(Payload->key.length());

		auto buff = Payload->key.c_str();
		
		writer.write(buff, strlen(buff));
		writer << Payload->count << BalanceFactor << LeftID << RightID;
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
// Nodes are kept in clusters to keep the number of files down. The number of nodes
// per cluster can be changed when creating a new tree, but cannot be altered once
// the tree is created. Clusters are named by monotonically increasing id's, starting
// from 1. The directory format is as follows:
// 
// .../Path/To/Some/Tree.avl
//    ├── _base.treedef
//    ├── 0.ncl
//    ├── 1.ncl
//    └── 2.ncl
// 
// In this case, the tree's base directory is ".../Path/To/Some/Tree.avl". Tree metadata
// is stored in _base.treedef, which has the following format:
// 
// 2 bytes: unsigned short containing the number of nodes per cluster
// 4 bytes: unsigned int containing the next node id
// 4 bytes: the ID of the root node, 0 if none
//
// To determine what cluster a node with an id of n resides in and its offset in the
// clsuter, the following formula is used:
//
// cluster_id  = floor(n/nodes_per_cluster)
// node_offset = mod(n, nodes_per_cluster)
//
// Node clusters have the following file format:
//
// variable: For each node in the cluster
//     2 bytes:  unsigned short containing the length of the key
//     variable: node key
//     4 bytes:  unsigned integer containing the number of times the word appears
//     1 byte:   signed char containing the balance factor of the node
//     4 bytes:  unsigned integer containing the ID of the left child, 0 if none
//     4 bytes:  unsigned integer containing the ID of the right child, 0 if none
//
// This structure is not thread-safe for inserts / writes
class DiskAVL : public IDiskStatisticsTracker
{
public:
	DiskAVL(std::string base, unsigned short nodesPerCluster);
	~DiskAVL();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	Word* add(std::string key);

	// Check to see if the tree is empty (the root is null)
	bool isEmpty() const { return RootID == 0; }

	// Returns: The number of times the balance factor of any node was updated
	size_t getBalanceFactorChangeCount() const { return balanceFactorChanges;  }
private:
	size_t balanceFactorChanges = 0;

	std::string    TreeBase = "";
	unsigned short NodesPerCluster = 100;
	unsigned int   NextNodeID = 1;
	unsigned int   RootID     = 0;

	// Allocate a new node. Returns the next available ID and updates the tree metadata
	unsigned int AllocateNode()
	{
		auto id = NextNodeID++;
		commitBase();
		return id;
	}

	// Attempt to load the specified node from disk. The caller is responsible for
	// freeing the returned resource. A runtime exception is thrown if the node
	// could not be read
	AVLDiskNode* loadNode(unsigned int id);

	// Write the specified node to a cluster on disk
	void commitNode(AVLDiskNode* node);
	// Write the tree metadata to disk
	void commitBase();

	unsigned int getClusterID(AVLDiskNode* node) const { return getClusterID(node->ID); }
	unsigned int getClusterID(unsigned int id) const { return id / NodesPerCluster; }
	
	unsigned int getClusterOffset(AVLDiskNode* node) const { return getClusterOffset(node->ID); }
	unsigned int getClusterOffset(unsigned int id) const { return id % NodesPerCluster; }
	
	std::string getClusterPath(AVLDiskNode* node) const { return getClusterPath(getClusterID(node)); }
	std::string getClusterPath(unsigned int id) const { return utils::join(TreeBase, std::to_string(id) + ".ncl"); }

	// Skip over the next node in the specified stream
	static unsigned char skipReadNode(std::ifstream& reader)
	{
		unsigned short len;
		reader >> len;

		unsigned char offset = len + 13;

		// Skip the key and other data
		reader.seekg(offset, std::ios::cur);

		return offset;
	}

	// Perform tree rotations at the specified rotation candidate according to its balance factor and the specified delta
	// This is required to keep the tree acceptably balanced.
	inline void doRotations(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate, char delta);

	// Performs a rotation to handle the Left-Left case at the specified rotation candidate
	inline void rotateLeftLeft(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate);
	// Performs a rotation to handle the Left-Right case at the specified rotation candidate
	inline void rotateLeftRight(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate);
	// Performs a rotation to handle the Right-Right case at the specified rotation candidate
	inline void rotateRightRight(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate);
	// Performs a rotation to handle the Right-Left case at the specified rotation candidate
	inline void rotateRightLeft(AVLDiskNode* lastRotationCandidate, AVLDiskNode*& nextAfterRotationCandidate);
};
