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


DiskAVL::DiskAVL(std::string path, unsigned short maxKeyLen) : TreePath(path), MaxKeyLen(maxKeyLen)
{
	// Ensure the directory the tree should be placed in exists
	utils::createDirectories(utils::parent(path));

	FileHandle.open(path, std::ios::binary | std::ios::in);

	readCount++;

	if(!FileHandle.good())
	{
		// We have to do this dance to ensure the file gets created...
		FileHandle.close();
		FileHandle.open(path, std::ios::out);
		FileHandle.close();
		FileHandle.open(path, std::ios::binary | std::ios::in | std::ios::out);

		// Probably a new tree, try to commit the empty metadata
		commitBase();
	}
	else
	{
		// Existing tree
		utils::read_binary(FileHandle, NextNodeID);
		utils::read_binary(FileHandle, RootID);
	}
}

DiskAVL::~DiskAVL()
{
	if (FileHandle.is_open()) FileHandle.close();
}

// Insert the specified string into the tree. If the word is not already in
// the tree, the balance factors of nodes along the insertion path are updated
// and rotations may be performed to keep the tree balanced.
void DiskAVL::add(std::string word)
{
	if (word.length() > MaxKeyLen) throw std::runtime_error("Key to large. Try again with a larger max key size");

	// The tree is empty, just update the root pointer
	if (isEmpty())
	{
		this->referenceChanges++;
		auto r = std::make_shared<AVLDiskNode>(AllocateNode(), MaxKeyLen, new Word(word));
		RootID = r->ID;

		commit(r, true);

		return;
	}

	// Otherwise, we need to find where to put it (P in the slides)
	std::shared_ptr<AVLDiskNode> previous(load(RootID));
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
		previous = load(branchComparisonResult < 0 ? previous->LeftID : previous->RightID);
	}

	// We didn't find the node already, so we have to insert a new one
	auto toInsert = std::make_shared<AVLDiskNode>(AllocateNode(), MaxKeyLen, new Word(word));
	// Commit the new node to disk so it is available if needed
	commit(toInsert, true);

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

		previous = load(lastRotationCandidate->LeftID);
		nextAfterRotationCandidate = previous;
	}
	else
	{
		delta = -1;

		previous = load(lastRotationCandidate->RightID);
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
			previous = load(previous->RightID);
		}
		else
		{
			previous->BalanceFactor = +1;
			commit(previous);
			previous = load(previous->LeftID);
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
	nextAfterRotationCandidate = doRotations(lastRotationCandidate, nextAfterRotationCandidate, delta);

	// did we rebalance the root?
	this->referenceChanges++;
	if (lastRotationCandidateParent == nullptr)
	{
		RootID = nextAfterRotationCandidate->ID;
		commitBase(true);
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

// Find the given key in the tree
// Returns:
//     A unique pointer to a word represented by the specified key, or a nullptr if not found
std::unique_ptr<Word> DiskAVL::find(std::string key)
{
	if (isEmpty()) return nullptr;

	auto candidate = load(RootID);

	while(candidate != nullptr)
	{
		this->comparisons++;
		auto cmp = key.compare(candidate->Payload->key);

		if (cmp == 0)
		{
			return std::make_unique<Word>(candidate->Payload->key, candidate->Payload->count);
		}

		candidate = load(cmp < 0 ? candidate->LeftID : candidate->RightID);
	}

	return nullptr;
}

// Attempt to load the specified node from disk. A runtime
// exception is thrown if the node could not be read
std::shared_ptr<AVLDiskNode> DiskAVL::load(unsigned int id)
{
	if (id == 0) return nullptr;
	readCount++;

	// Skip the metadata
	FileHandle.seekg(sizeof NextNodeID + sizeof RootID + (id-1)*(MaxKeyLen + 13), std::ios::beg);

	// Read the node from disk
	auto node = std::make_shared<AVLDiskNode>(id, MaxKeyLen, FileHandle);

	return node;
}

// Write the specified node (and optinally the tree metadata) to disk
void DiskAVL::commit(std::shared_ptr<AVLDiskNode> node, bool includeBase)
{
	writeCount++;
	readCount++;
	
	if(includeBase)
	{
		FileHandle.seekp(0, std::ios::beg);
		utils::write_binary(FileHandle, NextNodeID);
		utils::write_binary(FileHandle, RootID);
	}
	else
	{
		// We're noot writing the base metadata, skip over it
		FileHandle.seekp(sizeof(NextNodeID) + sizeof(RootID), std::ios::beg);
	}

	// Skip any nodes before this node and seek the writer
	FileHandle.seekg((node->ID-1)*(MaxKeyLen + 13), std::ios::cur);

	node->write(FileHandle);
}

// Write the tree metadata to disk
void DiskAVL::commitBase(bool append)
{
	writeCount++;

	FileHandle.seekp(0, std::ios::beg);
	utils::write_binary(FileHandle, NextNodeID);
	utils::write_binary(FileHandle, RootID);
}

// Perform rotations about the specified nodes to keep the tree balanced
std::shared_ptr<AVLDiskNode> DiskAVL::doRotations(std::shared_ptr<AVLDiskNode> lastRotationCandidate, std::shared_ptr<AVLDiskNode> nextAfterRotationCandidate, char delta)
{
	if (delta == 1) // left imbalance.  LL or LR?
	{
		if (nextAfterRotationCandidate->BalanceFactor == 1)
		{
			rotateLeftLeft(lastRotationCandidate, nextAfterRotationCandidate);
		}
		else
		{
			nextAfterRotationCandidate = rotateLeftRight(lastRotationCandidate, nextAfterRotationCandidate);
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
			nextAfterRotationCandidate = rotateRightLeft(lastRotationCandidate, nextAfterRotationCandidate);
		}
	}

	return nextAfterRotationCandidate;
}

void DiskAVL::rotateLeftLeft(std::shared_ptr<AVLDiskNode> A, std::shared_ptr<AVLDiskNode> B)
{
	// Change the child pointers at A and B to
	// reflect the rotation. Adjust the BFs at A & B
	this->referenceChanges += 2;
	this->balanceFactorChanges += 2;
	A->LeftID  = B->RightID;
	B->RightID = A->ID;
	A->BalanceFactor = B->BalanceFactor = 0;

	commit(A);
	commit(B);
}

std::shared_ptr<AVLDiskNode> DiskAVL::rotateLeftRight(std::shared_ptr<AVLDiskNode> A, std::shared_ptr<AVLDiskNode> B)
{
	// Adjust the child pointers of nodes A, B, & C
	// to reflect the new post-rotation structure
	auto C = load(B->RightID); // C is B's right child
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

	commit(A);
	commit(B);
	commit(C);
	return C;
}

void DiskAVL::rotateRightRight(std::shared_ptr<AVLDiskNode> A, std::shared_ptr<AVLDiskNode> B)
{
	// Change the child pointers at A and B to
	// reflect the rotation. Adjust the BFs at A & B
	this->referenceChanges += 2;
	this->balanceFactorChanges += 2;
	A->RightID = B->LeftID;
	B->LeftID  = A->ID;
	A->BalanceFactor = B->BalanceFactor = 0;

	commit(A);
	commit(B);
}

std::shared_ptr<AVLDiskNode> DiskAVL::rotateRightLeft(std::shared_ptr<AVLDiskNode> A, std::shared_ptr<AVLDiskNode> B)
{
	// Adjust the child pointers of nodes A, B, & C
	// to reflect the new post-rotation structure
	auto C  = load(B->LeftID); // C is B's left child
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

	commit(A);
	commit(B);
	commit(C);
	return C;
}
