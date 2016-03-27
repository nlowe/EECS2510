// TreeBenchmarks.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AVL.h"
#include <iostream>
#include <vector>
#include <ctime>
#include "RBT.h"

int main()
{
	BST bstree;
	AVL tree;
	RBT rbt;

	srand(unsigned(time(nullptr)));
	auto numbers = std::vector<std::string>();

	for (int i = 0; i < 8192; i++)
	{
		numbers.push_back(std::to_string(i));
	}

	random_shuffle(numbers.begin(), numbers.end());

	for (auto it = numbers.begin(); it != numbers.end(); ++it)
	{
		tree.add(*it);
		bstree.add(*it);
		rbt.add(*it);
	}

	std::cout << "AVL Tree Height: " << tree.height() << ", RBT Height: " << rbt.height() << ", BST Height: " << bstree.height() << std::endl;

    return 0;
}

