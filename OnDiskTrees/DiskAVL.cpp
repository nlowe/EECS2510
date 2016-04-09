/*
 * DiskAVL.cpp - Implementation of a disk-backed AVL tree
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

#include "DiskAVL.h"
#include "Utils.h"
#include <cassert>
#include <memory>


DiskAVL::DiskAVL(std::string path) : TreePath(path)
{
	std::ifstream reader;
	reader.open(path, std::ios::binary);

	readCount++;

	if(!reader.good())
	{
		// Ensure the directory the tree should be placed in exists
		utils::createDirectories(utils::parent(path));

		// Probably a new tree, try to commit the empty metadata
		commitBase();
	}
	else
	{
		// Existing tree
		reader >> NextNodeID;
		reader >> RootID;
	}

	reader.close();
}

DiskAVL::~DiskAVL()
{
}

// Insert the specified string into the tree. If the word is not already in
// the tree, the balance factors of nodes along the insertion path are updated
// and rotations may be performed to keep the tree balanced.
void DiskAVL::add(std::string word)
{
	// The tree is empty, just update the root pointer
	if (isEmpty())
	{
		this->referenceChanges++;
		auto r = std::make_shared<AVLDiskNode>(AllocateNode(), new Word(word));
		RootID = r->ID;

		commit(r);
	}

	// Otherwise, we need to find where to put it (P in the slides)
	std::shared_ptr<AVLDiskNode> previous(loadNode(RootID));
	// F in the slides
	std::shared_ptr<AVLDiskNode> lastRotationCandidateParent;
	// A in the slides
	std::shared_ptr<AVLDiskNode> lastRotationCandidate = previous;
	// B in the slides
	std::shared_ptr<AVLDiskNode> nextAfterRotationCandidate;
	// Q in the slides
	std::shared_ptr<AVLDiskNode> candidate;
	char delta;

	auto branchComparisonResult = 0;

	// search tree for insertion point
	while (previous != nullptr)
	{
		branchComparisonResult = word.compare(previous->Payload->key);
		this->comparisons++;

		if (branchComparisonResult == 0)
		{
			// The word we're inserting is already in the tree
			previous->Payload->count++;

			commit(previous);
			return;
		}

		// If this node's balance factor is already +/- 1 it may go to +/- 2 after the insertion
		// Remember where the last node like this is, since we may have to rotate around it later
		if (previous->BalanceFactor != 0)
		{
			lastRotationCandidate = previous;
			lastRotationCandidateParent = candidate;
		}

		// Remember where we used to be
		candidate = previous;

		auto p = loadNode(branchComparisonResult < 0 ? previous->LeftID : previous->RightID);
		if(p == nullptr)
		{
			previous.reset();
		}
		else
		{
			previous = p->shared_from_this();
		}
	}

	// We didn't find the node already, so we have to insert a new one
	auto toInsert = new AVLDiskNode(AllocateNode(), new Word(word));

	// Graft the new leaf node into the tree
	this->referenceChanges++;
	if (branchComparisonResult < 0)
	{
		candidate->LeftID = toInsert->ID;
	}
	else
	{
		candidate->RightID = toInsert->ID;
	}
	commit(candidate);

	// Figure out if we took the left or right branch after the last node with
	// a +/- 1 balance factor prior to the insert
	this->comparisons++;
	if (word.compare(lastRotationCandidate->Payload->key) < 0)
	{
		delta = 1;

		previous = loadNode(lastRotationCandidate->LeftID)->shared_from_this();
		nextAfterRotationCandidate = previous;
	}
	else
	{
		delta = -1;

		previous = loadNode(lastRotationCandidate->RightID)->shared_from_this();
		nextAfterRotationCandidate = previous;
	}

	// Update balance factors, moving pointers along the way
	while (previous->ID != toInsert->ID)
	{
		this->comparisons++;
		this->balanceFactorChanges++;
		if (word.compare(previous->Payload->key) > 0)
		{
			previous->BalanceFactor = -1;
			commit(previous);
			previous = loadNode(previous->RightID)->shared_from_this();
		}
		else
		{
			previous->BalanceFactor = +1;
			commit(previous);
			previous = loadNode(previous->LeftID)->shared_from_this();
		}
	}

	if (lastRotationCandidate->BalanceFactor == 0)
	{
		// Tree was perfectly balanced
		this->balanceFactorChanges++;
		lastRotationCandidate->BalanceFactor = delta;
		commit(lastRotationCandidate);
		return;
	}

	if (lastRotationCandidate->BalanceFactor == -delta)
	{
		// Tree was out of balance, but is now balanced
		this->balanceFactorChanges++;
		lastRotationCandidate->BalanceFactor = 0;
		commit(lastRotationCandidate);
		return;
	}

	// Otherwise, we have rotations to do
	doRotations(lastRotationCandidate, nextAfterRotationCandidate, delta);

	// did we rebalance the root?
	this->referenceChanges++;
	if (lastRotationCandidateParent == nullptr)
	{
		RootID = nextAfterRotationCandidate->ID;
		commitBase();
	}

	// otherwise, we rebalanced whatever was the
	// child (left or right) of F.
	else if (lastRotationCandidate->ID == lastRotationCandidateParent->LeftID)
	{
		lastRotationCandidateParent->LeftID = nextAfterRotationCandidate->ID;
		commit(lastRotationCandidateParent);
	}
	else if (lastRotationCandidate->ID == lastRotationCandidateParent->RightID)
	{
		lastRotationCandidateParent->RightID = nextAfterRotationCandidate->ID;
		commit(lastRotationCandidateParent);
	}
	else
	{
		assert(false);
	}
}

AVLDiskNode* DiskAVL::loadNode(unsigned int id)
{
	if (id == 0) return nullptr;

	std::ifstream reader;
	reader.open(TreePath, std::ios::binary);

	if (!reader.good())
	{
		reader.close();
		throw std::runtime_error("Unable to open tree for read: " + TreePath);
	}
	
	readCount++;

	// Skip nodes until we get to the node we're looking for
	for (unsigned short i = 0; i < id - 1; i++)
	{
		skipReadNode(reader);
	}

	auto node = new AVLDiskNode(id, reader);

	reader.close();

	return node;
}

void DiskAVL::commit(std::shared_ptr<AVLDiskNode>& node)
{
	commitBase();

	std::ifstream reader;
	std::ofstream writer;

	writer.open(TreePath, std::ios::binary);

	if (!writer.good())
	{
		writer.close();
		throw std::runtime_error("Unable to open cluster for read or create: " + TreePath);
	}

	writeCount++;
	readCount++;

	reader.open(TreePath, std::ios::binary);

	// Skip any nodes before this node and seek the writer
	size_t totalSeek = 0;
	for (unsigned short i = 0; i < node->ID - 1; i++)
	{
		totalSeek += skipReadNode(reader);
	}
	writer.seekp(totalSeek, std::ios::cur);

	node->write(writer);

	writer.close();
	reader.close();
}

void DiskAVL::commitBase()
{
	std::ofstream writer;
	writer.open(TreePath, std::ios::binary);

	if (!writer.good())
	{
		writer.close();
		throw std::runtime_error("Unable to open tree for write or create: " + TreePath);
	}

	writeCount++;

	writer.write(reinterpret_cast<const char*>(&NextNodeID), sizeof(NextNodeID));
	writer.write(reinterpret_cast<const char*>(&RootID), sizeof(RootID));

	writer.close();
}

void DiskAVL::doRotations(std::shared_ptr<AVLDiskNode>& lastRotationCandidate, std::shared_ptr<AVLDiskNode>& nextAfterRotationCandidate, char delta)
{
	if (delta == 1) // left imbalance.  LL or LR?
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

void DiskAVL::rotateLeftLeft(std::shared_ptr<AVLDiskNode>& A, std::shared_ptr<AVLDiskNode>& B)
{
	// Change the child pointers at A and B to
	// reflect the rotation. Adjust the BFs at A & B
	this->referenceChanges += 2;
	this->balanceFactorChanges += 2;
	A->LeftID  = B->RightID;
	B->RightID = A->ID;
	A->BalanceFactor = B->BalanceFactor = 0;
}

void DiskAVL::rotateLeftRight(std::shared_ptr<AVLDiskNode>& A, std::shared_ptr<AVLDiskNode>& B)
{
	// Adjust the child pointers of nodes A, B, & C
	// to reflect the new post-rotation structure
	auto C = loadNode(B->RightID)->shared_from_this(); // C is B's right child
	auto CL = C->LeftID;  // CL and CR are C's left
	auto CR = C->RightID; //    and right children

	this->referenceChanges += 4;
	B->RightID = CL;
	A->LeftID = CR;

	C->LeftID = B->ID;
	C->RightID = A->ID;
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

void DiskAVL::rotateRightRight(std::shared_ptr<AVLDiskNode>& A, std::shared_ptr<AVLDiskNode>& B)
{
	// Change the child pointers at A and B to
	// reflect the rotation. Adjust the BFs at A & B
	this->referenceChanges += 2;
	this->balanceFactorChanges += 2;
	A->RightID = B->LeftID;
	B->LeftID  = A->ID;
	A->BalanceFactor = B->BalanceFactor = 0;
}

void DiskAVL::rotateRightLeft(std::shared_ptr<AVLDiskNode>& A, std::shared_ptr<AVLDiskNode>& B)
{
	// Adjust the child pointers of nodes A, B, & C
	// to reflect the new post-rotation structure
	auto C  = loadNode(B->LeftID)->shared_from_this(); // C is B's left child
	auto CL = C->LeftID; // CL and CR are C's left
	auto CR = C->RightID;//    and right children

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
	A->RightID = CL;
	B->LeftID  = CR;

	C->RightID = B->ID;
	C->LeftID  = A->ID;

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
