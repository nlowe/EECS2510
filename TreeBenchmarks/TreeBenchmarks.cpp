// TreeBenchmarks.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AVL.h"
#include <cassert>

int main()
{
	AVL tree;

	tree.add("4");
	tree.add("1");
	tree.add("2");
	tree.add("3");


	tree.debug_PrintBalanceFactors();

	assert(tree.height() == 3);

    return 0;
}

