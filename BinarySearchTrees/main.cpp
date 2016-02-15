// BinarySearchTrees.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>

#include "BST.h"
#include "Util.h"

// The tree containing the words
BST* words = new BST();

static const std::string COMMAND_INSERT = "insert";
static const std::string COMMAND_DELETE = "delete";
static const std::string COMMAND_SEARCH = "search";
static const std::string COMMAND_MIN    = "min";
static const std::string COMMAND_MAX    = "max";
static const std::string COMMAND_NEXT   = "next";
static const std::string COMMAND_PREV   = "prev";
static const std::string COMMAND_LIST   = "list";
static const std::string COMMAND_HELP   = "help";
static const std::string COMMAND_EXIT   = "exit";

// A structure representing a Command and optional arguments
struct Command
{
	std::string name;
	std::string arguments;
};

// Gets the next Command from standard input
Command getNextCommand()
{
	auto result = Command();
	
	std::string line;
	getline(std::cin, line);

	auto index_of_separator = line.find(" ");
	if(index_of_separator == std::string::npos)
	{
		// The command name needs to be case-insensitive
		str_to_lower(line);

		result.name = line;
		result.arguments = "";
	}
	else
	{
		// The command name needs to be case-insensitive
		auto name = line.substr(0, index_of_separator);
		str_to_lower(name);

		result.name = name;
		result.arguments = line.substr(index_of_separator + 1);
	}

	return result;
}

// Prints the help documentation for the commands
void printHelp()
{
	std::cout << "Recognized commands:" << std::endl;

	std::cout << "insert <string>\tAdds the word <string> to the collection of words." << std::endl;
	std::cout << "\t\tIf the word already exists, the count is incremented by one" << std::endl;
	std::cout << "\t\tReturns: the word and its occurance count" << std::endl;
	std::cout << std::endl;

	std::cout << "delete <string>\tDecrements the occurrance account of the word <string> in the collection" << std::endl;
	std::cout << "\t\tIf the word is not in the set, returns the word with an occurrance count of -1" << std::endl;
	std::cout << "\t\tIf the last occurrance of the word is removed, returns the word with an occurrance count of 0" << std::endl;
	std::cout << std::endl;

	std::cout << "search <string>\tLooks for the word <string> in the collection of words and prints its occurrance count" << std::endl;
	std::cout << "\t\tIf the word is not in the set, returns the word with an occurrance count of 0" << std::endl;
	std::cout << std::endl;

	std::cout << "min\t\tFinds the word in the set that alphabetically comes before every other word in the set" << std::endl;
	std::cout << "\t\tIf the set is empty, a blank line is printed" << std::endl;
	std::cout << std::endl;

	std::cout << "max\t\tFinds the word in the set that alphabetically comes after every other word in the set" << std::endl;
	std::cout << "\t\tIf the set is empty, a blank line is printed" << std::endl;
	std::cout << std::endl;

	std::cout << "next <string>\tFinds the alphabetical successor to the word <string>" << std::endl;
	std::cout << "\t\tIf the word is not in the collection, or has no successor, a blank line is printed" << std::endl;
	std::cout << std::endl;

	std::cout << "prev <string>\tFinds the alphabetical predecessor to the word <string>" << std::endl;
	std::cout << "\t\tIf the word is not in the collection, or has no predecessor, a blank line is printed" << std::endl;
	std::cout << std::endl;

	std::cout << "list\t\tPrints all words and their occurrance counts in alphabetical order" << std::endl;
	std::cout << std::endl;

	std::cout << "help\t\tPrints this message" << std::endl;
	std::cout << std::endl;

	std::cout << "exit\t\tQuits the program" << std::endl;

}

int main()
{
	std::cout << "Enter a command (enter 'help' for a list of commands')" << std::endl;

	Command current_command;
	do
	{
		// Get the next command from stdin
		current_command = getNextCommand();

		// And process it
		if(current_command.name == COMMAND_INSERT)
		{
			auto result = words->add(current_command.arguments);
			std::cout << result->key << " " << result->count << std::endl;
		}
		else if(current_command.name == COMMAND_DELETE)
		{
			auto word = words->get(current_command.arguments);

			if(word == nullptr)
			{
				std::cout << current_command.arguments << " -1" << std::endl;
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

				std::cout << content << " " << count << std::endl;
			}
		}
		else if(current_command.name == COMMAND_SEARCH)
		{
			auto word = words->get(current_command.arguments);
			std::cout << current_command.arguments << (word == nullptr ? " 0" : (" " + std::to_string(word->count))) << std::endl;
		}
		else if(current_command.name == COMMAND_MIN)
		{
			auto word = words->minimum();

			if(word == nullptr)
			{
				std::cout << std::endl;
			}
			else
			{
				std::cout << word->key << " " << word->count << std::endl;
			}
		}
		else if(current_command.name == COMMAND_MAX)
		{
			auto word = words->maximum();

			if (word == nullptr)
			{
				std::cout << std::endl;
			}
			else
			{
				std::cout << word->key << " " << word->count << std::endl;
			}
		}
		else if(current_command.name == COMMAND_NEXT)
		{
			auto word = words->successor(current_command.arguments);
			
			if (word == nullptr)
			{
				std::cout << std::endl;
			}
			else
			{
				std::cout << word->key << " " << word->count << std::endl;
			}
		}
		else if(current_command.name == COMMAND_PREV)
		{
			auto word = words->predecessor(current_command.arguments);

			if (word == nullptr)
			{
				std::cout << std::endl;
			}
			else
			{
				std::cout << word->key << " " << word->count << std::endl;
			}
		}
		else if(current_command.name == COMMAND_LIST)
		{
			words->inOrderPrint();
		}
		else if(current_command.name == COMMAND_HELP)
		{
			printHelp();
		}
		else if(current_command.name != COMMAND_EXIT)
		{
			std::cout << "Unknown command: " << current_command.name << std::endl;
		}
		
	} while (current_command.name != COMMAND_EXIT);

	// Free the word tree
	delete words;

    return 0;
}
