/*
 * BST.cpp - implementation of the BinarySearchTree
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
	if (Root == nullptr)
	{
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
	if(branchComparisonResult < 0)
	{
		previous->Left = toInsert;
	}
	else
	{
		previous->Right = toInsert;
	}

	// And fix the parent pointer
	toInsert->Parent = previous;

	return toInsert->Payload;
}

// Finds the word in the tree with the specified tree by performing a binary search
Word* BST::get(std::string key) const
{
	auto node = find(key);

	// Make sure the key is in the tree to start with
	if (node == nullptr) return nullptr;
	return node->Payload;
}

// Removes the word represented by the key from the tree
bool BST::remove(std::string key)
{
	auto node = find(key);

	// Make sure the key is in the tree to start with
	if (node == nullptr) return false;
	removeNode(node);

	return true;
}

// Returns a pointer to the word that comes first, alphabetically, or null if the tree is empty
// This is the leftmost node's payload from the root node
Word* BST::minimum() const
{
	if (isEmpty()) return nullptr;

	return minimumOf(Root)->Payload;
}

// Returns a pointer to the word that comes last, alphabetically, or null if the tree is empty
// This is the rightmost node's payload from the root node
Word* BST::maximum() const
{
	if (isEmpty()) return nullptr;

	return maximumOf(Root)->Payload;
}

// Returns a pointer to the word that comes alphabetically before the specified word, or null
// if the tree is empty or the key has no predecessor
Word* BST::predecessor(std::string key) const
{
	// First, find the node represented by the specified key
	auto node = find(key);

	// If the node isn't in the tree, just return a null pointer
	if (node == nullptr) return nullptr;
	
	// Next, find the predecessor of the target node
	auto predecessor = predecessorOf(node);

	// Return null if there is no predecessor...
	if (predecessor == nullptr) return nullptr;

	// Or return the word contained within the predecessor node
	return predecessor->Payload;
}

// Returns a pointer to the word that comes alphabetically after the specified word, or null
// if the tree is empty or the key has no successor
Word* BST::successor(std::string key) const
{
	// First, find the node represented by the specified key
	auto node = find(key);

	// If the node isn't in the tree, just return a null pointer
	if (node == nullptr) return nullptr;

	// Next, find the successor of the target node
	auto successor = successorOf(node);

	// Return null if there is no successor...
	if (successor == nullptr) return nullptr;

	// Or return the word contained within the successor node
	return successor->Payload;
}

// Starts a recursive operation to print the words and occurrance counts
// in order from the root ndoe
void BST::inOrderPrint() const
{
	inOrderPrint(Root);
}

// A helper function to find a node in the tree with the specified key
BinaryTreeNode* BST::find(std::string key) const
{
	// The tree is empty, so there is no node that is identified by the specified key
	if (Root == nullptr) return nullptr;

	auto candidate = Root;
	do
	{
		int branch = key.compare(candidate->Payload->key);

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

// A helper function to find the minimum node of the sub-tree identified by the specified node
// This is the leftmost node in the specified sub-tree
BinaryTreeNode* BST::minimumOf(BinaryTreeNode* node)
{
	if (node == nullptr) return nullptr;

	auto result = node;
	while(result->Left != nullptr)
	{
		result = result->Left;
	}

	return result;
}

// A helper function to find the maximum node of the sub-tree identified by the specified node
// This is the rightmost node in the specified sub-tree
BinaryTreeNode* BST::maximumOf(BinaryTreeNode* node)
{
	if (node == nullptr) return nullptr;

	auto result = node;
	while (result->Right != nullptr)
	{
		result = result->Right;
	}

	return result;
}

// A helper function to find the node whose key comes immediately before that of the target node
BinaryTreeNode* BST::predecessorOf(BinaryTreeNode* node)
{
	if (node == nullptr) return nullptr;

	if (node->Left != nullptr)
	{
		// The predecessor is the maximum of the left sub-tree
		return maximumOf(node->Left);
	}

	auto previous = node;
	auto result = node->Parent;

	// Go up while we're following left nodes backwards
	while (result != nullptr && previous == result->Left)
	{
		previous = result;
		result = result->Parent;
	}

	return result;
}

// A helper function to find the node whose key comes immediately after that of the target node
BinaryTreeNode* BST::successorOf(BinaryTreeNode* node)
{
	if (node == nullptr) return nullptr;

	if(node->Right != nullptr)
	{
		// The successor is the minimum of the right sub-tree
		return minimumOf(node->Right);
	}

	auto previous = node;
	auto result = node->Parent;

	// Go up while we're following right nodes backwards
	while(result != nullptr && previous == result->Right)
	{
		previous = result;
		result = result->Parent;
	}

	return result;
}

// A helper function to help transplant the children of a node onto that of another node
// This is used when removing nodes from the tree that are too complex to simply bypass
void BST::transplant(BinaryTreeNode* u, BinaryTreeNode* v)
{
	if(u->Parent == nullptr)
	{
		// If U is the root, then V becomes the new root
		Root = v;
	}
	else if (u == u->Parent->Left)
	{
		// If U is a left child of its parent, then V becomes the new left child of U's parent
		u->Parent->Left = v;
	}
	else
	{
		// Otherwise, V becomes the new right child of U's parent
		u->Parent->Right = v;
	}

	if (v != nullptr)
	{
		// V's parent must now be U's parent if it is not null
		v->Parent = u->Parent;
	}
}

// A helper function to remove the specified node from the tree
void BST::removeNode(BinaryTreeNode* node)
{
	if(node->Left == nullptr)
	{
		// If the node has no left child, replace it with its right child
		transplant(node, node->Right);
	}
	else if (node->Right == nullptr)
	{
		// If the node has a left child but no right child, replace it with its left child
		transplant(node, node->Left);
	}
	else
	{
		// Otherwise, find the successor of the target node
		auto successor = successorOf(node);
		if (successor->Parent != node)
		{
			// If the successor isn't the node's immediate right child
			// Replace it with its right branch
			transplant(successor, successor->Right);

			// And fix parent pointers
			successor->Right = node->Right;
			successor->Right->Parent = successor;
		}

		// Now, replace the target node with the successor
		transplant(node, successor);

		// And fix parent pointers
		successor->Left = node->Left;
		successor->Left->Parent = successor;
	}

	// Free the node we just removed
	node->Left = nullptr;
	node->Right = nullptr;
	delete node;
}

// A helper function to recursively print the payloads of the specified sub-tree in-order
void BST::inOrderPrint(BinaryTreeNode* node) const
{
	if (node == nullptr) return;

	inOrderPrint(node->Left);
	std::cout << node->Payload->key << " " << node->Payload->count << std::endl;
	inOrderPrint(node->Right);
}
