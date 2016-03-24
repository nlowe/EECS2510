#include "stdafx.h"
#include "CppUnitTest.h"

#include "AVL.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CustomDataTypesTests
{		
	TEST_CLASS(AVLTrees)
	{
	public:
		TEST_METHOD_INITIALIZE(setup)
		{
			this->tree = new AVL;
		}

		TEST_METHOD_CLEANUP(teardown)
		{
			delete this->tree;
		}

		TEST_METHOD(TestMethod1)
		{
			this->tree->add("foobar");

			Assert::AreEqual(1ULL, this->tree->height());
		}

		TEST_METHOD(FailingTest)
		{
			Assert::AreEqual(1, 2);
		}
	private:
		AVL* tree;
	};
}