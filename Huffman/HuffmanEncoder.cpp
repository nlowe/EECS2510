/*
 * HuffmanEncoder.cpp - Implementation for a Huffman Encoder
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
#include "HuffmanEncoder.h"
#include <fstream>
#include <iostream>
#include "Verbose.h"

// Construct an Empty Huffman Encoder
HuffmanEncoder::HuffmanEncoder() : IsDirty(true) {}

// Construct a Huffman Encoder from the specified weight table
HuffmanEncoder::HuffmanEncoder(unsigned long long weights[256])
{
	// Set aside room for 256 leaf nodes
	HuffmanTreeNode* nodes[256] = { nullptr };

	// Initialize the nodes with weights
	for (auto b = 0; b < 256; b++)
	{
		if(weights[b] > 0) nodes[b] = new HuffmanTreeNode(b, weights[b]);
	}

	// Build the tree from the weights
	BuildTreeFromNodes(nodes);

	// Now, build the bitstring table
	// This assignment requires us to use std::strings and not bitsets
	verbose::write("Building Encoding Table...");
	BuildEncodingTable("", TreeRoot);

	verbose::write("Padding Hint: " + std::to_string(PaddingChar) + " (" + PaddingHint + ")");
}

HuffmanEncoder::~HuffmanEncoder()
{
	if (TreeRoot != nullptr) delete TreeRoot;
}

// Construct a huffman encoder, populating the weights table from the bytes at the
// specified file path.
//
// The file is read byte by byte, and the number of times each byte occurrs is recorded
// A Huffman Encoder is then constructed from the weight table
HuffmanEncoder* HuffmanEncoder::ForFile(std::string path)
{
	std::ifstream reader;
	reader.open(path, std::ios::binary);

	if (!reader.is_open()) throw std::runtime_error("Unable to open file for read");

	unsigned long long weight[256] = {0};

	// Read through the file to get the weight for all bytes
	while(!reader.eof())
	{
		char byte;
		reader.get(byte);

		weight[reinterpret_cast<unsigned char&>(byte)]++;
	}
	reader.close();

	// And build an encoder from the weights
	return new HuffmanEncoder(weight);
}

// Encodes the file at <input> with the pre-generated encoding table and writes to <output>
//
// File Format (Version 2):
//		2 Bytes - 0x687A - 'hz' Magic Header to distinguish file format
//		1 Byte  - 0x02   - File format version number
//		Decoding Tree - Variable, dependent on the number of internal nodes, up to 1022 bytes
//			The decoding tree is written as a pre-order traversal of the tree such that:
//				1 Byte  - 0x01 If the node being visited is an internal node or root with only a left child
//				1 Byte  - 0x02 If the node being visited is an internal node or root with only a right child
//				1 Byte	- 0x03 If the node being visited is an internal node or root with two children
//				2 Bytes - 0x00 and the payload character If the node being visited is a leaf node
//		Encoded Data
//			Variable - The raw bitstrings converted to binary. The last bitstring is padded with the beginning of the longest bitstring
void HuffmanEncoder::Encode(std::string input, std::string output, size_t& bytesRead, size_t& bytesWritten)
{
	// Somehow, we have an encoder that wasn't properly initialized
	if (TreeRoot == nullptr) throw std::runtime_error("Encoder not initialized");

	// The encoding table was marked dirty, it needs to be rebuilt from the tree
	if (IsDirty) BuildEncodingTable("", TreeRoot);

	std::ifstream reader;
	std::ofstream writer;

	reader.open(input, std::ios::binary);
	writer.open(output, std::ios::binary);

	if (!reader.is_open() || !reader.good()) throw std::runtime_error("Cannot open file for read");
	if (!writer.is_open() || !writer.good()) throw std::runtime_error("Cannot open file for write");

	verbose::write("Starting encode of " + input);

	// Write the header and file format version
	writer.put((HEADER >> 8) & 0xFF);
	writer.put(HEADER & 0xFF);
	writer.put(VERSION);
	bytesWritten += 3;

	//Write the decoding tree
	WriteEncodingTree(writer, TreeRoot, bytesWritten);

	std::string encodingBuffer = "";

	// Read the file one byte at a time
	char byte;
	while(reader.get(byte))
	{
		bytesRead++;

		// And add its encoding representation to the output buffer
		encodingBuffer += EncodingTable[reinterpret_cast<unsigned char&>(byte)];

		while (encodingBuffer.length() >= 8)
		{
			// Its time to flush the buffer
			bytesWritten++;

			// Extract 8 bits from the buffer
			auto slice = encodingBuffer.substr(0, 8);
			encodingBuffer.erase(0, 8);

			// And write the byte value to the file
			writer.put(BitfieldToByte(slice));
		}
	}

	// Now that a read failed, we should be at the end of the file
	if (!reader.eof()) throw std::invalid_argument("Falied to read file completely");
	reader.close();

	// Check to see if we have a partial byte to write
	if(encodingBuffer.length() > 0)
	{
		bytesWritten++;

		verbose::write(
			"Encoded output not byte-aligned. Need " + std::to_string(8 - encodingBuffer.length()) +
			" more bits (Buffer contains: " + encodingBuffer + ")"
		);

		// Pad the buffer in case we're not aligned to a byte
		// By padding with the longest bitstring, we ensure we will never reach a leaf node when decoding the final byte
		encodingBuffer += PaddingHint.substr(0, 8 - encodingBuffer.size());

		writer.put(BitfieldToByte(encodingBuffer));
	}

	writer.flush();
	writer.close();
}

// Read the subtree from the specified input stream
HuffmanTreeNode* HuffmanEncoder::ReadEncodingTree(std::ifstream& reader, size_t& bytesRead)
{
	// Read the node type from the stream
	char nodeType;
	reader.get(nodeType);
	bytesRead++;

	// If this is a leaf node, read its payload 
	if (nodeType == FLAG_LEAF_NODE)
	{
		char b;
		reader.get(b);
		bytesRead++;

		return new HuffmanTreeNode(reinterpret_cast<unsigned char&>(b), 0);
	}
	
	// If this isn't a type of node we recognized, then either the file format is corrupt
	// Or it wasn't encoded with this version of the software
	if (nodeType > FLAG_BOTH_NODES) throw std::invalid_argument("Unrecognized node type: " + static_cast<unsigned>(nodeType));
	
	// Otherwise, read the left and right sub trees from the stream if their bitmask is set
	auto result = new HuffmanTreeNode(0, 0);
	if ((nodeType & FLAG_LEFT_CHILD) == FLAG_LEFT_CHILD)   result->Left  = ReadEncodingTree(reader, bytesRead);
	if ((nodeType & FLAG_RIGHT_CHILD) == FLAG_RIGHT_CHILD) result->Right = ReadEncodingTree(reader, bytesRead);

	return result;
}

// Decodes the bit from ubyte masked by the specified mask, moving the specified node pointer along the tree
// If the bit is set, we're supposed to "take" the right branch, so we should expect a node to the right
// If the bit is not set, we're supposed to "take" the left branch, so we should expect a node to the left
//
// If for some reason we can't take the path indicated by the bit, the file is corrupt
// Since the currentNode pointer would have been reset by the call to WriteIfLeaf immediately before this method
void HuffmanEncoder::DecodeBit(HuffmanTreeNode*& currentNode, unsigned char ubyte, unsigned char mask)
{
	if((ubyte & mask) == mask)
	{
		if(currentNode->Right != nullptr) currentNode = currentNode->Right;
		else throw std::invalid_argument("Input file is corrupt (expected right treepath does not exist)");
	}
	else
	{
		if (currentNode->Left != nullptr) currentNode = currentNode->Left;
		else throw std::invalid_argument("Input file is corrupt (expected left treepath does not exist)");
	}
}

// Checks the specified node pointer, and if it is a leaf, writes its payload to the specified output stream
void HuffmanEncoder::WriteIfLeaf(std::ofstream& writer, HuffmanTreeNode*& currentNode, size_t& bytesWritten) const
{
	if(currentNode->IsLeaf())
	{
		// We're at a leaf. Write a new byte
		writer.put(currentNode->payload);
		bytesWritten++;

		// Reset the currentNode pointer to the root of the tree
		currentNode = TreeRoot;
	}
}

// Decodes the input file to the specified output file
//
// See Encode(...) for documentation on the file format
void HuffmanEncoder::Decode(std::string input, std::string output, size_t& bytesRead, size_t& bytesWritten)
{
	std::ifstream reader;
	std::ofstream writer;

	reader.open(input, std::ios::binary);
	writer.open(output, std::ios::binary);

	if (!reader.is_open()) throw std::runtime_error("Cannot open file for read");
	if (!writer.is_open()) throw std::runtime_error("Cannot open file for write");

	verbose::write("Starting decoding of " + input);

	// Read the file header and version
	short header = 0;
	char version;

	char h1;
	char h2;
	reader.get(h1);
	reader.get(h2);

	header |= h1 << 8;
	header |= h2;

	reader.get(version);

	bytesRead += 3;

	if (reinterpret_cast<unsigned short&>(header) != 0x687a)
	{
		reader.close();
		writer.close();

		throw std::invalid_argument("Not a huffman file");
	}

	if (version != VERSION)
	{
		reader.close();
		writer.close();

		throw std::invalid_argument("Don't know how to decode file version " + static_cast<unsigned>(version));
	}

	// We may have recycled an existing encoder. Get rid of its encoding tree
	if (TreeRoot != nullptr)
	{
		verbose::write("WARNING: An encoding tree already exists and will be overwritten");
		verbose::write("WARNING: This can be ignored if this encoder is only being used to decode a file");
		verbose::write("WARNING: Construct a new encoder if you intend to encode another file");
		delete TreeRoot;
		IsDirty = true;
	}

	// Read the decoding tree
	TreeRoot = ReadEncodingTree(reader, bytesRead);

	auto currentNode = TreeRoot;

	// Decode the file one byte at a time
	char byte;
	while(reader.get(byte))
	{
		// Read the next byte of the file
		bytesRead++;

		// Parse the read byte as unsigned
		auto ubyte = reinterpret_cast<unsigned char&>(byte);

		// Attempt to decode the read byte bit-by-bit
		// At each bit, check to see if we're at a leaf node
		// If we are, write the payload byte to the output file
		// And reset the node pointer to the root of the tree
		for (auto i = 7; i >= 0; i--)
		{
			WriteIfLeaf(writer, currentNode, bytesWritten);
			DecodeBit(currentNode, ubyte, 1 << i);
		}
	}

	// Now that a read failed, we should be at the end of the file
	if (!reader.eof()) throw std::invalid_argument("Falied to read file completely");
	reader.close();

	// Check if we're evenly alligned. If not, we won't be at a leaf node anyways
	WriteIfLeaf(writer, currentNode, bytesWritten);

	// Close the streams
	writer.flush();
	writer.close();
}

// Write the subtree from the specified node to the specified output stream
void HuffmanEncoder::WriteEncodingTree(std::ostream& output, HuffmanTreeNode* node, size_t& bytesWritten)
{
	if (node == nullptr) return;

	// Assume we're at a leaf node
	unsigned char nodeType = FLAG_LEAF_NODE;

	// If we are, write it to the stream
	if(node->IsLeaf())
	{
		output.put(nodeType);
		output.put(node->payload);

		bytesWritten += 2;
		return;
	}

	// Otherwise, set the node type bitmask correctly
	if(node->Left != nullptr) nodeType |= FLAG_LEFT_CHILD;
	if (node->Right != nullptr) nodeType |= FLAG_RIGHT_CHILD;

	// Write the node type
	output.put(nodeType);
	bytesWritten++;

	// And then write the left and right subtrees
	WriteEncodingTree(output, node->Left, bytesWritten);
	WriteEncodingTree(output, node->Right, bytesWritten);
}

// Builds the internal encoding tree from an array of nodes
void HuffmanEncoder::BuildTreeFromNodes(HuffmanTreeNode* nodes[256])
{
	verbose::write("Building Encoding Tree...");

	int firstSmallest = -1;
	int secondSmallest = -1;

	// Pair nodes until we have a single root node forming the tree
	do
	{
		for (int i = 0; i < 256; i++)
		{
			// Skip nodes that were not used or have already been joined
			if (nodes[i] == nullptr) continue;

			// Pick the first non null node to be the "smallest"
			if (firstSmallest == -1)
			{
				firstSmallest = i;
				continue;
			}

			// Pick the next non null node to be the "smallest"
			if (secondSmallest == -1)
			{
				secondSmallest = i;
				continue;
			}

			if (nodes[secondSmallest]->weight < nodes[firstSmallest]->weight)
			{
				// Make sure the first smallest is always <= to the second smallest
				std::swap(firstSmallest, secondSmallest);
			}

			if (nodes[i]->weight < nodes[firstSmallest]->weight)
			{
				// The first smallest is at least as small as the second smallest
				// So save firstSmallest in secondSmallest
				secondSmallest = firstSmallest;
				// Then remember the location of the next smallest
				firstSmallest = i;
			}
			else if (nodes[i]->weight < nodes[secondSmallest]->weight)
			{
				secondSmallest = i;
			}
		}

		// If we still have two nodes to merge, pair them
		if (firstSmallest != -1 && secondSmallest != -1)
		{
			// Construct a new internal node whose weight is the sum of its left and right sub-trees
			auto temp = new HuffmanTreeNode(0, nodes[firstSmallest]->weight + nodes[secondSmallest]->weight);
			temp->Left = nodes[firstSmallest];
			temp->Right = nodes[secondSmallest];

			verbose::write("\tMerging nodes at " + std::to_string(firstSmallest) + " and " + std::to_string(secondSmallest) + " into " + std::to_string(firstSmallest));

			nodes[firstSmallest] = temp;
			nodes[secondSmallest] = nullptr;
			
			firstSmallest = secondSmallest = -1;
		}
		else
		{
			break;
		}
	} while (true);

	// First Smallest should now be the index of the root of the huffman tree
	TreeRoot = nodes[firstSmallest];
	nodes[firstSmallest] = nullptr;
}

// Populates the encoding table from the subtree at the specified node
void HuffmanEncoder::BuildEncodingTable(std::string bitstring, HuffmanTreeNode* node)
{
	if (node == nullptr) return;

	if (node->IsLeaf())
	{
		// We found a leaf node, record the bitstring that got us here
		EncodingTable[node->payload] = bitstring;

		// Remember the largest bitstring for easy padding of non-aligned bytes when encoding
		if(bitstring.length() >= 8 && bitstring.length() > PaddingHint.length())
		{
			PaddingHint = bitstring;
			PaddingChar = node->payload;

			verbose::write("\tElecting new padding hint " + std::to_string(PaddingChar) + " (" + bitstring + ")");
		}
	}
	else
	{
		// Go find another leaf, 0 for left
		BuildEncodingTable(bitstring + "0", node->Left);
		// and 1 for right
		BuildEncodingTable(bitstring + "1", node->Right);
	}
}

// Converts the specified 8-character string to a byte
// 
// The input string MUST be 8 characters, and can only consist of '0' and '1'
unsigned char HuffmanEncoder::BitfieldToByte(std::string byte)
{
	if (byte.length() != 8) throw std::invalid_argument("Bitfield length not 8 bits");

	unsigned char result = 0;

	result |= (byte[0] == '1') << 7;
	result |= (byte[1] == '1') << 6;
	result |= (byte[2] == '1') << 5;
	result |= (byte[3] == '1') << 4;
	result |= (byte[4] == '1') << 3;
	result |= (byte[5] == '1') << 2;
	result |= (byte[6] == '1') << 1;
	result |= (byte[7] == '1') << 0;

	return result;
}
