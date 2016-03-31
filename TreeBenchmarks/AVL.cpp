#include "stdafx.h"
#include "AVL.h"
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
		this->comparisons++;

		if (branchComparisonResult == 0)
		{
			// The word we're inserting is already in the tree
			P->Payload->count++;
			return P->Payload;
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
	this->comparisons++;
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
		this->comparisons++;
		this->balanceFactorChanges++;
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
		this->balanceFactorChanges++;
		lastRotationCandidate->BalanceFactor = delta;
		return toInsert->Payload;
	}
	
	if (lastRotationCandidate->BalanceFactor == -delta)
	{
		// Tree was out of balance, but is now balanced
		this->balanceFactorChanges++;
		lastRotationCandidate->BalanceFactor = 0;
		return toInsert->Payload;
	}

	// Otherwise, we have rotations to do
	doRotations(lastRotationCandidate, nextAfterRotationCandidate, delta);

	// did we rebalance the root?
	this->referenceChanges++;
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

void AVL::doRotations(AVLTreeNode* A, AVLTreeNode*& B, char delta)
{
	if (delta == 1) // left imbalance.  LL or LR?
	{
		if (B->BalanceFactor == 1)
		{
			rotateLeftLeft(A, B);
		}
		else
		{
			rotateLeftRight(A, B);
		}
	}
	else // d=-1.  This is a right imbalance
	{
		if (B->BalanceFactor == -1)
		{
			rotateRightRight(A, B);
		}
		else
		{
			rotateRightLeft(A, B);
		}
	}
}

void AVL::rotateLeftLeft(AVLTreeNode* A, AVLTreeNode*& B)
{
	// Change the child pointers at A and B to
	// reflect the rotation. Adjust the BFs at A & B
	this->referenceChanges += 2;
	this->balanceFactorChanges += 2;
	A->Left  = B->Right;
	B->Right = A;
	A->BalanceFactor = B->BalanceFactor = 0;
}

void AVL::rotateLeftRight(AVLTreeNode* A, AVLTreeNode*& B)
{
	// Adjust the child pointers of nodes A, B, & C
	// to reflect the new post-rotation structure
	auto C  = static_cast<AVLTreeNode*>(B->Right); // C is B's right child
	auto CL = static_cast<AVLTreeNode*>(C->Left);  // CL and CR are C's left
	auto CR = static_cast<AVLTreeNode*>(C->Right); //    and right children

	this->referenceChanges += 4;
	B->Right = CL;
	A->Left = CR;

	C->Left = B;
	C->Right = A;
	/*
	   A              A                     C
	  /              /                   /    \
	 B       ->     C         ->        B      A
	  \            / \                   \    /
	   C          B   CR                 CL  CR
	  / \          \
	CL   CR         CL

	*/

	this->balanceFactorChanges += 3;
	switch (C->BalanceFactor)
	{
		// Set the new BF’s at A and B, based on the
		// BF at C. Note: There are 3 sub-cases
		case  1: A->BalanceFactor = -1; B->BalanceFactor = 0; break;
		case  0: A->BalanceFactor = B->BalanceFactor = 0; break;
		case -1: A->BalanceFactor = 0; B->BalanceFactor = 1; break;
		default: assert(false);
	}

	C->BalanceFactor = 0;
	B = C;
}

void AVL::rotateRightRight(AVLTreeNode* A, AVLTreeNode*& B)
{
	// Change the child pointers at A and B to
	// reflect the rotation. Adjust the BFs at A & B
	this->referenceChanges += 2;
	this->balanceFactorChanges += 2;
	A->Right = B->Left;
	B->Left  = A;
	A->BalanceFactor = B->BalanceFactor = 0;
}

void AVL::rotateRightLeft(AVLTreeNode* A, AVLTreeNode*& B)
{
	// Adjust the child pointers of nodes A, B, & C
	// to reflect the new post-rotation structure
	auto C  = static_cast<AVLTreeNode*>(B->Left); // C is B's left child
	auto CL = static_cast<AVLTreeNode*>(C->Left); // CL and CR are C's left
	auto CR = static_cast<AVLTreeNode*>(C->Right);//    and right children

	/*
			A              A                      C
			 \              \                   /   \
			  B       ->     C         ->      A     B
			 /              / \                 \   /
			C             CL   B                CL CR
		   / \                /
		 CL   CR             CR

	 */

	this->referenceChanges += 4;
	A->Right = CL;
	B->Left  = CR;

	C->Right = B;
	C->Left  = A;

	this->balanceFactorChanges += 3;
	switch (C->BalanceFactor)
	{
		// Set the new BF’s at A and B, based on the
		// BF at C. Note: There are 3 sub-cases
		case  1: A->BalanceFactor = 0; B->BalanceFactor = -1; break;
		case  0: A->BalanceFactor = B->BalanceFactor = 0; break;
		case -1: A->BalanceFactor = 1; B->BalanceFactor = 0; break;
		default: assert(false);
	}

	C->BalanceFactor = 0;
	B = C;
}
