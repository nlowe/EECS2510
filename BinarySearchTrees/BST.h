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
#include <string>

// A node in a Binary Tree
struct BinaryTreeNode
{
	// The payload the node contains
	Word* Payload;

	// The immediate parent node
	BinaryTreeNode* Parent = nullptr;
	// The Left Child Node
	BinaryTreeNode* Left = nullptr;
	// The Right Child Node
	BinaryTreeNode* Right = nullptr;

	// Construct a binary tree node with the specified Word as a payload
	explicit BinaryTreeNode(Word* payload) : Payload(payload){}

	~BinaryTreeNode()
	{
		delete Payload;
		if (Left != nullptr) delete Left;
		if (Right != nullptr) delete Right;
	}
};

// A Tree that exhibits the Binary Search Tree Property :
//
// For any given node with a key of k:
//		* All items on the leftBranch of the node are "less" than k
//		* All items on the rightBranch of the node are "greater" than k
//
// Due to time constraints, the tree only accepts payloads of type Word and is not templated
class BST
{
public:
	BST();
	~BST();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	Word* add(std::string key);

	// Finds the word in the tree with the specified tree. 
	// Returns:
	//		A pointer to the word represented by the specified key
	//		A null pointer if the key does not exist in the tree
	Word* get(std::string key) const;

	// Removes the word represented by the key from the tree
	// Returns:
	//		true iff the tree contained an element with the specified tree
	bool remove(std::string key);

	// Returns a pointer to the word that comes first, alphabetically, or null if the tree is empty
	Word* minimum() const;

	// Returns a pointer to the word that comes last, alphabetically, or null if the tree is empty
	Word* maximum() const;

	// Returns a pointer to the word that comes alphabetically before the specified word, or null
	// if the tree is empty or the key has no predecessor
	Word* predecessor(std::string key) const;

	// Returns a pointer to the word that comes alphabetically after the specified word, or null
	// if the tree is empty or the key has no successor
	Word* successor(std::string key) const;

	// Prints all words and their occurrance count in alphabetical order to std::cout
	void inOrderPrint() const;

	// Returns true iff the tree is empty
	bool isEmpty() const
	{
		return Root == nullptr;
	}
private:
	// The node at the root of the tree
	BinaryTreeNode* Root = nullptr;

	// Finds a node in the tree with the specified key
	BinaryTreeNode* find(std::string key) const;

	// Finds the smallest node in the subtree from the specified node
	static BinaryTreeNode* minimumOf(BinaryTreeNode* node);
	// Finds the largest node in the subtree from the specified node
	static BinaryTreeNode* maximumOf(BinaryTreeNode* node);

	// Finds the predecessor of the specified node
	static BinaryTreeNode* predecessorOf(BinaryTreeNode* node);
	// Finds the successor of the specified node
	static BinaryTreeNode* successorOf(BinaryTreeNode* node);

	// A helper function to take care of the edge cases when removing nodes from the tree
	void transplant(BinaryTreeNode* u, BinaryTreeNode* v);
	// Removes the specified node from the tree
	void removeNode(BinaryTreeNode* node);

	// Recursively prints the subtree starting from the specified node in order
	void inOrderPrint(BinaryTreeNode* node) const;
};

