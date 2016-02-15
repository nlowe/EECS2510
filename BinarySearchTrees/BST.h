#pragma once
#include "../CustomDataTypes/Word.h"
#include <functional>

// A structure for a node in a binary tree
struct BinaryTreeNode
{
	// The payload the node contains
	Word* Payload;
	// The immediate parent node
	BinaryTreeNode* Parent = nullptr;
	// The Left Child Node
	BinaryTreeNode* Left = nullptr;
	// The Right Child Node
	BinaryTreeNode* Right = nullptr;

	explicit BinaryTreeNode(Word* payload) : Payload(payload)
	{
	}

	~BinaryTreeNode()
	{
		delete Payload;
		if (Parent != nullptr) delete Parent;
		if (Left != nullptr) delete Left;
		if (Right != nullptr) delete Right;
	}

	// Join the specified node into this nodes left or right branch
	void Graft(BinaryTreeNode* node, int branch)
	{
		if (branch < 0)
		{
			Left = node;
		}
		else
		{
			Right = node;
		}
	}
};

// A binary search tree containing words. The key for each node is the string representation of the word
class BST
{
public:
	BST();
	~BST();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	Word* add(std::string key);

	// Finds the word in the tree with the specified tree. 
	// Returns:
	//		A pointer to the word represented by the specified key
	//		A null pointer if the key does not exist in the tree
	Word* get(std::string key) const;

	// Removes the word represented by the key from the tree
	// Returns:
	//		true iff the tree contained an element with the specified tree
	bool remove(std::string key);

	// Returns a pointer to the word that comes first, alphabetically, or null if the tree is empty
	Word* minimum() const;

	// Returns a pointer to the word that comes last, alphabetically, or null if the tree is empty
	Word* maximum() const;

	// Returns a pointer to the word that comes alphabetically before the specified word, or null
	// if the tree is empty or the key has no predecessor
	Word* predecessor(std::string key) const;

	// Returns a pointer to the word that comes alphabetically after the specified word, or null
	// if the tree is empty or the key has no successor
	Word* successor(std::string key) const;

	// Prints all words and their occurrance count in alphabetical order to std::cout
	void inOrderPrint() const;

	// Returns true iff the tree is empty
	bool isEmpty() const
	{
		return Root == nullptr;
	}
private:
	// The node at the root of the tree
	BinaryTreeNode* Root = nullptr;

	// Finds a node in the tree with the specified key
	BinaryTreeNode* find(std::string key) const;

	// Finds the smallest node in the subtree from the specified node
	static BinaryTreeNode* minimumOf(BinaryTreeNode* node);
	// Finds the largest node in the subtree from the specified node
	static BinaryTreeNode* maximumOf(BinaryTreeNode* node);

	// Finds the predecessor of the specified node
	static BinaryTreeNode* predecessorOf(BinaryTreeNode* node);
	// Finds the successor of the specified node
	static BinaryTreeNode* successorOf(BinaryTreeNode* node);

	// A helper function to take care of the edge cases when removing nodes from the tree
	void transplant(BinaryTreeNode* u, BinaryTreeNode* v);
	// Removes the specified node from the tree
	void removeNode(BinaryTreeNode* node);

	// Recursively prints the subtree starting from the specified node in order
	void inOrderPrint(BinaryTreeNode* node) const;
};

