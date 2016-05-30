#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef void* ValueType;
typedef struct { void* placeholder[0]; } PrefixTree;

PrefixTree *prefixTree_create(void);
void prefixTree_free(PrefixTree *const ptTree, _Bool fFreeKeys);

_Bool  prefixTree_set(const PrefixTree *const ptTree, char *const sKey, const ValueType pValue, _Bool fCopyKey);
_Bool  prefixTree_get(const PrefixTree *const ptTree, const char *const sKey, ValueType *pValue);
size_t prefixTree_count(const PrefixTree *const ptTree);

