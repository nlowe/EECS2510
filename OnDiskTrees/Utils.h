/*
 * Utils.h - Utility functions
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
#include <Shlobj.h>

namespace utils
{
#ifndef PATH_SEPERATOR
#	if defined(_WIN32)
#		define PATH_SEPERATOR '\\'
#	else
#		define PATH_SEPERATOR '/'
#	endif
#endif

	inline std::string join(std::string path, std::string child)
	{
		auto result = path;

		if(result[result.length()-1] != PATH_SEPERATOR)
		{
			result += PATH_SEPERATOR;
		}

		if(child[0] == PATH_SEPERATOR)
		{
			result += child.substr(1);
		}
		else
		{
			result += child;
		}

		return result;
	}

	inline bool CreateDirectories(std::string path)
	{
		return SHCreateDirectoryEx(nullptr, std::wstring(path.begin(), path.end()).c_str(), nullptr) == ERROR_SUCCESS;
	}
}

