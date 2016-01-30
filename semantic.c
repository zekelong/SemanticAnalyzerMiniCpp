/* ****************************************************************************
*******************************************************************************
** Zeke Long                                                                 **
** CS 445                                                                    **
** 10/14/2014                                                                **
**                                                                           **
**                     Semantic analyzer for 120++                           **
*******************************************************************************
******************************************************************************/

#include<stdlib.h>
#include<stdio.h>
#include<stdarg.h>
#include "semantic.h"
#include "symtab.h"
#include "120++.tab.h"

// Globals
char *tabName;
int inConstructorFlag = 0;
int scopeLevel;
symTabEntryPtr tmpSymTabEntry;
symTabEntryPtr tmpSymTabEntry2;
symTabPtr tmpSymTabPtr;
symTabValPtr tmpUnion;
classnameEntryPtr tmpClassEntryPtr;  /* for inserting new classnames */
fieldsAndMethodsPtr tmpClassValPtr;  /* for inserting new class values */

extern treeNodePtr rootPtr;
extern char *yyFileName;
extern symTabPtr globSymTablePtr;        /* global symbol table */
extern symTabPtr currSymTablePtr;        /* current symbol table */


/* ***************************************************************************
**            Main control function for semantic analysis                   **
*****************************************************************************/
int semanticAnalyze()
{
   scopeLevel = 0;        //set to zero to represent global scope   

   //creates symbol tables
   preorder(rootPtr, &symTableCreate);

   /* assigns types to expression nodes and does TYPE CHECKING when values 
      are assigned  */
   currSymTablePtr = globSymTablePtr;
   preorder(rootPtr, &typeCheckTree);

	free(tmpSymTabEntry->symbolName);          //(used in a few functions)
   free(tmpSymTabEntry);
	free(tmpSymTabEntry2->symbolName);
   free(tmpSymTabEntry2);
	free(tmpSymTabPtr->tableName);
   free(tmpSymTabPtr);
   free(tmpUnion);

   return 1;
}


/* ***************************************************************************
**  Performs preorder traversal of tree (ie. visits each par node then      **
**  calls a function that was passed as a parameter to do work at the child **
**  nodes).                                                                 **
*****************************************************************************/
void preorder(treeNodePtr ptr, int (*func)(treeNodePtr))
{
   int i;
   if (ptr == NULL) return;
   
   /* do work at current node */
   func(ptr);

   /* visit each child */
   for (i=0; i < ptr->numChildren; i++)
      preorder(ptr->children[i], func);
}


/* ***************************************************************************
**  Performs postorder traversal of tree (ie. visits each child node then   **
**  calls a function that was passed as a parameter to do work at the       **
**  par node).                                                              **
*****************************************************************************/
void postorder(treeNodePtr ptr, int (*func)(treeNodePtr))
{
   int i;
   if (ptr == NULL) return;

   /* visit each child */
   for (i=0; i < ptr->numChildren; i++)
      postorder(ptr->children[i], func);

   /* do work at par */
   func(ptr);
}


/* ****************************************************************************
** PREORDER function that creates a new symbol table whenever a new scope is **
** introduced. If a scope is being exited, the global integer named          **
** scopeLevel is decremented.                                                **
******************************************************************************/
int symTableCreate(treeNodePtr ptr)
{
   /* these were declared as globals */
   if(!tmpSymTabEntry)
      tmpSymTabEntry = (symTabEntryPtr)calloc(1, sizeof(struct symTabEntry));
   if(!tmpSymTabEntry2)
      tmpSymTabEntry2 = (symTabEntryPtr)calloc(1, sizeof(struct symTabEntry));
   if(!tmpSymTabPtr)
      tmpSymTabPtr = (symTabPtr)calloc(1, sizeof(struct symTab)); 
   classnameEntryPtr cPtr;
   int i = 0;    
   
   switch(ptr->ruleLabel)
   {
      /* *************************************************************
      *  if a new function scope is being entered  (i.e.             *
      *  function_definition. class_specifier scope is handled in a  *
      *  different case below).                                      *
      ***************************************************************/
      case 593: case 594: case 595: 
      {        
         //create a new symbol table
         scopeLevel++;	  
         tmpSymTabPtr->nestLevel = scopeLevel;
         //assign tmpSymTabPtr->tableName
         tmpSymTabPtr->tableName = strdup(getFuncName(ptr)); 
         if(ptr->ruleLabel == 595)
            tmpSymTabPtr->isClassInstance = 1;
         else
            tmpSymTabPtr->isClassInstance = 0;
         if(scopeLevel == 1)
         {
            tmpSymTabPtr->par = globSymTablePtr; //link to par symbol table
            for(i=0; i<MAX_CLASS_VALS; i++)
            {
               if(!globSymTablePtr->child[i])
               {
                  globSymTablePtr->child[i] = tmpSymTabPtr;  //link par to this one
                  break;
               }
            }
         }
         else
         {
            tmpSymTabPtr->par = currSymTablePtr; //link to par symbol table
            for(i=0; i<MAX_CLASS_VALS; i++)
            {
               if(!currSymTablePtr->child[i])
               {
                  currSymTablePtr->child[i] = tmpSymTabPtr;  //link par to this one
                  break;
               }
            }
         }         
         currSymTablePtr = tmpSymTabPtr;    //update to current symbol table
         tmpSymTabPtr = NULL;           
         break;         
      }
      /* ****************************
      *      constructor body       *
      ******************************/
      case 596:  
      {        
         scopeLevel++;
         // tmpClassEntryPtr is used in handleClassConstructor()
         tmpClassEntryPtr = 
            classnameTabLookup(ptr->children[0]->leafTokenPtr->text);
         postorder(ptr, &handleClassConstructor);         
         break;         
      }      
      /* *******************************************************************
      *  If a scope is being exited (i.e. is it's a token, then: if its    *
      *  par is a compound_statement                                       * 
      *  with a function_body for a par)                                   *
      *********************************************************************/
      case 900:  
      {
         if(!strcmp(ptr->leafTokenPtr->text, "}") ) 
             if( (ptr->par->ruleLabel == 520 && 
                  ptr->par->par->ruleLabel == 599) 
               )
                  scopeLevel--;
         break;
      }
      /* *******************************************************************
      * If VARIABLE(s) or FUNCTION are being declared (i.e.                *
      * simple_declaration or parameter_declaration), check if it is a     *
      * re-declaration. If not, then save it in the symbol table.          *                                           *       
      *********************************************************************/
      case 542: case 543: case 589: case 590: case 591: case 592:
      {
         /* WORKAROUND FOR CLASS_SPECIFIER. NOTE: classes were already 
            checked for redeclarations during syntax analysis    */
         if(ptr->children[0]->children[0]->ruleLabel == 607)
         {
            /* ************************************************************
            * Save the class's fields and methods in the CLASSNAME TABLE  *  
            **************************************************************/           
            handleClassDecl(
               ptr->children[0]->children[0]->children[0]->children[1]
               ->leafTokenPtr->text, ptr->children[0]->children[0]);         
         }
         else
         {
            tmpSymTabEntry2->dataType = 0;
            /* note: assignment_expressions are covered in 
               handleVarOrFuncDecl() */
            postorder(ptr, &handleVarOrFuncDecl);  
         }
      }    
      default:
         break;
   }
}


/* ***************************************************************************
**  Takes a function_definition, finds the name of the function, and saves  **
**  it as the symbol table name for the current scope.                      **
*****************************************************************************/
char *getFuncName(treeNodePtr ptr)
{
   int i; 
   for(i=0; i<HASH_TABLE_SIZE; i++)
      tmpSymTabPtr->table[i] = '\0';    
   
   for(i=0; i<ptr->numChildren; i++)
   {
      if(ptr->children[i]->ruleLabel == 561)   // if declarator
      {
         ptr = ptr->children[i];
         while(ptr->children[0] != NULL)
         {
            ptr = ptr->children[0];
            if(ptr->ruleLabel == 404)    // if function or method
               return ptr->children[0]->leafTokenPtr->text;                        
         }
      }
   }
}


/* ***************************************************************************
** Postorder function that takes a function_definition pointer of a         **
** class constructor (already determined to be a constructor function), and **
** UPDATES THE CLASSNAME TABLE WITH GIVEN VALUES if there are any           ** 
*****************************************************************************/
int handleClassConstructor(treeNodePtr tPtr)
{
   /* NOTE: tmpClassEntryPtr is pointing to the class's entry in the 
      classname table */
   char *tmpString;
   treeNodePtr treePtr2;
   int i;
   
   switch(tPtr->ruleLabel)
   {
      /* **********************************
      *      assignment_expression        *
      ************************************/
      case 498:
         //get the class variable that is being assigned to
         treePtr2 = tPtr;
         //get the identifier name
         while(treePtr2->children[0] != NULL)
         {
            treePtr2 = treePtr2->children[0];
         }          
         tmpString = strdup(treePtr2->leafTokenPtr->text);
         /* get the value that is being assigned. NOTE: tPtr->children[1] is
            the assignment_operator (i.e. "=", "+", etc.) */
         treePtr2 = tPtr->children[2];
         while(treePtr2->children[0] != NULL)
         {
            treePtr2 = treePtr2->children[0];
         }    
         // treePtr2 is now pointing to the initial value (i.e. a literal node)
         switch(treePtr2->leafTokenPtr->category)
         {
            case 292: case 259:                   /* INTEGER */
               for(i=0; i<MAX_CLASS_VALS; i++)
               {
                  if(tmpClassEntryPtr->vals[i])
                     if(!strcmp(tmpClassEntryPtr->vals[i]->name, tmpString))
                        tmpClassEntryPtr->vals[i]->valPtr->v.in = 
                           treePtr2->leafTokenPtr->ival; 
               }          
               break;               
            case 260: case 280:                   /* CHARACTER */
               for(i=0; i<MAX_CLASS_VALS; i++)
               {
                  if(tmpClassEntryPtr->vals[i])
                     if(!strcmp(tmpClassEntryPtr->vals[i]->name, tmpString))
                     {
                        tmpClassEntryPtr->vals[i]->valPtr->v.ch = 
                           treePtr2->leafTokenPtr->sval[0];                            
                     }
               }              
               break;               
            default:
               break;
         }
         break;
      default: 
         break;
   }
}



/* ***************************************************************************
**  Postorder function that checks for re-declarations of variables and     **
**  functions, then saves to the symbol table.  Takes a pointer to a        **
**  simple_declaration sub-tree. note: INITIAL VALUE DOESN'T MATTER YET; IT **
**  WILL BE SAVED DURING TYPE CHECKING                                      **
*****************************************************************************/
int handleVarOrFuncDecl(treeNodePtr ptr)
{ 
   int i;
   char *tmpString;
   treeNodePtr treePtr2;
   
   switch(ptr->ruleLabel)
   {   
      /* ***************************
      *  if it's a type_specifier  *
      *****************************/
      case 550:
      {      
         if( !strcmp("string", ptr->children[0]->leafTokenPtr->text))
            tmpSymTabEntry2->dataType = 4;
         else   
            tmpSymTabEntry2->dataType = 
               ptr->children[0]->leafTokenPtr->category;
         if(!tmpUnion)
            tmpUnion =(symTabValPtr)calloc(1, sizeof(union symTabVal));
         tmpUnion->cv.className = strdup(ptr->children[0]->leafTokenPtr->text);        
         break;      
      }
      case 551: case 552: case 553: case 554: case 555:
      {       
         tmpSymTabEntry2->dataType = ptr->children[0]->leafTokenPtr->category;      
         break;
      }
      /* ***************************************
      *  if it's a function's parameter types  *
      *****************************************/         
      case 582: case 583: case 584:
      {      
         /* a fix for multiple parameters, where the par is also a
            parameter_declaration list. NOTE: tmpSymTabEntry2->symbolName 
            and tmpSymTabEntry2->tokPtr were assigned earlier in the 
            postorder traversal(case 404) */ 
         if((ptr->par->ruleLabel < 582) || (ptr->par->ruleLabel > 584))
         {
            tmpSymTabEntry2->varConstFuncClass = 3;    //func
            //assign the parameter types to tmpSymTabEntry2
            tmpUnion = (symTabValPtr)calloc(1, sizeof(union symTabVal));
            postorder(ptr, &getParamTypes);
            /* add function prototype to the symbol table, and save a pointer 
               to it at the "identifier" tree node */               
            ptr->par->children[0]->children[0]->stEntryPtr = 
               enterSymbol(tmpSymTabEntry2, tmpUnion);  
         }            
         break;
      }      
      /* *************************
      *  if it's an identifier   *
      ***************************/      
      case 404:
      {      
         tmpSymTabEntry2->symbolName = 
            strdup(ptr->children[0]->leafTokenPtr->text);              
         tmpSymTabEntry2->tokPtr = ptr->children[0]->leafTokenPtr;
         /* ********************************************************
         *  IF IT'S A CLASS VARIABLE   (tmpUnion->cv.className and *
         *  tmpSymTabEntry2->datatype were assigned in case 550)   *
         **********************************************************/
         if(tmpSymTabEntry2->dataType == 262)
         {
            tmpSymTabEntry2->varConstFuncClass = 4;   //class instance  
            //construct the class instance (i.e. fields and methods)
            classInstanceConstruct();
            /* save the class instance in symbol table, and add a pointer
               to the entry in the "identifier" tree node */              
            ptr->stEntryPtr = 
               enterSymbol(tmpSymTabEntry2, tmpUnion);         
         }
         /* ****************************************
         *     IF IT'S A POINTER TO A VARIABLE     *
         *     (i.e. Not a function_identifier)    *
         ******************************************/
         else if(ptr->par->par->par->ruleLabel == 562)
         {
            tmpSymTabEntry2->varConstFuncClass = 1;   //var 
            if(!tmpUnion)
               tmpUnion =(symTabValPtr)calloc(1, sizeof(union symTabVal));
               tmpSymTabEntry2->isPtr = 1;
            //get the pointer's optional value (saved to tmpSymTabEntry2)
            getVarVal(
              ptr->par->par->par->par->children[1]);
            /* save the pointer variable in symbol table, and add a pointer
               to the entry in the "identifier" tree node */              
            ptr->stEntryPtr =               
               enterSymbol(tmpSymTabEntry2, tmpUnion);           
         }           
         /* **************************************
         *          IF IT'S A VARIABLE           *
         * (i.e. Also not a function_identifier) *
         ****************************************/
         else if(ptr->par->par->ruleLabel == 561)
         {
            tmpSymTabEntry2->varConstFuncClass = 1;   //var 
            if(!tmpUnion)
               tmpUnion =(symTabValPtr)calloc(1, sizeof(union symTabVal));
            tmpSymTabEntry2->isPtr = 0;
            //get the variable's optional value (saved to tmpSymTabEntry2)
            getVarVal(
              ptr->par->par->par->children[1]); 
            /* save the variable in symbol table, and add a pointer
               to the entry in the "identifier" tree node */               
            ptr->stEntryPtr = 
               enterSymbol(tmpSymTabEntry2, tmpUnion);           
         } 
         /* **************************************************
         *      IF IT'S AN ASSIGNMENT_EXPRESSION             *
         *   (VARIABLE SHOULD HAVE ALREADY BEEN DECLARED)    *
         ****************************************************/
         else if(ptr->par->par->ruleLabel == 498)
         {              
            treePtr2 = ptr->par->par;
            //get identifier name of the variable that is being assigned to 
            tmpString = strdup(ptr->children[0]->leafTokenPtr->text);
            tmpSymTabEntry = symTableLookup(currSymTablePtr, tmpString);              
         }           
         break;
      }
	}
}


/* ***************************************************************************
**  Looks up the class name in the classname table and uses the info from   **
**  the table to create a symTabEntry for the class instance. NOTE:         **
**  tmpSymTabEntry2 and tmpUnion are assigned some of their values already  **
*****************************************************************************/
void classInstanceConstruct()
{
   int i;
   classnameEntryPtr dontDeletePtr;
   dontDeletePtr = classnameTabLookup(tmpUnion->cv.className);
   for(i=0; i<MAX_CLASS_VALS; i++)
   {   
      if(dontDeletePtr->vals[i])
      {         
         tmpUnion->cv.vals[i] = 
            (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
         tmpUnion->cv.vals[i]->valPtr =
            (symTabValPtr)calloc(1, sizeof(union symTabVal));
            
         tmpUnion->cv.vals[i]->table = tmpSymTabEntry2->table;            
         tmpUnion->cv.vals[i]->symbolName = 
            strdup(dontDeletePtr->vals[i]->name);            
         tmpUnion->cv.vals[i]->varConstFuncClass = 
            dontDeletePtr->vals[i]->varConstFunc;
         tmpUnion->cv.vals[i]->dataType = dontDeletePtr->vals[i]->dataType;
         tmpUnion->cv.vals[i]->isPtr = dontDeletePtr->vals[i]->isPtr;       
         tmpUnion->cv.vals[i]->tokPtr = dontDeletePtr->vals[i]->tokPtr;
         tmpUnion->cv.vals[i]->nextInBucket = NULL;
         switch(dontDeletePtr->vals[i]->varConstFunc)
         {
            case 1: case 2:     /* VAR and CONST */
               tmpUnion->cv.vals[i]->valPtr->v.in = 
                  dontDeletePtr->vals[i]->valPtr->v.in;  
               tmpUnion->cv.vals[i]->valPtr->v.ch = 
                  dontDeletePtr->vals[i]->valPtr->v.ch;    
               tmpUnion->cv.vals[i]->valPtr->v.ptrAddr = 
                  dontDeletePtr->vals[i]->valPtr->v.ptrAddr;                  
               break;
            case 3:    /* FUNCTION */
               tmpUnion->cv.vals[i]->valPtr->ft.pOneType = 
                  dontDeletePtr->vals[i]->valPtr->ft.pOneType;  
               tmpUnion->cv.vals[i]->valPtr->ft.pOneIsPtr = 
                  dontDeletePtr->vals[i]->valPtr->ft.pOneIsPtr; 
               tmpUnion->cv.vals[i]->valPtr->ft.pTwoType = 
                  dontDeletePtr->vals[i]->valPtr->ft.pTwoType; 
               tmpUnion->cv.vals[i]->valPtr->ft.pTwoIsPtr = 
                  dontDeletePtr->vals[i]->valPtr->ft.pTwoIsPtr;  
               break;
            default:
               break;
         }
      }
   }
}


/* ***************************************************************************
**  Function that finds the value of the variable (possibly a pointer or    **
**  class variable) and saves it in tmpSymTabEntry2 (takes an               **
**  initializer_opt)                                                        **
*****************************************************************************/
void getVarVal(treeNodePtr ptr)
{
   int flag = 0;

   if((ptr->ruleLabel != 690) && (ptr->ruleLabel != 561))
   {
      ptr = ptr->children[0];   
      while(flag == 0)
      {
         switch(ptr->ruleLabel)
         {
            /* save integer value */
            case 405: case 408: case 409: 
               tmpUnion->v.in = ptr->children[0]->leafTokenPtr->ival;
               flag = 1;
               break;
            /* save character value */
            case 406: case 407:
               tmpUnion->v.ch = ptr->children[0]->leafTokenPtr->sval[0];  
               flag = 1;
               break;         
            default:
               break;
         }
         if(ptr->children[0])
            ptr = ptr->children[0];
         else
            break;
      }
      if(flag == 0)
      {
         if(tmpSymTabEntry2->dataType == 280)
            tmpUnion->v.ch = NULL;   
         else if(tmpSymTabEntry2->dataType == 292)
            tmpUnion->v.in = 0;
      }
   }
}


/* ***************************************************************************
**  Function that inserts a classname into the classname table along with   **
**  its field and method names and types (note: takes a class_specifier     **
**  tree node)                                                              **   
*****************************************************************************/
int handleClassDecl(char *name, treeNodePtr ptr)
{   
   int i, flag = 1;
   int index = hash(name);
   classnameEntryPtr curr = NULL;
   
   tmpClassEntryPtr = 
      (classnameEntryPtr)calloc(1,sizeof(struct classnameEntry));    
   tmpClassValPtr = 
      (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
   tmpClassValPtr->valPtr = 
      (symTabValPtr)calloc(1, sizeof(union symTabVal));       
   tmpClassValPtr->dataType = 0;  
   for(i=0; i<MAX_CLASS_VALS; i++)
      tmpClassEntryPtr->vals[i] = NULL;
   tmpClassEntryPtr->className = strdup(name);
   tmpClassEntryPtr->file = strdup(yyFileName);
   tmpClassEntryPtr->nextInBucket = NULL;    

   //Initialize the global classname table if it doesn't exist yet
   if(globClassnameTabPtr == NULL)
   {
      globClassnameTabPtr = 
        (classnameTabPtr)calloc(1, sizeof(struct classnameTab));
      globClassnameTabPtr->numBuckets = 0;
      globClassnameTabPtr->numEntries = 0;
      for(i=0; i<HASH_TABLE_SIZE; i++)
         globClassnameTabPtr->table[i] = NULL;
   }  

   //if the bucket is not empty 
   if(globClassnameTabPtr->table[index] != NULL)                             
   {   
      curr = globClassnameTabPtr->table[index];
      //walk to the end of the list
      while(flag == 1)
      {   		  
         /* ************************************************
         *  print error message if it is a re-declaration  *
         **************************************************/		 
         if( !strcmp(curr->className, name))
         {
            for(i=0; i<MAX_CLASS_VALS; i++)
            {
               if(curr->vals[i] != NULL) 
                  yyerror("semantic error (class already declared)");
            }
            //if not a redeclaration
            free(tmpClassEntryPtr->className);
            free(tmpClassEntryPtr);
            tmpClassEntryPtr = curr;
            flag = 0;
         }		 
         if((curr->nextInBucket != NULL) && (flag == 1))
         {        
            curr = curr->nextInBucket;
         }
         else if(flag == 1)
         {
            //append the entry to the end of the list     
            curr->nextInBucket = tmpClassEntryPtr;
            flag = 0;
         }
      }      
      postorder(ptr, &classnameTabGetVars);
      globClassnameTabPtr->numEntries++;
   }
   else
   {     
      postorder(ptr, &classnameTabGetVars);  
      //make the entry the first item in the bucket      
      globClassnameTabPtr->table[index] = tmpClassEntryPtr;
      globClassnameTabPtr->numEntries++;
      globClassnameTabPtr->numBuckets++; 	  
   }     
}


/* ***************************************************************************
** Postorder function that finds a class's var/func names and types so      **
** they can be added to the classname table                                 **   
*****************************************************************************/
void classnameTabGetVars(treeNodePtr ptr)
{
   treeNodePtr tmpTreePtr;
   switch(ptr->ruleLabel)
   {
      /* ************************************************************
      *  if it's an access_specifier (i.e public/private/protected  *
      **************************************************************/
      case 628:  
      {       
         tmpClassValPtr->privOrPub = 1;
         break;
      }   
      case 630:
      {       
         tmpClassValPtr->privOrPub = 2;
         break;
      }         
       /* ***************************
      *  if it's a type_specifier  *
      *****************************/
      case 550: case 551: case 552: case 553: case 554: case 555:
      {       
         if((ptr->par->par->ruleLabel < 589) ||
            (ptr->par->par->ruleLabel > 592))
            tmpClassValPtr->dataType =ptr->children[0]->leafTokenPtr->category;
         break;
      }
      /* *************************
      *  if it's an identifier   *
      ***************************/      
      case 404:
      {      
         tmpClassValPtr->name = 
            strdup(ptr->children[0]->leafTokenPtr->text);              
         tmpClassValPtr->tokPtr = ptr->children[0]->leafTokenPtr;
         //if it's a pointer to a variable (i.e. NOT A METHOD IDENTIFIER)         
         if(ptr->par->par->par->ruleLabel == 562)
         {
            tmpClassValPtr->varConstFunc = 1;   //var
            tmpClassValPtr->isPtr = 1;
            /* add the field/method info to the class struct */             
            appendFieldOrMethod(); 
            break;
         }           
         //if it's a variable (ALSO NOT A METHOD IDENTIFIER)
         else if(ptr->par->par->ruleLabel == 561)
         {
            tmpClassValPtr->varConstFunc = 1;   //var    
            tmpClassValPtr->isPtr = 0;
            /* add the field/method info to the class struct */             
            appendFieldOrMethod(); 
            break;
         }   
         //IF IT'S A METHOD IDENTIFIER (i.e. method declaration)
         else if(ptr->par->par->ruleLabel == 564)
         {     
            tmpTreePtr = ptr->par->par->children[2];          
            tmpClassValPtr->varConstFunc = 3;    //func
            /* note: tmpClassValPtr->name and tmpClassValPtr->tokPtr 
               were assigned at the top of case 404 */
            //assign the parameter types to tmpClassValPtr
            postorder(tmpTreePtr, &getMethodParamTypes);
            /* add the field/method info to the class struct */ 
            appendFieldOrMethod();             
            break;       
         }
         break;
      }
	}
   //convert datatype to a smaller number that holds more useful information
   tmpClassValPtr->dataType = 
      simplifyType(tmpClassValPtr->dataType, tmpClassValPtr->isPtr);
}


/* ***************************************************************************
** Uses the global variables tmpClassEntryPtr and tmpClassValPtr to add a   **
** copy of tmpClassValPtr onto tmpClassEntryPtr's array of values that are  **
** within the scope of tmpClassEntryPtr                                     **
*****************************************************************************/
void appendFieldOrMethod()
{
   int i;
   fieldsAndMethodsPtr tmpPtr = 
      (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
   tmpPtr->valPtr = 
      (symTabValPtr)calloc(1, sizeof(union symTabVal));      
   tmpPtr->name = strdup(tmpClassValPtr->name);   
   tmpPtr->privOrPub = tmpClassValPtr->privOrPub;      
   tmpPtr->varConstFunc = tmpClassValPtr->varConstFunc;  
   tmpPtr->dataType = tmpClassValPtr->dataType; 
   tmpPtr->isPtr = tmpClassValPtr->isPtr;
   tmpPtr->tokPtr = tmpClassValPtr->tokPtr;
   switch(tmpPtr->varConstFunc)
   {
      case 1: case 2:
         tmpPtr->valPtr->v.in = tmpClassValPtr->valPtr->v.in;
         tmpPtr->valPtr->v.ch = tmpClassValPtr->valPtr->v.ch;
         tmpPtr->isPtr = tmpClassValPtr->isPtr;
         tmpPtr->valPtr->v.ptrAddr = tmpClassValPtr->valPtr->v.ptrAddr;
         tmpClassValPtr->valPtr->v.in = 0;
         tmpClassValPtr->valPtr->v.ch = NULL;
         tmpClassValPtr->isPtr = 0;
         tmpClassValPtr->valPtr->v.ptrAddr = 0;         
         break;
      case 3:
         tmpPtr->valPtr->ft.pOneType = simplifyType(
            tmpClassValPtr->valPtr->ft.pOneType, 
            tmpClassValPtr->valPtr->ft.pOneIsPtr);
         tmpPtr->valPtr->ft.pOneIsPtr = tmpClassValPtr->valPtr->ft.pOneIsPtr;
         tmpPtr->valPtr->ft.pTwoType = simplifyType(
            tmpClassValPtr->valPtr->ft.pTwoType, 
            tmpClassValPtr->valPtr->ft.pTwoIsPtr);
         tmpPtr->valPtr->ft.pTwoIsPtr = tmpClassValPtr->valPtr->ft.pTwoIsPtr; 
         tmpClassValPtr->valPtr->ft.pOneType = 0;
         tmpClassValPtr->valPtr->ft.pOneIsPtr = 0;
         tmpClassValPtr->valPtr->ft.pTwoType = 0;
         tmpClassValPtr->valPtr->ft.pTwoIsPtr = 0;         
         break;
   }
      
   for(i=0; i<MAX_CLASS_VALS; i++)
   {
      if(tmpClassEntryPtr->vals[i] == NULL)
      {
         tmpClassEntryPtr->vals[i] = tmpPtr;
         break;
      }
   }   
}


/* ***************************************************************************
** Recursive postorder function that assigns function parameter types and   **
** saves them to a global variable(takes ptr to parameter_declaration_list) **
*****************************************************************************/
void getParamTypes(treeNodePtr ptr)
{
   static int flag = 0;
   switch(ptr->ruleLabel)
   {
      case 551: case 552: case 553: case 554: case 555:
         if(tmpUnion->ft.pOneType == 0)
         {      
            tmpUnion->ft.pOneType = 
               ptr->children[0]->leafTokenPtr->category;                         
            flag = 1;
         }
         else if(tmpUnion->ft.pTwoType == 0)
         {
            tmpUnion->ft.pTwoType = 
               ptr->children[0]->leafTokenPtr->category;                             
            flag = 2;
         }  
         else
            yyerror("Too many function parameters (2 max)\n");         
         break;
      default:
         break;
   }   
}


/* ***************************************************************************
** Recursive postorder function that assigns method parameter types and     **
** saves them to a global variable(takes ptr to parameter_declaration_list) **
*****************************************************************************/
void getMethodParamTypes(treeNodePtr ptr)
{
   static int flag = 0;
   switch(ptr->ruleLabel)
   {
      case 551: case 552: case 553: case 554: case 555:
         if(tmpClassValPtr->valPtr->ft.pOneType == 0)
         {      
            tmpClassValPtr->valPtr->ft.pOneType = 
               ptr->children[0]->leafTokenPtr->category;  
            flag = 1;
         }
         else if(tmpClassValPtr->valPtr->ft.pTwoType == 0)
         {
            tmpClassValPtr->valPtr->ft.pTwoType = 
               ptr->children[0]->leafTokenPtr->category;  
            flag = 2;
         }  
         else
         {        
            yyerror("Too many method parameters (2 max)\n");         
         }
         break;
      default:
         break;
   }   
}



/* ***************************************************************************
**  Preorder function that finds all of the subtrees that need to by type   **
**  checked, then calls a postorder function to assign types and pass them  **
**  up the tree. NOTE: Redeclarations have already been checked for.        **
*****************************************************************************/
int typeCheckTree(treeNodePtr ptr)
{
   int i;
   switch(ptr->ruleLabel)
   {    
      /* *********************************************
      *  Change of scope, so update currSymTablePtr  *
      ***********************************************/      
      case 593: case 594: case 595: case 596:
      {  
         if(ptr->ruleLabel == 596)
            tabName = strdup(ptr->children[0]->leafTokenPtr->text); 
         else
            tabName = strdup(getFuncName(ptr));           
         for(i=0; i<MAX_CLASS_VALS; i++)
            if(currSymTablePtr->child[i])         
               if(!strcmp(currSymTablePtr->child[i]->tableName, tabName))
               {
                  currSymTablePtr = currSymTablePtr->child[i];                  
                  break;
               } 
         if(ptr->ruleLabel == 593)
            postorder(ptr->children[1], &synthesizeTypeHelper);
         else if(ptr->ruleLabel == 594)
            postorder(ptr->children[2], &synthesizeTypeHelper);
         else if(ptr->ruleLabel == 595)
            postorder(ptr->children[4], &synthesizeTypeHelper);  
         if(ptr->ruleLabel == 596)
         {
            inConstructorFlag = 1;
            tabName = strdup(ptr->children[0]->leafTokenPtr->text);
            postorder(ptr->children[3], &synthesizeTypeHelper);
         }
         break;
      }
      case 900:  
      {
         if(!strcmp(ptr->leafTokenPtr->text, "}") ) 
             if( (ptr->par->ruleLabel == 520) && 
                  (ptr->par->par->ruleLabel == 599) 
               )
                  if(currSymTablePtr->par)
                     currSymTablePtr = currSymTablePtr->par;
         if(inConstructorFlag == 1)
            inConstructorFlag = 0;
         break;  
      }
      default:
         break;
   }
}


/* ***************************************************************************
** Postorder helper function that assigns types and passes them up the tree **
*****************************************************************************/
void synthesizeTypeHelper(treeNodePtr ptr)
{
   treeNodePtr tmpNodePtr;
   symTabEntryPtr theEntry, tmpSTE, tmpInstance;
   classnameEntryPtr tmpClassEntry;
   fieldsAndMethodsPtr tmpFME;
   int i;
   
   //synthesize up the tree
   if((ptr->children[0]) && (ptr->numChildren > 0))
   {
      if(ptr->children[0]->type[0] != 0)
      {
         for(i=0; i<5; i++)
            ptr->type[i] = ptr->children[0]->type[i];
         if(ptr->type[0] == 9)
            ptr->returnType = ptr->children[0]->returnType;
      }
   }
         
   switch(ptr->ruleLabel)
   {
      /* *******************************************
      *                identifier                  *
      *********************************************/    
      case 404:
      {      
         if(!strcmp(ptr->children[0]->leafTokenPtr->text, "cout"))
            ptr->type[0] = 11;
         else if(!strcmp(ptr->children[0]->leafTokenPtr->text, "cin"))
            ptr->type[0] = 10;
            
            
         if(inConstructorFlag)
         {
            tmpClassEntry = classnameTabLookup(tabName); 
            if(!tmpClassEntry)
               yyerror("semantic error (class name not found)", tabName);
            tmpFME = classnameFieldLookup(
               tmpClassEntry, ptr->children[0]->leafTokenPtr->text);
            if(!tmpFME)
               yyerror(
                  "semantic error (class field or method name not declared)",
                  ptr->children[0]);               
         }      
         else if( (ptr->par->ruleLabel != 429) &&
             (ptr->par->ruleLabel != 430) )
         {
            tmpSTE = symTableLookup(
               currSymTablePtr, ptr->children[0]->leafTokenPtr->text);
            if(!tmpSTE)
               yyerror("semantic error (variable or function not declared)", 
               ptr->children[0]);               
         }
         else      //field or method within a class instance
         {
            tmpInstance = symTableLookup(currSymTablePtr, ptr->par-> 
               children[0]->children[0]->children[0]->leafTokenPtr->text);
            if(!tmpInstance)
               yyerror("semantic error (class instance not declared)",
               ptr->par->children[0]->children[0]->children[0]);               
            tmpSTE = symTabFieldLookup(
               tmpInstance, ptr->children[0]->leafTokenPtr->text);
            if(!tmpSTE)
               yyerror("semantic error (class field or method not declared)",
               ptr->children[0]);               
         }

         if(inConstructorFlag)
         {
            switch(tmpFME->varConstFunc)
            {
               case 1: case 2:
                  ptr->type[0] = tmpFME->dataType;
                  break;
               case 3:
                  ptr->returnType = tmpFME->dataType;
                  ptr->type[0] = 9;
                  ptr->type[1] = tmpFME->valPtr->ft.pOneType;
                  ptr->type[2] = tmpFME->valPtr->ft.pTwoType;
                  break;
            }         
         }
         else
         {
            switch(tmpSTE->varConstFuncClass)
            {
               case 1: case 2:
                  ptr->type[0] = tmpSTE->dataType;
                  break;
               case 3:
                  ptr->returnType = tmpSTE->dataType;
                  ptr->type[0] = 9;
                  ptr->type[1] = tmpSTE->valPtr->ft.pOneType;
                  ptr->type[2] = tmpSTE->valPtr->ft.pTwoType;
                  break;
               default:
                  break;
            }
         }         
         break;
      }
      /* *******************************************
      *           simple_type_specifier            *
      *********************************************/      
      case 550:
      {
         if(!strcmp("string", ptr->children[0]->leafTokenPtr->text))
            ptr->type[0] = 4;
         else if(!strcmp("ofstream", ptr->children[0]->leafTokenPtr->text))
            ptr->type[0] = 11;
         else if(!strcmp("ifstream", ptr->children[0]->leafTokenPtr->text))
            ptr->type[0] = 10;
         break;
      }
      /* *******************************************
      *         postfix_expression (array)         *
      *********************************************/   
      case 427:                        
      {
         //subscript must be an int (or bool)
         if((ptr->children[2]->type[0] != 1) ||
            (ptr->children[2]->returnType != 1))
            yyerror("semantic error (invalid array subscript)"); 
         break;      
      }     
      /* *******************************************
      *    postfix_expression (function call)      *
      *********************************************/
      case 428:                       
      {     
         //function call signature must match what is in sym table
         if((ptr->children[0]->ruleLabel != 429)&& 
            (ptr->children[0]->ruleLabel != 430) )
         {                            
            theEntry = symTableLookup(currSymTablePtr, 
               ptr->children[0]->children[0]->children[0]->leafTokenPtr->text);
            ptr->type[0] = 9;    //function
            //copy of the return type from the symbol table
            ptr->returnType = theEntry->dataType;             
            /* Get the rest of the type information for the tree node 
               (up to two parameter types) */            
            tmpNodePtr = ptr->children[2]->children[0];
            for(i=2; i<4; i++)     
            {
               while(tmpNodePtr)
               {
                  if(tmpNodePtr->ruleLabel == 900)
                  {  //if it's an identifier, it is in the symbol table                  
                     if(tmpNodePtr->leafTokenPtr->category == 258)
                     {
                        tmpSTE = symTableLookup(
                           currSymTablePtr, tmpNodePtr->leafTokenPtr->text);                       
                        ptr->type[i-1] = 
                           simplifyType(tmpSTE->dataType, tmpSTE->isPtr);
                     }
                     else
                     {  //if it's a literal
                        ptr->type[i-1] = 
                        simplifyType(tmpNodePtr->leafTokenPtr->category, 0);
                     }
                     if(ptr->children[2]->children[0]->ruleLabel == 436)
                        tmpNodePtr =ptr->children[2]->children[0]->children[2];
                     else
                        tmpNodePtr = NULL;
                     break;
                  }  
                  tmpNodePtr = tmpNodePtr->children[0];
               }               
            }            
            checkTypes(9, ptr, theEntry);
         }
         /* *******************************************
         *   postfix_expression (class method call)   *
         *********************************************/         
         else                          
         {                         
       
         }
         
         break;      
      }          
      /* *******************************************
      *   postfix_expression (class var or func)   *
      *********************************************/       
      case 429: case 430:         
      {
         //children[0] must be a class instance
         tmpInstance = symTableLookup(currSymTablePtr, 
            ptr->children[0]->children[0]->children[0]->leafTokenPtr->text);
         if(! tmpInstance)
            yyerror(
               "semantic error (. or -> was used on something that isn't a class)",
               ptr->children[0]->children[0]->children[0]);
         //children[2] must be a member of the class              
         tmpSTE = symTabFieldLookup(
            tmpInstance, ptr->children[2]->children[0]->leafTokenPtr->text);            
         if(! tmpSTE)
            yyerror("semantic error (unknown class field or method)", 
            ptr->children[2]->children[0]);             
         switch(tmpSTE->varConstFuncClass)
         {
            case 1: case 2:           
               ptr->type[0] = tmpSTE->dataType;
               break;
            case 3:
               ptr->returnType = tmpSTE->dataType;
               ptr->type[0] = 9;
               ptr->type[1] = tmpSTE->valPtr->ft.pOneType;
               ptr->type[2] = tmpSTE->valPtr->ft.pTwoType;
               break;
         }
         break;      
      } 
      /* *******************************************
      *      expression and expression_list        *
      *********************************************/ 
      case 507: case 436:                          
      {                                             
         ptr->type[0] = ptr->children[0]->type[0];
         ptr->type[1] = ptr->children[2]->type[0];
         break;      
      }            
      /* *******************************************
      *             unary_expression               *
      *********************************************/      
      case 440:  case 442:                        
      {
         ptr->type[0] = 1;
         break;      
      }   
      /* *******************************************
      *            delete_expression               *
      *********************************************/     
      case 462:                                    
      {
         //right operand must be a pointer variable
         if( (ptr->children[1]->type[0] != 2) && 
             (ptr->children[1]->type[0] != 4) &&
             (ptr->children[1]->type[0] != 6) &&
             (ptr->children[1]->returnType != 2) &&
             (ptr->children[1]->returnType != 4) &&
             (ptr->children[1]->returnType != 6) )
             yyerror("semantic error (can only use 'delete' on pointers)");
          ptr->type[0] = 1;
         break;         
      }         
      /* *******************************************
      *  additive_expression, equality_expression, *
      *  multiplicative_expression                 *
      *********************************************/       
      case 469: case 473: case 474: case 484: case 485:      
      {                                             
                                                    
         //both operands must be int or bool
         if((ptr->children[0]->type[0] != 1) || 
            (ptr->children[2]->type[0] != 1) )
            {
               if((ptr->children[0]->returnType != 1) || 
                  (ptr->children[2]->returnType != 1) )            
                     yyerror("semantic error (incompatible operands)",  
                     ptr->children[1]);
            }
         ptr->type[0] = 1;
          break;
      }         
      /* **********************************************
      * relational_expression, logical_and_expression,*
      * logical_or_expression                         * 
      ************************************************/       
      case 478: case 479: case 492:  case 494:                        
      {                                           
         //both operands must be int or bool    
         if((ptr->children[0]->type[0] != 1) || 
            (ptr->children[2]->type[0] != 1) )
            {
               if((ptr->children[0]->returnType != 1) || 
                  (ptr->children[2]->returnType != 1) )            
                     yyerror("semantic error (incompatible operands)",  
                     ptr->children[1]);
            }
         ptr->type[0] = 1;
          break;         
      }
      /* *******************************************
      *             output_expression              *
      *********************************************/      
      case 476:                                    
      {
    
         //left operand must be type ofstream
         if(ptr->children[0]->type[0] != 11)
            yyerror("semantic error (left operand not ofstream)", 
               ptr->children[1]);
         ptr->type[0] = 11;
         break;         
      }  
      /* *******************************************
      *             input_expression              *
      *********************************************/       
      case 477:                                    
      {
         //left operand must be type ifstream 
         if(ptr->children[0]->type[0] != 10)
            yyerror("semantic error (left operand not ifstream)", 
               ptr->children[1]);
         ptr->type[0] = 10;        
         break;         
      } 
      /* *******************************************
      *          assignment_expression             *
      *********************************************/       
      case 498:                                    
      {       
         //children[0] and children[2] must be the same type
         if( (ptr->children[0]->type[0] == ptr->children[2]->type[0]) ||
             (ptr->children[0]->type[0] == ptr->children[2]->returnType) )
         {
            ptr->type[0] = ptr->children[0]->type[0];         
            break;
         }
         else if((ptr->children[0]->returnType == ptr->children[2]->type[0]) ||
                (ptr->children[0]->returnType == ptr->children[2]->returnType))
         {
            ptr->type[0] = ptr->children[0]->returnType;         
            break;         
         }
         else
         {
            if(ptr->children[0]->children[0]->ruleLabel == 404)
               yyerror("semantic error (incompatible types in assignment)", 
               ptr->children[0]->children[0]->children[0]);
            else
               yyerror("semantic error (incompatible types in assignment)", 
               ptr->children[0]->children[0]->children[0]->children[0]);               
         }      
      }   

      default:
         break;
   }
}



/* ***************************************************************************
** Helper function that checks if types are compatible or allowed for the   **
** given operand.                                                           **
*****************************************************************************/
void checkTypes(int operand, treeNodePtr tNode, symTabEntryPtr symEntry)
{
   switch(operand)
   {
      case 9:    //(function call)
         if(! tNode->type[1])      //if call has no parameters
         {
            if((symEntry->valPtr->ft.pOneType) || 
               (symEntry->valPtr->ft.pTwoType) )
               yyerror("semantic error (not enough params in function call)",
                  tNode->children[0]->children[0]->children[0]);
         }   
         else                           //if call has at least one param
         {
            if(! symEntry->valPtr->ft.pOneType)
               yyerror("semantic error (too many params in function call)", 
                  tNode->children[0]->children[0]->children[0]);
            else if(tNode->type[1] != symEntry->valPtr->ft.pOneType)
               yyerror("semantic error (wrong param types in function call)",
                  tNode->children[0]->children[0]->children[0]);
         }  
         if(! tNode->type[2])           //if call has exactly one param
         {
            if(symEntry->valPtr->ft.pTwoType)
               yyerror("semantic error (not enough params in function call)", 
                  tNode->children[0]->children[0]->children[0]);
         }          
         else                            //if call has two parameters
         {
            if(! symEntry->valPtr->ft.pTwoType)
               yyerror("semantic error (too many params in function call)", 
                  tNode->children[0]->children[0]->children[0]);
            else if(tNode->type[2] != symEntry->valPtr->ft.pTwoType)
               yyerror("semantic error (wrong param types in function call)", 
                  tNode->children[0]->children[0]->children[0]);
         }       
         
         break;
      
   }
   return;
}


/* ***************************************************************************
**  Converts the category integer code into a smaller number that is used   **
**  to simplify type checking                                               **
*****************************************************************************/
int simplifyType(int cat, int isPtr)
{
   switch(cat)
   {
      case INTEGER: case INT: case BOOL: 
         if(isPtr)
            return 2; 
         else
            return 1;
      case CHARACTER: case CHAR: 
         if(isPtr)
            return 4; 
         else
            return 3; 
      case STRING: 
         return 4; 
      case VOID:
         if(isPtr)
            return 6; 
         else
            return 5;
      case CLASS_NAME:
         if(isPtr)
            return 8; 
         else
            return 7; 
      default:
         return cat;
   }
}


/* ***************************************************************************
**  Sums the widths of all entries in the table. ("widths" = #bytes, sum    **
**  of widths = #bytes needed for an "activation record" or "global data    **
**  section").                                                              **
*****************************************************************************/

int addWidth(char *table)
{
   /* ONLY NEED THIS FUNCTION FOR CODE GENERATION */
}

