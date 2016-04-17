/*
 * Options.h - Definition for the Command-Line options struct
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

// Parses any options passed on the command line
struct Options
{
	// The path to the file to benchmark against
	std::string TestFilePath = "";
	// The number of random words to insert
	size_t RandomCount = 0;
	// The size of random words to insert
	size_t RandomSize = 0;
	// The T-Factor (Degree) of the B-Tree
	uint16_t TFactor = 3;
	// The maximum key size for keys that will be inserted into the b-tree
	uint16_t MaxKeySize = 32;

	// Whether or not the help menu was requested
	bool help = false;
	// Whether or not errors were encountered while parsing arguments
	bool errors = false;
	// Whether or not the data should be output in CSV format
	bool csvMode = false;
	// Whether or not the CSV headers should not be included
	bool noHeaders = false;
	// Whether or not to keep existing trees on disk before running benchmarks
	bool keepExisting = false;
	// Whether or not to run an in-order traversal on each tree after benchmarking
	bool quiet = false;

	// Any errors encountered while parsing arguments
	std::string errorMessage = "";

	explicit Options(int argc, char* argv[])
	{
		for (int i = 1; i < argc; i++)
		{
			std::string arg = argv[i];

			if(arg == "-h" || arg == "--help")
			{
				help = true;
			}
			else if(arg == "-f" || arg == "--file")
			{
				if(i < argc-1)
				{
					TestFilePath = argv[++i];
				}
				else
				{
					errors = true;
					errorMessage += "\t* ";
					errorMessage += arg;
					errorMessage += ": Not enough parameters (must be <string>)\n";
				}
			}
			else if(arg == "-r" || arg == "--random-count")
			{
				if (i < argc - 1)
				{
					try
					{
						RandomCount = std::stoi(argv[++i]);
					}
					catch(std::exception ex)
					{
						errors = true;
						errorMessage += "\t* ";
						errorMessage += arg;
						errorMessage += ": Unable to parse argument (";
						errorMessage += ex.what();
						errorMessage += ")";
					}
				}
				else
				{
					errors = true;
					errorMessage += "\t* ";
					errorMessage += arg;
					errorMessage += ": Not enough parameters (must be <string>)\n";
				}
			}
			else if(arg == "-s" || arg == "--random-size")
			{
				if (i < argc - 1)
				{
					try
					{
						RandomSize = std::stoi(argv[++i]);
					}
					catch (std::exception ex)
					{
						errors = true;
						errorMessage += "\t* ";
						errorMessage += arg;
						errorMessage += ": Unable to parse argument (";
						errorMessage += ex.what();
						errorMessage += ")";
					}
				}
				else
				{
					errors = true;
					errorMessage += "\t* ";
					errorMessage += arg;
					errorMessage += ": Not enough parameters (must be <string>)\n";
				}
			}
			else if(arg == "-c" || arg == "--csv")
			{
				csvMode = true;
			}
			else if(arg == "-n" || arg == "--no-headers")
			{
				csvMode = noHeaders = true;
			}
			else if(arg == "-k" || arg == "--keep")
			{
				keepExisting = true;
			}
			else if(arg == "-q" || arg == "--quiet")
			{
				quiet = true;
			}
			else if(arg == "-d" || arg == "--degree")
			{
				if (i < argc - 1)
				{
					try
					{
						TFactor = static_cast<uint16_t>(std::stoul(argv[++i]));
						if(TFactor < 2)
						{
							errors = true;
							errorMessage += "\t* ";
							errorMessage += arg;
							errorMessage += ": B-Tree degree must be at least 2";
						}
					}
					catch (std::exception ex)
					{
						errors = true;
						errorMessage += "\t* ";
						errorMessage += arg;
						errorMessage += ": Unable to parse argument (";
						errorMessage += ex.what();
						errorMessage += ")";
					}
				}
				else
				{
					errors = true;
					errorMessage += "\t* ";
					errorMessage += arg;
					errorMessage += ": Not enough parameters (must be <string>)\n";
				}
			}
			else if(arg == "-m" || arg == "--max-key-size")
			{
				if (i < argc - 1)
				{
					try
					{
						MaxKeySize = static_cast<uint16_t>(std::stoul(argv[++i]));
					}
					catch (std::exception ex)
					{
						errors = true;
						errorMessage += "\t* ";
						errorMessage += arg;
						errorMessage += ": Unable to parse argument (";
						errorMessage += ex.what();
						errorMessage += ")";
					}
				}
				else
				{
					errors = true;
					errorMessage += "\t* ";
					errorMessage += arg;
					errorMessage += ": Not enough parameters (must be <string>)\n";
				}
			}
			else
			{
				errors = true;
				errorMessage += "\t* ";
				errorMessage += arg;
				errorMessage += ": unrecognized argument\n";
			}
		}
	}
};
