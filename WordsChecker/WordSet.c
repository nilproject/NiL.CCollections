#include <stdlib.h>
#include <stdbool.h>

#include "WordSet.h"

#define NODES_IN_BLOCK 16
#define NODES_IN_GROUP (4 * 1024 * 1024 / sizeof(node))
#define BLOCKS_IN_GROUP (NODES_IN_GROUP / NODES_IN_BLOCK)
#define IN_BLOCK_INDEX(str, position) ((str[(position) / 2] >> (((position) & 1) * 4)) & 0xF)

typedef struct node
{
	size_t prev;
	size_t next;
} node;

typedef struct wordSet
{
	node **graph;
	size_t blocksCount;
	size_t blocksRezerved;
	bool emptyKeyContains;
} wordSet;

static size_t allocateBlock(wordSet* set)
{
	if (set->blocksRezerved == set->blocksCount)
	{
		size_t newBlocksRezerved = set->blocksRezerved + BLOCKS_IN_GROUP;
		size_t newGroupsCount = newBlocksRezerved / BLOCKS_IN_GROUP;

		void* newNodes = realloc(set->graph, newGroupsCount * sizeof(node*));
		if (newNodes == NULL)
			return 0;

		set->graph = newNodes;

		void* newGroup = calloc(NODES_IN_GROUP, sizeof(node));
		if (newGroup == NULL)
			return 0;

		set->graph[newGroupsCount - 1] = newGroup;

		set->blocksRezerved = newBlocksRezerved;
	}

	return set->blocksCount++;
}

WordSet *wordSet_create()
{
	wordSet *set = malloc(sizeof(wordSet));
	if (!set)
		return NULL;

	set->blocksCount = 2;
	set->blocksRezerved = BLOCKS_IN_GROUP;
	set->graph = calloc(NODES_IN_GROUP, sizeof(node));
	set->graph[0] = calloc(NODES_IN_GROUP, sizeof(node));
	set->graph[1] = calloc(NODES_IN_GROUP, sizeof(node));
}

void wordSet_free(wordSet *const pSet)
{
	if (!pSet)
		return;

	wordSet *set = (wordSet*)pSet;

	for (size_t i = 0; i < set->blocksRezerved / BLOCKS_IN_GROUP; i++)
	{
		free(set->graph[i]);
	}

	free(set->graph);
	free(set);
}

bool wordSet_insert(wordSet *const pSet, const char *const sKey)
{
	if (!pSet || !sKey)
		return false;

	size_t keyLen = strlen(sKey);
	wordSet *set = (wordSet*)pSet;

	if (keyLen == 0)
	{
		set->emptyKeyContains = true;
		return true;
	}

	// Находим существующий суфикс
	for (size_t i = keyLen * 2 - 1; i > 0; i--)
	{

	}
}