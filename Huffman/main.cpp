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
#include "HuffmanEncoder.h"

using namespace std;

HuffmanEncoder* encoder = nullptr;

static const int EXIT_OK = 0;
static const int EXIT_BAD_ARGUMENTS = -1;
static const int EXIT_ENCODE_FAILED = -2;
static const int EXIT_DECODE_FAILED = -3;

struct CommandLineOptions
{
	bool parseError = false;
	bool printHelp = false;
	bool encode = false;
	bool decode = false;
	bool verbose = false;
	string input = "";
	string output = "";

	string ToString() const
	{
		string result = "Parse Error: ";
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

string PrependExtension(string input, string extension)
{
	auto period = input.find_last_of('.');
	if(period != string::npos)
	{
		return input.substr(0, period) + '.' + extension + input.substr(period);
	}
	
	throw std::invalid_argument("Unable to find extension in " + input);
}

inline void verbose(CommandLineOptions opts, string msg)
{
	if (opts.verbose) cout << msg << endl;
}

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
	
	verbose(options, options.ToString());

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

	if (options.encode)
	{
		encoder = HuffmanEncoder::ForFile(options.input);
		
		size_t read = 0;
		size_t written = 0;

		try
		{
			encoder->Encode(options.input, options.output, read, written);

			auto ratio = static_cast<double>(written) / static_cast<double>(read);

			cout << "File encoded. In: " << read << " bytes, Out: " << written << " bytes. Ratio: " << ratio << endl;
		}
		catch(exception& e)
		{
			cerr << "An error occurred while encoding: " << e.what() << endl;

			return EXIT_ENCODE_FAILED;
		}
	}

	if (options.decode)
	{
		string inFile = options.input;
		string outFile = options.output;
			
		if (!options.encode)
		{
			encoder = new HuffmanEncoder();
		}
		else
		{
			inFile = options.output;
			outFile = PrependExtension(options.input, "hz");
		}

		size_t read = 0;
		size_t written = 0;

		try
		{
			encoder->Decode(inFile, outFile, read, written);

			cout << "File decoded. In: " << read << " bytes, Out: " << written << " bytes" << endl;
		}
		catch (exception& e)
		{
			cerr << "An error occurred while decoding: " << e.what() << endl;

			return EXIT_ENCODE_FAILED;
		}
	}

	if (encoder != nullptr)
	{
		delete encoder;
	}

    return EXIT_OK;
}
