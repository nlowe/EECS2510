/*
 * BST.h - interface for a Binary Search Tree
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
#include "Word.h"
#include "IPerformanceStatsTracker.h"
#include <algorithm>

// A node in a Binary Tree
struct BinaryTreeNode
{
	// The payload the node contains
	Word* Payload;

	// The Left Child Node
	BinaryTreeNode* Left = nullptr;
	// The Right Child Node
	BinaryTreeNode* Right = nullptr;

	// Construct a binary tree node with the specified Word as a payload
	explicit BinaryTreeNode(Word* payload) : Payload(payload){}

	virtual ~BinaryTreeNode()
	{
		if (Payload != nullptr) delete Payload;
		if (Left != nullptr) delete Left;
		if (Right != nullptr) delete Right;
	}

	virtual size_t height() const { return 1 + std::max(Left == nullptr ? 0 : Left->height(), Right == nullptr ? 0 : Right->height()); }

	virtual size_t totalHeight() const { return 1 + (Left == nullptr ? 0 : Left->totalHeight()) + (Right == nullptr ? 0 : Right->totalHeight()); }

	virtual size_t payloadSum() const { return Payload->count + (Left == nullptr ? 0 : Left->payloadSum()) + (Right == nullptr ? 0 : Right->payloadSum()); }

	friend std::ostream& operator<<(std::ostream& os, const BinaryTreeNode& obj)
	{
		return os
			<< "Payload: " << obj.Payload
			<< " Left: " << obj.Left
			<< " Right: " << obj.Right;
	}

};

// A Tree that exhibits the Binary Search Tree Property :
//
// For any given node with a key of k:
//		* All items on the leftBranch of the node are "less" than k
//		* All items on the rightBranch of the node are "greater" than k
//
// Due to time constraints, the tree only accepts payloads of type Word and is not templated
class BST : public IPerformanceStatsTracker
{
public:
	BST();
	~BST();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	virtual Word* add(std::string key);

	// Finds the word in the tree with the specified tree. 
	// Returns:
	//		A pointer to the word represented by the specified key
	//		A null pointer if the key does not exist in the tree
	Word* get(std::string key);

	// Prints all words and their occurrance count in alphabetical order to std::cout
	void inOrderPrint() const { inOrderPrint(Root); }

	// Returns true iff the tree is empty
	bool isEmpty() const { return Root == nullptr; }

	size_t height() const { return isEmpty() ? 0 : Root->height(); }
	size_t totalWords() const { return isEmpty() ? 0 : Root->payloadSum(); }
	size_t totalNodes() const { return isEmpty() ? 0 : Root->totalHeight(); }
protected:
	// The node at the root of the tree
	BinaryTreeNode* Root = nullptr;

	// Finds a node in the tree with the specified key
	BinaryTreeNode* find(std::string key);

	// Recursively prints the subtree starting from the specified node in order
	void inOrderPrint(BinaryTreeNode* node) const;
};
