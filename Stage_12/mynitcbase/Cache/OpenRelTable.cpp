
#include "OpenRelTable.h"
#include <stdlib.h>
#include <cstring>
#include <iostream>


OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    OpenRelTable::tableMetaInfo[i].free= true;
  }

  
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /* setting up Attribute Catalog relation in the Relation Cache Table */

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  RecBuffer relCatBlock1(RELCAT_BLOCK);
  Attribute relCatRecord1[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord1, RELCAT_SLOTNUM_FOR_ATTRCAT);
  
  struct RelCacheEntry attrRelCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord1,&attrRelCacheEntry.relCatEntry);
  attrRelCacheEntry.recId.block= RELCAT_BLOCK;
  attrRelCacheEntry.recId.slot= RELCAT_SLOTNUM_FOR_ATTRCAT;

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrRelCacheEntry;



  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  struct AttrCacheEntry* headrel=NULL;
  for(int i=RELCAT_NO_ATTRS-1; i>=0; i--){
  	struct  AttrCacheEntry* relAttrCacheEntry=(struct AttrCacheEntry*) malloc (sizeof(struct AttrCacheEntry));
  	attrCatBlock.getRecord(attrCatRecord, i);
  	AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&relAttrCacheEntry->attrCatEntry);
  	relAttrCacheEntry->recId.block = ATTRCAT_BLOCK;
  	relAttrCacheEntry->recId.slot = i;
  	relAttrCacheEntry->next=headrel;
  	headrel=relAttrCacheEntry;
  }
  
  
  AttrCacheTable::attrCache[RELCAT_RELID] = headrel;
  
  /* setting up Attribute Catalog relation in the Attribute Cache Table */
    RecBuffer attrCatBlock1(ATTRCAT_BLOCK);

  Attribute attrCatRecord1[ATTRCAT_NO_ATTRS];
  struct AttrCacheEntry* headattr=NULL;
  for(int i=11; i>=6; i--){
  	struct  AttrCacheEntry* AttrCacheEntry=(struct AttrCacheEntry*) malloc (sizeof(struct AttrCacheEntry));
  	attrCatBlock.getRecord(attrCatRecord1, i);
  	AttrCacheTable::recordToAttrCatEntry(attrCatRecord1,&AttrCacheEntry->attrCatEntry);
  	AttrCacheEntry->recId.block = ATTRCAT_BLOCK;
  	AttrCacheEntry->recId.slot = i;
  	AttrCacheEntry->next=headattr;
  	headattr=AttrCacheEntry;
  }
  
  
  AttrCacheTable::attrCache[ATTRCAT_RELID] = headattr;
  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]





 

  tableMetaInfo[RELCAT_RELID].free = false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);
  tableMetaInfo[ATTRCAT_RELID].free = false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);

}

int OpenRelTable::getFreeOpenRelTableEntry() {

 
  int i;
  for(i=2; i<MAX_OPEN; i++){
  	if(tableMetaInfo[i].free == true)
  		return i;
  }
  
  return E_CACHEFULL;
}
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

    for(int i=0; i<MAX_OPEN; i++){
  	if(!tableMetaInfo[i].free && strcmp(tableMetaInfo[i].relName, (char*)relName)==0)
  		return i;
  }
  return E_RELNOTOPEN;
 
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {

  int RelIdCheck;
  RelIdCheck = getRelId(relName);
  if(RelIdCheck >= 0 && RelIdCheck < MAX_OPEN){
    return RelIdCheck;
  }

  /* find a free slot in the Open Relation Table*/

  int freeSlot = getFreeOpenRelTableEntry();
  
  if (freeSlot == E_CACHEFULL){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.
  int relId = freeSlot;

  
  
  Attribute relationName;
  strcpy(relationName.sVal, relName);
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  
  // relcatRecId stores the rec-id of the relation relName in the Relation Catalog.
  RecId relcatRecId;
  char attributeRelName[10];
  strcpy(attributeRelName, "RelName");
  
  relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, attributeRelName, relationName, EQ);
  
  if ( relcatRecId.block == -1 && relcatRecId.slot == -1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  
 RecBuffer recBuff (relcatRecId.block);
 Attribute relCatRecord[RELCAT_NO_ATTRS];
 recBuff.getRecord(relCatRecord, relcatRecId.slot);
  struct RelCacheEntry* relCacheEntry = (struct RelCacheEntry*) malloc (sizeof(RelCacheEntry));
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry->relCatEntry);
  relCacheEntry->recId = relcatRecId;
  RelCacheTable::relCache[relId] = relCacheEntry;	
 
  //herelisthead
  int numOfBlocks=0;
  int block=relCacheEntry->relCatEntry.firstBlk;
  while(block!=-1){
    numOfBlocks++;
    RecBuffer buffer(block);
    struct HeadInfo head;
    buffer.getHeader(&head);
    block=head.rblock;
  }
  relCacheEntry->relCatEntry.numOfBlocks=numOfBlocks;
  RelCacheTable::relCache[relId]=relCacheEntry;
  printf("Number of blocks: %d  ",numOfBlocks);
  

  /*int numRecs=0;
  int recs=relCacheEntry->relCatEntry.numRecs;
  while(recs!=-1){
    numRecs++;
    RecBuffer buffer(recs);
    struct HeadInfo head;
    buffer.getRecord(recs,slotNum);
    //block=head.rblock;
  }
  relCacheEntry->relCatEntry.numRecs=numRecs;
  RelCacheTable::relCache[relId]=relCacheEntry;
  printf("Number of tuples: %d  ",numRecs);*/

  AttrCacheEntry* listHead = NULL, *prev = NULL;

  
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  Attribute currentRelName;
  strcpy(currentRelName.sVal, relName);
  
  while(true)
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
    RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, attributeRelName, currentRelName, EQ);
    
    if(attrcatRecId.block == -1 && attrcatRecId.slot == -1)
      break;
    RecBuffer currentRecBlock (attrcatRecId.block);
    Attribute currentRec[ATTRCAT_NO_ATTRS];
    
    currentRecBlock.getRecord(currentRec, attrcatRecId.slot);
   
    
   
    struct AttrCacheEntry* AttrCacheEntry = (struct AttrCacheEntry*) malloc (sizeof (struct AttrCacheEntry));
  	
    AttrCacheTable::recordToAttrCatEntry(currentRec, &AttrCacheEntry->attrCatEntry);
    AttrCacheEntry->recId = attrcatRecId;
    AttrCacheEntry->next = NULL;
     //std::cout<<"check:"<<freeSlot<<"\n";
    //std::cout<<AttrCacheEntry->attrCatEntry.attrName<<'\n';
    if(prev == NULL)
      listHead = AttrCacheEntry;
    else
      prev->next = AttrCacheEntry;
    prev = AttrCacheEntry;
    
     
  }
  AttrCacheTable::attrCache[relId] = listHead;
  prev = listHead;
 
  tableMetaInfo[relId].free = false;
  strcpy(tableMetaInfo[relId].relName, relName);
  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  return relId;
}


// int OpenRelTable::closeRel(int relId) {
//   if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) {
//     return E_NOTPERMITTED;
//   }

//   if (relId < 0 || relId >= MAX_OPEN) {
//     return E_OUTOFBOUND;
//   }

//   if (tableMetaInfo[relId].free == true) {
//     return E_RELNOTOPEN;
//   }

//   /* Releasing the Relation Cache Entry of the Relation */

//   if(RelCacheTable::relCache[relId]->dirty == true){
//     // if the relation cache entry is dirty, write it back to the relation catalog
//     Attribute relCatRecord[RELCAT_NO_ATTRS];
//     RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[relId]->relCatEntry, relCatRecord);
//     RecBuffer relCatBlock(RelCacheTable::relCache[relId]->recId.block);
//     relCatBlock.setRecord(relCatRecord, RelCacheTable::relCache[relId]->recId.slot);

//   }

//   /* Releasing the Attribute Cache Entry of the relation */

//   // free the memory allocated in the attribute caches which was allocated in openRel
//   AttrCacheEntry* current = AttrCacheTable::attrCache[relId];
//   while(current != NULL){
//     AttrCacheEntry* temp = current;
//     current = current->next;
//     free(temp);
//   }

//   // free the memory allocated in the relation and attribute caches which was
//   // allocated in the OpenRelTable::openRel() function

//   free(RelCacheTable::relCache[relId]);

//   // update tableMetaInfo to set relId as a free slot
//   tableMetaInfo[relId].free = true;
//   // update relCache and attrCache to set the entry at relId to nullptr
//   RelCacheTable::relCache[relId] = nullptr;
//   AttrCacheTable::attrCache[relId] = nullptr;
  
//   return SUCCESS;
// }
//dont use for stage 8
// OpenRelTable::~OpenRelTable() {
  
//   for (int i = 2; i < MAX_OPEN; ++i) {
//     if (!tableMetaInfo[i].free) {
//       OpenRelTable::closeRel(i); // we will implement this function later
//     }
//   }
//   // free all the memory that you allocated in the constructor
// }
//stage 8
OpenRelTable::~OpenRelTable() {
  
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  if(RelCacheTable::relCache[ATTRCAT_RELID]->dirty == true){
    // if the relation cache entry is dirty, write it back to the relation catalog
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[ATTRCAT_RELID]->relCatEntry, relCatRecord);
    RecBuffer relCatBlock(RelCacheTable::relCache[ATTRCAT_RELID]->recId.block);
    relCatBlock.setRecord(relCatRecord, RelCacheTable::relCache[ATTRCAT_RELID]->recId.slot);

  }

  if(RelCacheTable::relCache[RELCAT_RELID]->dirty == true){
    // if the relation cache entry is dirty, write it back to the relation catalog
    Attribute relCatRecord[RELCAT_NO_ATTRS];
    RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[RELCAT_RELID]->relCatEntry, relCatRecord);
    RecBuffer relCatBlock(RelCacheTable::relCache[RELCAT_RELID]->recId.block);
    relCatBlock.setRecord(relCatRecord, RelCacheTable::relCache[RELCAT_RELID]->recId.slot);

  }

  free(RelCacheTable::relCache[RELCAT_RELID]);
  free(RelCacheTable::relCache[ATTRCAT_RELID]);

  AttrCacheEntry* current = AttrCacheTable::attrCache[RELCAT_RELID];
  while(current != NULL){
    AttrCacheEntry* temp = current;
    current = current->next;
    free(temp);
  }

  current = AttrCacheTable::attrCache[ATTRCAT_RELID];
  while(current != NULL){
    AttrCacheEntry* temp = current;
    current = current->next;
    free(temp);
  }
}
int OpenRelTable::closeRel(int relId){
  if(relId == RELCAT_RELID || relId == ATTRCAT_RELID){
      return E_NOTPERMITTED;
  }
  if(relId < 0 || relId >= MAX_OPEN){
      return E_OUTOFBOUND;
  }
  if(tableMetaInfo[relId].free == true){
      return E_RELNOTOPEN;
  }

  if(RelCacheTable::relCache[relId]->dirty == true){
      /*
          Get the Relation Catalog entry from RelCacheTable::relCache,
          then convert it to a record using RelCacheTable::relCatToRecord().
      */
      Attribute relCatRecord[RELCAT_NO_ATTRS];
      RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[relId]->relCatEntry, relCatRecord);
      RecBuffer relCatBlock(RelCacheTable::relCache[relId]->recId.block);
      relCatBlock.setRecord(relCatRecord,RelCacheTable::relCache[relId]->recId.slot);
  }

  // free the memory allocated in the relation and attribute caches which was allocated in the openRel()
  free(RelCacheTable::relCache[relId]);
  AttrCacheEntry *entry, *temp;
  entry = AttrCacheTable::attrCache[relId];
  while(entry!= nullptr){
      if(entry->dirty){
          Attribute record[ATTRCAT_NO_ATTRS];
          AttrCacheTable::attrCatEntryToRecord(&entry->attrCatEntry, record);
          RecBuffer attrCatBlock(entry->recId.block);
          attrCatBlock.setRecord(record, entry->recId.slot);
      }
      
      temp = entry;
      entry = entry->next;
      free(temp);
  }

  // update 'tableMetaInfo' to set 'relId' as a free slot;
  tableMetaInfo[relId].free = true;
// update 'relCache' and 'attrCache' to set the entry at 'relId' to nullptr;
RelCacheTable::relCache[relId] = nullptr;
AttrCacheTable::attrCache[relId] = nullptr;

return SUCCESS;

}
