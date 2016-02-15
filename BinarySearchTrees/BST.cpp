#include "stdafx.h"
#include "BST.h"
#include <iostream>


BST::BST()
{
}

BST::~BST()
{
	delete Root;
}

Word* BST::add(std::string word)
{
	if (Root == nullptr)
	{
		Root = new BinaryTreeNode(new Word(word, 1));
		return Root->Payload;
	}
	
	BinaryTreeNode* previous;
	BinaryTreeNode* candidate = Root;

	int branchComparisonResult;

	do
	{
		// Remember where we used to be
		previous = candidate;

		// Find which branch to take
		branchComparisonResult = word.compare(candidate->Payload->content);

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

	candidate = new BinaryTreeNode(new Word(word, 1));

	// Graft the new leaf node into the tree
	previous->Graft(candidate, branchComparisonResult);

	return candidate->Payload;
}

Word* BST::get(std::string key) const
{
	auto node = find(key);

	// Make sure the key is in the tree to start with
	if (node == nullptr) return nullptr;
	return node->Payload;
}

bool BST::remove(std::string key)
{
	auto node = find(key);

	// Make sure the key is in the tree to start with
	if (node == nullptr) return false;
	removeNode(node);

	return true;
}

Word* BST::minimum() const
{
	if (isEmpty()) return nullptr;

	return minimumOf(Root)->Payload;
}

Word* BST::maximum() const
{
	if (isEmpty()) return nullptr;

	return maximumOf(Root)->Payload;
}

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

void BST::inOrderPrint() const
{
	inOrderPrint(Root);
}

BinaryTreeNode* BST::find(std::string key) const
{
	if (Root == nullptr) return nullptr;

	auto candidate = Root;
	do
	{
		int branch = key.compare(candidate->Payload->content) < 0;

		if (branch < 0)
		{
			candidate = candidate->Left;
		}
		else if(branch == 0)
		{
			return candidate;
		}
		else
		{
			candidate = candidate->Right;
		}
	} while (candidate != nullptr);

	return nullptr;
}

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

BinaryTreeNode* BST::predecessorOf(BinaryTreeNode* node)
{
	if (node == nullptr) return nullptr;

	if (node->Left != nullptr)
	{
		// The predecessor is the maximum of the left sub-tree
		return maximumOf(node->Left);
	}
	else
	{
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
}

BinaryTreeNode* BST::successorOf(BinaryTreeNode* node)
{
	if (node == nullptr) return nullptr;

	if(node->Right != nullptr)
	{
		// The successor is the minimum of the right sub-tree
		return minimumOf(node->Right);
	}
	else
	{
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
}

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

void BST::inOrderPrint(BinaryTreeNode* node) const
{
	if (node == nullptr) return;

	inOrderPrint(node->Left);
	std::cout << node->Payload->content << " " << node->Payload->count << std::endl;
	inOrderPrint(node->Right);
}
