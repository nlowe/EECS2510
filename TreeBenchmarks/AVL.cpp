#include "stdafx.h"
#include "AVL.h"
#include <iostream>
#include <cassert>

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
	AVLTreeNode* P = static_cast<AVLTreeNode*>(Root);
	// F in the slides
	AVLTreeNode* lastRotationCandidateParent = nullptr;
	// A in the slides
	AVLTreeNode* lastRotationCandidate = static_cast<AVLTreeNode*>(Root);
	// B in the slides
	AVLTreeNode* nextAfterRotationCandidate;
	// Q in the slides
	AVLTreeNode* candidate = nullptr;
	char delta = 0;

	int branchComparisonResult;

	// search tree for insertion point
	while (P != nullptr)
	{
		branchComparisonResult = word.compare(P->Payload->key);

		if (branchComparisonResult == 0)
		{
			// The word we're inserting is already in the tree
			candidate->Payload->count++;
			return candidate->Payload;
		}

		// If this node's balance factor is already +/- 1 it may go to +/- 2 after the insertion
		// Remember where the last node like this is, since we may have to rotate around it later
		if (P->BalanceFactor != 0)
		{
			lastRotationCandidate = P;
			lastRotationCandidateParent = candidate;
		}

		// Remember where we used to be
		candidate = P;
		P = static_cast<AVLTreeNode*>((branchComparisonResult < 0) ? P->Left : P->Right);
	}

	// We didn't find the node already, so we have to insert a new one
	auto toInsert = new AVLTreeNode(new Word(word));

	// Graft the new leaf node into the tree
	this->referenceChanges++;
	if (branchComparisonResult < 0)
	{
		candidate->Left = toInsert;
	}
	else
	{
		candidate->Right = toInsert;
	}

	// Figure out if we took the left or right branch after the last node with
	// a +/- 1 balance factor prior to the insert
	if (word.compare(lastRotationCandidate->Payload->key) < 0)
	{
		delta = 1;

		P = static_cast<AVLTreeNode*>(lastRotationCandidate->Left);
		nextAfterRotationCandidate = P;
	}
	else
	{
		delta = -1;

		P = static_cast<AVLTreeNode*>(lastRotationCandidate->Right);
		nextAfterRotationCandidate = P;
	}

	// Update balance factors, moving pointers along the way
	while (P != toInsert)
	{
		if (word.compare(P->Payload->key) > 0)
		{
			P->BalanceFactor = -1;
			P = static_cast<AVLTreeNode*>(P->Right);
		}
		else
		{
			P->BalanceFactor = +1;
			P = static_cast<AVLTreeNode*>(P->Left);
		}
	}

	if (lastRotationCandidate->BalanceFactor == 0)
	{
		// Tree was perfectly balanced
		lastRotationCandidate->BalanceFactor = delta;
		return toInsert->Payload;
	}
	
	if (lastRotationCandidate->BalanceFactor == -delta)
	{
		// Tree was out of balance, but is now balanced
		lastRotationCandidate->BalanceFactor = 0;
		return toInsert->Payload;
	}

	// Otherwise, we have rotations to do
	doRotations(lastRotationCandidate, nextAfterRotationCandidate, delta);

	// did we rebalance the root?
	if (lastRotationCandidateParent == nullptr)
	{
		Root = nextAfterRotationCandidate;
	}

	// otherwise, we rebalanced whatever was the
	// child (left or right) of F.
	else if (lastRotationCandidate == lastRotationCandidateParent->Left)
	{
		lastRotationCandidateParent->Left = nextAfterRotationCandidate;
	}
	else if (lastRotationCandidate == lastRotationCandidateParent->Right)
	{
		lastRotationCandidateParent->Right = nextAfterRotationCandidate;
	}
	else
	{
		assert(false);
	}

	return toInsert->Payload;
}

void AVL::doRotations(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate, char delta)
{
	if (delta == +1) // left imbalance.  LL or LR?
	{
		if (nextAfterRotationCandidate->BalanceFactor == 1)
		{
			rotateLeftLeft(lastRotationCandidate, nextAfterRotationCandidate);
		}
		else
		{
			rotateLeftRight(lastRotationCandidate, nextAfterRotationCandidate);
		}
	}
	else // d=-1.  This is a right imbalance
	{
		if (nextAfterRotationCandidate->BalanceFactor == -1)
		{
			rotateRightRight(lastRotationCandidate, nextAfterRotationCandidate);
		}
		else
		{
			rotateRightLeft(lastRotationCandidate, nextAfterRotationCandidate);
		}
	}
}

void AVL::rotateLeftLeft(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate)
{
	// Change the child pointers at A and B to
	// reflect the rotation. Adjust the BFs at A & B
	// <<< LEFT FOR YOU TO WRITE (3-4 LOC) >>>
	// See Schematic (1)
	lastRotationCandidate->Left = nextAfterRotationCandidate->Right;
	nextAfterRotationCandidate->Right = lastRotationCandidate;
	lastRotationCandidate->BalanceFactor = nextAfterRotationCandidate->BalanceFactor = 0;
}

void AVL::rotateLeftRight(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate)
{
	// Adjust the child pointers of nodes A, B, & C
	// to reflect the new post-rotation structure
	// <<< LEFT FOR YOU TO WRITE, BUT HERE’S >>>
	// <<< A HEAD START (4 LOC here)         >>>
	auto C = static_cast<AVLTreeNode*>(nextAfterRotationCandidate->Right); // C is B's right child
	auto CL = static_cast<AVLTreeNode*>(C->Left);  // CL and CR are C's left
	auto CR = static_cast<AVLTreeNode*>(C->Right); //    and right children
	// See Schematic (2) and (3)

	lastRotationCandidate->Left = CR;
	nextAfterRotationCandidate->Right = CL;

	C->Left = nextAfterRotationCandidate;
	C->Right = lastRotationCandidate;

	switch (C->BalanceFactor)
	{
		// Set the new BF’s at A and B, based on the
		// BF at C. Note: There are 3 sub-cases
		// <<< LEFT FOR YOU TO WRITE (3 LOC/CASE) >>>
		case  1: lastRotationCandidate->BalanceFactor = -1; nextAfterRotationCandidate->BalanceFactor = 0; break;
		case  0: lastRotationCandidate->BalanceFactor = nextAfterRotationCandidate->BalanceFactor; break;
		case -1: lastRotationCandidate->BalanceFactor = 0; nextAfterRotationCandidate->BalanceFactor = 1; break;
		default: assert(false);
	}

	C->BalanceFactor = 0;
	nextAfterRotationCandidate = C;
}

void AVL::rotateRightRight(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate)
{
	// Change the child pointers at A and B to
	// reflect the rotation. Adjust the BFs at A & B
	// <<< LEFT FOR YOU TO WRITE (3-4 LOC) >>>
	// See Schematic (1)
	lastRotationCandidate->Right = nextAfterRotationCandidate->Left;
	nextAfterRotationCandidate->Left = lastRotationCandidate;
	lastRotationCandidate->BalanceFactor = nextAfterRotationCandidate->BalanceFactor = 0;
}

void AVL::rotateRightLeft(AVLTreeNode* lastRotationCandidate, AVLTreeNode*& nextAfterRotationCandidate)
{
	// Adjust the child pointers of nodes A, B, & C
	// to reflect the new post-rotation structure
	// <<< LEFT FOR YOU TO WRITE, BUT HERE’S >>>
	// <<< A HEAD START (4 LOC here)         >>>
	auto C = static_cast<AVLTreeNode*>(nextAfterRotationCandidate->Left); // C is B's right child
	auto CL = static_cast<AVLTreeNode*>(C->Left); // CL and CR are C's left
	auto CR = static_cast<AVLTreeNode*>(C->Right); //    and right children
	// See Schematic (2) and (3)

	lastRotationCandidate->Right = CL;
	nextAfterRotationCandidate->Left = CR;

	C->Right = nextAfterRotationCandidate;
	C->Left = lastRotationCandidate;

	switch (C->BalanceFactor)
	{
		// Set the new BF’s at A and B, based on the
		// BF at C. Note: There are 3 sub-cases
		// <<< LEFT FOR YOU TO WRITE (3 LOC/CASE) >>>
	case  1: lastRotationCandidate->BalanceFactor = 1; nextAfterRotationCandidate->BalanceFactor = 0; break;
	case  0: lastRotationCandidate->BalanceFactor = nextAfterRotationCandidate->BalanceFactor; break;
	case -1: lastRotationCandidate->BalanceFactor = 0; nextAfterRotationCandidate->BalanceFactor = -1; break;
	default: assert(false);
	}

	C->BalanceFactor = 0;
	nextAfterRotationCandidate = C;
}
