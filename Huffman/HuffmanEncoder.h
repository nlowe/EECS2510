/*
 * HuffmanEncoder.h - Public interface for a huffman encoder
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
#include <string>

struct HuffmanTreeNode
{
	unsigned char payload;
	unsigned long long weight;
	HuffmanTreeNode* Left = nullptr;
	HuffmanTreeNode* Right = nullptr;

	explicit HuffmanTreeNode(unsigned char p, unsigned long long w) : payload(p), weight(w) {}
	~HuffmanTreeNode()
	{
		if (Left != nullptr) delete Left;
		if (Right != nullptr) delete Right;
	}
};

class HuffmanEncoder
{
public:
	explicit HuffmanEncoder(unsigned long long weights[256]);
	~HuffmanEncoder();

	static HuffmanEncoder* ForFile(std::string path);

	void Encode(std::string input, std::string output, size_t& bytesRead, size_t& bytesWritten1) const;
private:
	HuffmanTreeNode* TreeRoot = nullptr;
	std::string EncodingTable[256] = {};

	void BuildTreeFromNodes(HuffmanTreeNode* nodes[256]);
	void BuildEncodingTable(std::string bitstring, HuffmanTreeNode* node);
	static unsigned char BitfieldToByte(std::string in);
};
