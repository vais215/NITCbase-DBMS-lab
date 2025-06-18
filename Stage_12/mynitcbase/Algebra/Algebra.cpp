
#include "Algebra.h"
#include<iostream>
#include <cstring>
using namespace std;

bool isNumber(char *str) {
  int len;
  float ignore;
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]){
  
  if(strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0)
    return E_NOTPERMITTED;

  int relId = OpenRelTable::getRelId(relName);
  if(relId == E_RELNOTOPEN)
    return E_RELNOTOPEN;

  RelCatEntry relCatEntry;
  RelCacheTable::getRelCatEntry(relId, &relCatEntry);
  if(relCatEntry.numAttrs != nAttrs)
    return E_NATTRMISMATCH;

  Attribute recordValues[nAttrs];

  for(int attrIndex=0; attrIndex < nAttrs; attrIndex++){
    AttrCatEntry attrCatEntry;
    AttrCacheTable::getAttrCatEntry(relId, attrIndex, &attrCatEntry);
    int attrType = attrCatEntry.attrType;
    if(attrType == NUMBER){
      if(isNumber(record[attrIndex])){
        recordValues[attrIndex].nVal = atof(record[attrIndex]);
      }
      else{
        return E_ATTRTYPEMISMATCH;
      }
    }
    else{
      strcpy((char *) &(recordValues[attrIndex].sVal), record[attrIndex]);
      //strcpy(recordValues[attrIndex].sVal, record[attrIndex]);
    }
  }

  int retVal = BlockAccess::insert(relId, recordValues);
  return retVal;

}

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {
  // SELECT * FROM TABLE INTO TABLE 2 WHERE KEY > 1000
  
  int srcRelId = OpenRelTable::getRelId(srcRel);      
  if (srcRelId == E_RELNOTOPEN) {
    return E_RELNOTOPEN;
  }

  AttrCatEntry attrCatEntry;
  int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr,&attrCatEntry);
  
  if(ret == E_ATTRNOTEXIST){
  	return E_ATTRNOTEXIST;
  }

  int type = attrCatEntry.attrType;
  Attribute attrVal;
  if (type == NUMBER) {
    if (isNumber(strVal)) {       // the isNumber() function is implemented below
      attrVal.nVal = atof(strVal);
    } 
    else {
      return E_ATTRTYPEMISMATCH;
    }
  } 
  else if (type == STRING) {
    strcpy(attrVal.sVal, strVal);
  }

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId,&relCatEntry);

    int src_nAttrs = relCatEntry.numAttrs;

    char attr_names[src_nAttrs][ATTR_SIZE];
    int attr_types[src_nAttrs];
    
    for (int attrIndex = 0; attrIndex < src_nAttrs; attrIndex++) {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId, attrIndex, &attrCatEntry);
        strcpy(attr_names[attrIndex], attrCatEntry.attrName);
        attr_types[attrIndex] = attrCatEntry.attrType;
    }

    int retVal = Schema::createRel(targetRel, src_nAttrs, attr_names, attr_types);
    if(retVal != SUCCESS)
    	return retVal;

    int openedRelId = OpenRelTable::openRel(targetRel);
    
    if(openedRelId < 0 || openedRelId >= MAX_OPEN){
    	Schema::deleteRel(targetRel);
    	return openedRelId;
    }
  
  //selecting and inserting records into target rel
    Attribute record[src_nAttrs];

    RelCacheTable::resetSearchIndex(srcRelId);
    AttrCacheTable::resetSearchIndex(srcRelId, attr);


    //BlockAccess::ls=0;
    //BPlusTree::bs=0;

    while (BlockAccess::search(srcRelId, record, attr, attrVal, op) == SUCCESS) {
        // RecId recId = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);

        // if(recId.block == -1 && recId.slot == -1){
        //   break;
        // }
        // RecBuffer recBuff(recId.block);
        // recBuff.getRecord(record, recId.slot);

        ret = BlockAccess::insert(openedRelId, record);
        if(ret != SUCCESS){
          Schema::closeRel(targetRel);
          Schema::deleteRel(targetRel);
          return ret;
        }
    }

    
    /*if(attrCatEntry.rootBlock==-1){
      printf("Linear Search: %d\n",BlockAccess::ls);
    }
    else{
      printf("B+ Search: %d\n",BPlusTree::bs);
    }*/
    Schema::closeRel(targetRel);
    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);

   if (srcRelId < 0 || srcRelId >= MAX_OPEN) 
    return E_RELNOTOPEN;


    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    int numAttrs = relCatEntry.numAttrs;

    char attrNames[numAttrs][ATTR_SIZE];
    int attrTypes[numAttrs];

    for(int attrIndex = 0; attrIndex < numAttrs; attrIndex++){

      AttrCatEntry attrCatEntry;
      AttrCacheTable::getAttrCatEntry(srcRelId, attrIndex, &attrCatEntry);
      strcpy(attrNames[attrIndex], attrCatEntry.attrName);
      attrTypes[attrIndex] = attrCatEntry.attrType;

    } 

    int ret = Schema::createRel(targetRel, numAttrs, attrNames, attrTypes);

    if(ret != SUCCESS)
      return ret;
    
    int targetRelId = OpenRelTable::openRel(targetRel);

    if(targetRelId < 0 || targetRelId >= MAX_OPEN){
      Schema::deleteRel(targetRel);
      return targetRelId;
    }
    
    RelCacheTable::resetSearchIndex(srcRelId);

    Attribute record[numAttrs];

    while (true)
    {
        int retVal = BlockAccess::project(srcRelId, record);

        if(retVal != SUCCESS)
          break;

        ret = BlockAccess::insert(targetRelId, record);

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    Schema::closeRel(targetRel);
    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);

    if (srcRelId < 0 || srcRelId >= MAX_OPEN) 
      return E_RELNOTOPEN;

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    // get the no. of attributes present in relation from the fetched RelCatEntry.
    int numAttrs = relCatEntry.numAttrs;
    int attr_offset[tar_nAttrs];

    int attr_types[tar_nAttrs];

    for(int tar_AttrIndex = 0; tar_AttrIndex < tar_nAttrs; tar_AttrIndex++){
      AttrCatEntry attrCatEntry;
      int ret = AttrCacheTable::getAttrCatEntry(srcRelId, tar_Attrs[tar_AttrIndex], &attrCatEntry);
      if(ret == E_ATTRNOTEXIST)
        return E_ATTRNOTEXIST;
      attr_offset[tar_AttrIndex] = attrCatEntry.offset;
      attr_types[tar_AttrIndex] = attrCatEntry.attrType;
    }

    int retVal = Schema::createRel(targetRel, tar_nAttrs, tar_Attrs, attr_types);

    if(retVal != SUCCESS)
      return retVal;

    int targetRelId = OpenRelTable::openRel(targetRel);

    if(targetRelId < 0 || targetRelId >= MAX_OPEN){
      Schema::deleteRel(targetRel);
      return targetRelId;
    }

    RelCacheTable::resetSearchIndex(srcRelId);

    Attribute record[numAttrs];
    Attribute proj_record[tar_nAttrs];

    while (true) {

        int ret = BlockAccess::project(srcRelId, record);

        if(ret != SUCCESS)
          break;
        //Attribute proj_record[tar_nAttrs];

        for(int tar_AttrIndex = 0; tar_AttrIndex < tar_nAttrs; tar_AttrIndex++){
          proj_record[tar_AttrIndex] = record[attr_offset[tar_AttrIndex]];
        }

        ret = BlockAccess::insert(targetRelId, proj_record);

        if (ret != SUCCESS) {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    Schema::closeRel(targetRel);
    return SUCCESS;
}


//stage 12


int Algebra::join(char srcRelation1[ATTR_SIZE], char srcRelation2[ATTR_SIZE], char targetRelation[ATTR_SIZE], char attribute1[ATTR_SIZE], char attribute2[ATTR_SIZE])
{
    int ret;
    int srcRelId_1 = OpenRelTable::getRelId(srcRelation1);
    int srcRelId_2 = OpenRelTable::getRelId(srcRelation2);

    if (srcRelId_1 == E_RELNOTOPEN || srcRelId_2 == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    AttrCatEntry attrCatEntry1, attrCatEntry2;
    ret = AttrCacheTable::getAttrCatEntry(srcRelId_1, attribute1, &attrCatEntry1);
    if (ret != SUCCESS)
    {
        return E_ATTRNOTEXIST;
    }
    ret = AttrCacheTable::getAttrCatEntry(srcRelId_2, attribute2, &attrCatEntry2);
    if (ret != SUCCESS)
    {
        return E_ATTRNOTEXIST;
    }

    if(attrCatEntry1.attrType != attrCatEntry2.attrType) {
        return E_ATTRTYPEMISMATCH;
    }

    RelCatEntry relCatEntryBuf1, relCatEntryBuf2;

    RelCacheTable::getRelCatEntry(srcRelId_1, &relCatEntryBuf1);
    RelCacheTable::getRelCatEntry(srcRelId_2, &relCatEntryBuf2);

    int numOfAttributes_1 = relCatEntryBuf1.numAttrs;
    int numOfAttributes_2 = relCatEntryBuf2.numAttrs;

    for(int i = 0; i < numOfAttributes_1; i++) {
        AttrCatEntry attrCatEntry_1;
        AttrCacheTable::getAttrCatEntry(srcRelId_1, i, &attrCatEntry_1);
        
        if(strcmp(attrCatEntry_1.attrName, attribute1) == 0) continue;

        for(int j = 0; j < numOfAttributes_2; j++) {
            AttrCatEntry attrCatEntry_2;
            AttrCacheTable::getAttrCatEntry(srcRelId_2, j, &attrCatEntry_2);

            if(strcmp(attrCatEntry_2.attrName, attribute2) == 0) continue;

            if(strcmp(attrCatEntry_1.attrName, attrCatEntry_2.attrName) == 0) {
                return E_DUPLICATEATTR;
            }
        }
    }

    int rootBlock = attrCatEntry2.rootBlock;
    
    // creating B+ Tree for the 2nd relation
    if(rootBlock == -1) {
        int ret = BPlusTree::bPlusCreate(srcRelId_2, attribute2);
        if(ret == E_DISKFULL) return E_DISKFULL;
        
        rootBlock =attrCatEntry2.rootBlock;
    }

    int numOfAttributesInTarget = numOfAttributes_1 + numOfAttributes_2 - 1;
    char targetRelAttrNames[numOfAttributesInTarget][ATTR_SIZE];
    int targetRelAttrTypes[numOfAttributesInTarget];

    for(int i = 0; i < numOfAttributes_1; i++) {
        AttrCatEntry attrCatEntry;

        AttrCacheTable::getAttrCatEntry(srcRelId_1, i, &attrCatEntry);
        strcpy(targetRelAttrNames[i], attrCatEntry.attrName);
        targetRelAttrTypes[i] = attrCatEntry.attrType;
    }

    bool inserted = false;

    for(int i = 0; i < numOfAttributes_2; i++) {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId_2, i, &attrCatEntry);

        if(strcmp(attrCatEntry.attrName, attribute2) == 0) {
            inserted = true;
            continue;
        }

        if(!inserted) {
            strcpy(targetRelAttrNames[numOfAttributes_1 + i], attrCatEntry.attrName);
            targetRelAttrTypes[numOfAttributes_1 + i] = attrCatEntry.attrType;
        } else {
            strcpy(targetRelAttrNames[numOfAttributes_1 + i - 1], attrCatEntry.attrName);
            targetRelAttrTypes[numOfAttributes_1 + i - 1] = attrCatEntry.attrType;
        }
    }

    ret = Schema::createRel(targetRelation, numOfAttributesInTarget, targetRelAttrNames, targetRelAttrTypes);

    if(ret != SUCCESS) return ret;

    int targetRelId = OpenRelTable::openRel(targetRelation);

    if(targetRelId < 0) {
        Schema::deleteRel(targetRelation);
        return targetRelId;
    }

    Attribute record1[numOfAttributes_1];
    Attribute record2[numOfAttributes_2];
    Attribute targetRecord[numOfAttributesInTarget];

    RelCacheTable::resetSearchIndex(srcRelId_1);

    while(BlockAccess::project(srcRelId_1, record1) == SUCCESS) {
        RelCacheTable::resetSearchIndex(srcRelId_2);
        AttrCacheTable::resetSearchIndex(srcRelId_2, attribute2);


        while(BlockAccess::search(srcRelId_2, record2, attribute2, record1[attrCatEntry1.offset], EQ) == SUCCESS) {
            
            for(int i = 0; i < numOfAttributes_1; i++) {
                targetRecord[i] = record1[i];
            }

            bool inserted = false;
            for(int i = 0; i < numOfAttributes_2; i++) {
                if(i == attrCatEntry2.offset) {
                    inserted = true;
                    continue;
                }

                if(!inserted) {
                    targetRecord[numOfAttributes_1 + i] = record2[i];
                } else {
                    targetRecord[numOfAttributes_1 + i - 1] = record2[i];
                }
            }

            ret = BlockAccess::insert(targetRelId, targetRecord);

            if(ret == E_DISKFULL) {
                ret = OpenRelTable::closeRel(targetRelId);
                
                ret = Schema::deleteRel(targetRelation);

                return E_DISKFULL;
            }
        }
    }

    return SUCCESS;
} 
