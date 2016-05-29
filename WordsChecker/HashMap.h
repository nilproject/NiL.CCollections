#pragma once

#include <stdbool.h>

typedef void* ValueType;
typedef struct { void* placeholder[0]; } HashMap;

HashMap *hashMap_create();

void hashMap_free(HashMap *ptTree, _Bool fFreeKeys);

_Bool hashMap_insert(const HashMap *pHashMap, const char *sKey, const ValueType value, _Bool fCopyKey);

_Bool hashMap_get(const HashMap *pHashMap, const char *sKey, ValueType *value);
