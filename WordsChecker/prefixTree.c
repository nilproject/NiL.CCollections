
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "prefixTree.h"

#ifndef max
#define max(x,y) ((x)>(y)?x:y)
#endif

#define EMPTY_ITEM_MASK (1 << (sizeof(size_t) * 8 - 1))
#define IN_BLOCK_INDEX(str, position) ((str[(position) / 2] >> (((position) & 1) * 4)) & 0xF)

typedef struct treeNode
{
	char* key;
	int32_t childsBlockIndex;
	ValueType value;
} treeNode;

typedef struct prefixTree
{
	ValueType emptyKeyValue;
	size_t itemsCount;
	size_t blocksCount;
	size_t blocksRezerved;
	treeNode* nodes;
} prefixTree;

size_t allocateBlock(prefixTree* tree)
{
	if (tree->blocksRezerved == tree->blocksCount)
	{
		size_t newBlocksRezerved = max(tree->blocksRezerved * 9 / 5, 2);

		void* newNodes = realloc(tree->nodes, newBlocksRezerved * 16 * sizeof(treeNode));
		if (newNodes == NULL)
			return 0;

		tree->nodes = newNodes;

		memset(
			&tree->nodes[tree->blocksRezerved * 16],
			0,
			16 * sizeof(treeNode) * (newBlocksRezerved - tree->blocksRezerved));

		tree->blocksRezerved = newBlocksRezerved;
	}

	return tree->blocksCount++;
}

PrefixTree *prefixTree_create()
{
	prefixTree *tree = calloc(1, sizeof(prefixTree));
	if (!tree)
		return NULL;

	tree->blocksCount = 1;
	tree->blocksRezerved = 1000;

	tree->nodes = (treeNode*)calloc(tree->blocksRezerved * 16, sizeof(treeNode));

	return tree;
}

void prefixTree_free(PrefixTree *ptTree, _Bool fFreeKeys)
{
	if (!ptTree)
		return;

	prefixTree *tree = ptTree;

	if (fFreeKeys)
	{
		for (size_t i = 0; i < tree->blocksCount * 16; i++)
			free(tree->nodes[i].key);
	}

	free(tree->nodes);
	free(tree);
}

_Bool prefixTree_set(const PrefixTree *ptTree, const char* sKey, const ValueType value, _Bool fCopyKey)
{
	if (!ptTree)
		return false;

	if (!sKey)
		return false;

	size_t keyLen = strlen(sKey);

	prefixTree *tree = ptTree;
	char *sKey_i = sKey;
	ValueType value_i = value;

	if (keyLen == 0)
	{
		tree->emptyKeyValue = value;
		tree->itemsCount |= EMPTY_ITEM_MASK;
		return true;
	}

	_Bool tailRecursion = true;
	while (tailRecursion)
	{
		tailRecursion = false;

		size_t blockIndex = 0;
		for (size_t i = 0; i < keyLen * 2; i++)
		{
			size_t inBlockI = IN_BLOCK_INDEX(sKey_i, i);

			treeNode *node = &tree->nodes[blockIndex * 16 + inBlockI];

			if (node->key != NULL)
			{
				if (strcmp(&node->key[i / 2], &sKey_i[i / 2]) == 0)
				{
					node->value = value_i;
					return true;
				}
				else if (node->childsBlockIndex == 0)
				{
					size_t newBlockIndex = allocateBlock(tree);
					if (newBlockIndex == 0)
						return false;

					node = &tree->nodes[blockIndex * 16 + inBlockI];
					node->childsBlockIndex = newBlockIndex;

					if (strlen(node->key) * 2 - 1 > i)
					{
#if 1
						tree->nodes[newBlockIndex * 16 + IN_BLOCK_INDEX(node->key, i + 1)].key = node->key;
						tree->nodes[newBlockIndex * 16 + IN_BLOCK_INDEX(node->key, i + 1)].value = node->value;

						tree->itemsCount++;
						node->value = value_i;

						if (fCopyKey)
						{
							node->key = calloc(keyLen + 1, sizeof(char));
							strcpy(node->key, sKey_i);
						}
						else
						{
							node->key = sKey_i;
						}

						return true;
#else						
						tree->nodes[newBlockIndex * 16 + IN_BLOCK_INDEX(node->key, i + 1)].key = node->key;
						tree->nodes[newBlockIndex * 16 + IN_BLOCK_INDEX(node->key, i + 1)].value = node->value;
						node->key = NULL;
						node->value = NULL;

						tailRecursion = true;
						break;
#endif					
					}
				}
				else if (i + 1 == keyLen * 2)
				{
					char *nkey = node->key;
					ValueType nvalue = node->value;

					node->key = sKey_i;
					node->value = value_i;

					sKey_i = nkey;
					value_i = nvalue;
					keyLen = strlen(sKey_i);

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
						strcpy(node->key, sKey_i);
					}
					else
					{
						node->key = sKey_i;
					}

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

	prefixTree *tree = ptTree;

	size_t keyLen = strlen(sKey);

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
	treeNode *nodes = tree->nodes;
	for (size_t i = 0; i < keyLen * 2; i++)
	{
		size_t inBlockI = IN_BLOCK_INDEX(sKey, i);
		treeNode *node = &nodes[blockIndex * 16 + inBlockI];

		if (node->key != NULL)
		{
			if (strcmp(&node->key[i / 2], &sKey[i / 2]) == 0)
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

size_t prefixTree_count(const PrefixTree *ptTree)
{
	if (!ptTree)
		return false;

	prefixTree *tree = ptTree;

	return (tree->itemsCount & ~EMPTY_ITEM_MASK) + ((tree->itemsCount & EMPTY_ITEM_MASK) ? 1 : 0);
}
