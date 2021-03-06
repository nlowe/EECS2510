/*
 * IWordCounter.h - Interface and helper classes for an abstract word counter
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

// A structure representing statistics about a given document
struct DocumentStatistics
{
	DocumentStatistics(size_t height, size_t total, size_t distinct) : TreeHeight(height), TotalWords(total), TotalNodes(total), DistinctWords(distinct) {}
	DocumentStatistics(size_t height, size_t total, size_t distinct, size_t nodes) : TreeHeight(height), TotalWords(total), TotalNodes(nodes), DistinctWords(distinct) {}

	// The height of the tree used to count words in this document
	const size_t TreeHeight;
	// The total number of words encountered while parsing the document
	const size_t TotalWords;
	// The total number of nodes in the tree
	const size_t TotalNodes;
	// The total number of distinct words encountered while parsing the document
	const size_t DistinctWords;
};