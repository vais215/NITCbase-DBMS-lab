

#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>


//stage 3


/*int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;
  

  // Create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // Load the headers of both the blocks into relCatHeader and attrCatHeader
  relCatBuffer.getHeader(&relCatHeader);
  attrCatBuffer.getHeader(&attrCatHeader);

  int totalrelations = relCatHeader.numEntries;
  
  for (int i = 0; i < totalrelations; ++i) {
    Attribute relCatRecord[RELCAT_NO_ATTRS]; // Will store the record from the relation catalog
    relCatBuffer.getRecord(relCatRecord, i);

    // Print the relation name
    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    int currentBlock=ATTRCAT_BLOCK;
    do{
         attrCatBuffer = RecBuffer(currentBlock);
         attrCatBuffer.getHeader(&attrCatHeader);

         for(int j=0;j<attrCatHeader.numEntries;j++){
          Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
          attrCatBuffer.getRecord(attrCatRecord,j);

          if(strcmp(attrCatRecord[0].sVal,relCatRecord[0].sVal)==0){
           if(strcmp(attrCatRecord[1].sVal, "class")==0){
              strcpy(attrCatRecord[1].sVal,"Batch");
              printf("Updated attribute: %s\n",attrCatRecord[1].sVal);
            }
            if(strcmp(attrCatRecord[1].sVal, "name")==0){
              strcpy(attrCatRecord[1].sVal,"ct");
              printf("Updated attribute: %s\n",attrCatRecord[1].nVal);
            }

            const char *attrType = attrCatRecord[2].nVal == NUMBER ? "NUM" : "STR";
            printf(" %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,attrType);

          }
         }

         currentBlock = attrCatHeader.rblock;
   }while(currentBlock!= -1);
    // Only process the "Students" relation
   
    
    printf("\n");
  }
  
  
  for (int relId = 0; relId <= 2; ++relId) {
    RelCatEntry relCatBuffer;
    RelCacheTable::getRelCatEntry(relId, &relCatBuffer);
    
    std::cout << "Relation: " << relCatBuffer.relName << '\n';

    for (int attrIndex = 0; attrIndex < relCatBuffer.numAttrs; ++attrIndex) {
        AttrCatEntry attrCatBuffer;
        AttrCacheTable::getAttrCatEntry(relId, attrIndex, &attrCatBuffer);

        std::string attrType = (attrCatBuffer.attrType == NUMBER) ? "NUM" : "STR";
        std::cout << "  " << attrCatBuffer.attrName << ": " << attrType << '\n';
    }

    std::cout << '\n';
}

  return 0;
}
}*/

//stage 4

int main(int argc,char *argv[]){
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  return FrontendInterface::handleFrontend(argc,argv);
}

