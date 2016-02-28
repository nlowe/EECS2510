/*
 * CommandLineOptions.h - Defines a struct which command line options
 *							are parsed into
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