/*
 * main.cpp - Main entry point for the executable
 *
 * Contains command processing functions, and uses
 * a Huffman Tree to encode and decode files
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

using namespace std;

static const int EXIT_OK = 0;
static const int EXIT_BAD_ARGUMENTS = -1;

struct CommandLineOptions
{
	bool parseError = false;
	bool printHelp = false;
	bool encode = false;
	bool decode = false;
	bool verbose = false;
	string input = "";
	string output = "";
};

void printHelp()
{
	cout << "Huffman Encoder and Decoder" << endl;
	cout << "Usage: huffman <options> -i <input_file> -o <output_file>" << endl << endl;

	cout << "Options:" << endl;
	cout << "\t-i, --input\tSpecifies the input file to encode or decode" << endl;
	cout << "\t-o, --output\tSpecifies the output file to encode or decode to" << endl;
	cout << "\t-e, --encode\tEncode <input_file> and write to <output_file>" << endl;
	cout << "\t-d, --decode\tDecode <input_file> and write to <output_file>" << endl;
	cout << "\t-t,--test\tEncode <input_file> to <output>.hz, then decode back to <output_file>" << endl;
	cout << "\t-v, --verbose\tPrint verbose messages" << endl;
	cout << "\t-h, --help\tPrint this help message" << endl;
}

CommandLineOptions parseArguments(int argc, char* argv[])
{
	CommandLineOptions result;

	for (int i = 1; i < argc; i++)
	{
		auto arg = string(argv[i]);
		if(arg == "-e" || arg == "--encode")
		{
			result.encode = true;
		}
		else if(arg == "-d" || arg == "--decode")
		{
			result.decode = true;
		}
		else if(arg == "-t" || arg == "--test")
		{
			result.encode = result.decode = true;
		}
		else if(arg == "-v" || arg == "--verbose")
		{
			result.verbose = true;
		}
		else if(arg == "-h" || arg == "--help")
		{
			result.printHelp = true;
		}
		else if(arg == "-i" || arg == "--input")
		{
			if(i >= argc-1)
			{
				result.parseError = true;
				cout << "Missing Parameter for " << argv[i] << endl;
			}
			else
			{
				result.input = string(argv[++i]);
			}
		}
		else if(arg == "-o" || arg == "--output")
		{
			if (i >= argc - 1)
			{
				result.parseError = true;
				cout << "Missing Parameter for " << argv[i] << endl;
			}
			else
			{
				result.output = string(argv[++i]);
			}
		}
		else
		{
			result.parseError = true;
			cout << "Unrecognized Argument: " << argv[i] << endl;
		}
	}

	return result;
}

int main(int argc, char* argv[])
{
	auto options = parseArguments(argc, argv);
	
	if(options.printHelp)
	{
		printHelp();
		return EXIT_OK;
	}

	if (options.parseError)
	{
		return EXIT_BAD_ARGUMENTS;
	}

	if (!(options.encode || options.decode))
	{
		cout << "Nothing to do (specify one of -e, -d, or -t" << endl;
		return EXIT_OK;
	}

	if (options.input == "" || options.output == "")
	{
		cout << "Both the input and output files must be specified" << endl;
		return EXIT_BAD_ARGUMENTS;
	}

    return EXIT_OK;
}
