#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

#if _WIN32 || _WIN64
#include <intrin.h>
#include <sys\timeb.h>
#include <Windows.h>
#endif

#include "prefixTree.h"
#include "HashMap.h"
#include "HashSet.h"
#include "Utils.h"

bool gotoxy(short x, short y)
{
#if _WIN32 || _WIN64
	COORD coord = { .X = x, .Y = y };
	return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
	printf("%c[%d;%df", 0x1B, y, x);
	return true;
#endif
}

char *fgetline(FILE *file)
{
	char buffer[1024];
	char *result = NULL;
	size_t resultLen = 0;

	for (;;)
	{
		char *s = fgets(buffer, sizeof(buffer) / sizeof(*buffer), file);
		if (s == NULL)
			return result;

		size_t len = strlen(s);
		if (len == 0)
			return calloc(1, sizeof(char));

		int endOfLine = (s[len - 1] == '\n' || s[len - 1] == '\r') ? 1 : 0;
		len -= endOfLine;
		s[len] = 0;

		result = realloc(result, (resultLen + len + 1) * sizeof(char));
		strcpy(&result[resultLen], s);
		resultLen += len;

		if (endOfLine || feof(file))
			return result;
	}
}

int main_pt()
{
	FILE *file = fopen("words.txt", "r");

	if (file == NULL)
	{
		printf("Unable to open file");
		return 1;
	}

	fpos_t fileSize;
	fseek(file, 0, SEEK_END);
	fgetpos(file, &fileSize);
	fseek(file, 0, SEEK_SET);

	PrefixTree *tree = prefixTree_create();
	char *line = NULL;
	size_t linesAllocated = 128;
	char **lines = calloc(128, sizeof(char*));
	size_t linesCount = 0;

	printf("Loading");

	while (!feof(file))
	{
		line = fgetline(file);
		if (line == NULL)
			break;

		if (!prefixTree_set(tree, line, (ValueType)linesCount, false))
		{
			printf("\nout-of-memory (%zd)", linesCount);
			getchar();
			return 1;
		}

		lines[linesCount++] = line;

		ValueType value;
		if (!prefixTree_get(tree, line, &value) || (size_t)value != linesCount - 1)
		{
			printf("\nERROR!");
			getchar();
			return 1;
		}

		if (linesCount == linesAllocated)
		{
			lines = realloc(lines, (linesAllocated *= 2) * sizeof(char*));
		}
	}

	fclose(file);

	printf("\nNumber of lines: %zu", linesCount);

#if _WIN32 || _WIN64
	struct timeb bstart, bend;
	ftime(&bstart);
#endif
	for (size_t i = 0; i < linesCount; i++)
	{
		ValueType value;
		if (!prefixTree_get(tree, lines[i], &value) || (size_t)value != i)
		{
			printf("\nERROR!");
			getchar();
			return 1;
		}
	}
#if _WIN32 || _WIN64
	ftime(&bend);
	time_t time = (bend.time - bstart.time) * 1000 + bend.millitm - bstart.millitm;
	printf("\nTime: %i (%f per line)", (int32_t)time, (float)time / (float)linesCount);
#endif

	free(lines);

	printf("\nComplite");

	getchar();

	prefixTree_free(tree, true);

	getchar();

	return 0;
}

int main_hm()
{
	FILE *file = fopen("words.txt", "r");

	if (file == NULL)
	{
		printf("Unable to open file");
		return 1;
	}

	fpos_t fileSize;
	fseek(file, 0, SEEK_END);
	fgetpos(file, &fileSize);
	fseek(file, 0, SEEK_SET);

	HashMap *map = hashMap_create();
	char *line = NULL;
	size_t linesAllocated = 128;
	char **lines = calloc(128, sizeof(char*));
	size_t linesCount = 0;

	printf("Loading");

	while (!feof(file))
	{
		//if (linesCount >= 5000000)
		//	break;

		line = fgetline(file);
		if (line == NULL)
			break;

		if (!hashMap_insert(map, line, (ValueType)linesCount, false))
		{
			printf("\nout-of-memory (%zd)", linesCount);
			getchar();
			return 1;
		}

		lines[linesCount++] = line;

		ValueType value;
		if (!hashMap_get(map, line, &value) || (size_t)value != linesCount - 1)
		{
			printf("\nERROR!");
			getchar();
			return 1;
		}

		if (linesCount == linesAllocated)
		{
			lines = realloc(lines, (linesAllocated *= 2) * sizeof(char*));
		}
	}

	fclose(file);

	printf("\nNumber of lines: %zu", linesCount);

#if _WIN32 || _WIN64
	struct timeb bstart, bend;
	ftime(&bstart);
#endif
	for (size_t i = 0; i < linesCount; i++)
	{
		ValueType value;
		if (!hashMap_get(map, lines[i], &value) || (size_t)value != i)
		{
			printf("\nERROR!");
			getchar();
			return 1;
		}
	}
#if _WIN32 || _WIN64
	ftime(&bend);
	time_t time = (bend.time - bstart.time) * 1000 + bend.millitm - bstart.millitm;
	printf("\nTime: %i (%f per line)", (int32_t)time, (float)time / (float)linesCount);
#endif

	free(lines);

	printf("\nComplite");

	getchar();

	hashMap_free(map, true);

	getchar();

	return 0;
}

int main_hs()
{
	FILE *file = fopen("words.txt", "r");

	if (file == NULL)
	{
		printf("Unable to open file");
		return 1;
	}

	fpos_t fileSize;
	fseek(file, 0, SEEK_END);
	fgetpos(file, &fileSize);
	fseek(file, 0, SEEK_SET);

	HashSet *set = hashSet_create();
	char *line = NULL;
	size_t linesAllocated = 128;
	char **lines = calloc(128, sizeof(char*));
	size_t linesCount = 0;

	printf("Loading");

	while (!feof(file))
	{
		//if (linesCount >= 5000000)
		//	break;

		line = fgetline(file);
		if (line == NULL)
			break;

		if (!hashSet_insert(set, line, false))
		{
			printf("\nout-of-memory (%zd)", linesCount);
			getchar();
			return 1;
		}

		lines[linesCount++] = line;

		if (!hashSet_contains(set, line))
		{
			printf("\nERROR!");
			getchar();
			return 1;
		}

		if (linesCount == linesAllocated)
		{
			lines = realloc(lines, (linesAllocated *= 2) * sizeof(char*));
		}
	}

	fclose(file);

	printf("\nNumber of lines: %zu", linesCount);

#if _WIN32 || _WIN64
	struct timeb bstart, bend;
	ftime(&bstart);
#endif
	for (size_t i = 0; i < linesCount; i++)
	{
		if (!hashSet_contains(set, lines[i]))
		{
			printf("\nERROR!");
			getchar();
			return 1;
		}
	}
#if _WIN32 || _WIN64
	ftime(&bend);
	time_t time = (bend.time - bstart.time) * 1000 + bend.millitm - bstart.millitm;
	printf("\nTime: %i (%f per line)", (int32_t)time, (float)time / (float)linesCount);
#endif

	free(lines);

	printf("\nComplite");

	getchar();

	hashSet_free(set, true);

	getchar();

	return 0;
}

int main_bs()
{
	size_t len = 10000;
	intptr_t *array = calloc(len, sizeof(intptr_t));
	intptr_t index;

	for (size_t i = 0; i < len; i++)
	{
		array[i] = ((i + (i / (len / 5)) * 3) / 4) * 4;
	}

#if _WIN32 || _WIN64
	struct timeb bstart, bend;
	ftime(&bstart);
#endif

	for (int loop = 1; loop < len; loop++)
		for (int i = 0; i < loop + 16; i++)
		{
			index = binarySearchMore(array, loop, i - 1);

			if (index >= 0 && array[index] <= i - 1)
			{
				printf("Error #1: %i/%i\n", i, loop);
				break;
			}

			if (index > 0 && array[index - 1] > i - 1)
			{
				printf("Error #2: %i/%i\n", i, loop);
				break;
			}
		}

#if _WIN32 || _WIN64
	ftime(&bend);
	time_t time = (bend.time - bstart.time) * 1000 + bend.millitm - bstart.millitm;
	printf("Time: %i\n", (int32_t)time);
#endif

	printf("%zi", index);
	getchar();
}

void main()
{
}
