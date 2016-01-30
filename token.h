typedef struct token        /* global token structure */
{
   int nestLevel;          //scope nesting level of inherited symbol table
   int dataType;     /* INT, CHAR, etc. */
   int category;     /* an integer returned by yylex() */
   char *text;       /* the actual string (lexeme) matched */
   int lineNumber;   /* line number where the token occurs */
   char *fileName;   /* file where token occurs */
   int ival;         /* if integer constant, store its value here */
   char *sval;       /* if string constant, calloc space and store
                        the string (w/o quotes and escapes) */
}*tokenPtr;

