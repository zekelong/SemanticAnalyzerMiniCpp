/* ***************************************************************************
******************************************************************************
** Zeke Long                                                                **
** CS 445                                                                   **
** 10/14/2014                                                               **
**                                                                          **
**                Symbol Table implementation for 120++ compiler            **
******************************************************************************
*****************************************************************************/

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "token.h"
#include "tree.h"
#include "120++.tab.h"
#include "symtab.h"

extern char *yyFileName;
classnameTabPtr globClassnameTabPtr;    /* global classname table */
symTabPtr globSymTablePtr;              /* global symbol table */
symTabPtr currSymTablePtr;              /* current symbol table */


/* ***************************************************************************
**             Hash function that returns an index value                    **
*****************************************************************************/
int hash(char *key)
{
   int temp = 0;
   int i = 0;

   while( key[i] != '\0' )
   {
      temp = ((temp << SHIFT) + key[i]) % HASH_TABLE_SIZE;
      ++i;
   }
   return temp;
}


/* ***************************************************************************
**      Initializes the symbol table to allow symbols from #includes        **   
*****************************************************************************/
void initializeSymtab()
{
   int i;
   symTabPtr ptr = (symTabPtr)calloc(1, sizeof(struct symTab)); 
   
   ptr->tableName = strdup("global");
   //make ptr the global pointer (already has null values)
   ptr->nestLevel = 0;
   ptr->numBuckets = 0;
   ptr->numEntries = 0;
   for(i=0; i<HASH_TABLE_SIZE; i++)
      ptr->table[i] = '\0';
   currSymTablePtr = ptr;
   globSymTablePtr = ptr;   
}

/* ***************************************************************************
**      Inserts a symbol into the symbol table of the current scope         **   
*****************************************************************************/
symTabEntryPtr enterSymbol(symTabEntryPtr se, symTabValPtr values)
{   
   int i, flag = 1;
   int index = hash(se->symbolName); 
   symTabEntryPtr newEntry = 
      (symTabEntryPtr)calloc(1, sizeof(struct symTabEntry));
   newEntry->valPtr = (symTabValPtr)calloc(1, sizeof(union symTabVal));
      
   switch(se->varConstFuncClass)
   {
      case 1: case 2:  //var or const     
         newEntry->isPtr = se->isPtr;
         if(se->dataType == 1)
            newEntry->valPtr->v.in = values->v.in;
         else if(se->dataType == 3)  
            newEntry->valPtr->v.ch = values->v.ch;                        
         break;
      case 3:   //func   
         newEntry->valPtr->ft.pOneType = 
            simplifyType(values->ft.pOneType, values->ft.pOneIsPtr);
         newEntry->valPtr->ft.pOneIsPtr = values->ft.pOneIsPtr;            
         newEntry->valPtr->ft.pTwoType = 
            simplifyType(values->ft.pTwoType, values->ft.pTwoIsPtr);
         newEntry->valPtr->ft.pTwoIsPtr = values->ft.pTwoIsPtr;          
         break;
      case 4:    //class instance
         newEntry->valPtr->cv.className = strdup(values->cv.className);        
         free(values->cv.className);
         for(i=0; i<MAX_CLASS_VALS; i++)
         {
            if(values->cv.vals[i])
            {          
               newEntry->valPtr->cv.vals[i] = 
                  (symTabEntryPtr)calloc(1, sizeof(struct symTabEntry));
               newEntry->valPtr->cv.vals[i]->symbolName = 
                  strdup(values->cv.vals[i]->symbolName);                
               newEntry->valPtr->cv.vals[i]->varConstFuncClass = 
                  values->cv.vals[i]->varConstFuncClass;
               newEntry->valPtr->cv.vals[i]->dataType = simplifyType(
                  values->cv.vals[i]->dataType,values->cv.vals[i]->isPtr); 
               newEntry->valPtr->cv.vals[i]->isPtr = 
                  values->cv.vals[i]->isPtr;                   
               newEntry->valPtr->cv.vals[i]->valPtr = 
                  values->cv.vals[i]->valPtr;                    
               newEntry->valPtr->cv.vals[i]->tokPtr = 
                  values->cv.vals[i]->tokPtr;                 
               newEntry->valPtr->cv.vals[i]->nextInBucket = NULL; 
      
               values->cv.vals[i] = NULL;
            }
            else
               newEntry->valPtr->cv.vals[i] = NULL;               
         }
         break;
      default:
         break;  
   }  
   newEntry->symbolName = strdup(se->symbolName);
   newEntry->varConstFuncClass = se->varConstFuncClass;
   newEntry->dataType = simplifyType(se->dataType, se->isPtr);
   newEntry->tokPtr = se->tokPtr;  
   newEntry->nextInBucket = NULL;   
   
   /* point to position in the array of symtab entries for the current scope */
   symTabEntryPtr curr = currSymTablePtr->table[index];   
   
   //if the bucket is not empty 
   if(curr != NULL)                             
   {
      //walk to the end of the list
      while(flag == 1)
      {   
         /* *************************************************************
         * print error message if it is a re-declaration (no function   *
         * overloading in 120++, so just the name is all that's needed  *
         * for functions)                                               *
         ***************************************************************/
         if( !strcmp(curr->symbolName, newEntry->symbolName) )
         {
            if(newEntry->tokPtr != NULL)
            {
               fprintf( stderr, 
               "\n\n%s: line %d: error: redeclaration of '%s'\n", 
               newEntry->tokPtr->fileName, newEntry->tokPtr->lineNumber, 
               newEntry->symbolName);
            }
            else
               fprintf( stderr, "\n\nerror: redeclaration of '%s'\n", 
                  newEntry->symbolName);
            exit(-3);
         }        
         if(curr->nextInBucket != NULL)
            curr = curr->nextInBucket;
         else
            flag = 0;
      }
      //append the node to the end of the list
      curr->nextInBucket = 
         (symTabEntryPtr)calloc(1, sizeof(struct symTabEntry));
      curr->nextInBucket = newEntry;
   }
   else
   {
      currSymTablePtr->table[index] = newEntry;
      currSymTablePtr->numBuckets++;
   }  
   currSymTablePtr->numEntries++;
   //the following will help with type checking
   newEntry->table = currSymTablePtr;
     
   se = NULL;
   values = NULL;
   return newEntry;     //so a pointer to it can be saved in a tree node
}


/* ***************************************************************************
** Looks up a symbol in the symbol table. Starts at local scope, and if the **
** symbol isn't found there, it checks the next outer scope, and so on.     **
** Returns a pointer if the symbol is found or NULL otherwise.              **
*****************************************************************************/
symTabEntryPtr symTableLookup(symTabPtr st, char *sName)
{
   int index = hash(sName);
   symTabEntryPtr tmpNodePtr;

   while(st)
   {
      if(st->table[index] != NULL)
      {
         tmpNodePtr = st->table[index];
         while(tmpNodePtr)
         {
            if( !strcmp(tmpNodePtr->symbolName, sName) )
            {
               return tmpNodePtr;
            }
            tmpNodePtr = tmpNodePtr->nextInBucket;
         }
      }
      if(st->par)
         st = st->par;
      else
         st = NULL;   //To end the while loop
   }   
   return NULL;
}


/* ***************************************************************************
** Takes a symTabEntryPtr of a class instance, along with a given           **
** field/method name, and returns the pointer to the field/method if it     **
** exists                                                                   **
*****************************************************************************/
symTabEntryPtr symTabFieldLookup(symTabEntryPtr ptr, char *fName)
{
   int i;
   for(i=0; i<MAX_CLASS_VALS; i++)
      if(ptr->valPtr->cv.vals[i])
         if(!strcmp(ptr->valPtr->cv.vals[i]->symbolName, fName))
            return ptr->valPtr->cv.vals[i];
   return NULL;
}


/* ***************************************************************************
**                          Deletes a symbol table.                         **
*****************************************************************************/
void symTableDelete(symTabPtr st)
{
   symTabEntryPtr se, se1;
   int h, i;

   for (h = 0; h < st->numBuckets; ++h)
      for (se = st->table[h]; se != NULL; se = se1) 
      {
         se1 = se->nextInBucket;			
         free(se->symbolName);  
         free(&se->valPtr->v);         
         free(&se->valPtr->ft);
         for(i=0; i<MAX_CLASS_VALS; i++)
         {
            if(se->valPtr->cv.vals[i])
            {
               free(se->valPtr->cv.vals[i]->symbolName);
               free(se->valPtr->cv.vals[i]);
            }
         }
         free(&se->valPtr->cv.className);
         free(&se->valPtr->cv);
         free(se);
      }
   free(st->table);
   free(st->tableName);   
   free(st);
}


/* ***************************************************************************
**       Prints all of the symbol tables (for debugging purposes)           **
*****************************************************************************/
void printSymTables()
{
   int h = 0, i;
   symTabPtr tmpPtr = globSymTablePtr;
   symTabPtr saveTmpPtr = globSymTablePtr;
   symTabEntryPtr tmpEntryPtr;
   
   printf(
   "\n*******************************************************************\n");
   printf(
   "Symbol Table for %s\n", yyFileName);
   printf(
   "*******************************************************************\n");   
   
   while(tmpPtr)
   {
      if(tmpPtr->par)
         printf("TableName: %s,  Parent: %s", 
            tmpPtr->tableName, tmpPtr->par->tableName);
      else
         printf("TableName: %s", tmpPtr->tableName);
      printf(",   Nest Level: %d,   Symbols:\n", tmpPtr->nestLevel);

      for(i=0; i<HASH_TABLE_SIZE; i++)
      {	  
         if( tmpPtr->table[i] != '\0' )
         {	 
            tmpEntryPtr = tmpPtr->table[i];		
            while(tmpEntryPtr != '\0')
            {			
               switch(tmpEntryPtr->varConstFuncClass)
               {
                  case 1:
                     printf("  Variable: ");
                     if(tmpEntryPtr->isPtr)
                        printf("*");
                     printf("%s,   Type: %d", 
                        tmpEntryPtr->symbolName, tmpEntryPtr->dataType);
                     switch(tmpEntryPtr->dataType)
                     {
                        /* ******************************************
                        *       display the variable's value        *
                        ********************************************/
                        case 1:       /* int */
                           if(tmpEntryPtr->valPtr->v.in)
                              printf(",   value: %d\n", 
                                 tmpEntryPtr->valPtr->v.in);
                           else
                              printf(",   value: NULL\n");                           
                           break;
                        case 3:       /* char */
                           if(tmpEntryPtr->valPtr->v.ch)                        
                              printf(",   value: %c\n", 
                                 tmpEntryPtr->valPtr->v.ch); 
                           else
                              printf(",   value: NULL\n"); 
                           break;
                        default:
                           printf("\n");
                           break;
                     }                        
                     break;
                  case 2:
                     printf("  Constant: %s,   Type: %d", 
                        tmpEntryPtr->symbolName, tmpEntryPtr->dataType);
                     if(tmpEntryPtr->dataType == 1)     //(int or bool)
                        printf(",   Value: %d\n", 
                           tmpEntryPtr->valPtr->v.in);
                     else if(tmpEntryPtr->dataType == 3)   //(char)
                     {                    
                        switch(tmpEntryPtr->valPtr->v.ch)
                        {
                           case '\n':
                              printf(",   Value: '\\n'\n");
                              break;
                           case '\r':
                              printf(",   Value: '\\r'\n");
                              break;
                           case '\t':
                              printf(",   Value: '\\t'\n");
                              break;                              
                           default:
                              printf(",   Value: '%c'\n", 
                                 tmpEntryPtr->valPtr->v.ch);
                              break;  
                        }
                     }
                     break;                     
                  case 3: 
                     printf("  Function: %s(", tmpEntryPtr->symbolName);
                     if(tmpEntryPtr->valPtr->ft.pOneType)
                        printf("%d", tmpEntryPtr->valPtr->ft.pOneType);
                     if(tmpEntryPtr->valPtr->ft.pTwoType)
                        printf(", %d)", tmpEntryPtr->valPtr->ft.pTwoType);
                     else
                        printf(")");
                     printf(",   Return Type: %d\n", tmpEntryPtr->dataType);
                     break; 
                  case 4: 
                     printf("  Class Instance: %s,   Class: %s", 
                        tmpEntryPtr->symbolName, tmpEntryPtr->valPtr->cv.className);
                     printf(",   Fields and Methods:\n");
                     printSymTabClassVals(tmpEntryPtr);
                     break;
               }
               if( tmpEntryPtr->nextInBucket == '\0' )
                  break;
               else 
                  tmpEntryPtr = tmpEntryPtr->nextInBucket;               
            }
         }
      }
      printf("\n"); 
      
      tmpPtr = saveTmpPtr;
      if(tmpPtr->child[h])
      {
         tmpPtr = tmpPtr->child[h];
      }
      else if(tmpPtr->child[0])
      {
         saveTmpPtr = tmpPtr->child[0];
         tmpPtr = tmpPtr->child[0]->child[0];
      }
      else
         tmpPtr = NULL;
      h++;         
   }  
}


/* ***************************************************************************
**  Helper function for printSymTables() that prints the values of a class  **
**  instance                                                                **
*****************************************************************************/
void printSymTabClassVals(symTabEntryPtr ptr)
{
   int i;  
   for(i=0; i<MAX_CLASS_VALS; i++)
   {
      if(ptr->valPtr->cv.vals[i])
      { 		
         while(ptr->valPtr->cv.vals[i] != NULL)
         {	      
            switch(ptr->valPtr->cv.vals[i]->varConstFuncClass)
            {
               case 1: case 2:
                  printf("     Field: ");
                  if(ptr->valPtr->cv.vals[i]->isPtr)
                     printf("*");
                  printf("%s,   Type: %d", 
                     ptr->valPtr->cv.vals[i]->symbolName, 
                     ptr->valPtr->cv.vals[i]->dataType);
                  switch(ptr->valPtr->cv.vals[i]->dataType)
                  {
                     /* ******************************************
                     *           display the value               *
                     ********************************************/
                     case 1:       /* int */
                        printf(",   value: %d", 
                           ptr->valPtr->cv.vals[i]->valPtr->v.in);
                        break;
                     case 3:       /* char */  
                        if(ptr->valPtr->cv.vals[i]->valPtr->v.ch)
                           printf(",   value: %c", 
                              ptr->valPtr->cv.vals[i]->valPtr->v.ch); 
                        else  
                           printf(",   value: (NULL)");                               
                        break;
                     case 4:       /* string */                          
                        printf(",   value: some string");
                        break;                           
                     default:
                        //printf("\n");
                        break;                        
                  }
                  break;
               case 3: 
                  printf("     Method: %s(", 
                     ptr->valPtr->cv.vals[i]->symbolName);
                  if(ptr->valPtr->cv.vals[i]->valPtr->ft.pOneType)
                     printf("%d", ptr->valPtr->cv.vals[i]->valPtr->ft.pOneType);
                  if(ptr->valPtr->cv.vals[i]->valPtr->ft.pTwoType)
                     printf(", %d)", 
                        ptr->valPtr->cv.vals[i]->valPtr->ft.pTwoType);
                  else
                     printf(")");
                  printf(",   Return Type: %d", ptr->valPtr->cv.vals[i]->dataType);
                  break; 
            }
            printf("\n");
            if( ptr->valPtr->cv.vals[i]->nextInBucket != '\0' )
               ptr->valPtr->cv.vals[i] = ptr->valPtr->cv.vals[i]->nextInBucket; 
            else 
               break; 
         }
      }     
   }  
}



/* **************************************************************************
*****************************************************************************
**     FUNCTIONS FOR CLASSNAME TABLE (SEPARATE FROM SYMBOL TABLE)          **
**                   |                             |                       **
**                   V                             V                       **
*****************************************************************************
****************************************************************************/

/* ***************************************************************************
**  Enters a class into the classname table. NOTE: called by                **
**  activateSystemFuncs() when certain #includes are encountered            **
*****************************************************************************/
void enterSystemClass(classnameEntryPtr ptr)
{
   int i;
   int index = hash(ptr->className);
   classnameEntryPtr curr = NULL;

   //Initialize the global classname table if it doesn't exist yet
   if(globClassnameTabPtr == NULL)
      globClassnameTabPtr = 
        (classnameTabPtr)calloc(1, sizeof(struct classnameTab));  

   if(globClassnameTabPtr->table[index] == NULL)                             
   {   
      globClassnameTabPtr->table[index] = ptr;
      globClassnameTabPtr->numEntries++;
      globClassnameTabPtr->numBuckets++; 
   }
   else
   {
      curr = globClassnameTabPtr->table[index];
      while(curr != NULL)
      {
         if(curr->nextInBucket != NULL)
         {
            curr->nextInBucket = ptr;
            globClassnameTabPtr->numEntries++;
            break;
         }
         curr = curr->nextInBucket;
      }
   }
   ptr = NULL;
}


/* **************************************************************************
**                         Deletes the classname table.                     **
****************************************************************************/
void classnameTabDestroy()
{
   classnameEntryPtr te, te1;
   int h, i;

   if(globClassnameTabPtr)
   {
      for (h = 0; h < HASH_TABLE_SIZE; h++)
      {	
         if(globClassnameTabPtr->table[h] == NULL)
            free(globClassnameTabPtr->table[h]);
         else
            for (te = globClassnameTabPtr->table[h]; te != NULL; te = te1) 
            {
               te1 = te->nextInBucket;				 
               free(te->className);		
               free(te->file);
               for(i=0; i<MAX_CLASS_VALS; i++)
               {
                  if(te->vals[i])
                  {
                     free(te->vals[i]->name);
                     free(te->vals[i]->valPtr);
                     free(te->vals[i]);
                  }
               }
               free(te);
            }
      }
      free(globClassnameTabPtr);
      globClassnameTabPtr = NULL;	  
   }
}


/* ***************************************************************************
**   Searches classname table and returns a pointer to the entry if found   **
*****************************************************************************/
classnameEntryPtr classnameTabLookup(char *name)
{
   classnameEntryPtr ce, ce1;
   int index = hash(name);

   if(globClassnameTabPtr)
   {
      if(globClassnameTabPtr->table[index] != NULL)
         for (ce = globClassnameTabPtr->table[index]; ce != NULL; ce = ce1) 
         {
            ce1 = ce->nextInBucket;				 
            if( !strcmp(name, ce->className) )
               return ce;				   
         }	  	 
      return NULL;    /* i.e. class name not found */
   }
}


/* ***************************************************************************
**  Returns a fieldsAndMethodsPtr if the field/method is found in the class **
**  table                                                                   **
*****************************************************************************/
fieldsAndMethodsPtr classnameFieldLookup(classnameEntryPtr ptr, char *fName)
{
   int i;
   for(i=0; i<MAX_CLASS_VALS; i++)
      if(ptr->vals[i])
         if(!strcmp(ptr->vals[i]->name, fName))
            return ptr->vals[i];
   return NULL;
}


/* ***************************************************************************
**     prints the classname table for the current file being compiled       **
*****************************************************************************/
void printClassnameTab()
{
   classnameEntryPtr ce, ce1;
   int i = 0, j = 0;

   printf(
   "\n*******************************************************************\n");
   printf(
   "Class Names for %s (and included files)\n", yyFileName);
   printf(
   "*******************************************************************\n");
   if(globClassnameTabPtr)
   {
      for(i = 0; i < HASH_TABLE_SIZE; i++)
      {
         if(globClassnameTabPtr->table[i] != NULL)
         {
            for (ce = globClassnameTabPtr->table[i]; ce != NULL; ce = ce1) 
            {				 
               printf("Class name: %s", ce->className);
               ce1 = ce->nextInBucket;		
               printf(",  Declared in: %s\n", ce->file);
               printf("  Fields and Methods:\n");
               for(j = 0; j < MAX_CLASS_VALS; j++)
               {
                  if(ce->vals[j] != NULL)
                  {
                     if(ce->vals[j]->privOrPub == 1)
                        printf("    private");
                     else
                        printf("    public");                  
                     switch(ce->vals[j]->varConstFunc)
                     {
                        case 1: case 2:
                           printf(" field: ");
                           if(ce->vals[j]->isPtr)
                              printf("*");
                           printf("%s", ce->vals[j]->name);
                           printf(",   type: %d", ce->vals[j]->dataType);                          
                           switch(ce->vals[j]->dataType)
                           {
                              /* ******************************************
                              * display the value assigned by constructor *
                              ********************************************/
                              case 1:       /* int or bool*/
                                 printf(",   value: %d\n", 
                                    ce->vals[j]->valPtr->v.in);
                                 break;
                              case 3:       /* char */
                                 if(ce->vals[j]->valPtr->v.ch)                              
                                    printf(",   value: %c\n", 
                                       ce->vals[j]->valPtr->v.ch); 
                                 else  
                                    printf(",   value: (NULL)\n");                                    
                                 break;                                 
                              default:
                                 printf("\n");
                                 break;
                           }
                           break;
                        case 3:
                           printf(" method: %s(", ce->vals[j]->name);
                           if(ce->vals[j]->valPtr->ft.pOneType)
                              printf("%d", ce->vals[j]->valPtr->ft.pOneType);
                           if(ce->vals[j]->valPtr->ft.pTwoType)
                              printf(", %d)", ce->vals[j]->valPtr->ft.pTwoType);
                           else
                              printf(")");
                           printf(",   return type: %d\n", ce->vals[j]->dataType);
                           break;
                     }
                  }
               }  
               printf("\n");               
            }	
         }
      }	  	 
   }
   else
      printf("No Classes are declared in the file\n");
   
   printf("\n");
}
