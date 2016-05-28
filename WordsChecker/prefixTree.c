
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prefixTree.h"

#ifndef max
#define max(x,y) ((x)>(y)?x:y)
#endif

#define EMPTY_ITEM_MASK (1 << (sizeof(size_t) * 8 - 1))
#define IN_BLOCK_INDEX(str, position) ((str[(position) / 2] >> (((position) & 1) * 4)) & 0xF)

typedef struct treeNode
{
	char* key;
	size_t childsBlockIndex;
	size_t bucketIndex;
} treeNode;

typedef struct prefixTree
{
	size_t valuesSize;
	size_t blocksCount;
	size_t blocksRezerved;
	size_t itemsCount;
	ValueType* values;
	treeNode* nodes;
} prefixTree;

size_t allocateBlock(prefixTree* tree)
{
	if (tree->blocksRezerved == tree->blocksCount)
	{
		size_t newBlocksRezerved = max(tree->blocksRezerved * 8 / 5, 2);

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

	tree->itemsCount = 0;
	tree->blocksCount = 1;
	tree->blocksRezerved = 10000;

	tree->valuesSize = 4;
	tree->values = (ValueType*)calloc(tree->valuesSize, sizeof(ValueType));

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
	free(tree->values);
	free(tree);
}

_Bool prefixTree_set(const PrefixTree *ptTree, const char* sKey, const ValueType value, _Bool fCopyKey)
{
	if (!ptTree)
		return false;

	if (!sKey)
		return false;

	prefixTree *tree = ptTree;

	size_t keyLen = strlen(sKey);

	if (keyLen == 0)
	{
		tree->values[0] = value;
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
			size_t inBlockI = IN_BLOCK_INDEX(sKey, i);

			treeNode *node = &tree->nodes[blockIndex * 16 + inBlockI];

			if (node->key != NULL)
			{
				if (strcmp(&node->key[i / 2], &sKey[i / 2]) == 0)
				{
					tree->values[node->bucketIndex] = value;
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
						tree->nodes[newBlockIndex * 16 + IN_BLOCK_INDEX(node->key, i + 1)].key = node->key;
						tree->nodes[newBlockIndex * 16 + IN_BLOCK_INDEX(node->key, i + 1)].bucketIndex = node->bucketIndex;
						node->key = NULL;
						node->bucketIndex = 0;

						tailRecursion = true;
						break;
					}
				}
			}
			else
			{
				if (node->childsBlockIndex == 0 || (keyLen * 2 - 1) == i)
				{
					if (tree->valuesSize <= (tree->itemsCount + 2))
					{
						size_t newbucketsSize = max(tree->valuesSize * 8 / 5, 2);
						void* newValues = realloc(tree->values, sizeof(ValueType) * newbucketsSize);
						if (newValues == NULL)
							return false;

						tree->values = newValues;
						tree->valuesSize = newbucketsSize;
					}

					tree->itemsCount++;
					tree->values[tree->itemsCount] = value;
					
					if (fCopyKey)
					{
						node->key = calloc(keyLen + 1, sizeof(char));
						strcpy(node->key, sKey);
					}
					else
					{
						node->key = sKey;
					}

					node->bucketIndex = tree->itemsCount;

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
		if (tree->itemsCount & (1 << (sizeof(size_t) * 8 - 1)))
		{
			*pValue = tree->values[0];
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
				*pValue = tree->values[node->bucketIndex];
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
