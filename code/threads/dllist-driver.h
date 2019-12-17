#pragma once
#include "dllist.h"

void DLListRandomInsertKeys(int n, DLList* list, int threadID);
void DLListSequentialPrependKeys(int n, DLList* list, int threadID);
void DLListSequentialAppendKeys(int n, DLList* list, int threadID);
void DLListRemoveHeadItems(int n, DLList* list, int threadID);
void DLListRandomRemoveItems(int n, DLList* list, int threadID);

void SynchDLListRandomInsertKeys(int n, SynchDLList* list, int threadID);
void SynchDLListSequentialPrependKeys(int n, SynchDLList* list, int threadID);
void SynchDLListSequentialAppendKeys(int n, SynchDLList* list, int threadID);
void SynchDLListRemoveHeadItems(int n, SynchDLList* list, int threadID);
void SynchDLListRandomRemoveItems(int n, SynchDLList* list, int threadID);