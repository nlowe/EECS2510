/*
 * BST.cpp - Partial implementation of the BinarySearchTree
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
#include "BST.h"
#include <iostream>


BST::BST()
{
}

BST::~BST()
{
	// Free the root pointer. This will also free all child nodes
	delete Root;
}

// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
// This method will take care of maintaining the Binary Search Tree Property:
// For a given key k,
//		* All elements in the left subtree of a node with key k are "less" than k
//		* All elements in the right subtree of a node with key k are "greater" than k
Word* BST::add(std::string word)
{
	// The tree is empty, just update the root pointer
	if (isEmpty())
	{
		this->referenceChanges++;
		Root = new BinaryTreeNode(new Word(word));
		return Root->Payload;
	}
	
	// Otherwise, we need to find where to put it
	BinaryTreeNode* previous;
	BinaryTreeNode* candidate = Root;

	int branchComparisonResult;

	do
	{
		// Remember where we used to be
		previous = candidate;

		// Find which branch to take
		branchComparisonResult = word.compare(candidate->Payload->key);
		this->comparisons++;

		if (branchComparisonResult < 0)
		{
			// The word we're inserting is less than the candidate
			// Take the left branch
			candidate = candidate->Left;
		}
		else if (branchComparisonResult == 0)
		{
			// The word we're inserting is already in the tree
			candidate->Payload->count++;
			return candidate->Payload;
		}
		else
		{
			// The word we're inserting is greater than the candidate
			// Take the right branch
			candidate = candidate->Right;
		}
	} while (candidate != nullptr);

	auto toInsert = new BinaryTreeNode(new Word(word));

	// Graft the new leaf node into the tree
	this->referenceChanges++;
	if(branchComparisonResult < 0)
	{
		previous->Left = toInsert;
	}
	else
	{
		previous->Right = toInsert;
	}

	return toInsert->Payload;
}

// Finds the word in the tree with the specified tree by performing a binary search
Word* BST::get(std::string key)
{
	auto node = find(key);

	// Make sure the key is in the tree to start with
	if (node == nullptr) return nullptr;
	return node->Payload;
}

// A helper function to find a node in the tree with the specified key
BinaryTreeNode* BST::find(std::string key)
{
	// The tree is empty, so there is no node that is identified by the specified key
	if (Root == nullptr) return nullptr;

	auto candidate = Root;
	do
	{
		int branch = key.compare(candidate->Payload->key);
		this->comparisons++;

		if (branch < 0)
		{
			candidate = candidate->Left;
		}
		else if(branch == 0)
		{
			// We found the node!
			return candidate;
		}
		else
		{
			candidate = candidate->Right;
		}
	} while (candidate != nullptr);

	// We didn't find the node :(
	return nullptr;
}

// A helper function to recursively print the payloads of the specified sub-tree in-order
void BST::inOrderPrint(BinaryTreeNode* node) const
{
	if (node == nullptr) return;

	inOrderPrint(node->Left);
	std::cout << *node << std::endl;
	inOrderPrint(node->Right);
}
