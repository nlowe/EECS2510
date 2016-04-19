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
#include <string>

#include "IDiskStatisticsTracker.h"
#include "IPerformanceStatsTracker.h"
#include "Word.h"
#include "DocumentStatistics.h"

// An interface for an abstract, disk-based word counter
class IWordCounter : public IDiskStatisticsTracker, public IPerformanceStatsTracker
{
public:
	virtual ~IWordCounter()
	{
	}

	// Add the specified word to the counter
	virtual void add(std::string word) = 0;

	// Search for the word identified by the specified key, and if available return
	// the number of times the word has been encountered
	virtual std::unique_ptr<Word> find(std::string key) = 0;
	
	// Print the words and their occurrance counts in alphabetical order
	virtual void inOrderPrint() = 0;
	
	// Get statistics about the tree, such as its height and the number of total and distinct words
	// Returns:
	//     A unique pointer to a DocumentStatistics object
	virtual std::unique_ptr<DocumentStatistics> getDocumentStatistics() = 0;
};
