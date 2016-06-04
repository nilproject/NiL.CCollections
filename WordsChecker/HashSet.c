#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>

#include "HashSet.h"

#ifndef max
#define max(x,y) ((x)>(y)?x:y)
#endif

#define NODES_IN_GROUP (1 << 17)
#define record(index) groupsOfNodes[index / NODES_IN_GROUP][index % NODES_IN_GROUP]

typedef struct node
{
	int32_t hash;
	int32_t next;
	char *key;
} node;

typedef struct hashSet
{
	_Bool emptyKeyValueExists;
	size_t itemsCount;
	size_t nodesAllocated;
	node **groupsOfNodes;
} hashSet;

static _Bool increaseSize(hashSet *const set)
{
	size_t newNodesAllocated = set->nodesAllocated << 1;
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

	node **oldNodes = set->groupsOfNodes;
	set->groupsOfNodes = newNodes;
	set->nodesAllocated = newNodesAllocated;
	set->itemsCount = 0;

	for (size_t i = 0; i < newGroupsCount >> 1; i++)
	{
		for (size_t j = 0; j < NODES_IN_GROUP; j++)
		{
			if (oldNodes[i][j].key != NULL)
			{
				hashSet_insert((HashSet*)set, oldNodes[i][j].key, false);
			}
		}

		free(oldNodes[i]);
	}

	free(oldNodes);

	return true;
}

static inline int32_t computeHash(const char *const key, const size_t keyLen)
{
	int32_t hash;
	hash = keyLen * 0x55 ^ 0xe5b5c5;
	for (size_t i = 0; i < keyLen; i++)
		hash += ((hash >> 25) + (hash << 7)) ^ key[i];
	return hash;
}

extern HashSet *hashSet_create()
{
	hashSet *tree = calloc(1, sizeof(hashSet));
	if (!tree)
		return NULL;

	tree->nodesAllocated = NODES_IN_GROUP;
	tree->itemsCount = 0;
	tree->groupsOfNodes = (node**)malloc(1 * sizeof(node*));
	tree->groupsOfNodes[0] = (node*)calloc(NODES_IN_GROUP, sizeof(node));

	return (HashSet*)tree;
}

extern void hashSet_free(HashSet *const pHashSet, _Bool fFreeKeys)
{
	if (!pHashSet)
		return;

	hashSet *set = (hashSet*)pHashSet;

	for (size_t i = 0; i < set->nodesAllocated / NODES_IN_GROUP; i++)
	{
		if (fFreeKeys)
		{
			for (size_t j = 0; j < NODES_IN_GROUP; j++)
				free(set->groupsOfNodes[i][j].key);
		}

		free(set->groupsOfNodes[i]);
	}

	free(set->groupsOfNodes);
	free(set);
}

extern _Bool hashSet_insert(HashSet *const pHashSet, char *const sKey, const _Bool fCopyKey)
{
	if (!pHashSet)
		return false;

	if (!sKey)
		return false;

	size_t keyLen = strlen(sKey);

	hashSet *set = (hashSet*)pHashSet;

	if (keyLen == 0)
	{
		set->emptyKeyValueExists = true;
		return true;
	}

	int32_t hash = computeHash(sKey, keyLen);

	int32_t mask = set->nodesAllocated - 1;
	int32_t index = hash & mask;
	int32_t colisionCount = 0;

	do
	{
		if (set->record(index).hash == hash && strcmp(set->record(index).key, sKey) == 0)
		{
			return true;
		}

		index = set->record(index).next - 1;
	} 
	while (index >= 0);

	if ((set->itemsCount > 50 && set->itemsCount * 7 / 4 >= mask) || set->itemsCount == mask + 1)
	{
		if (!increaseSize(set))
			return false;

		mask = set->nodesAllocated - 1;
	}

	int prewIndex = -1;
	index = hash & mask;

	if (set->record(index).key != NULL)
	{
		while (set->record(index).next > 0)
		{
			index = set->record(index).next - 1;
			colisionCount++;
		}

		prewIndex = index;
		while (set->record(index).key != NULL)
			index = (index + 3) & mask;
	}

	set->record(index).hash = hash;
	if (fCopyKey)
	{
		set->record(index).key = calloc(keyLen + 1, sizeof(char));
		strcpy(set->record(index).key, sKey);
	}
	else
	{
		set->record(index).key = sKey;
	}

	if (prewIndex >= 0)
		set->record(prewIndex).next = index + 1;

	set->itemsCount++;

	if (colisionCount > 29)
		increaseSize(set);

	return true;
}

extern _Bool hashSet_contains(const HashSet *pHashSet, const char *const sKey)
{
	if (!pHashSet)
		return false;

	if (!sKey)
		return false;

	hashSet *set = (hashSet*)pHashSet;

	size_t keyLen = strlen(sKey);

	if (keyLen == 0)
	{
		if (set->emptyKeyValueExists)
		{
			return true;
		}

		return false;
	}

	int32_t elen = set->nodesAllocated - 1;
	if (set->nodesAllocated == 0)
		return false;

	int32_t hash = computeHash(sKey, keyLen);
	int32_t index = hash & elen;

	do
	{
		node *node = &set->record(index);
		if (node->hash == hash && strcmp(node->key, sKey) == 0)
		{
			return true;
		}

		index = node->next - 1;
	} 
	while (index >= 0);

	return false;
}

extern size_t hashSet_count(const HashSet *ptTree)
{
	if (!ptTree)
		return false;

	hashSet *tree = (hashSet*)ptTree;

	return tree->itemsCount + (tree->emptyKeyValueExists ? 1 : 0);
}
