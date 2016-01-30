typedef struct treeNode 
{
   struct treeNode *par;        //pointer to par
   int ruleLabel;                  //label of production rule
   char *rule;                     //name of production rule
   int numChildren;
   struct treeNode *children[9];
   struct token *leafTokenPtr;     //pointer to terminal token struct
   
   /* the following are used for type checking */   
   int nestLevel;                  //nest level in symbol table
   int type[5];      /* intBool: 1, intBoolPointer: 2, char: 3, charPointer: 4, 
                        void: 5, voidPointer: 6, className: 7, classPointer: 8,
                        func: 9, ifstream: 10, ofstream: 11
                     */
   int returnType;   /* (if it's a function) intBool: 1, intBoolPointer: 2, 
                        char: 3, charPointer: 4, void: 5, voidPointer: 6, 
                        className: 7, classPointer: 8 
                     */     
   //pointer to linked list of TAC instructions
   struct codeStruct{
      //NEED TO POPULATE
   }code;                          
   
   //ptr to symbol table entry (if there is one)
   struct symTabEntry *stEntryPtr;                   
 
}*treeNodePtr;

