#include "AttrCacheTable.h"

#include <cstring>

AttrCacheEntry* AttrCacheTable::attrCache[MAX_OPEN];


int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset, AttrCatEntry* attrCatBuf) {
    if (relId < 0 || relId > MAX_OPEN)
        return E_OUTOFBOUND;

    for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
        if (entry->attrCatEntry.offset == attrOffset) {
            *attrCatBuf = entry->attrCatEntry;
            return SUCCESS;
        }
    }

    return E_ATTRNOTEXIST;
}

void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS], AttrCatEntry* attrCatEntry) {
    strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);
    strcpy(attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);
    attrCatEntry->attrType = (int)record[ATTRCAT_ATTR_TYPE_INDEX].nVal;
    attrCatEntry->primaryFlag = (bool)record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;
    attrCatEntry->rootBlock = (int)record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
    attrCatEntry->offset = (int)record[ATTRCAT_OFFSET_INDEX].nVal;
}

int AttrCacheTable::getAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;

    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    AttrCacheEntry* attrCacheEntry = nullptr;
    for (auto iter = AttrCacheTable::attrCache[relId]; iter != nullptr; iter = iter->next) {
        if (strcmp(attrName, (iter->attrCatEntry).attrName) == 0) {
            attrCacheEntry = iter;
            break;
        }
    }

    if (attrCacheEntry == nullptr)
        return E_ATTRNOTEXIST;

    *attrCatBuf = attrCacheEntry->attrCatEntry;
    return SUCCESS;
}
