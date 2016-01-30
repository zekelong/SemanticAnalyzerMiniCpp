%{
#include<stdio.h>
#include<string.h>
#include <ctype.h> 
#include "token.h"
#include "tree.h"
#include "120++.tab.h"
#include "symtab.h"

extern classnameEntryPtr tmpClassEntryPtr; /* for inserting new classnames */
extern fieldsAndMethodsPtr tmpClassValPtr; /* for inserting new class values */
extern symTabPtr globSymTablePtr;
extern symTabEntryPtr tmpSymTabEntry2;
extern symTabValPtr tmpUnion;
extern char *yyFileName, *saveFileName1, *saveFileName2, *yytext;
extern tokenPtr yytokenStruct;
extern int yychar, numUserIncludes, errors, yylineno;
extern treeNodePtr rootPtr;
FILE *outfile, *inclFile1, *inclFile2;
YY_BUFFER_STATE origFileBuffer, inclFileBuffer1 = 0, inclFileBuffer2 = 0;

int using_namespace_std = 0; /* standard namespace flag */
int include_iostream = 0;   /* cout, cin, endl flag */
int include_fstream = 0;    /* input fstream flag (out stream not allowed)*/
int include_stdlib = 0;    /* rand() and srand() function flag */
int include_time = 0;      /* time() function flag */
int include_string = 0;     /* string definition class flag */

/* function declarations */
int comment();	
void handleUserInclude();							
void identifierOrClassName();
void activateSysFuncs(int);
%}


OctDig                  [0-7]
Dig                     [0-9]
Lett                    [a-zA-Z_]
HexDig                  [a-fA-F0-9]
Exp                     [Ee][+-]?{Dig}+
BExp                    "P"[+-]?{Dig}+
FS                      (f|F|l|L)
IS                      (u|ul|uL|ull|uLL|U|Ul|UL|Ull|ULL|l|lu|lU|L|Lu|LU|ll|llu|llU|LL|LLu|LLU)
ESeq					      \\((['\"\?\\abfnrtv])|({OctDig}{0,3})|(x{HexDig}+))
CChar					      (\\.|[^\\'])|{ESeq}											 
WS                      [ \t\f]*
LIT                     \"(\\.|[^\\"])*\" 


%%


"//".*					   { /* one line comment */ }
"/*"                    { comment(); /* multi-line comment */ }

\n                      { yylineno++; }
[ \t\f]+                { }

"#include"{WS}"<iostream>"      { activateSysFuncs(1);
                                  include_iostream = 1; include_string = 1; }
"#include"{WS}"<stdio.h>"       { activateSysFuncs(1);
                                  include_iostream = 1; include_string = 1; }
"#include"{WS}"<cstdlib>"       { include_stdlib = 1; }
"#include"{WS}"<stdlib.h>"      { include_stdlib = 1; }
"#include"{WS}"<ctime>"         { include_time = 1; }
"#include"{WS}"<time.h>"        { include_time = 1; }
"#include"{WS}"<string.h>"      { activateSysFuncs(3); include_string = 1; }
"#include"{WS}"<cstring>"       { activateSysFuncs(3); include_string = 1; }
"#include"{WS}"<string>"        { activateSysFuncs(3); include_string = 1; }
"#include"{WS}"<fstream>"       { activateSysFuncs(4); include_fstream = 1; 
                                  include_string = 1; }
"#include"{WS}\"[^\\"]+\"       { numUserIncludes += 1; handleUserInclude(); }	
"using"{WS}"namespace"{WS}"std"{WS}";"  { activateSysFuncs(3); 
                                 using_namespace_std = 1;include_string = 1; }
                                 
"extern"                { /* no-op... allowed but ignored */ }																					
"long"                  { /* no-op... allowed but ignored */ }					  
"short"                 { /* no-op... allowed but ignored */ }
"signed"                { /* no-op... allowed but ignored */ }
"unsigned"              { /* no-op... allowed but ignored */ }
"static"                {   }	
"const"                 { yylval.t = saveTokStruct(CONST); 
                          return yylval.t->leafTokenPtr->category;} 			  
">>"                    { yylval.t = saveTokStruct(SR); 
                          return yylval.t->leafTokenPtr->category;}
"<<"                    { yylval.t = saveTokStruct(SL); 
                          return yylval.t->leafTokenPtr->category;}
"class"	               { yylval.t = saveTokStruct(CLASS); 
                          return yylval.t->leafTokenPtr->category;}	
"struct"                { yylval.t = saveTokStruct(STRUCT); 
                          return yylval.t->leafTokenPtr->category;}						  
"..."                   { yylval.t = saveTokStruct(ELLIPSIS); 
                          return yylval.t->leafTokenPtr->category;}
"_"                     { yylval.t = saveTokStruct(UNDERSCORE); 
                          return yylval.t->leafTokenPtr->category;}						  
"break"                 { yylval.t = saveTokStruct(BREAK); 
                          return yylval.t->leafTokenPtr->category;}
"case"                  { yylval.t = saveTokStruct(CASE); 
                          return yylval.t->leafTokenPtr->category;}
"continue"              { yylval.t = saveTokStruct(CONTINUE); 
                          return yylval.t->leafTokenPtr->category;}
"default"               { yylval.t = saveTokStruct(DEFAULT); 
                          return yylval.t->leafTokenPtr->category;}
"do"                    { yylval.t = saveTokStruct(DO); 
                          return yylval.t->leafTokenPtr->category;}
"else"                  { yylval.t = saveTokStruct(ELSE); 
                          return yylval.t->leafTokenPtr->category;}
"for"                   { yylval.t = saveTokStruct(FOR); 
                          return yylval.t->leafTokenPtr->category;}
"if"                    { yylval.t = saveTokStruct(IF); 
                          return yylval.t->leafTokenPtr->category;}
"int"                   { yylval.t = saveTokStruct(INT); 
                          return yylval.t->leafTokenPtr->category;}                         
"float"                 { yylval.t = saveTokStruct(INT); 
                          return yylval.t->leafTokenPtr->category;}
"double"                { yylval.t = saveTokStruct(INT); 
                          return yylval.t->leafTokenPtr->category;}
"char"                  { yylval.t = saveTokStruct(CHAR); 
                          return yylval.t->leafTokenPtr->category;}                           
"return"                { yylval.t = saveTokStruct(RETURN); 
                          return yylval.t->leafTokenPtr->category;}
"switch"                { yylval.t = saveTokStruct(SWITCH); 
                          return yylval.t->leafTokenPtr->category;}
"void"                  { yylval.t = saveTokStruct(VOID); 
                          return yylval.t->leafTokenPtr->category;}
"while"                 { yylval.t = saveTokStruct(WHILE); 
                          return yylval.t->leafTokenPtr->category;}
"FILE"                  { yylval.t = saveTokStruct(FILEPTR); 
                          return yylval.t->leafTokenPtr->category;}
"new"	                  { yylval.t = saveTokStruct(NEW); 
                          return yylval.t->leafTokenPtr->category;}
"private"               { yylval.t = saveTokStruct(PRIVATE); 
                          return yylval.t->leafTokenPtr->category;}
"protected"	            { yylval.t = saveTokStruct(PRIVATE); 
                          return yylval.t->leafTokenPtr->category;}
"public"	               { yylval.t = saveTokStruct(PUBLIC); 
                          return yylval.t->leafTokenPtr->category;}		
"bool"                  { yylval.t = saveTokStruct(BOOL); 
                          return yylval.t->leafTokenPtr->category;}
"true"	               { yylval.t = saveTokStruct(TRUE); 
                          return yylval.t->leafTokenPtr->category;}
"false"	               { yylval.t = saveTokStruct(FALSE); 
                          return yylval.t->leafTokenPtr->category;}	
"compl"                 { yylval.t = saveTokStruct(COMPL); 
                          return yylval.t->leafTokenPtr->category;}
"not"                   { yylval.t = saveTokStruct(NOT); 
                          return yylval.t->leafTokenPtr->category;}							  
"and"                   { yylval.t = saveTokStruct(ANDAND); 
                          return yylval.t->leafTokenPtr->category;}	
"or"                    { yylval.t = saveTokStruct(OROR); 
                          return yylval.t->leafTokenPtr->category;}							  
"not_eq"                { yyerror("semantic error: not_eq not supported"); }							  
"asm"                   { yyerror("semantic error: asm not supported"); }
"using"                 { yyerror("semantic error: using only supported for namespace std");}
"auto"                  { yyerror("semantic error: auto not supported"); }
"const_cast"            { yyerror("semantic error: const_cast not supported");}
"dynamic_cast"          { yyerror(
                          "semantic error: dynamic_cast not supported"); }
"enum"                  { yyerror("semantic error: enum not supported"); }
"explicit"              { yyerror("semantic error: explicit not supported"); }
"export"                { yyerror("semantic error: export not supported"); }
"friend"                { yyerror("semantic error: friend not supported"); }
"inline"                { yyerror("semantic error: inline not supported"); }
"mutable"	            { yyerror("semantic error: mutable not supported"); }
"operator"	            { yyerror("semantic error: operator not supported"); }
"register"              { yyerror("semantic error: register not supported"); }
"reinterpret_cast"      { yyerror(
                          "semantic error: reinterpret_cast not supported"); }
"sizeof"                { yyerror("semantic error: sizeof not supported"); }
"static_cast"           { yyerror("semantic error: static_cast not supported");}
"template"              { yyerror("semantic error: template not supported"); }
"this"                  { yyerror("semantic error: this not supported"); }
"throw"                 { yyerror("semantic error: throw not supported"); }
"try"                   { yyerror("semantic error: try not supported"); }
"catch"                 { yyerror("semantic error: catch not supported"); }
"typeid"                { yyerror("semantic error: typeid not supported"); }
"typename"              { yyerror("semantic error: typename not supported"); }
"union"                 { yyerror("semantic error: union not supported"); }                
"virtual"               { yyerror("semantic error: virtual not supported"); }
"volatile"              { yyerror("semantic error: volatile not supported"); }
"wchar_t"               { yyerror("semantic error: wchar_t not supported"); }					  
"bitand"                { yyerror("semantic error: bitand not supported");  }						  
"bitor"                 { yyerror("semantic error: bitor not supported");  }
"and_eq"                { yyerror("semantic error: and_eq not supported");  }
"xor_eq"                { yyerror("semantic error: xor_eq not supported");  }
"or_eq"                 { yyerror("semantic error: or_eq not supported");  }
"xor"                   { yyerror("semantic error: xor not supported"); }
"typedef"               { yyerror("semantic error: typedef not supported"); }	
"->*"                   { yyerror("semantic error: arrowstar not supported"); }			  
0[xX]{HexDig}+{IS}?     { 
                          yyerror("semantic error: hexidecimal not supported"); 
                        }
0{OctDig}+{IS}?         { 
                          yyerror("semantic error: octals not supported"); 
                        }
{Lett}({Lett}|{Dig})*   {  /* check if yytext is actually an IDENTIFIER or
                              a previously assigned class name, then
                              set yylval.t values */
                           identifierOrClassName();
                           return yylval.t->leafTokenPtr->category;						  
                        }
{Dig}+{IS}?             {  yylval.t = saveTokStruct(INTEGER);
                           return yylval.t->leafTokenPtr->category;
                        }	
[LuU]?"'"{CChar}+"'"   	{  yylval.t = saveTokStruct(CHARACTER);
                           return yylval.t->leafTokenPtr->category;
                        }						
{Dig}+{Exp}{FS}?        {  yylval.t = saveTokStruct(INTEGER);
                           return yylval.t->leafTokenPtr->category;
                        }
{Dig}*"."{Dig}+({Exp}|{BExp})?{FS}? { yylval.t = saveTokStruct(INTEGER);
                           return yylval.t->leafTokenPtr->category;
                        }
{Dig}+"."{Dig}*({Exp}|{BExp})?{FS}? { yylval.t = saveTokStruct(INTEGER); 
                           return yylval.t->leafTokenPtr->category;
                        }						
("u8"|u|U|L)?{LIT}      { yylval.t = saveTokStruct(STRING);
                          return yylval.t->leafTokenPtr->category;
                        }
						
">>="                   { yyerror("semantic error: >>= not supported"); }
"<<="                   { yyerror("semantic error: <<= not supported"); }						  
"&="                    { yyerror("semantic error: &= not supported");  }					  
"^="                    { yyerror("semantic error: ^= not supported");  }
"|="                    { yyerror("semantic error: |= not supported");  }
".*"                    { yyerror("semantic error: .* not supported");  }	
"^"                     { yyerror("semantic error: ^ not supported");   }	
"+="                    { yyerror("semantic error: += not supported");   }	
"-="                    { yyerror("semantic error: -= not supported");   }	
"*="                    { yyerror("semantic error: *= not supported");   }	
"/="                    { yyerror("semantic error: /= not supported");   }	
"%="                    { yyerror("semantic error: %= not supported");   }			
"++"                    { yyerror("semantic error: ++ not supported");   }
"--"                    { yyerror("semantic error: -- not supported");   }
"<="                    { yyerror("semantic error: <= not supported");   }
">="                    { yyerror("semantic error: >= not supported");   }
"?"                     { yyerror("semantic error: ? not supported");   }
"%"                     { yyerror("semantic error: % not supported");   }
"&"                     { yyerror("semantic error: & not supported");   }
"&&"                    { yylval.t = saveTokStruct(ANDAND); 
                          return yylval.t->leafTokenPtr->category;}					  
"||"                    { yylval.t = saveTokStruct(OROR); 
                          return yylval.t->leafTokenPtr->category;}					  
"=="                    { yylval.t = saveTokStruct(EQEQ); 
                          return yylval.t->leafTokenPtr->category;}
"!="                    { yylval.t = saveTokStruct(NOTEQ); 
                          return yylval.t->leafTokenPtr->category;}                          
";"                     { yylval.t = saveTokStruct(SM); 
                          return yylval.t->leafTokenPtr->category;}
"{"                     { yylval.t = saveTokStruct(LC); 
                          return yylval.t->leafTokenPtr->category;}
"<%"                    { yylval.t = saveTokStruct(LC); 
                          return yylval.t->leafTokenPtr->category;}						  
"}"                     { yylval.t = saveTokStruct(RC); 
                          return yylval.t->leafTokenPtr->category;}
"%>"                    { yylval.t = saveTokStruct(RC); 
                          return yylval.t->leafTokenPtr->category;}							  
","                     { yylval.t = saveTokStruct(CM); 
                          return yylval.t->leafTokenPtr->category;}
":"                     { yylval.t = saveTokStruct(COLON); 
                          return yylval.t->leafTokenPtr->category;}
"::"                    { yylval.t = saveTokStruct(COLONCOLON); 
                          return yylval.t->leafTokenPtr->category;}
"="                     { yylval.t = saveTokStruct(EQ); 
                          return yylval.t->leafTokenPtr->category;}
"("                     { yylval.t = saveTokStruct(LP); 
                          return yylval.t->leafTokenPtr->category;}
")"                     { yylval.t = saveTokStruct(RP); 
                          return yylval.t->leafTokenPtr->category;}
"["                     { yylval.t = saveTokStruct(LB); 
                          return yylval.t->leafTokenPtr->category;}
"<:"                    { yylval.t = saveTokStruct(LB); 
                          return yylval.t->leafTokenPtr->category;}						  
"]"                     { yylval.t = saveTokStruct(RB); 
                          return yylval.t->leafTokenPtr->category;}
":>"                    { yylval.t = saveTokStruct(RB); 
                          return yylval.t->leafTokenPtr->category;}						  
"."                     { yylval.t = saveTokStruct(DOT); 
                          return yylval.t->leafTokenPtr->category;}		  
"-"                     { yylval.t = saveTokStruct(MINUS); 
                          return yylval.t->leafTokenPtr->category;}
"+"                     { yylval.t = saveTokStruct(PLUS); 
                          return yylval.t->leafTokenPtr->category;}
"*"                     { yylval.t = saveTokStruct(MULT); 
                          return yylval.t->leafTokenPtr->category;}
"/"                     { yylval.t = saveTokStruct(DIV); 
                          return yylval.t->leafTokenPtr->category;}
"<"                     { yylval.t = saveTokStruct(LT); 
                          return yylval.t->leafTokenPtr->category;}
">"                     { yylval.t = saveTokStruct(GT); 
                          return yylval.t->leafTokenPtr->category;}
"|"                     { yylval.t = saveTokStruct(OR); 
                          return yylval.t->leafTokenPtr->category;}					  
"!"                     { yylval.t = saveTokStruct(NOT); 
                          return yylval.t->leafTokenPtr->category;}
"~"                     { yylval.t = saveTokStruct(COMPL); 
                          return yylval.t->leafTokenPtr->category;}						  
"->"                    { yylval.t = saveTokStruct(ARROW); 
                          return yylval.t->leafTokenPtr->category;}									  

%%


/* ***************************************************************************
**                       Function to deal with comments                     **
*****************************************************************************/
comment()
{
   int splat_seen = 0;
   int savedLineNum = yylineno;
	
   for(;;) {
      int ch = input();
      switch (ch) {
         case '\n':
            yylineno++;
            break;	   
         case '*':
            splat_seen = 1;
            break;
         case '/':
            if(splat_seen)
               return;
            else
               splat_seen = 0;
               break;
         case 0: 
            yychar = BAD_TOKEN;
            yylineno = savedLineNum;
            yyerror("Unterminated comment");
            exit(-1);
         case EOF: 
            yychar = BAD_TOKEN;  
            yylineno = savedLineNum;
            yyerror("Unterminated comment");
            exit(-1);
         default:
            splat_seen = 0;
            break;
	   }
	}
}


/* ***************************************************************************
**    Function that determines if yytext is an identifier or class name,    **
**    and sets yylval.t fields accordingly                                  **
*****************************************************************************/
void identifierOrClassName()
{
   char *name = strdup(yytext);
   /* check the classname table */
   classnameEntryPtr type = classnameTabLookup(name);   

   if(type == NULL)
      yylval.t = saveTokStruct(IDENTIFIER);	
   else
      yylval.t = saveTokStruct(CLASS_NAME);        
   free(name);
}


/* ***************************************************************************
**         Called to handle #includes for system functions                  **
*****************************************************************************/
void activateSysFuncs(int val)
{
   if(!tmpClassEntryPtr)
      tmpClassEntryPtr = 
         (classnameEntryPtr)calloc(1, sizeof(struct classnameEntry));
   
   /* handle system includes */    
   switch(val)
   {	  						
      case 3: 
      {
         if(!include_string)
         {
            tmpClassEntryPtr = 
               (classnameEntryPtr)calloc(1, sizeof(struct classnameEntry));      
            tmpClassEntryPtr->className = strdup("string");
            tmpClassEntryPtr->file = strdup(yyFileName);
            tmpClassEntryPtr->vals[0] = 
               (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
            tmpClassEntryPtr->vals[0]->valPtr = 
               (symTabValPtr)calloc(1, sizeof(union symTabVal));               
            tmpClassEntryPtr->vals[0]->name = strdup("c_str");
            tmpClassEntryPtr->vals[0]->privOrPub = 2;
            tmpClassEntryPtr->vals[0]->varConstFunc = 3;
            tmpClassEntryPtr->vals[0]->dataType = simplifyType(CHAR, 1);
            tmpClassEntryPtr->vals[0]->isPtr = 1;
            //add STRING class to classname table
            enterSystemClass(tmpClassEntryPtr);			
         }
         break;
      }
      case 1: case 4: 
      {
         if(!include_string)
         {
            tmpClassEntryPtr = 
               (classnameEntryPtr)calloc(1, sizeof(struct classnameEntry));      
            tmpClassEntryPtr->className = strdup("string");
            tmpClassEntryPtr->file = strdup(yyFileName);
            tmpClassEntryPtr->nextInBucket = NULL;
            tmpClassEntryPtr->vals[0] = 
               (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
            tmpClassEntryPtr->vals[0]->valPtr = 
               (symTabValPtr)calloc(1, sizeof(union symTabVal));               
            tmpClassEntryPtr->vals[0]->name = strdup("c_str");
            tmpClassEntryPtr->vals[0]->privOrPub = 2;
            tmpClassEntryPtr->vals[0]->varConstFunc = 3;
            tmpClassEntryPtr->vals[0]->dataType = simplifyType(CHAR, 1);
            tmpClassEntryPtr->vals[0]->isPtr = 1;
            //add STRING class to classname table
            enterSystemClass(tmpClassEntryPtr);			
         }      
         
         if((!include_fstream)&&(!include_iostream))
         {
            tmpClassEntryPtr = 
               (classnameEntryPtr)calloc(1, sizeof(struct classnameEntry));
            tmpClassEntryPtr->className = strdup("ifstream");
            tmpClassEntryPtr->file = strdup(yyFileName);
 
            tmpClassEntryPtr->vals[0] = 
               (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
            tmpClassEntryPtr->vals[0]->valPtr = 
               (symTabValPtr)calloc(1, sizeof(union symTabVal));               
            tmpClassEntryPtr->vals[0]->name = strdup("close");
            tmpClassEntryPtr->vals[0]->privOrPub = 2;
            tmpClassEntryPtr->vals[0]->varConstFunc = 3;
            tmpClassEntryPtr->vals[0]->dataType = simplifyType(VOID, 0);
            
            tmpClassEntryPtr->vals[1] = 
               (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
            tmpClassEntryPtr->vals[1]->valPtr = 
               (symTabValPtr)calloc(1, sizeof(union symTabVal));               
            tmpClassEntryPtr->vals[1]->name = strdup("eof");
            tmpClassEntryPtr->vals[1]->privOrPub = 2;
            tmpClassEntryPtr->vals[1]->varConstFunc = 3;
            tmpClassEntryPtr->vals[1]->dataType = simplifyType(BOOL, 0);

            tmpClassEntryPtr->vals[2] = 
               (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
            tmpClassEntryPtr->vals[2]->valPtr = 
               (symTabValPtr)calloc(1, sizeof(union symTabVal));               
            tmpClassEntryPtr->vals[2]->name = strdup("ignore");
            tmpClassEntryPtr->vals[2]->privOrPub = 2;
            tmpClassEntryPtr->vals[2]->varConstFunc = 3;
            tmpClassEntryPtr->vals[2]->dataType = simplifyType(VOID, 0);
            
            tmpClassEntryPtr->vals[3] = 
               (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
            tmpClassEntryPtr->vals[3]->valPtr = 
               (symTabValPtr)calloc(1, sizeof(union symTabVal));               
            tmpClassEntryPtr->vals[3]->name = strdup("open");
            tmpClassEntryPtr->vals[3]->privOrPub = 2;
            tmpClassEntryPtr->vals[3]->varConstFunc = 3;
            tmpClassEntryPtr->vals[3]->dataType = simplifyType(VOID, 0);
            tmpClassEntryPtr->vals[3]->valPtr->ft.pOneType = simplifyType(CHAR, 1);
            tmpClassEntryPtr->vals[3]->valPtr->ft.pOneIsPtr = 1;            
            
            //add IFSTREAM class to classname table
            enterSystemClass(tmpClassEntryPtr);   

            tmpClassEntryPtr = 
               (classnameEntryPtr)calloc(1, sizeof(struct classnameEntry));
            tmpClassEntryPtr->className = strdup("ofstream");
            tmpClassEntryPtr->file = strdup(yyFileName);
 
            tmpClassEntryPtr->vals[0] = 
               (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
            tmpClassEntryPtr->vals[0]->valPtr = 
               (symTabValPtr)calloc(1, sizeof(union symTabVal));               
            tmpClassEntryPtr->vals[0]->name = strdup("close");
            tmpClassEntryPtr->vals[0]->privOrPub = 2;
            tmpClassEntryPtr->vals[0]->varConstFunc = 3;
            tmpClassEntryPtr->vals[0]->dataType = simplifyType(VOID, 0);
            
            tmpClassEntryPtr->vals[1] = 
               (fieldsAndMethodsPtr)calloc(1, sizeof(struct fieldsAndMethods));
            tmpClassEntryPtr->vals[1]->valPtr = 
               (symTabValPtr)calloc(1, sizeof(union symTabVal));               
            tmpClassEntryPtr->vals[1]->name = strdup("open");
            tmpClassEntryPtr->vals[1]->privOrPub = 2;
            tmpClassEntryPtr->vals[1]->varConstFunc = 3;
            tmpClassEntryPtr->vals[1]->dataType = simplifyType(VOID, 0);
            tmpClassEntryPtr->vals[1]->valPtr->ft.pOneType = simplifyType(CHAR, 1);
            tmpClassEntryPtr->vals[1]->valPtr->ft.pOneIsPtr = 1;            

            //add OFSTREAM class to classname table
            enterSystemClass(tmpClassEntryPtr);    

            /* *********************************************
            *   Save cout class INSTANCE to symbol table   *
            ***********************************************/ 
            tmpUnion = (symTabValPtr)calloc(1, sizeof(union symTabVal));
            tmpSymTabEntry2 = 
               (symTabEntryPtr)calloc(1, sizeof(struct symTabEntry)); 
            tmpSymTabEntry2->table = globSymTablePtr;
            tmpSymTabEntry2->symbolName = strdup("cout");
            tmpSymTabEntry2->varConstFuncClass = 4;
            tmpSymTabEntry2->dataType = simplifyType(CLASS_NAME, 0);
            tmpUnion->cv.className = strdup("ofstream");             
            classInstanceConstruct();
            enterSymbol(tmpSymTabEntry2, tmpUnion);    

            /* *********************************************
            *   Save cin class INSTANCE to symbol table    *
            ***********************************************/ 
            if(!tmpUnion)
               tmpUnion = (symTabValPtr)calloc(1, sizeof(union symTabVal));
            if(!tmpSymTabEntry2)
               tmpSymTabEntry2 = 
                  (symTabEntryPtr)calloc(1, sizeof(struct symTabEntry)); 
            tmpSymTabEntry2->table = globSymTablePtr;
            tmpSymTabEntry2->symbolName = strdup("cin");
            tmpSymTabEntry2->varConstFuncClass = 4;
            tmpSymTabEntry2->dataType = simplifyType(CLASS_NAME, 0);
            tmpUnion->cv.className = strdup("ifstream");             
            classInstanceConstruct();
            enterSymbol(tmpSymTabEntry2, tmpUnion);    

            /* ***********************************************
            * Save endl const char variable to symbol table  *
            *************************************************/ 
            if(!tmpUnion)
               tmpUnion = (symTabValPtr)calloc(1, sizeof(union symTabVal));
            if(!tmpSymTabEntry2)
               tmpSymTabEntry2 = 
                  (symTabEntryPtr)calloc(1, sizeof(struct symTabEntry));
            tmpSymTabEntry2->table = globSymTablePtr;
            tmpSymTabEntry2->symbolName = strdup("endl");
            tmpSymTabEntry2->varConstFuncClass = 2;
            tmpSymTabEntry2->dataType = simplifyType(CHAR, 0);
            tmpUnion->v.ch = '\n';             
            enterSymbol(tmpSymTabEntry2, tmpUnion);            
         }
         break;
      }
      default:
         break;
   }
}   



/* ***************************************************************************
**  Saves the current filename and changes yyin to be the included file.    **
**  This is because the included files have to parse first to populate the  **
**  classname table                                                         **
*****************************************************************************/
void handleUserInclude()
{  
   char *tmpFileName = NULL;  
   
   /* if the first user include has just been seen */
   if(numUserIncludes == 1)    
   {
      /* save the name of the original file to parse later */
      saveFileName1 = strdup(yyFileName);	
      /* set yyFileName to be the name of the included file */
      tmpFileName = strdup(strchr(yytext, '"')+1);
      free(yyFileName);
      yyFileName = strndup(tmpFileName, strlen(tmpFileName)-1);	  
      free(tmpFileName);
      /* inclFile1 is a global FILE ptr */
      inclFile1 = fopen(yyFileName,"r");    
      if(!inclFile1)
      {
         fprintf(stderr, "\nCould not open included file: %s\n", yyFileName);
         exit(2);
      }
      /* parse the include file before the original file */	  
      inclFileBuffer1 = yy_create_buffer(inclFile1, YY_BUF_SIZE);
      /* save the original buffer for later */
      origFileBuffer = YY_CURRENT_BUFFER;
      yy_switch_to_buffer(inclFileBuffer1);   
   }
   /* if the second user include file has just been seen */
   else if(numUserIncludes == 2)
   {
      /* save the name of the first include file to parse next */
      saveFileName2 = strdup(yyFileName);     
      /* set yyFileName to be the second (nested) user include file */
      tmpFileName = strdup(strchr(yytext, '"')+1);  
      free(yyFileName);	  
      yyFileName = strndup(tmpFileName, strlen(tmpFileName)-1);
      free(tmpFileName);
      /* inclFile2 is a global FILE ptr */	  
      inclFile2 = fopen(yyFileName,"r");    
      if(!inclFile2)
      {
         fprintf(stderr, "\nCould not open included file: %s\n", yyFileName);
         exit(2);
      }
      /* parse the second include file before the first include file */	
      inclFileBuffer2 = yy_create_buffer(inclFile2, YY_BUF_SIZE);
      yy_switch_to_buffer(inclFileBuffer2);   	  
   }
   else if(numUserIncludes > 3)
      yyerror("Too many user include files! (2 maximum)");
}



/* ***************************************************************************
**                                 yywrap()                                 **
*****************************************************************************/
int yywrap()
{	
    if(saveFileName2 != NULL)
   {
      /* Note: the second include file has just been parsed */
      free(yyFileName);
      yyFileName = strdup(saveFileName2);	
      free(saveFileName2);	   
      saveFileName2 = NULL;	   
      yylineno = 1;
      inclFile1 = fopen(yyFileName,"r");    
      if(!inclFile1)
      {
         fprintf(stderr, "\nCould not open included file: %s\n", yyFileName);
         exit(2);
      }	
      yy_switch_to_buffer(inclFileBuffer1);	  				      
      return 0;
   }
    else if(saveFileName1 != NULL)
   {	
      /* Note: the first include file has just been parsed */
      free(yyFileName);
      yyFileName = strdup(saveFileName1);
       free(saveFileName1);	 	   
      saveFileName1 = NULL;	   
      yylineno = 1;
      yyin = fopen(yyFileName,"r");    
      if(!yyin)
      {
         fprintf(stderr, "\nCould not open included file: %s\n", yyFileName);
         exit(2);
      }
      yy_switch_to_buffer(origFileBuffer);	  	   
      return 0;
   }	
   else
   {
      /* free and/or reset the globals that will be reused */
      free(saveFileName1);  free(saveFileName2);
      saveFileName1 = NULL;
      saveFileName2 = NULL;
      numUserIncludes = 0;
      errors = 0;
      if(inclFile1)
      {
         pclose(inclFile1);
         inclFile1 = NULL;
      }
      if(inclFile2)
      {
         pclose(inclFile2);
         inclFile2 = NULL;
      }		  
      return 1;
   }
}
