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
#include <chrono>
#include <iomanip>

#include "CommandLineOptions.h"
#include "HuffmanEncoder.h"
#include "Verbose.h"

using namespace std;

// Since the verbose namespace is used multiple times in different files,
// we need to define an implementation of the enable flag in exactly one
// translation unit so that it is shared between all units
namespace verbose
{
	bool enable = false;
}

// The encoder we'll use for encoding and decoding files
HuffmanEncoder* encoder = nullptr;

// The return code for normal operations
static const int EXIT_OK = 0;
// The return code for bad arguments
static const int EXIT_BAD_ARGUMENTS = -1;
// The return code for a failed encoding job
static const int EXIT_ENCODE_FAILED = -2;
// The return code for a failed decoding job
static const int EXIT_DECODE_FAILED = -3;

// Forward declare these methods so main is at the top as per project spec
CommandLineOptions parseArguments(int argc, char* argv[]);
string PrependExtension(string input, string extension);
void printHelp();
bool doEncode(CommandLineOptions options);
bool doDecode(CommandLineOptions options);

// The main entry point of the application
int main(int argc, char* argv[])
{
	// The project spec requires main to encode and decode a file that is hard-coded for easy testing
	// If we don't have any arguments and we're in debug mode, assume it's being tested
#ifdef DEBUG
	if(argc == 1)
	{
		string INPUT_FILE   = "Shakespeare.txt";
		string ENCODED_FILE = "Shakespeare.enc";
		string DECODED_FILE = "Shakespeare.dec";

		CommandLineOptions interactiveOptions;

		interactiveOptions.input  = INPUT_FILE;
		interactiveOptions.output = ENCODED_FILE;

		cout << "Encoding file " << INPUT_FILE << " to " << ENCODED_FILE << endl;
		if (!doEncode(interactiveOptions)) return EXIT_ENCODE_FAILED;

		interactiveOptions.input  = ENCODED_FILE;
		interactiveOptions.output = DECODED_FILE;

		cout << "Decoding file " << ENCODED_FILE << " to " << DECODED_FILE << endl;
		if (!doDecode(interactiveOptions)) return EXIT_DECODE_FAILED;

		return EXIT_OK;
	}
#endif

	// Parse command line options
	auto options = parseArguments(argc, argv);

	// Enable verbose if requested
	if (options.verbose) verbose::enable = true;

	verbose::write(options.ToString());

	// If the help flag was specified, print the help message and exit
	if (options.printHelp)
	{
		printHelp();
		return EXIT_OK;
	}

	// If there was an error parsing command line options, just exit
	if (options.parseError) return EXIT_BAD_ARGUMENTS;

	// If neither encode nor decode modes were specified, exit
	if (!(options.encode || options.decode))
	{
		cout << "Nothing to do (specify one of -e, -d, or -t)" << endl;
		return EXIT_OK;
	}

	// If the input file or output file are blank, exit
	if (options.input == "" || options.output == "")
	{
		cout << "Both the input and output files must be specified" << endl;
		return EXIT_BAD_ARGUMENTS;
	}

	// Perform an encode operation if specified
	if (options.encode && !doEncode(options)) return EXIT_ENCODE_FAILED;
	// Perform a decode operation if specified
	if (options.decode && !doDecode(options)) return EXIT_DECODE_FAILED;

	// Free the encoder if we created one
	if (encoder != nullptr) delete encoder;

	return EXIT_OK;
}

// Perform an encoding operation using the specified command-line options
bool doEncode(CommandLineOptions options)
{
	try
	{
		// Build the encoder from the input file and record how long that takes
		auto ctor_start = chrono::system_clock::now();
		encoder = HuffmanEncoder::InitializeFromFile(options.input);
		auto ctor_end = chrono::system_clock::now();

		size_t read = 0;
		size_t written = 0;

		// Encode the file and record how long that takes
		auto encode_start = chrono::system_clock::now();
		encoder->EncodeFile(options.input, options.output, read, written);
		auto encode_end = chrono::system_clock::now();

		// Calculate the compression ratio
		auto ratio = static_cast<double>(written) / static_cast<double>(read);

		cout << setiosflags(ios::fixed) << setprecision(3);
		cout << "File encoded. In: " << read << " bytes, Out: " << written << " bytes. Ratio: " << ratio;
		cout << "% Time: " << chrono::duration_cast<chrono::duration<float>>(ctor_end - ctor_start).count();
		cout << "s initialization, " << chrono::duration_cast<chrono::duration<float>>(encode_end - encode_start).count();
		cout << "s encode" << endl;
	}
	catch (exception& e)
	{
		cerr << "An error occurred while encoding: " << e.what() << endl;

		return false;
	}
	return true;
}

// Perform a decode operation using the specified options
bool doDecode(CommandLineOptions options)
{
	try
	{
		auto inFile = options.input;
		auto outFile = options.output;

		// If we haven't already encoded a file, then we need to make an empty encoder/decoder
		if (!options.encode)
		{
			encoder = new HuffmanEncoder();
		}
		else
		{
			// If we have, we need to change the output file, since it gets used twice
			// (once to save encoding output, and once to save decoding output)
			inFile = options.output;
			outFile = PrependExtension(options.input, "hz");
		}

		size_t read = 0;
		size_t written = 0;

		// Decode the file and record how long that takes
		auto start = chrono::system_clock::now();
		encoder->DecodeFile(inFile, outFile, read, written);
		auto end = chrono::system_clock::now();

		cout << setiosflags(ios::fixed) << setprecision(3);
		cout << "File decoded. In: " << read << " bytes, Out: " << written << " bytes, Time: ";
		cout << chrono::duration_cast<chrono::duration<float>>(end - start).count() << "s" << endl;
	}
	catch (exception& e)
	{
		cerr << "An error occurred while decoding: " << e.what() << endl;

		return false;
	}
	return true;
}

// Prepend the specified extension before the actual extension of the input string
// Example: PrependExtension("test.txt", "hz")  returns "test.hz.txt"
string PrependExtension(string input, string extension)
{
	auto period = input.find_last_of('.');
	if(period != string::npos)
	{
		return input.substr(0, period) + '.' + extension + input.substr(period);
	}
	
	throw invalid_argument("Unable to find extension in " + input);
}

// Prints the help message to standard output
void printHelp()
{
	cout << "Huffman Encoder and Decoder" << endl;
	cout << "Usage: huffman <options> -i <input_file> -o <output_file>" << endl << endl;

	cout << "Options:" << endl;
	cout << "\t-i, --input\tSpecifies the input file to encode or decode" << endl;
	cout << "\t-o, --output\tSpecifies the output file to encode or decode to" << endl;
	cout << "\t-e, --encode\tEncode <input_file> and write to <output_file>" << endl;
	cout << "\t-d, --decode\tDecode <input_file> and write to <output_file>" << endl;
	cout << "\t-t, --test\tEncode <input_file> to <output>.hz, then decode back to <output_file>" << endl;
	cout << "\t-v, --verbose\tPrint verbose messages" << endl;
	cout << "\t-h, --help\tPrint this help message" << endl;
}

// Processes the command-line arguments and returns a CommandLineOptions struct
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
