#pragma once
namespace utils
{
	template <typename T> static void swap(T*& a, T*& b)
	{
		T* c = b;
		b = a;
		a = c;
	}
}