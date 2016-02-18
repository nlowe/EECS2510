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


HuffmanEncoder::HuffmanEncoder(unsigned long long weights[256])
{
	// Set aside room for 256 leaf nodes
	HuffmanTreeNode* nodes[256] = { nullptr };

	// Initialize the nodes with weights
	for (unsigned char b = 0; b < 256; b++)
	{
		if(weights[b] > 0) nodes[b] = new HuffmanTreeNode(b, weights[b]);
	}

	// Build the tree from the weights
	BuildTreeFromNodes(nodes);

	// Now, build the bitstring table
	// This assignment requires us to use std::strings and not bitsets
	BuildEncodingTable("", TreeRoot);
}

HuffmanEncoder::~HuffmanEncoder()
{
	if (TreeRoot != nullptr) delete TreeRoot;
}

HuffmanEncoder* HuffmanEncoder::ForFile(std::string path)
{
	std::ifstream reader;
	reader.open(path, std::ios::binary);

	if (!reader.is_open()) throw std::runtime_error("Unable to open file for read");

	unsigned long long weight[256] = {0};

	while(!reader.eof())
	{
		char byte;
		reader.get(byte);

		weight[reinterpret_cast<unsigned char&>(byte)]++;
	}
	reader.close();

	return new HuffmanEncoder(weight);
}

void HuffmanEncoder::Encode(std::string input, std::string output, size_t& bytesRead, size_t& bytesWritten) const
{
	if (TreeRoot == nullptr) throw std::runtime_error("Encoder not initialized");

	std::ifstream reader;
	std::ofstream writer;

	reader.open(input, std::ios::binary);
	writer.open(output, std::ios::binary);

	if (!reader.is_open()) throw std::runtime_error("Cannot open file for read");
	if (!writer.is_open()) throw std::runtime_error("Cannot open file for write");

	//Write the encoding table
	for (unsigned char b = 0; b < 256; b++)
	{
		// We may not have a full alphabet, skip unused characters
		if (EncodingTable[b] == "") continue;

		writer.put(b);
		writer.put(EncodingTable[b].length());
		writer.write(EncodingTable[b].c_str(), sizeof(char)*EncodingTable[b].size());
	}

	std::string encodingBuffer = "";

	while(!reader.eof())
	{
		bytesRead++;

		char byte;
		reader.get(byte);

		encodingBuffer += EncodingTable[reinterpret_cast<unsigned char&>(byte)];

		if (encodingBuffer.length() >= 8)
		{
			// Its time to flush the buffer
			int writeCount = encodingBuffer.length() / 8;

			// Start writing bytes to the output file
			for (int i = 0; i < writeCount; i++)
			{
				bytesWritten++;

				// Extract 8 bits from the buffer
				auto slice = encodingBuffer.substr(0, 8);
				encodingBuffer.erase(0, 8);

				// And write the byte value to the file
				writer.put(BitfieldToByte(slice));
			}
		}
	}
	reader.close();

	//TODO: Write the remaining bits in the buffer, padded to a byte
	writer.close();
}

void HuffmanEncoder::BuildTreeFromNodes(HuffmanTreeNode* nodes[256])
{
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
			auto temp = new HuffmanTreeNode(0, nodes[firstSmallest]->weight + nodes[secondSmallest]->weight);
			temp->Left = nodes[firstSmallest];
			temp->Right = nodes[secondSmallest];

			nodes[firstSmallest] = temp;
			nodes[secondSmallest] = nullptr;
		}
	} while (firstSmallest != -1 && secondSmallest != -1);

	// First Smallest should now be the index of the root of the huffman tree
	TreeRoot = nodes[firstSmallest];
	nodes[firstSmallest] = nullptr;
}

void HuffmanEncoder::BuildEncodingTable(std::string bitstring, HuffmanTreeNode* node)
{
	if (node == nullptr) return;

	if (node->Left == nullptr && node->Right == nullptr)
	{
		// We found a leaf node, record the bitstring that got us here
		EncodingTable[node->payload] = bitstring;
	}
	else
	{
		// Go find another leaf, 0 for left
		BuildEncodingTable("0" + bitstring, node->Left);
		// and 1 for right
		BuildEncodingTable("1" + bitstring, node->Right);
	}
}

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
