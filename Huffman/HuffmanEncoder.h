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

// The next node in the stream is a leaf node
static const unsigned char FLAG_LEAF_NODE   = 0x00;
// The next node in the stream has a left child
static const unsigned char FLAG_LEFT_CHILD  = 0x01;
// The next node in the stream has a right child
static const unsigned char FLAG_RIGHT_CHILD = 0x02;
// The next node in the stream has both a left and right child
static const unsigned char FLAG_BOTH_NODES  = FLAG_LEFT_CHILD | FLAG_RIGHT_CHILD;

// A structure for representing a node in a Huffman Tree
struct HuffmanTreeNode
{
	// The payload of the node
	unsigned char payload;
	// The weight of this node
	unsigned long long weight;

	// The left child
	HuffmanTreeNode* Left = nullptr;
	// The right child
	HuffmanTreeNode* Right = nullptr;

	explicit HuffmanTreeNode(unsigned char p, unsigned long long w) : payload(p), weight(w) {}
	~HuffmanTreeNode()
	{
		if (Left != nullptr) delete Left;
		if (Right != nullptr) delete Right;
	}

	// Returns: True iff this node is a leaf (has no children)
	bool IsLeaf() const
	{
		return Left == nullptr && Right == nullptr;
	}
};

// A class for Encoding and Decoding files with the Huffman scheme
//
// The encoder must be initialized (constructed) from the weights of occurrance of
// bytes in a specific file before it can encode the file.
//
// While re-using the same encoder for multiple files will work, the compression will
// not be ideal since each encoding scheme is optimized per file.
//
// During encoding, the encoding tree is written to the output file. This means that
// if you just need to decode a file, you do not need to have a copy of the original
// file.
//
// If you try to encode another file using an Encoder built by decoding a file,
// the encoding table will first be rebuilt
class HuffmanEncoder
{
public:
	// A magic header written to differentiate huffman encoded files from other file types
	static const unsigned short HEADER = 0x687A;
	// The file format version that this copy of the program will support
	static const unsigned short VERSION = 0x02;

	explicit HuffmanEncoder();
	explicit HuffmanEncoder(unsigned long long weights[256]);
	~HuffmanEncoder();

	// Construct a huffman encoder, populating the weights table from the bytes at the
	// specified file path
	static HuffmanEncoder* ForFile(std::string path);

	// Encodes the file at <input> with the pre-generated encoding table and writes to <output>
	void Encode(std::string input, std::string output, size_t& bytesRead, size_t& bytesWritten);

	// Decodes the input file to the specified output file
	//
	// Note that the encoding tree will be overwritten with the one stored in the file (if it is valid)
	// Any subsequent encodings with this Encoder will use this new tree for encoding.
	//
	// If this is undesired, a new Encoder must be constructed
	void Decode(std::string input, std::string output, size_t& bytesRead, size_t& bytesWritten);

	// A table of bitstrings used for encoding
	std::string EncodingTable[256] = {};
private:
	// The root of the encoding tree
	HuffmanTreeNode* TreeRoot = nullptr;

	// The longest bitstring, used for padding to the nearest byte when encoding the last byte of a file
	std::string PaddingHint = "";
	unsigned char PaddingChar = 0;

	// Write the subtree from the specified node to the specified output stream
	static void WriteEncodingTree(std::ostream& output, HuffmanTreeNode* node, size_t& bytesWritten);
	// Read the subtree from the specified input stream
	static HuffmanTreeNode* ReadEncodingTree(std::ifstream& reader, size_t& bytesRead);

	// Decodes the bit from ubyte masked by the specified mask, moving the specified node pointer along the tree
	static void DecodeBit(HuffmanTreeNode*& currentNode, unsigned char ubyte, unsigned char mask);
	// Checks the specified node pointer, and if it is a leaf, writes its payload to the specified output stream
	void WriteIfLeaf(std::ofstream& writer, HuffmanTreeNode*& currentNode, size_t& bytesWritten) const;

	// Builds the internal encoding tree from an array of nodes
	void BuildTreeFromNodes(HuffmanTreeNode* nodes[256]);
	// Populates the encoding table from the subtree at the specified node
	void BuildEncodingTable(std::string bitstring, HuffmanTreeNode* node);

	// Converts the specified 8-character string to a byte
	// 
	// The input string MUST be 8 characters, and can only consist of '0' and '1'
	static unsigned char BitfieldToByte(std::string in);

	// If set to true, the encoding table must be rebuilt from the tree
	//
	// Set after a file is decoded, since the tree is replaced with the one in the file
	bool IsDirty = true;
};
