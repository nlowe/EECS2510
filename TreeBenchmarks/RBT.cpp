/*
 * RBT.cpp - Implementation of a Red-Black Tree
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
#include "RBT.h"
#include <iostream>


RBT::RBT()
{
	this->recolorCount++;
	this->referenceChanges += 3;
	leafNodes = new RedBlackNode(new Word(""));
	leafNodes->Color = BLACK;
	leafNodes->Left = leafNodes->Right = leafNodes->Parent = leafNodes;
}


RBT::~RBT()
{
	// We have to delete the nodes ourselves first so the leaf supernode can be properly free'd
	if (Root != nullptr) delete Root;
	delete leafNodes;

	// We have to set the root to a null pointer so the base destructor doesn't try to double-free the nodes
	Root = nullptr;
	leafNodes = nullptr;
}

Word* RBT::add(std::string word)
{
	// The tree is empty, just update the root pointer
	if (isEmpty())
	{
		this->referenceChanges += 4;
		this->recolorCount++;
		Root = new RedBlackNode(new Word(word));
		(static_cast<RedBlackNode*>(Root))->Color = BLACK;
		(static_cast<RedBlackNode*>(Root))->Parent = leafNodes;
		Root->Left = Root->Right = leafNodes;
		return Root->Payload;
	}

	// Otherwise, we need to find where to put it
	RedBlackNode* previous = static_cast<RedBlackNode*>(Root);
	RedBlackNode* candidate = nullptr;

	int branchComparisonResult;

	// search tree for insertion point
	while (previous != leafNodes)
	{
		branchComparisonResult = word.compare(previous->Payload->key);
		this->comparisons++;

		if (branchComparisonResult == 0)
		{
			// The word we're inserting is already in the tree
			previous->Payload->count++;
			return previous->Payload;
		}

		// Remember where we used to be
		candidate = previous;
		previous = static_cast<RedBlackNode*>((branchComparisonResult < 0) ? previous->Left : previous->Right);
	}

	// We didn't find the node already, so we have to insert a new one
	auto toInsert = new RedBlackNode(new Word(word));
	this->referenceChanges += 4;
	toInsert->Parent = candidate;
	toInsert->Left = toInsert->Right = leafNodes;

	// Graft the new leaf node into the tree
	if (branchComparisonResult < 0)
	{
		candidate->Left = toInsert;
	}
	else
	{
		candidate->Right = toInsert;
	}

	// Recolor and rotate if needed to keep the tree balanced
	fixup(toInsert);

	return toInsert->Payload;
}

// Recolors and optionally rotates the nodes starting at the specified node
// to keep the tree balanced.
void RBT::fixup(RedBlackNode* z)
{
	while(z->Parent->Color == RED)
	{
		if(z->Parent == z->Parent->Parent->Left)
		{
			auto y = static_cast<RedBlackNode*>(z->Parent->Parent->Right);
			if (y->Color == RED)
			{
				// Case 1, re-color only
				this->recolorCount += 3;
				z->Parent->Color = BLACK;
				y->Color = BLACK;
				z->Parent->Parent->Color = RED;
				z = z->Parent->Parent;
			}
			else
			{
				if(z == z->Parent->Right)
				{
					// Case 2
					z = z->Parent;
					rotateLeft(z);
				}
				// Case 3
				this->recolorCount += 3;
				z->Parent->Color = BLACK;
				z->Parent->Parent->Color = RED;
				rotateRight(z->Parent->Parent);
			}
		}
		else
		{
			auto y = static_cast<RedBlackNode*>(z->Parent->Parent->Left);
			if (y->Color == RED)
			{
				// Case 1, re-color only
				this->recolorCount += 3;
				z->Parent->Color = BLACK;
				y->Color = BLACK;
				z->Parent->Parent->Color = RED;
				z = z->Parent->Parent;
			}
			else
			{
				if (z == z->Parent->Left)
				{
					// Case 2
					z = z->Parent;
					rotateRight(z);
				}
				// Case 3
				this->recolorCount += 2;
				z->Parent->Color = BLACK;
				z->Parent->Parent->Color = RED;
				rotateLeft(z->Parent->Parent);
			}
		}
	}

	// The root should always be black
	(static_cast<RedBlackNode*>(Root))->Color = BLACK;
}

// Rotate the sub-tree pointed at by node x to the left
void RBT::rotateLeft(RedBlackNode* x)
{
	auto y = static_cast<RedBlackNode*>(x->Right);
	x->Right = y->Left;
	this->referenceChanges++;

	if(y->Left != leafNodes)
	{
		(static_cast<RedBlackNode*>(y->Left))->Parent = x;
		this->referenceChanges++;
	}

	this->referenceChanges += 4;
	y->Parent = x->Parent;
	if (x->Parent == leafNodes)
	{
		Root = y;
	}
	else if (x == x->Parent->Left)
	{
		x->Parent->Left = y;
	}
	else
	{
		x->Parent->Right = y;
	}

	y->Left = x;
	x->Parent = y;
}

// Rotate the sub-tree pointed at by the node y to the right
void RBT::rotateRight(RedBlackNode* x)
{
	auto y = static_cast<RedBlackNode*>(x->Left);
	x->Left = y->Right;
	this->referenceChanges++;

	if (y->Right != leafNodes)
	{
		this->referenceChanges++;
		(static_cast<RedBlackNode*>(y->Right))->Parent = x;
	}
	
	this->referenceChanges += 4;
	y->Parent = x->Parent;
	if (x->Parent == leafNodes)
	{
		Root = y;
	}
	else if (x == x->Parent->Right)
	{
		x->Parent->Right = y;
	}
	else
	{
		x->Parent->Left = y;
	}

	y->Right = x;
	x->Parent = y;
}

void RBT::inOrderPrint(BinaryTreeNode* node) const
{
	// Don't try to print the leaf supernode
	if ((static_cast<RedBlackNode*>(node))->isMasterLeaf()) return;

	this->BST::inOrderPrint(node);
}
