
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "prefixTree.h"

#ifndef max
#define max(x,y) ((x)>(y)?x:y)
#endif

#define NODES_IN_BLOCK 16
#define NODES_IN_GROUP (4 * 1024 * 1024 / sizeof(treeNode))
#define BLOCKS_IN_GROUP (NODES_IN_GROUP / NODES_IN_BLOCK)
#define EMPTY_ITEM_MASK (1LU << (sizeof(size_t) * 8 - 1))
#define IN_BLOCK_INDEX(str, position) ((str[(position) / 2] >> (((position) & 1) * 4)) & 0xF)

typedef struct treeNode
{
	char* key;
	int32_t hashKey;
	int32_t childsBlockIndex;
	ValueType value;
} treeNode;

typedef struct prefixTree
{
	ValueType emptyKeyValue;
	size_t itemsCount;
	size_t blocksCount;
	size_t blocksRezerved;
	treeNode **groupsOfBlocks;
} prefixTree;

static size_t allocateBlock(prefixTree* tree)
{
	if (tree->blocksRezerved == tree->blocksCount)
	{
		size_t newBlocksRezerved = tree->blocksRezerved + BLOCKS_IN_GROUP;
		size_t newGroupsCount = newBlocksRezerved / BLOCKS_IN_GROUP;

		void* newNodes = realloc(tree->groupsOfBlocks, newGroupsCount * sizeof(treeNode*));
		if (newNodes == NULL)
			return 0;

		tree->groupsOfBlocks = newNodes;

		void* newGroup = calloc(NODES_IN_GROUP, sizeof(treeNode));
		if (newGroup == NULL)
			return 0;

		tree->groupsOfBlocks[newGroupsCount - 1] = newGroup;

		tree->blocksRezerved = newBlocksRezerved;
	}

	return tree->blocksCount++;
}

static int32_t computeHash(const char* key, size_t keyLen)
{
	int32_t hash;
	hash = keyLen * 0x55 ^ 0xe5b5c5;
	for (size_t i = 0; i < keyLen; i++)
		hash += (hash >> 25) + (hash << 7) + key[i];
	return hash;
}

PrefixTree *prefixTree_create()
{
	prefixTree *tree = calloc(1, sizeof(prefixTree));
	if (!tree)
		return NULL;

	tree->blocksCount = 1;
	tree->blocksRezerved = BLOCKS_IN_GROUP;

	tree->groupsOfBlocks = (treeNode**)malloc(1 * sizeof(treeNode*));
	tree->groupsOfBlocks[0] = (treeNode*)calloc(NODES_IN_GROUP, sizeof(treeNode));

	return (PrefixTree*)tree;
}

void prefixTree_free(PrefixTree *ptTree, _Bool fFreeKeys)
{
	if (!ptTree)
		return;

	prefixTree *tree = (prefixTree*)ptTree;
	
	for (size_t i = 0; i < tree->blocksRezerved / BLOCKS_IN_GROUP; i++)
	{
		if (fFreeKeys)
		{
			for (size_t j = 0; j < NODES_IN_GROUP; j++)
				free((void*)tree->groupsOfBlocks[i][j].key);
		}
		free(tree->groupsOfBlocks[i]);
	}

	free(tree->groupsOfBlocks);
	free(tree);
}

_Bool prefixTree_set(const PrefixTree *ptTree, char *const sKey, const ValueType value, _Bool fCopyKey)
{
	if (!ptTree)
		return false;

	if (!sKey)
		return false;

	size_t keyLen = strlen(sKey);

	prefixTree *tree = (prefixTree*)ptTree;
	char *sKey_i = sKey;
	ValueType value_i = value;

	if (keyLen == 0)
	{
		tree->emptyKeyValue = value;
		tree->itemsCount |= EMPTY_ITEM_MASK;
		return true;
	}

	int32_t hash = computeHash(sKey, keyLen);

	_Bool tailRecursion = true;
	while (tailRecursion)
	{
		tailRecursion = false;

		size_t blockIndex = 0;
		for (size_t i = 0; i < keyLen * 2; i++)
		{
			size_t inBlockI = IN_BLOCK_INDEX(sKey_i, i);

			treeNode *node = &tree->groupsOfBlocks[blockIndex / BLOCKS_IN_GROUP][(blockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + inBlockI];

			if (node->key != NULL)
			{
				if (node->hashKey == hash && strcmp(&node->key[i / 2], &sKey_i[i / 2]) == 0)
				{
					node->value = value_i;
					return true;
				}
				else if (node->childsBlockIndex == 0)
				{
					size_t newBlockIndex = allocateBlock(tree);
					if (newBlockIndex == 0)
						return false;

					node = &tree->groupsOfBlocks[blockIndex / BLOCKS_IN_GROUP][(blockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + inBlockI];
					node->childsBlockIndex = newBlockIndex;

					if (strlen(node->key) * 2 - 1 > i)
					{
						// Первый подход работает медленнее, но требует меньше памяти. 
						// Второй — быстрее, но затраты памяти больше. 
						// Разница по памяти и скорости 10-15%
#if 1
						tree->groupsOfBlocks[newBlockIndex / BLOCKS_IN_GROUP][(newBlockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + IN_BLOCK_INDEX(node->key, i + 1)].key = node->key;
						tree->groupsOfBlocks[newBlockIndex / BLOCKS_IN_GROUP][(newBlockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + IN_BLOCK_INDEX(node->key, i + 1)].hashKey = node->hashKey;
						tree->groupsOfBlocks[newBlockIndex / BLOCKS_IN_GROUP][(newBlockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + IN_BLOCK_INDEX(node->key, i + 1)].value = node->value;

						tree->itemsCount++;
						node->value = value_i;

						if (fCopyKey)
						{
							node->key = calloc(keyLen + 1, sizeof(char));
							strcpy((char*)node->key, sKey_i);
						}
						else
						{
							node->key = sKey_i;
						}

						node->hashKey = hash;

						return true;
#else						
						tree->groupsOfBlocks[newBlockIndex / BLOCKS_IN_GROUP][(newBlockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + IN_BLOCK_INDEX(node->key, i + 1)].key = node->key;
						tree->groupsOfBlocks[newBlockIndex / BLOCKS_IN_GROUP][(newBlockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + IN_BLOCK_INDEX(node->key, i + 1)].hashKey = node->hashKey;
						tree->groupsOfBlocks[newBlockIndex / BLOCKS_IN_GROUP][(newBlockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + IN_BLOCK_INDEX(node->key, i + 1)].value = node->value;
						node->key = NULL;
						node->hashKey = 0;
						node->value = NULL;

						tailRecursion = true;
						break;
#endif					
					}
				}
				else if (i + 1 == keyLen * 2)
				{
					char *nkey = node->key;
					int32_t nhash = node->hashKey;
					ValueType nvalue = node->value;

					node->key = sKey_i;
					node->hashKey = hash;
					node->value = value_i;

					sKey_i = nkey;
					value_i = nvalue;
					hash = nhash;
					keyLen = strlen(nkey);

					tailRecursion = true;
					break;
				}
			}
			else
			{
				if (node->childsBlockIndex == 0 || (keyLen * 2 - 1) == i)
				{
					tree->itemsCount++;
					node->value = value_i;

					if (fCopyKey)
					{
						node->key = calloc(keyLen + 1, sizeof(char));
						strcpy((char*)node->key, sKey_i);
					}
					else
					{
						node->key = sKey_i;
					}

					node->hashKey = hash;

					return true;
				}
			}

			blockIndex = node->childsBlockIndex;
		}
	}

	return false;
}

_Bool prefixTree_get(const PrefixTree *ptTree, const char* sKey, ValueType* pValue)
{
	if (!ptTree)
		return false;

	if (!sKey || !pValue)
		return false;

	prefixTree *tree = (prefixTree*)ptTree;

	size_t keyLen = strlen(sKey);
	int32_t hash = computeHash(sKey, keyLen);

	if (keyLen == 0)
	{
		if (tree->itemsCount & EMPTY_ITEM_MASK)
		{
			*pValue = tree->emptyKeyValue;
			return true;
		}

		return false;
	}

	size_t blockIndex = 0;
	treeNode **nodes = tree->groupsOfBlocks;
	for (size_t i = 0; i < keyLen * 2; i++)
	{
		size_t inBlockI = IN_BLOCK_INDEX(sKey, i);
		treeNode *node = &nodes[blockIndex / BLOCKS_IN_GROUP][(blockIndex % BLOCKS_IN_GROUP) * NODES_IN_BLOCK + inBlockI];

		if (node->key != NULL)
		{
			if (node->hashKey == hash && strcmp(&node->key[i / 2], &sKey[i / 2]) == 0)
			{
				*pValue = node->value;
				return true;
			}
			else if (node->childsBlockIndex == 0)
			{
				return false;
			}
		}
		else if (node->childsBlockIndex == 0)
		{
			return false;
		}

		blockIndex = node->childsBlockIndex;
	}

	return false;
}

size_t prefixTree_count(const PrefixTree *const ptTree)
{
	if (!ptTree)
		return false;

	prefixTree *tree = (prefixTree*)ptTree;

	return (tree->itemsCount & ~EMPTY_ITEM_MASK) + ((tree->itemsCount & EMPTY_ITEM_MASK) ? 1 : 0);
}
