#include <stdlib.h>
#include <stdio.h>

#if _WIN32 || _WIN64
#include <sys\timeb.h>
#include <Windows.h>
#endif

#include <string.h>
#include <time.h>
#include "prefixTree.h"

bool gotoxy(short x, short y)
{
#if _WIN32 || _WIN64
	COORD coord = { .X = x,.Y = y };
	return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
	printf("%c[%d;%df",0x1B,y,x);
	return true;
#endif
}

char *fgetline(FILE *file)
{
	char buffer[128];
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

int main()
{
	FILE *file = fopen("words.txt", "r");

	if (file == NULL)
	{
		printf("Unable to open file");
		return 1;
	}

	fpos_t fileSize;
	fpos_t filePos;
	time_t lastUpdated = 0;
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
		prefixTree_set(tree, line, (ValueType)linesCount, false);
		lines[linesCount++] = line;

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
		prefixTree_get(tree, lines[i], &value);

		if ((size_t)value != i)
		{
			printf("\nERROR!");
			getchar();
			return 1;
		}
	}
#if _WIN32 || _WIN64
	ftime(&bend);
	time_t time = (bend.time - bstart.time) * 1000 + bend.millitm - bstart.millitm;
	printf("\nTime: %lu (%f per line)", time, (float)time / linesCount);
#endif

	free(lines);

	printf("\nComplite");
	
	getchar();

	prefixTree_free(tree, true);

	getchar();

	return 0;
}
