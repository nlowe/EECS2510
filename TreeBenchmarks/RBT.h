/*
 * RBT.h - interface for a Red-Black tree
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

#pragma once
#include "BST.h"

// The color of a RBT node. Either red or black
enum NodeColor
{
	RED, BLACK
};

// A node in an Red-Black tree. Basically, a Binary Tree Node
// with an additional field for keeping track of the node color
// and the parent pointer
struct RedBlackNode : BinaryTreeNode
{
	explicit RedBlackNode(Word* payload) : BinaryTreeNode(payload) {}

	~RedBlackNode()
	{
		if (Left != nullptr && !(static_cast<RedBlackNode*>(Left))->isMasterLeaf()) delete Left;
		if (Right != nullptr && !(static_cast<RedBlackNode*>(Right))->isMasterLeaf()) delete Right;
		
		Parent = nullptr;
		Left = Right = nullptr;
	}

	RedBlackNode* Parent = nullptr;
	NodeColor Color = RED;

	size_t height() const override { return 1 + std::max(Left == this ? -1 : Left->height(), Right == this ? -1 : Right->height()); }
	size_t totalHeight() const override { return isMasterLeaf() ? 0 : this->BinaryTreeNode::totalHeight(); }
	size_t payloadSum() const override { return isMasterLeaf() ? 0 : this->BinaryTreeNode::payloadSum(); }

private:
	bool isMasterLeaf() const
	{
		return Left == Right && Left == Parent && Color == BLACK;
	}

};

class RBT : public BST
{
public:
	RBT();
	~RBT();

	// Adds the word to the tree. If the word already exists, its occurrance count is incremeneted
	// Returns:
	//		A pointer to the word represented by the key
	Word* add(std::string key) override;

	size_t getRecolorCount() const { return recolorCount; }
private:
	size_t recolorCount = 0;
	RedBlackNode* leafNodes;

	void fixup(RedBlackNode* z);
	void rotateLeft(RedBlackNode* x);
	void rotateRight(RedBlackNode* x);
};

