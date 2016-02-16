/*
 * main.cpp - Main entry point for the executable
 * 
 * Contains command processing functions, and uses
 * a Binary Search Tree as the backing data type
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
#include <iostream>
#include <string>

#include "BST.h"
#include "Util.h"

using namespace std;

// The tree containing the words
BST* words = new BST();

// ================== Constants ==================
static const string COMMAND_INSERT = "insert";
static const string COMMAND_DELETE = "delete";
static const string COMMAND_SEARCH = "search";
static const string COMMAND_MIN    = "min";
static const string COMMAND_MAX    = "max";
static const string COMMAND_NEXT   = "next";
static const string COMMAND_PREV   = "prev";
static const string COMMAND_LIST   = "list";
static const string COMMAND_HELP   = "help";
static const string COMMAND_EXIT   = "exit";
// ===============================================

// Gets the next Command from standard input
// Returns:
//		* A pair of two strings. The first element is the command name, the second is the arguments (empty if none)
pair<string, string> getNextCommand()
{
	// Read a line from standard input
	string line;
	getline(cin, line);

	// See if we were given a command with arguments
	auto index_of_separator = line.find(" ");
	if(index_of_separator == string::npos)
	{
		// The command name needs to be case-insensitive
		str_to_lower(line);

		return make_pair(line, "");
	}
	else
	{
		// The command name needs to be case-insensitive
		auto name = line.substr(0, index_of_separator);
		str_to_lower(name);

		return make_pair(name, line.substr(index_of_separator + 1));
	}
}

// Prints the help documentation for the commands
void printHelp()
{
	cout << "Recognized commands:" << endl;

	cout << "insert <string>\tAdds the word <string> to the collection of words." << endl;
	cout << "\t\tIf the word already exists, the count is incremented by one" << endl;
	cout << "\t\tReturns: the word and its occurance count" << endl;
	cout << endl;

	cout << "delete <string>\tDecrements the occurrance account of the word <string> in the collection" << endl;
	cout << "\t\tIf the word is not in the set, returns the word with an occurrance count of -1" << endl;
	cout << "\t\tIf the last occurrance of the word is removed, returns the word with an occurrance count of 0" << endl;
	cout << endl;

	cout << "search <string>\tLooks for the word <string> in the collection of words and prints its occurrance count" << endl;
	cout << "\t\tIf the word is not in the set, returns the word with an occurrance count of 0" << endl;
	cout << endl;

	cout << "min\t\tFinds the word in the set that alphabetically comes before every other word in the set" << endl;
	cout << "\t\tIf the set is empty, a blank line is printed" << endl;
	cout << endl;

	cout << "max\t\tFinds the word in the set that alphabetically comes after every other word in the set" << endl;
	cout << "\t\tIf the set is empty, a blank line is printed" << endl;
	cout << endl;

	cout << "next <string>\tFinds the alphabetical successor to the word <string>" << endl;
	cout << "\t\tIf the word is not in the collection, or has no successor, a blank line is printed" << endl;
	cout << endl;

	cout << "prev <string>\tFinds the alphabetical predecessor to the word <string>" << endl;
	cout << "\t\tIf the word is not in the collection, or has no predecessor, a blank line is printed" << endl;
	cout << endl;

	cout << "list\t\tPrints all words and their occurrance counts in alphabetical order" << endl;
	cout << endl;

	cout << "help\t\tPrints this message" << endl;
	cout << endl;

	cout << "exit\t\tQuits the program" << endl;
}

// Perform the delete command with the specified argument on the tree
void doDeleteCommand(string key)
{
	// First, find the word
	auto word = words->get(key);

	if(word == nullptr)
	{
		// The word wasn't in the tree
		cout << key << " -1" << endl;
	}
	else
	{
		// Decrement the occurrance count
		word->count--;

		// Save the word and occurrance count in case we delete it from the tree in the next step
		auto content = word->key;
		auto count = word->count;
				
		if(word->count == 0)
		{
			// If this is the last occurrance of the word, remove it from the tree
			words->remove(word->key);
		}

		cout << content << " " << count << endl;
	}
}

// Perform the minimum command against the tree
void doMinimumCommand()
{
	auto word = words->minimum();

	if(word == nullptr)
	{
		cout << endl;
	}
	else
	{
		cout << word->key << endl;
	}
}

// Perform the maximum command against the tree
void doMaximumCommand()
{
	auto word = words->maximum();

	if (word == nullptr)
	{
		cout << endl;
	}
	else
	{
		cout << word->key << endl;
	}
}

// Perform the successor command with the specified argument against the tree
void doSuccessorCommand(string key)
{
	auto word = words->successor(key);
			
	if (word == nullptr)
	{
		cout << endl;
	}
	else
	{
		cout << word->key << " " << word->count << endl;
	}
}

// Perform the predecessor command with the specified argument against the tree
void doPredecessorCommand(string key)
{
	auto word = words->predecessor(key);

	if (word == nullptr)
	{
		cout << endl;
	}
	else
	{
		cout << word->key << " " << word->count << endl;
	}
}

// The main entry point of the program
int main()
{
	cout << "Enter a command (enter 'help' for a list of commands')" << endl;

	pair<string, string> current_command;
	do
	{
		// Get the next command from stdin
		current_command = getNextCommand();

		// And process it
		if(current_command.first == COMMAND_INSERT)
		{
			auto result = words->add(current_command.second);
			cout << result->key << " " << result->count << endl;
		}
		else if(current_command.first == COMMAND_DELETE)
		{
			doDeleteCommand(current_command.second);
		}
		else if(current_command.first == COMMAND_SEARCH)
		{
			auto word = words->get(current_command.second);
			cout << current_command.second << (word == nullptr ? " 0" : (" " + to_string(word->count))) << endl;
		}
		else if(current_command.first == COMMAND_MIN)
		{
			doMinimumCommand();
		}
		else if(current_command.first == COMMAND_MAX)
		{
			doMaximumCommand();
		}
		else if(current_command.first == COMMAND_NEXT)
		{
			doSuccessorCommand(current_command.second);
		}
		else if(current_command.first == COMMAND_PREV)
		{
			doPredecessorCommand(current_command.second);
		}
		else if(current_command.first == COMMAND_LIST)
		{
			words->inOrderPrint();
		}
		else if(current_command.first == COMMAND_HELP)
		{
			printHelp();
		}
		else if(current_command.first != COMMAND_EXIT)
		{
			cout << "Unknown command: " << current_command.first << endl;
		}
		
	} while (current_command.first != COMMAND_EXIT);

	// Free the word tree
	delete words;

    return 0;
}
