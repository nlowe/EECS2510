#include "stdafx.h"
#include "AVL.h"
#include <iostream>

AVL::AVL()
{
}

AVL::~AVL()
{
}


Word* AVL::add(std::string word)
{
	// The tree is empty, just update the root pointer
	if (isEmpty())
	{
		this->referenceChanges++;
		Root = new AVLTreeNode(new Word(word));
		return Root->Payload;
	}

	// Otherwise, we need to find where to put it (P in the slides)
	AVLTreeNode* previous;
	// F in the slides
	AVLTreeNode* lastRotateCandidateParent = nullptr; 
	// A in the slides
	AVLTreeNode* lastRotateCandidate = static_cast<AVLTreeNode*>(Root);
	// B in the slides
	AVLTreeNode* nextAfterRotationCandidate;
	// Q in the slides
	AVLTreeNode* candidate = static_cast<AVLTreeNode*>(Root);
	char delta = 0;

	int branchComparisonResult;

	do
	{
		// Remember where we used to be
		previous = candidate;

		// If this node's balance factor is already +/- 1 it may go to +/- 2 after the insertion
		// Remember where the last node like this is, since we may have to rotate around it later
		if(candidate->BalanceFactor != 0)
		{
			lastRotateCandidateParent = lastRotateCandidate;
			lastRotateCandidate = candidate;
		}

		// Find which branch to take
		branchComparisonResult = word.compare(candidate->Payload->key);
		this->comparisons++;

		if (branchComparisonResult < 0)
		{
			// The word we're inserting is less than the candidate
			// Take the left branch
			candidate = static_cast<AVLTreeNode*>(candidate->Left);
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
			candidate = static_cast<AVLTreeNode*>(candidate->Right);
		}
	} while (candidate != nullptr);

	// We didn't find the node already, so we have to insert a new one
	auto toInsert = new AVLTreeNode(new Word(word));

	// Graft the new leaf node into the tree
	this->referenceChanges++;
	if (branchComparisonResult < 0)
	{
		previous->Left = toInsert;
	}
	else
	{
		previous->Right = toInsert;
	}

	// Figure out if we took the left or right branch after the last node with
	// a +/- 1 balance factor prior to the insert
	if(word.compare(lastRotateCandidate->Payload->key) < 0)
	{
		delta = 1;

		candidate = static_cast<AVLTreeNode*>(lastRotateCandidate->Left);
		nextAfterRotationCandidate = candidate;
	}
	else
	{
		delta = -1;

		candidate = static_cast<AVLTreeNode*>(lastRotateCandidate->Right);
		nextAfterRotationCandidate = candidate;
	}

	// Update balance factors, moving pointers along the way
	updateBalanceFactors(word, candidate, toInsert);

	if(lastRotateCandidate->BalanceFactor == 0)
	{
		// Tree was perfectly balanced
		lastRotateCandidate->BalanceFactor = delta;
	}
	else if(lastRotateCandidate->BalanceFactor == -delta)
	{
		// Tree was out of balance, but is now balanced enough
		lastRotateCandidate->BalanceFactor = 0;
	}
	else
	{
		// Otherwise, we have rotations to do
		doRotations(lastRotateCandidateParent, lastRotateCandidate, nextAfterRotationCandidate, delta);
	}

	return toInsert->Payload;
}

void AVL::updateBalanceFactors(std::string word, AVLTreeNode*& lastRotateCandidate, AVLTreeNode* toInsert)
{
	while (lastRotateCandidate != toInsert)
	{
		int branch = word.compare(lastRotateCandidate->Payload->key);

		if (branch < 0)
		{
			lastRotateCandidate->BalanceFactor += 1;
			lastRotateCandidate = static_cast<AVLTreeNode*>(lastRotateCandidate->Left);
		}
		else
		{
			lastRotateCandidate->BalanceFactor -= 1;
			lastRotateCandidate = static_cast<AVLTreeNode*>(lastRotateCandidate->Right);
		}
	}
}

void AVL::doRotations(AVLTreeNode* F, AVLTreeNode* A, AVLTreeNode* B, char delta)
{
	AVLTreeNode* newSubRoot;

	if(A->BalanceFactor > 0)
	{
		// Somewhere in the left sub-tree
		if(B->BalanceFactor == -1)
		{
			// Left Right case, perform a left rotation first to make it a LL case
			A->Left = rotateLeft(B);
			B->BalanceFactor = 0;
		}

		// Now, rotate right
		newSubRoot = rotateRight(A);
		A->BalanceFactor -= 1;
	}
	else
	{
		if(B->BalanceFactor == 1)
		{
			// Right Left case, perform a right rotation first to make it a RR case
			A->Right = rotateRight(B);
			B->BalanceFactor = 0;
		}

		// Now, rotate left
		newSubRoot = rotateLeft(A);
		A->BalanceFactor += 1;
	}

	if(F != nullptr && F != A)
	{
		// We didn't rotate around the root
		// So update A's parent to point to the new sub-root
		if (F->Left == A) F->Left = newSubRoot;
		else F->Right = newSubRoot;
	}
	else
	{
		// We re-balanced around the root
		// The new root is now the new sub root
		this->Root = newSubRoot;
	}
	
}

AVLTreeNode* AVL::rotateLeft(AVLTreeNode* n)
{
	auto c = static_cast<AVLTreeNode*>(n->Right);

	n->Right = c->Left;
	c->Left = n;
	c->BalanceFactor += 1;

	return c;
}

AVLTreeNode* AVL::rotateRight(AVLTreeNode* n)
{
	auto c = static_cast<AVLTreeNode*>(n->Left);

	n->Left = c->Right;
	c->Right = n;
	c->BalanceFactor -= 1;

	return c;
}
