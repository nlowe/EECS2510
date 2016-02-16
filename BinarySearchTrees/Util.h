#pragma once
#include <algorithm>

// Converts the specified string to lower case in-place based on the current collation
static void str_to_lower(std::string& input)
{
	transform(input.begin(), input.end(), input.begin(), tolower);
}
