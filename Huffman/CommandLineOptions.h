#pragma once
#include <string>

// A simple structure for managing command-line options and flags
struct CommandLineOptions
{
	// An error occurred while parsing command line arguments
	bool parseError = false;
	// The help flag was specified
	bool printHelp = false;
	// The encode mode was requested
	bool encode = false;
	// The decode mode was requested
	bool decode = false;
	// The verbose flag was specified
	bool verbose = false;

	// The path to the input file
	std::string input = "";
	// The path to the output file
	std::string output = "";

	std::string ToString() const
	{
		std::string result = "Parse Error: ";
		result += parseError ? "true\n" : "false\n";

		result += "Print Help: ";
		result += printHelp ? "true\n" : "false\n";

		result += "Encode: ";
		result += encode ? "true\n" : "false\n";

		result += "Decode: ";
		result += decode ? "true\n" : "false\n";

		result += "Verbose: ";
		result += verbose ? "true\n" : "false\n";

		result += "Input File: " + input + "\n";
		result += "Output File: " + output += "\n";

		return result;
	}
};