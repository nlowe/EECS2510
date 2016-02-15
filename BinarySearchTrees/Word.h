#pragma once
#include <string>

// A structure for counting the number of times a word appears
struct Word
{
	// The string representation of the word
	std::string key;
	// The number of times the word has occurred
	uint64_t count;

	explicit Word(std::string w, uint64_t c)
	{
		key = w;
		count = c;
	}
};
