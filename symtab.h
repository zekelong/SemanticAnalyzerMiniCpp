/* ****************************************************************************
*******************************************************************************
** Zeke Long                                                                 **
** CS 445                                                                    **
** HW 3                                                                      **
** 10/14/2014                                                                **
**                                                                           **
**   Symbol Table (and Class Name Table) header file for 120++ compiler      **
*******************************************************************************
******************************************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_       

#define HASH_TABLE_SIZE 101
#define MAX_CLASS_VALS 15
#define SHIFT 4


/* ****************************************************************************
**                         symbol table structures                           **
******************************************************************************/
typedef struct symTab                       
{
   char *tableName;
   int memSize;             //# of bytes used at this scope
   int isClassInstance;         //1 if class instance, 0 otherwise
   int nestLevel;
   int numBuckets;                                 
   int numEntries;     
   struct symTabEntry *table[HASH_TABLE_SIZE];  //symbol table entries
   struct symTab *par;                 //table with next outer scope
   struct symTab *child[MAX_CLASS_VALS];   /* tables with next inner scope. 
                                             NOTE: class methods will all
                                             have the same parent sym table */
}*symTabPtr;


typedef struct symTabEntry
{
   symTabPtr table;                   //symbol table that the entry belongs to
   char *symbolName;                  //the actual symbol name
   int memOffset;                     /* for use in activation record */
   int varConstFuncClass;             //var: 1, const: 2, func: 3, classInst: 4
   int dataType;                      /* data type of the symbol 
                                        (or return type if function, or type  
                                         being pointed to if pointer) */
   int isPtr;                         // 1 if ptr, 0 otherwise
   union symTabVal *valPtr;           /* ptr to symbol's symTabVal */
   struct token *tokPtr;              /* ptr to symbol's token data 
                                         for error reporting, etc. */
   struct symTabEntry *nextInBucket;  /* pointer to next symbol 
                                             with same hashValue */
}*symTabEntryPtr;


typedef union symTabVal
{	
   struct varVal{
      int in;               //integer value
      char ch;              //char value  
      long int ptrAddr;     //relative address of pointer
      /* NOTE: string values are stored in a string class instance 
         (i.e. cv struct)
      */
   }v;
   struct funcTypes{
      int pOneType;       //parameter type
      int pOneIsPtr;      //yes: 1, no:0
      int pTwoType;       //parameter type
      int pTwoIsPtr;      //yes: 1, no:0
   }ft;
   struct classVals{	
      char *className;    /* name of the classnameEntry struct that the 
                             variable is declared as */
      struct symTabEntry *vals[MAX_CLASS_VALS];
   }cv;
}*symTabValPtr;


/* ****************************************************************************
**                        classname table structures                         **
******************************************************************************/
typedef struct classnameTab                     
{
   int numBuckets;                                 
   int numEntries;                                  
   struct classnameEntry *table[HASH_TABLE_SIZE];  //array of table entries
}*classnameTabPtr;


/* struct to store name, field names, and function names of each Class 
   and file it was declared in  */
typedef struct classnameEntry
{
   char *className;          //the replacement name for the type
   char *file;               //file that it was declared in
   struct fieldsAndMethods *vals[MAX_CLASS_VALS];
   struct classnameEntry *nextInBucket;  //for class names with same hashValue
}*classnameEntryPtr;	


typedef struct fieldsAndMethods
{
   char *name;
   int privOrPub;       //private: 1, public: 2
   int varConstFunc;    //var: 1, const: 2, func: 3
   int dataType;        //(note: return type if function)
   int isPtr;           //yes: 1, no: 0
   struct token *tokPtr;   //pointer to token data of name
   union symTabVal *valPtr;    //pointer to initial value set by constructor
}*fieldsAndMethodsPtr;

/* ****************************************************************************	
**                            Function Declarations                          **
******************************************************************************/	
   
//Declarations for SYMBOL TABLE functions
int hash(char *);  
void initializeSymTab();
symTabEntryPtr enterSymbol(symTabEntryPtr, symTabValPtr);  /* insert symbol 
                                                              into table */   
void symTableDelete(symTabPtr);                  /* destroy symbol table */
symTabEntryPtr symTableLookup(symTabPtr, char *);     /* lookup symbol */
int addWidth(char *);
void printSymTables();

//Declarations for CLASSNAME TABLE functions   
void enterSystemClass(classnameEntryPtr); /* classes from #includes */                                        
void classnameTabDestroy();                   /* destroy classname table */
classnameEntryPtr classnameTabLookup(char *);     /* lookup classname */
fieldsAndMethodsPtr classnameFieldLookup(classnameEntryPtr, char *);
void printClassnameTab();

/* ****************************************************************************	
**                                 externs                                   **
******************************************************************************/
extern classnameTabPtr globClassnameTabPtr;        /* global classname table */ 
extern symTabPtr globSymTablePtr;              /* global symbol table */
extern symTabPtr currSymTablePtr;	           /* current scope symbol table */
       
       
#endif
