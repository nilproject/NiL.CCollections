#include <stdlib.h>

#include "Utils.h"

intptr_t binarySearchMore(const intptr_t *const pArray, const size_t arrayLength, const intptr_t x)
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

	while (start != end)
	{
		intptr_t item = pArray[index];

		if (item > x)
		{
			end = index;
		}
		else if (item < x)
		{
			start = index;
		}
		else
		{
			while (index < arrayLength && pArray[index] == x)
				index++;

			if (index == arrayLength)
				return -1;
			return index;
		}

		index = start + ((end - start) >> 1);
	}

	return -1;
}