/*
 * AVL.h - interface for an AVL Tree
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
#include "IPerformanceStatsTracker.h"
#include "../BinarySearchTrees/BST.h"

// A node in an AVL Tree. Basically, a Binary Tree Node
// with an additional field for keeping track of the "balance factor"
struct AVLTreeNode : BinaryTreeNode
{
	explicit AVLTreeNode(Word* payload) : BinaryTreeNode(payload) {}

	char BalanceFactor = 0;
};

class AVL : public IPerformanceStatsTracker
{
public:
	AVL();
	~AVL();
	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	Word* add(std::string key);

	// Finds the word in the tree with the specified tree. 
	// Returns:
	//		A pointer to the word represented by the specified key
	//		A null pointer if the key does not exist in the tree
	Word* get(std::string key);

	// Prints all words and their occurrance count in alphabetical order to std::cout
	void inOrderPrint() const { inOrderPrint(Root); }

	// Returns true iff the tree is empty
	bool isEmpty() const { return Root == nullptr; }
private:
	AVLTreeNode* Root = nullptr;

	static void updateBalanceFactors(std::string word, AVLTreeNode*& previous, AVLTreeNode* toInsert);
	void doRotations(AVLTreeNode* F, AVLTreeNode* A, AVLTreeNode* B, char delta);
	static void rotateLeftLeft(AVLTreeNode* F, AVLTreeNode* A, AVLTreeNode* B);
	static void rotateRightRight(AVLTreeNode* F, AVLTreeNode* A, AVLTreeNode* B);


	// Finds a node in the tree with the specified key
	AVLTreeNode* find(std::string key);

	// Recursively prints the subtree starting from the specified node in order
	void inOrderPrint(AVLTreeNode* node) const;
};

