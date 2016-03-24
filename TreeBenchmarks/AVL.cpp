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
	AVLTreeNode* lastRotateCandidate = Root;
	// B in the slides
	AVLTreeNode* nextAfterRotationCandidate;
	// Q in the slides
	AVLTreeNode* candidate = Root;
	char delta = 0;

	int branchComparisonResult;

	do
	{
		// Remember where we used to be
		previous = candidate;

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

	auto toInsert = new AVLTreeNode(new Word(word));

	// Graft the new leaf node into the tree, and then fix the balance factors
	this->referenceChanges++;
	if (branchComparisonResult < 0)
	{
		delta = 1;
		previous->Left = toInsert;
		
		previous = lastRotateCandidate;
		previous->BalanceFactor += delta;
		nextAfterRotationCandidate = previous;
	}
	else
	{
		delta = -1;
		previous->Right = toInsert;

		previous = lastRotateCandidate;
		previous->BalanceFactor += delta;
		nextAfterRotationCandidate = previous;
	}

	// Update balance factors, moving pointers along the way
	updateBalanceFactors(word, previous, toInsert);

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
			lastRotateCandidate = static_cast<AVLTreeNode*>(lastRotateCandidate->Left);
		}
	}
}

void AVL::doRotations(AVLTreeNode* F, AVLTreeNode* A, AVLTreeNode* B, char delta)
{
	if (delta == 1)
	{
		if (B->BalanceFactor == 1)
		{
			rotateLeftLeft(F, A, B);
		}
		else
		{
			// TODO: Left-Right rotation
			auto pivot = static_cast<AVLTreeNode*>(B->Right);
			auto pivotLeft = static_cast<AVLTreeNode*>(pivot->Left);
			auto pivotRight = static_cast<AVLTreeNode*>(pivot->Right);

			switch (pivot->BalanceFactor)
			{
				// ???
			}

			pivot->BalanceFactor = 0;
			B = pivot;
		}
	}
	else
	{
		if (B->BalanceFactor == -1)
		{
			rotateRightRight(F, A, B);
		}
		else
		{
			// TODO: Right-left rotation
			auto pivot = static_cast<AVLTreeNode*>(B->Left);
		}
	}

	// Check to see if the root was re-balanced
	// Otherwise, perform the final child pointer changes
	if (F == nullptr)
	{
		this->Root = B;
	}
	else if (A == F->Left)
	{
		F->Left = B;
	}
	else
	{
		F->Right = B;
	}
}

void AVL::rotateLeftLeft(AVLTreeNode* F, AVLTreeNode* A, AVLTreeNode* B)
{
	// Perform a Left-Left rotation
	// The rotation point's left child becomes the right sub-tree if it's left child
	A->Left = B->Right;
	// The right subtree of the rotation point is now rotated "up" and left
	B->Right = A;
	// Finally, update the node above the rotation point to point to the new sub-root
	if (F->Left == A) F->Left = B;
	else F->Right = B;
}

void AVL::rotateRightRight(AVLTreeNode* F, AVLTreeNode* A, AVLTreeNode* B)
{
	// Perform a Right-Right rotation
	// The rotation point's right child becomes the left sub-tree if it's right child
	A->Right = B->Left;
	// The left subtree of the rotation point is now rotated "up" and right
	B->Left = A;
	// Finally, update the node above the rotation point to point to the new sub-root
	if (F->Left == A) F->Left = B;
	else F->Right = B;
}

Word* AVL::get(std::string key)
{
	auto node = find(key);

	// Make sure the key is in the tree to start with
	if (node == nullptr) return nullptr;
	return node->Payload;
}

// A helper function to find a node in the tree with the specified key
AVLTreeNode* AVL::find(std::string key)
{
	// The tree is empty, so there is no node that is identified by the specified key
	if (isEmpty()) return nullptr;

	BinaryTreeNode* candidate = Root;
	do
	{
		auto branch = key.compare(candidate->Payload->key);
		this->comparisons++;

		if (branch < 0)
		{
			candidate = candidate->Left;
		}
		else if (branch == 0)
		{
			// We found the node!
			return static_cast<AVLTreeNode*>(candidate);
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
void AVL::inOrderPrint(AVLTreeNode* node) const
{
	if (node == nullptr) return;

	inOrderPrint(static_cast<AVLTreeNode*>(node->Left));
	std::cout << node->Payload->key << " " << node->Payload->count << std::endl;
	inOrderPrint(static_cast<AVLTreeNode*>(node->Right));
}
