#include "Frontend.h"

#include <cstring>
#include <iostream>

int Frontend::create_table(char relname[ATTR_SIZE], int no_attrs, char attributes[][ATTR_SIZE],
                           int type_attrs[]) {
  // Schema::createRel
  //return SUCCESS;
  return Schema::createRel(relname,no_attrs,attributes,type_attrs);
}

int Frontend::drop_table(char relname[ATTR_SIZE]) {
  // Schema::deleteRel
  //return SUCCESS;
  return Schema::deleteRel(relname);
}

int Frontend::open_table(char relname[ATTR_SIZE]) {
  // Schema::openRel
  //stage 5
  return Schema::openRel(relname);
  //return SUCCESS;
}

int Frontend::close_table(char relname[ATTR_SIZE]) {
  // Schema::closeRel
  //stage 5
  return Schema::closeRel(relname);
  //return SUCCESS;
}

int Frontend::alter_table_rename(char relname_from[ATTR_SIZE], char relname_to[ATTR_SIZE]) {
  // Schema::renameRel
  //return SUCCESS;
  return Schema::renameRel(relname_from,relname_to);
}

int Frontend::alter_table_rename_column(char relname[ATTR_SIZE], char attrname_from[ATTR_SIZE],
                                        char attrname_to[ATTR_SIZE]) {
  // Schema::renameAttr
  //return SUCCESS;
  return Schema::renameAttr(relname,attrname_from,attrname_to);
}

int Frontend::create_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]) {
  // Schema::createIndex
  //return SUCCESS;
  return Schema::createIndex(relname,attrname);
}

int Frontend::drop_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]) {
  // Schema::dropIndex
  //return SUCCESS;
  return Schema::dropIndex(relname,attrname);
}

int Frontend::insert_into_table_values(char relname[ATTR_SIZE], int attr_count, char attr_values[][ATTR_SIZE]) {
  // Algebra::insert
  //return SUCCESS;
  //stage 7
  return Algebra::insert(relname, attr_count, attr_values);
}

int Frontend::select_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE]) {
  // Algebra::project
  //return SUCCESS;
  return Algebra::project(relname_source,relname_target);
}

int Frontend::select_attrlist_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                         int attr_count, char attr_list[][ATTR_SIZE]) {
  // Algebra::project
  //return SUCCESS;
  return Algebra::project(relname_source,relname_target, attr_count,attr_list);
}

int Frontend::select_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                      char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]) {

  return Algebra::select(relname_source, relname_target, attribute, op, value);                                 
  //Algebra::select;
  //return SUCCESS;
}

int Frontend::select_attrlist_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                               int attr_count, char attr_list[][ATTR_SIZE],
                                               char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]) {
  // Algebra::select + Algebra::project??
  //return SUCCESS;
  char temp[20]= ".temp";
  int ret = Algebra::select(relname_source,temp,attribute,op,value);
  if(ret!=SUCCESS)
  return ret;

  int relId = OpenRelTable::openRel(temp);
  if(relId < 0 || relId >= MAX_OPEN)
  {
    Schema::deleteRel(temp);
    return relId;
  }
  ret = Algebra::project(temp,relname_target,attr_count,attr_list);
  OpenRelTable::closeRel(relId);
  Schema::deleteRel(temp);

  return ret;
}

int Frontend::select_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
                                     char relname_target[ATTR_SIZE],
                                     char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE]) {
  // Algebra::join
  return Algebra::join(relname_source_one, relname_source_two, relname_target, join_attr_one, join_attr_two);
 // return SUCCESS;
}

int Frontend::select_attrlist_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
                                              char relname_target[ATTR_SIZE],
                                              char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE],
                                              int attr_count, char attr_list[][ATTR_SIZE]) {
  // Algebra::join + project
  //return SUCCESS;
  char relname_temp[ATTR_SIZE] = TEMP;

  int ret = Algebra::join(relname_source_one, relname_source_two, relname_temp, join_attr_one, join_attr_two);
  if (ret != SUCCESS)
  return ret;

  int tempRelId = OpenRelTable::openRel(relname_temp);
  if (tempRelId < 0 || tempRelId >= MAX_OPEN) {
  Schema::deleteRel(relname_temp);
  return tempRelId;
  } 

  ret = Algebra::project(relname_temp, relname_target, attr_count, attr_list);
  OpenRelTable::closeRel(tempRelId);
  Schema::deleteRel(relname_temp);

  return ret;
}


int Frontend::custom_function(int argc, char argv[][ATTR_SIZE]) {
  // argc gives the size of the argv array
  // argv stores every token delimited by space and comma

  // implement whatever you desire

  return SUCCESS;
}

