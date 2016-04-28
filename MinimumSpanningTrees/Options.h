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
	// The path to the graph
	std::string TestFilePath = "";

	// Whether or not the help menu was requested
	bool help = false;
	// Whether or not errors were encountered while parsing arguments
	bool errors = false;
	// Whether or not to enable verbose mode
	bool verboseEnable = false;

	// Any errors encountered while parsing arguments
	std::string errorMessage = "";

	explicit Options(int argc, char* argv[])
	{
		for (auto i = 1; i < argc; i++)
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
			else if(arg == "-v" || arg == "--verbose")
			{
				verboseEnable = true;
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
