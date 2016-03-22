// TreeBenchmarks.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AVL.h"
#include <cassert>

int main()
{
	AVL tree;

	tree.add("3");
	tree.add("2");
	tree.add("1");

	assert(tree.height() == 2);

    return 0;
}

