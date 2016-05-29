#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <intrin.h>

#include "HashMap.h"

#ifndef max
#define max(x,y) ((x)>(y)?x:y)
#endif

#define NODES_IN_GROUP (1 << 17) // должно быть степенью двойки
#define record(index) groupsOfNodes[index / NODES_IN_GROUP][index % NODES_IN_GROUP]

typedef struct node
{
	int32_t hash;
	int32_t next;
	char* key;
	ValueType value;
} node;

typedef struct hashMap
{
	_Bool emptyKeyValueExists;
	ValueType emptyKeyValue;
	size_t itemsCount;
	size_t nodesAllocated;
	node **groupsOfNodes;
} hashMap;

_Bool increaseSize(hashMap* map)
{
	size_t newNodesAllocated = map->nodesAllocated << 1;
	size_t newGroupsCount = newNodesAllocated / NODES_IN_GROUP;

	node **newNodes = malloc(newGroupsCount * sizeof(node*));
	if (newNodes == NULL)
		return false;

	for (size_t i = 0; i < newGroupsCount; i++)
	{
		void* newGroup = calloc(NODES_IN_GROUP, sizeof(node));
		if (newGroup == NULL)
		{
			while (i-- > 0)
				free(newNodes[i]);
			free(newNodes);

			return false;
		}

		newNodes[i] = newGroup;
	}

	node **oldNodes = map->groupsOfNodes;
	map->groupsOfNodes = newNodes;
	map->nodesAllocated = newNodesAllocated;
	map->itemsCount = 0;

	for (size_t i = 0; i < newGroupsCount >> 1; i++)
	{
		for (size_t j = 0; j < NODES_IN_GROUP; j++)
		{
			if (oldNodes[i][j].key != NULL)
			{
				hashMap_insert(map, oldNodes[i][j].key, oldNodes[i][j].value, false);
			}
		}

		free(oldNodes[i]);
	}

	free(oldNodes);

	return true;
}

static inline int32_t computeHash(char* key, size_t keyLen)
{
	int32_t hash;
	hash = keyLen * 0x55 ^ 0xe5b5c5;
	for (size_t i = 0; i < keyLen; i++)
		hash += (hash >> 25) + (hash << 7) ^ key[i];
	return hash;
}

HashMap *hashMap_create()
{
	hashMap *tree = calloc(1, sizeof(hashMap));
	if (!tree)
		return NULL;

	tree->nodesAllocated = NODES_IN_GROUP;
	tree->itemsCount = 0;
	tree->groupsOfNodes = (node**)malloc(1 * sizeof(node*));
	tree->groupsOfNodes[0] = (node*)calloc(NODES_IN_GROUP, sizeof(node));

	return tree;
}

void hashMap_free(HashMap *pHashMap, _Bool fFreeKeys)
{
	if (!pHashMap)
		return;

	hashMap *map = pHashMap;

	for (size_t i = 0; i < map->nodesAllocated / NODES_IN_GROUP; i++)
	{
		if (fFreeKeys)
		{
			for (size_t j = 0; j < NODES_IN_GROUP; j++)
				free(map->groupsOfNodes[i][j].key);
		}

		free(map->groupsOfNodes[i]);
	}

	free(map->groupsOfNodes);
	free(map);
}

_Bool hashMap_insert(const HashMap *pHashMap, const char* sKey, const ValueType value, _Bool fCopyKey)
{
	if (!pHashMap)
		return false;

	if (!sKey)
		return false;

	size_t keyLen = strlen(sKey);

	hashMap *map = pHashMap;
	char *sKey_i = sKey;
	ValueType value_i = value;

	if (keyLen == 0)
	{
		map->emptyKeyValue = value;
		map->emptyKeyValueExists = true;
		return true;
	}

	int32_t hash = computeHash(sKey, keyLen);

	int32_t mask = map->nodesAllocated - 1;
	int32_t index = hash & mask;
	int32_t colisionCount = 0;

	do
	{
		if (map->record(index).hash == hash && strcmp(map->record(index).key, sKey_i) == 0)
		{
			map->record(index).value = value;
			return true;
		}

		index = map->record(index).next - 1;
	} 
	while (index >= 0);

	if ((map->itemsCount > 50 && map->itemsCount * 7 / 4 >= mask) || map->itemsCount == mask + 1)
	{
		if (!increaseSize(map))
			return false;

		mask = map->nodesAllocated - 1;
	}

	int prewIndex = -1;
	index = hash & mask;

	if (map->record(index).key != NULL)
	{
		while (map->record(index).next > 0)
		{
			index = map->record(index).next - 1;
			colisionCount++;
		}

		prewIndex = index;
		while (map->record(index).key != NULL)
			index = (index + 3) & mask;
	}

	map->record(index).value = value;
	map->record(index).hash = hash;
	if (fCopyKey)
	{
		map->record(index).key = calloc(keyLen + 1, sizeof(char));
		strcpy(&map->record(index).key, sKey_i);
	}
	else
	{
		map->record(index).key = sKey;
	}

	if (prewIndex >= 0)
		map->record(prewIndex).next = index + 1;

	map->itemsCount++;

	if (colisionCount > 29)
		increaseSize(map);

	return true;
}

_Bool hashMap_get(const HashMap *pHashMap, const char* sKey, ValueType* value)
{
	if (!pHashMap)
		return false;

	if (!sKey || !value)
		return false;

	hashMap *map = pHashMap;

	size_t keyLen = strlen(sKey);

	if (keyLen == 0)
	{
		if (map->emptyKeyValueExists)
		{
			*value = map->emptyKeyValue;
			return true;
		}

		return false;
	}

	int32_t elen = map->nodesAllocated - 1;
	if (map->nodesAllocated == 0)
		return false;

	int32_t hash = computeHash(sKey, keyLen);
	int32_t index = hash & elen;

	do
	{
		node *node = &map->record(index);
		if (node->hash == hash && strcmp(node->key, sKey) == 0)
		{
			*value = node->value;
			return true;
		}

		index = node->next - 1;
	} 
	while (index >= 0);

	return false;
}

size_t hashMap_count(const HashMap *ptTree)
{
	if (!ptTree)
		return false;

	hashMap *tree = ptTree;

	return tree->itemsCount + (tree->emptyKeyValueExists ? 1 : 0);
}