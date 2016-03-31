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
#include "BST.h"

// A node in an AVL Tree. Basically, a Binary Tree Node
// with an additional field for keeping track of the "balance factor"
struct AVLTreeNode : BinaryTreeNode
{
	explicit AVLTreeNode(Word* payload) : BinaryTreeNode(payload) {}

	// The balance factor of the node
	// This is the height of the left sub-tree minus the height of the right sub-tree
	char BalanceFactor = 0;
};

// An implementation of an AVL Tree. This tree keeps its height balanced by keeping track
// of the "Balance Factors" of each node (the height difference between the left and right sub-trees)
//
// When a node's height is different by more than two nodes between its left and right sub-trees,
// rotations are performed to return the tree to an acceptably balanced state.
class AVL : public BST
{
public:
	AVL();
	~AVL();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	Word* add(std::string key) override;

	// Returns: The number of times the balance factor of any node was updated
	size_t getBalanceFactorChangeCount() const { return balanceFactorChanges;  }

private:
	size_t balanceFactorChanges = 0;

	// Perform tree rotations at the specified rotation candidate according to its balance factor and the specified delta
	// This is required to keep the tree acceptably balanced.
	inline void doRotations(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate, char delta);

	// Performs a rotation to handle the Left-Left case at the specified rotation candidate
	inline void rotateLeftLeft(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate);
	// Performs a rotation to handle the Left-Right case at the specified rotation candidate
	inline void rotateLeftRight(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate);
	// Performs a rotation to handle the Right-Right case at the specified rotation candidate
	inline void rotateRightRight(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate);
	// Performs a rotation to handle the Right-Left case at the specified rotation candidate
	inline void rotateRightLeft(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate);
};

