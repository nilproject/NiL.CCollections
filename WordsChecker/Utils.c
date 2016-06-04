#include <stdlib.h>
#include <stddef.h>

#include "Utils.h"

extern ptrdiff_t binarySearchMore(const ptrdiff_t *const pArray, const size_t arrayLength, const ptrdiff_t x)
{
	if (!pArray || arrayLength <= 0)
		return -1;

	size_t start = 0;
	size_t end = arrayLength;
	size_t index = arrayLength >> 1;

	if (end <= 3)
	{
		for (size_t i = 0; i < end; i++)
		{
			if (pArray[i] > x)
				return i;
		}

		return -1;
	}

	if (pArray[0] > x)
		return 0;

	if (pArray[end - 1] <= x)
		return -1;

	for (;;)
	{
		ptrdiff_t item = pArray[index];

		if (end - start == 1)
		{
			if (item <= x)
				index++;
			break;
		}

		if (item > x)
		{
			end = index;
		}
		else if (item <= x)
		{
			start = index;
		}

		index = start + ((end - start) >> 1);
	}

	return index;
}
