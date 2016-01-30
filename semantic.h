/* ****************************************************************************
*******************************************************************************
** Zeke Long                                                                 **
** CS 445                                                                    **
** HW 3                                                                      **
** 10/14/2014                                                                **
**                                                                           **
**               semantics header file for 120++ compiler                    **
*******************************************************************************
******************************************************************************/

#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_   

#include "token.h"
#include "tree.h"
#include "symtab.h"

typedef int (*funcPtr)(treeNodePtr);  /* declares funcPtr as a function 
                                         pointer that takes a treeNodePtr 
                                         and returns an int */
int semanticAnalyze();
void preorder(treeNodePtr, funcPtr);
void postorder(treeNodePtr, funcPtr);
int symTableCreate(treeNodePtr);
int handleVarOrFuncDecl(treeNodePtr);
int handleClassDecl(char *, treeNodePtr);
int handleClassConstructor(treeNodePtr);
void appendFieldOrMethod();
void classnameTabGetVars(treeNodePtr);  
void getVarVal(treeNodePtr);
void getParamTypes(treeNodePtr);
void getMethodParamTypes(treeNodePtr);
char *getFuncName(treeNodePtr);	
void classInstanceConstruct();
int simplifyType(int, int);
int typeCheckTree(treeNodePtr);
void synthesizeTypeHelper(treeNodePtr);
void checkTypes(int, treeNodePtr, symTabEntryPtr);		

#endif
