/* ***************************************************************************
******************************************************************************
**  Zeke Long                                                               **
**  CS 445                                                                  **
**  HW3                                                                     **
**  10/14/2014                                                              **
**                         Yacc parser for 120++                            **
******************************************************************************
*****************************************************************************/


/*
 * Grammar for 120++, a subset of C++ used in CS 120 at University of Idaho
 *
 * Adaptation by Clinton Jeffery, with help from Matthew Brown, Ranger
 * Adams, and Shea Newton. Further adaptation by Zeke Long.
 *
 * Based on Sandro SigalaMULs transcription of the ISO C++ 1996 draft standard.
 * 
 */

%{
#include<stdlib.h>
#include<stdio.h>
#include<stdarg.h>
#include<string.h>
#include "semantic.h"
#include "symtab.h"


#define PRINT_SYNTAX_TREE
#define PRINT_CLASSNAME_TABLE
#define PRINT_SYMBOL_TABLE



extern FILE *yyin;
extern int yylineno;

char *yyFileName, *yytext;
char *saveFileName1 = NULL;  /* used when there are include files */
char *saveFileName2 = NULL;  
tokenPtr yytokenStruct;	   
int errors = 0;
int numUserIncludes = 0;  /* nonzero if there are included files */
int parsingCode = 0;
treeNodePtr rootPtr = NULL;

/* function declarations */
treeNodePtr saveTokStruct(int);				
treeNodePtr allocTree(int, char *, int, ...);
int treePrint(treeNodePtr, int);
int freeTreeMemory(treeNodePtr);
int yyerror(char *, ...);

%}

%union{
   treeNodePtr t;
}

%type < t > file translation_unit primary_expression input_output_expression
%type < t > unqualified_id expression_list id_expression
%type < t > postfix_expression unary_expression unary_operator new_expression
%type < t > new_placement new_type_id new_declarator direct_new_declarator
%type < t > new_initializer delete_expression 
%type < t > multiplicative_expression additive_expression 
%type < t > equality_expression and_expression relational_expression
%type < t > exclusive_or_expression inclusive_or_expression
%type < t > logical_and_expression logical_or_expression conditional_expression
%type < t > assignment_expression assignment_operator expression declarator
%type < t > constant_expression statement labeled_statement 
%type < t > expression_statement statement_seq selection_statement condition
%type < t > iteration_statement for_init_statement jump_statement ptr_operator
%type < t > declaration_statement declaration_seq declaration block_declaration
%type < t > simple_declaration decl_specifier 
%type < t > type_specifier type_id compound_statement simple_type_specifier 
%type < t > init_declarator_list init_declarator direct_declarator declarator_id
%type < t > type_specifier_seq abstract_declarator direct_abstract_declarator
%type < t > parameter_declaration_list initializer
%type < t > parameter_declaration function_definition function_body
%type < t > initializer_clause initializer_list class_specifier class_head
%type < t > member_specification member_declaration access_specifier
%type < t > member_declarator_list member_declarator constant_initializer
%type < t > conversion_function_id conversion_type_id conversion_declarator 
%type < t > declaration_seq_opt expression_list_opt
%type < t > new_placement_opt COMMA_opt operator_function_id operator  
%type < t > expression_opt statement_seq_opt 
%type < t > condition_opt initializer_opt SEMICOLON_opt
%type < t > constant_expression_opt abstract_declarator_opt 
%type < t > type_specifier_seq_opt direct_abstract_declarator_opt
%type < t > member_specification_opt conversion_declarator_opt   
%type < t > identifier boolean_literal literal

%token < t > IDENTIFIER INTEGER CHARACTER STRING CLASS_NAME 

%token < t > LP RP LC RC LB RB SM DOT COLON UNDERSCORE 
%token < t > ELLIPSIS COLONCOLON ARROW CONST

%token < t > BOOL BREAK CASE CHAR CLASS DEFAULT DELETE DO   
%token < t > FALSE TRUE TYPENAME STRUCT SWITCH FOR   
%token < t > IF INT OPERATOR PRIVATE PUBLIC RETURN
%token < t > VOID WHILE NEW CONTINUE FILEPTR BAD_TOKEN

/* Lowest Precedence */
%left < t > THEN ELSE CM OROR ANDAND OR SL SR
%left < t > EQEQ GT LT PLUS MINUS MULT DIV MOD
%left < t > NOT COMPL EQ NOTEQ

%start file

%%

file:
   translation_unit
      {$$ = rootPtr = $1 = allocTree(400, "file", 1, $1); }
   ;

/*----------------------------------------------------------------------
 * Translation unit.
 *----------------------------------------------------------------------*/

translation_unit:
   declaration_seq_opt
      {$$ = allocTree(401, "translation_unit", 1, $1); }
   ;

/*----------------------------------------------------------------------
 * Lexical elements.
 *----------------------------------------------------------------------*/

identifier:
   IDENTIFIER
      {$$ = allocTree(404, "identifier", 1, $1); }
   ;

literal:
   INTEGER
      {$$ = allocTree(405, "literal", 1, $1); 
       $$->type[0] = 1; }
   | CHARACTER
      {$$ = allocTree(406, "literal", 1, $1); 
       $$->type[0] = 3; }
   | STRING
      {$$ = allocTree(407, "literal", 1, $1); 
       $$->type[0] = 4; }
   | boolean_literal
      {$$ = allocTree(409, "literal", 1, $1); 
       $$->type[0] = 1; }
   ;

boolean_literal:
   TRUE
      {$$ = allocTree(414, "boolean_literal", 1, $1); 
       $$->type[0] = 1; }
   | FALSE
      {$$ = allocTree(415, "boolean_literal", 1, $1); 
       $$->type[0] = 1; }
   ;

/*----------------------------------------------------------------------
 * Expressions.
 *----------------------------------------------------------------------*/

primary_expression:
   literal
      {$$ = allocTree(416, "primary_expression", 1, $1); 
       $$->type[0] = $1->type[0]; }       
   | LP expression RP
      {$$ = allocTree(417, "primary_expression", 1, $2); 
       $$->type[0] = $2->type[0]; }
   | id_expression
      {$$ = allocTree(418, "primary_expression", 1, $1); 
       $$->type[0] = $1->type[0]; }
   ;

id_expression:
   unqualified_id
      {$$ = $1;}
   ;

unqualified_id:
   identifier
      {$$ = $1;}
   | operator_function_id
      {$$ = $1;}
   | conversion_function_id
      {$$ = $1;}
   | COMPL CLASS_NAME
      {$$ = allocTree(424, "unqualified_id", 2, $1, $2); }
   ;

postfix_expression:
   primary_expression
      {$$ = $1;}
   | postfix_expression LB expression RB
      {$$ = allocTree(427, "postfix_expression", 4, $1, $2, $3, $4); 
       //(NOTE: array) 
      }
   | postfix_expression LP expression_list_opt RP
      {$$ = allocTree(428, "postfix_expression", 4, $1, $2, $3, $4); 
        //(NOTE: function call)
      }
   | postfix_expression DOT id_expression
      {$$ = allocTree(429, "postfix_expression", 3, $1, $2, $3); 
        //(NOTE: class variable)
      }
   | postfix_expression ARROW id_expression
      {$$ = allocTree(430, "postfix_expression", 3, $1, $2, $3); 
        //(NOTE: class variable)
      }
   ;

expression_list:
   assignment_expression
      {$$ = allocTree(435, "expression_list", 1, $1); }
   | expression_list CM assignment_expression
      {$$ = allocTree(436, "expression_list", 3, $1, $2, $3); }
   ;

unary_expression:
   postfix_expression
      {$$ = $1;}
   | MULT unary_expression
      {$$ = allocTree(440, "unary_expression", 2, $1, $2); }
   | unary_operator unary_expression
      {$$ = allocTree(442, "unary_expression", 2, $1, $2); }
   | new_expression
      {$$ = $1;}
   | delete_expression
      {$$ = $1;}
   ;

unary_operator:
   PLUS
      {$$ = allocTree(445, "unary_operator", 1, $1); }
   | MINUS
      {$$ = allocTree(446, "unary_operator", 1, $1); }
   | NOT
      {$$ = allocTree(447, "unary_operator", 1, $1); }
   | COMPL
      {$$ = allocTree(448, "unary_operator", 1, $1); }
   ;

new_expression:
   NEW new_placement_opt new_type_id new_initializer
      {$$ = allocTree(449, "new_expression", 4, $1, $2, $3, $4); }
   | NEW new_placement_opt new_type_id
      {$$ = allocTree(450, "new_expression", 3, $1, $2, $3); }
   | NEW new_placement_opt LP type_id RP new_initializer
      {$$ = allocTree(451, "new_expression", 6, $1, $2, $3, $4, $5, $6); }
   | NEW new_placement_opt LP type_id RP
      {$$ = allocTree(452, "new_expression", 5, $1, $2, $3, $4, $5); }
   ;

new_placement:
   LP expression_list RP
      {$$ = allocTree(453, "new_placement", 3, $1, $2, $3); }
   ;

new_type_id:
   type_specifier_seq new_declarator
      {$$ = allocTree(454, "new_type_id", 2, $1, $2); }
   | type_specifier_seq 
      {$$ = allocTree(455, "new_type_id", 1, $1); }
   ;

new_declarator:
   ptr_operator
      {$$ = allocTree(456, "new_declarator", 1, $1); }
   | ptr_operator new_declarator
      {$$ = allocTree(457, "new_declarator", 2, $1, $2); }
   | direct_new_declarator
      {$$ = allocTree(458, "new_declarator", 1, $1); }
   ;

direct_new_declarator:
   LB expression RB
      {$$ = allocTree(459, "direct_new_declarator", 3, $1, $2, $3); }
   | direct_new_declarator LB constant_expression RB
      {$$ = allocTree(460, "direct_new_declarator", 4, $1, $2, $3, $4); }
   ;

new_initializer:
   LP expression_list_opt RP
      {$$ = allocTree(461, "new_initializer", 3, $1, $2, $3); }
   ;

delete_expression:
    DELETE identifier
      {$$ = allocTree(462, "delete_expression", 2, $1, $2); }
   ;

multiplicative_expression:
   unary_expression
      {$$ = $1;}
   | multiplicative_expression MULT unary_expression
      {$$ = allocTree(469, "multiplicative_expression", 3, $1, $2, $3); }
   ;

additive_expression:
   multiplicative_expression
      {$$ = $1;}
   | additive_expression PLUS multiplicative_expression
      {$$ = allocTree(473, "additive_expression", 3, $1, $2, $3); }
   | additive_expression MINUS multiplicative_expression
      {$$ = allocTree(474, "additive_expression", 3, $1, $2, $3); }
   ;

input_output_expression:
   additive_expression
      {$$ = $1;}
   | input_output_expression SL additive_expression
      {$$ = allocTree(476, "input_output_expression", 3, $1, $2, $3); }
   | input_output_expression SR additive_expression
      {$$ = allocTree(477, "input_output_expression", 3, $1, $2, $3); }
   ;
   
relational_expression:
	input_output_expression
	| relational_expression LT input_output_expression
      {$$ = allocTree(478, "relational_expression", 3, $1, $2, $3); }   
	| relational_expression GT input_output_expression 
      {$$ = allocTree(479, "relational_expression", 3, $1, $2, $3); }
   ;

equality_expression:
   relational_expression
      {$$ = $1;}
   | equality_expression EQEQ relational_expression
      {$$ = allocTree(484, "equality_expression", 3, $1, $2, $3); }
   | equality_expression NOTEQ relational_expression
      {$$ = allocTree(485, "equality_expression", 3, $1, $2, $3); }      
   ;

and_expression:
   equality_expression
      {$$ = $1;}
   ;

exclusive_or_expression:
   and_expression
      {$$ = $1;}
   ;

inclusive_or_expression:
   exclusive_or_expression
      {$$ = $1;}
   ;

logical_and_expression:
   inclusive_or_expression
      {$$ = $1;}
   | logical_and_expression ANDAND inclusive_or_expression
      {$$ = allocTree(492, "logical_and_expression", 3, $1, $2, $3); }
   ;

logical_or_expression:
   logical_and_expression
      {$$ = $1;}
   | logical_or_expression OROR logical_and_expression
      {$$ = allocTree(494, "logical_or_expression", 3, $1, $2, $3); }
   ;

conditional_expression:
   logical_or_expression
      {$$ = $1;}
   ;

assignment_expression:
   conditional_expression
      {$$ = $1;}
   | logical_or_expression assignment_operator assignment_expression
      {$$ = allocTree(498, "assignment_expression", 3, $1, $2, $3); }
   ;

assignment_operator:
   EQ
      {$$ = allocTree(499, "assignment_operator", 1, $1); }
   ;

expression:
   assignment_expression
      {$$ = allocTree(506, "expression", 1, $1); }
   | expression CM assignment_expression
      {$$ = allocTree(507, "expression", 3, $1, $2, $3); }
   ;

constant_expression:
   conditional_expression
      {$$ = $1; }
   ;

/*----------------------------------------------------------------------
 * Statements.
 *----------------------------------------------------------------------*/

statement:
    labeled_statement
      {$$ = $1;}    
   | expression_statement
      {$$ = $1;}
   | compound_statement
      {$$ = $1;}
   | selection_statement
      {$$ = $1;}
   | iteration_statement
      {$$ = $1;}
   | jump_statement
      {$$ = $1;}
   | declaration_statement
      {$$ = $1;}
   ;

labeled_statement:
    CASE constant_expression COLON statement
      {$$ = allocTree(517, "labeled_statement", 4, $1, $2, $3, $4); }
   | DEFAULT COLON statement
      {$$ = allocTree(518, "labeled_statement", 3, $1, $2, $3); }
   ;

expression_statement:
   expression_opt SM
      {$$ = $1;}
   ;

compound_statement:
   LC statement_seq_opt RC
      {$$ = allocTree(520, "compound_statement", 3, $1, $2, $3); }
   ;

statement_seq:
   statement
      {$$ = $1;}
   | statement_seq statement
      {$$ = allocTree(522, "statement_seq", 2, $1, $2); }
   ;

selection_statement:
   IF LP condition RP statement
      {$$ = allocTree(523, "selection_statement", 5, $1, $2, $3, $4, $5); }
   | IF LP condition RP statement ELSE statement
      {$$ = allocTree(524, "selection_statement", 
                        7, $1, $2, $3, $4, $5, $6, $7); }
   | SWITCH LP condition RP statement
      {$$ = allocTree(525, "selection_statement", 5, $1, $2, $3, $4, $5); }
   ;

condition:
   expression
      {$$ = $1;}
   | type_specifier_seq declarator EQ assignment_expression
      {$$ = allocTree(527, "condition", 4, $1, $2, $3, $4); }
   ;

iteration_statement:
   WHILE LP condition RP statement
      {$$ = allocTree(528, "iteration_statement", 5, $1, $2, $3, $4, $5); }
   | DO statement WHILE LP expression RP SM
      {$$ = allocTree(529, "iteration_statement", 
                       7, $1, $2, $3, $4, $5, $6, $7); }
   | FOR LP for_init_statement condition_opt SM expression_opt RP statement
      {$$ = allocTree(530, "iteration_statement", 
                       8, $1, $2, $3, $4, $5, $6, $7, $8); }
   ;

for_init_statement:
   expression_statement
      {$$ = $1;}
   | simple_declaration
      {$$ = $1;}
   ;

jump_statement:
   BREAK SM
      {$$ = allocTree(533, "jump_statement", 2, $1, $2); }
   | CONTINUE SM
      {$$ = allocTree(534, "jump_statement", 2, $1, $2); }
   | RETURN expression_opt SM
      {$$ = allocTree(535, "jump_statement", 3, $1, $2, $3); }
   ;

declaration_statement:
   block_declaration
      {$$ = $1;}
   ;

/*----------------------------------------------------------------------
 * Declarations.
 *----------------------------------------------------------------------*/

declaration_seq:
   declaration
      {$$ = allocTree(537, "declaration_seq", 1, $1); }
   | declaration_seq declaration
      {$$ = allocTree(538, "declaration_seq", 2, $1, $2); }
;

declaration:
   block_declaration
      {$$ = allocTree(539, "declaration", 1, $1); }
   | function_definition
      {$$ = allocTree(540, "declaration", 1, $1); }
   ;

block_declaration:
   simple_declaration
      {$$ = allocTree(541, "block_declaration", 1, $1); }
   ;

simple_declaration:
   decl_specifier init_declarator_list SM
      {$$ = allocTree(542, "simple_declaration", 3, $1, $2, $3); }
   | decl_specifier SM	
      {$$ = allocTree(543, "simple_declaration", 2, $1, $2); }
   ;

decl_specifier:
   type_specifier
      {$$ = $1; }
   ;	

type_specifier:
   simple_type_specifier
      {$$ = allocTree(548, "type_specifier", 1, $1); }
   | class_specifier 
      {$$ = allocTree(549, "type_specifier", 1, $1); }
   ;

simple_type_specifier:
   CLASS_NAME
      {$$ = allocTree(550, "simple_type_specifier", 1, $1); }
   | CHAR
      {$$ = allocTree(551, "simple_type_specifier", 1, $1); 
        $$->type[0] = 3;}	       
   | BOOL 
      {$$ = allocTree(552, "simple_type_specifier", 1, $1); 
        $$->type[0] = 1;}
   | INT 
      {$$ = allocTree(553, "simple_type_specifier", 1, $1); 
        $$->type[0] = 1;}
   | VOID 
      {$$ = allocTree(555, "simple_type_specifier", 1, $1); 
        $$->type[0] = 5;}
   ;

/*----------------------------------------------------------------------
 * Declarators.
 *----------------------------------------------------------------------*/

init_declarator_list:
   init_declarator
      {$$ = allocTree(558, "init_declarator_list", 1, $1); }
   | init_declarator_list CM init_declarator
      {$$ = allocTree(559, "init_declarator_list", 3, $1, $2, $3); }
   ;

init_declarator:
   declarator initializer_opt
      {$$ = allocTree(560, "init_declarator", 2, $1, $2); }
   ;

declarator:
   direct_declarator
      {$$ = allocTree(561, "declarator", 1, $1); }
   | ptr_operator declarator
      {$$ = allocTree(562, "declarator", 2, $1, $2); }
   ;

direct_declarator:
   declarator_id 
      {$$ = $1;}
   | direct_declarator LP parameter_declaration_list RP
      {$$ = allocTree(564, "direct_declarator", 4, $1, $2, $3, $4); }
   | direct_declarator LB constant_expression_opt RB 
      {$$ = allocTree(568, "direct_declarator", 4, $1, $2, $3, $4); }
   | LP declarator RP 
      {$$ = allocTree(569, "direct_declarator", 3, $1, $2, $3); }
   | CLASS_NAME LP parameter_declaration_list RP
      {$$ = allocTree(570, "direct_declarator", 4, $1, $2, $3, $4);
      /* THIS IS A CLASS CONSTRUCTOR DECLARATION */
      }      
   ;

ptr_operator:
   MULT 
      {$$ = allocTree(571, "ptr_operator", 1, $1); }
   ;

declarator_id:
   id_expression
       {$$ = allocTree(573, "declarator_id", 1, $1); }
   ;

type_id:
   type_specifier_seq abstract_declarator_opt
      {$$ = allocTree(575, "type_id", 2, $1, $2); }
   ;

type_specifier_seq:
   type_specifier type_specifier_seq_opt
      {$$ = allocTree(576, "type_specifier_seq", 2, $1, $2); }
   ;

abstract_declarator:
   ptr_operator abstract_declarator_opt
      {$$ = allocTree(577, "abstract_declarator", 2, $1, $2); }
   | direct_abstract_declarator
      {$$ = allocTree(578, "abstract_declarator", 1, $1); }
   ;

direct_abstract_declarator:	
    direct_abstract_declarator_opt LP parameter_declaration_list RP 
      {$$ = allocTree(579, "direct_abstract_declarator", 4, $1, $2, $3, $4);}
   | direct_abstract_declarator_opt LB constant_expression_opt RB
      {$$ = allocTree(580, "direct_abstract_declarator", 4, $1, $2, $3, $4);}
   | LP abstract_declarator RP
      {$$ = allocTree(581, "direct_abstract_declarator", 3, $1, $2, $3); }
   ;

parameter_declaration_list:
    /* epsilon */
       {$$ = allocTree(582, "parameter_declaration_list", 0); }
   | parameter_declaration
      {$$ = allocTree(583, "parameter_declaration_list", 1, $1); }
   | parameter_declaration_list CM parameter_declaration
      {$$ = allocTree(584, "parameter_declaration_list", 3, $1, $2, $3); }
   ;

parameter_declaration:
    decl_specifier declarator
      {$$ = allocTree(589, "parameter_declaration", 2, $1, $2); }
   | decl_specifier declarator EQ assignment_expression
      {$$ = allocTree(590, "parameter_declaration", 4, $1, $2, $3, $4); }
   | decl_specifier abstract_declarator_opt
      {$$ = allocTree(591, "parameter_declaration", 2, $1, $2); }
   | decl_specifier abstract_declarator_opt EQ assignment_expression
      {$$ = allocTree(592, "parameter_declaration", 4, $1, $2, $3, $4); }
   ;

function_definition:
    declarator function_body
      {$$ = allocTree(593, "function_definition", 2, $1, $2); }
   | decl_specifier declarator function_body
      {$$ = allocTree(594, "function_definition", 3, $1, $2, $3); }
   | decl_specifier CLASS_NAME COLONCOLON declarator function_body
      {$$ = allocTree(595, "function_definition", 5, $1, $2, $3, $4, $5); }
   | CLASS_NAME COLONCOLON declarator function_body
      {$$ = allocTree(596, "function_definition", 4, $1, $2, $3, $4); 
       // ACTUAL CLASS CONSTRUCTOR BODY
      }      
   ;

function_body:
   compound_statement
      {$$ = allocTree(599, "function_body", 1, $1); }
   ;

initializer:
   EQ initializer_clause
      {$$ = allocTree(600, "initializer", 2, $1, $2); }
   | LP expression_list RP
      {$$ = allocTree(601, "initializer", 3, $1, $2, $3); }
   ;

initializer_clause:
   assignment_expression
      {$$ = allocTree(602, "initializer_clause", 1, $1); }
   | LC initializer_list COMMA_opt RC
      {$$ = allocTree(603, "initializer_clause", 4, $1, $2, $3, $4); }
   | LC RC
      {$$ = allocTree(604, "initializer_clause", 2, $1, $2); }
   ;

initializer_list:
   initializer_clause
      {$$ = allocTree(605, "initializer_list", 1, $1); }
   | initializer_list CM initializer_clause
      {$$ = allocTree(606, "initializer_list", 3, $1, $2, $3); }
   ;

/*----------------------------------------------------------------------
 * Classes.
 *----------------------------------------------------------------------*/

class_specifier:
   class_head LC member_specification_opt RC
      {$$ = allocTree(607, "class_specifier", 4, $1, $2, $3, $4); }
   ;

class_head:
     CLASS IDENTIFIER 
      {$$ = allocTree(608, "class_head", 2, $1, $2); 
      /*  ADD THE CLASS NAME TO THE CLASSNAME TABLE 
          note: the field and method names will be added 
          later during a tree traversal */
      handleClassDecl(yylval.t->leafTokenPtr->text, NULL); } 		
   ;

member_specification:
   member_declaration member_specification_opt
      {$$ = allocTree(609, "member_specification", 2, $1, $2); }
   | access_specifier COLON member_specification_opt
      {$$ = allocTree(610, "member_specification", 3, $1, $2, $3); }
   ;

member_declaration:
   decl_specifier member_declarator_list SM
      {$$ = allocTree(611, "member_declaration", 3, $1, $2, $3); }
   | decl_specifier SM
      {$$ = allocTree(612, "member_declaration", 2, $1, $2); }
   | member_declarator_list SM
      {$$ = allocTree(613, "member_declaration", 2, $1, $2); 
       /* CLASS CONSTRUCTOR DECLARATION (ex: construct();) */
      }
   | SM
      {$$ = allocTree(614, "member_declaration", 1, $1); }
   | function_definition SEMICOLON_opt
      {$$ = allocTree(615, "member_declaration", 2, $1, $2); }
   ;

member_declarator_list:
   member_declarator
      {$$ = allocTree(617, "member_declarator_list", 1, $1); }
   | member_declarator_list CM member_declarator
      {$$ = allocTree(618, "member_declarator_list", 3, $1, $2, $3); }
   ;

member_declarator:
   declarator
      {$$ = allocTree(619, "member_declarator", 1, $1); }
   | declarator constant_initializer
      {$$ = allocTree(620, "member_declarator", 2, $1, $2); }
   ;

constant_initializer:
   EQ constant_expression
      {$$ = allocTree(622, "constant_initializer", 2, $1, $2); }
   ;

access_specifier:
   PRIVATE
      {$$ = allocTree(628, "access_specifier", 1, $1); }
   | PUBLIC
      {$$ = allocTree(630, "access_specifier", 1, $1); }
   ;

/*----------------------------------------------------------------------
 * Special member functions.
 *----------------------------------------------------------------------*/

conversion_function_id:
   OPERATOR conversion_type_id
      {$$ = allocTree(631, "conversion_function_id", 2, $1, $2); }
   ;

conversion_type_id:
   type_specifier_seq conversion_declarator_opt
      {$$ = allocTree(632, "conversion_type_id", 2, $1, $2); }
   ;

conversion_declarator:
   ptr_operator conversion_declarator_opt
      {$$ = allocTree(633, "conversion_declarator", 2, $1, $2); }
   ;

/*----------------------------------------------------------------------
 * Overloading.
 *----------------------------------------------------------------------*/

operator_function_id:
   OPERATOR operator
      {$$ = allocTree(641, "operator_function_id", 2, $1, $2); }
   ;

operator:
   NEW
      {$$ = allocTree(642, "operator", 1, $1); }
   | DELETE
      {$$ = allocTree(643, "operator", 1, $1); }
   | NEW LB RB
      {$$ = allocTree(644, "operator", 3, $1, $2, $3); }
   | DELETE LB RB
      {$$ = allocTree(645, "operator", 3, $1, $2, $3); }
   | PLUS
      {$$ = allocTree(646, "operator", 1, $1); }
   | UNDERSCORE
      {$$ = allocTree(647, "operator", 1, $1); }
   | MULT
      {$$ = allocTree(648, "operator", 1, $1); }
   | DIV
      {$$ = allocTree(649, "operator", 1, $1); }
   | COMPL
      {$$ = allocTree(653, "operator", 1, $1); }
   | NOT
      {$$ = allocTree(654, "operator", 1, $1); }
   | EQ
      {$$ = allocTree(655, "operator", 1, $1); }
   | LT
      {$$ = allocTree(656, "operator", 1, $1); }
   | GT
      {$$ = allocTree(657, "operator", 1, $1); }
   | EQEQ
      {$$ = allocTree(665, "operator", 1, $1); }
   | ANDAND
      {$$ = allocTree(669, "operator", 1, $1); }
   | OROR
      {$$ = allocTree(670, "operator", 1, $1); }
   | CM
      {$$ = allocTree(673, "operator", 1, $1); }
   | ARROW
      {$$ = allocTree(675, "operator", 1, $1); }
   | LP RP
      {$$ = allocTree(676, "operator", 2, $1, $2); }
   | LB RB
      {$$ = allocTree(677, "operator", 2, $1, $2); }
   ;

/*----------------------------------------------------------------------
 * Templates.
 *----------------------------------------------------------------------*/
/*
   None in 120++ language
*/	

/*----------------------------------------------------------------------
 * Exception handling.
 *----------------------------------------------------------------------*/
/*
   None in 120++ language
*/
 
 
/*----------------------------------------------------------------------
 * Epsilon (optional) definitions.
 *----------------------------------------------------------------------*/

declaration_seq_opt:
   /* epsilon */ 
      {$$ = allocTree(678, "declaration_seq_opt", 0); }
   | declaration_seq
      {$$ = allocTree(679, "declaration_seq_opt", 1, $1); }
   ;

expression_list_opt:
   /* epsilon */
      {$$ = allocTree(680, "expression_list_opt", 0); }
   | expression_list
      {$$ = allocTree(681, "expression_list_opt", 1, $1); }
   ;

new_placement_opt:
   /* epsilon */
      {$$ = allocTree(682, "new_placement_opt", 0); }
   | new_placement
      {$$ = allocTree(683, "new_placement_opt", 1, $1); }
   ;

expression_opt:
   /* epsilon */
      {$$ = allocTree(684, "expression_opt", 0); }
   | expression
      {$$ = allocTree(685, "expression_opt", 1, $1); }
   ;

statement_seq_opt:
   /* epsilon */
      {$$ = allocTree(686, "statement_seq_opt", 0); }
   | statement_seq
      {$$ = allocTree(687, "statement_seq_opt", 1, $1); }
   ;

condition_opt:
   /* epsilon */
      {$$ = allocTree(688, "condition_opt", 0); }
   | condition
      {$$ = allocTree(689, "condition_opt", 1, $1);} 
   ;

initializer_opt:
   /* epsilon */
      {$$ = allocTree(690, "initializer_opt", 0); }
   | initializer
      {$$ = allocTree(691, "initializer_opt", 1, $1);} 
   ;

constant_expression_opt:
   /* epsilon */
      {$$ = allocTree(692, "constant_expression_opt", 0); }
   | constant_expression
      {$$ = allocTree(693, "constant_expression_opt", 1, $1); }
   ;

abstract_declarator_opt:
   /* epsilon */
      {$$ = allocTree(694, "abstract_declarator_opt", 0); }
   | abstract_declarator
      {$$ = allocTree(695, "abstract_declarator_opt", 1, $1); }
   ;

type_specifier_seq_opt:
   /* epsilon */
      {$$ = allocTree(696, "type_specifier_seq_opt", 0); }
   | type_specifier_seq
      {$$ = allocTree(697, "type_specifier_seq_opt", 1, $1);} 
   ;

direct_abstract_declarator_opt:
   /* epsilon */
      {$$ = allocTree(698, "direct_abstract_declarator_opt", 0); }
   | direct_abstract_declarator
      {$$ = allocTree(699, "direct_abstract_declarator_opt", 1, $1);} 
   ;

COMMA_opt:
   /* epsilon */
      {$$ = allocTree(702, "COMMA_opt", 0); }
   | CM
      {$$ = allocTree(703, "COMMA_opt", 1, $1); }
   ;

member_specification_opt:
   /* epsilon */
      {$$ = allocTree(704, "member_specification_opt", 0); }
   | member_specification
      {$$ = allocTree(705, "member_specification_opt", 1, $1); }
   ;

SEMICOLON_opt:
   /* epsilon */
      {$$ = allocTree(706, "SEMICOLON_opt", 0); }
   | SM
      {$$ = allocTree(707, "SEMICOLON_opt", 1, $1);} 
   ;

conversion_declarator_opt:
   /* epsilon */
      {$$ = allocTree(708, "conversion_declarator_opt", 0); }
   | conversion_declarator
      {$$ = allocTree(709, "conversion_declarator_opt", 1, $1);} 
   ;
	
%%


/* ***************************************************************************
**  Main function that opens specified files, calls parser, and prints      **
**  results.                                                                **
*****************************************************************************/
int main(int argc, char *argv[])
{
   //yydebug = 1;
   int i;

   //for each filename from the command line
   for(i=1; i<argc; i++)
   {
      //reset variables and free memory from previous file
      errors = 0;
      yylineno = 1;
      numUserIncludes = 0;
      freeTreeMemory(rootPtr);
      classnameTabDestroy();
      rootPtr = NULL;
      
      yyFileName = strdup(argv[i]);    	//save the fileName	
      printf("\nOpening %s ...", yyFileName);

      //open the file and store its reference in global variable yyin
      yyin = fopen(argv[i], "r");
      if(!yyin)
         printf("\nCould not open %s!\n", argv[i]);
      else
      {
         initializeSymtab();    //necessary for when there are #includes
         yyparse();	
         pclose(yyin);	        
         
         /* ***********************************************
         *  perform semantic analysis of the syntax tree  *
         *************************************************/
         semanticAnalyze();                 
         
         #ifdef PRINT_CLASSNAME_TABLE    
            /* *******************************************************
            *  print classname table (just classes... typedef and    *
            *  struct not supported)                                 *
            *********************************************************/         
            printClassnameTab();
         #endif
         
         #ifdef PRINT_SYNTAX_TREE  
            /* *******************************************************
            * print the parse tree (note: treePrint() is recursive)  *
            *********************************************************/
            if(rootPtr != NULL)
            {
               printf("\n*********************************");
               printf("**********************************\n");            
               printf("Syntax Tree for %s (and included files)\n", yyFileName);
               printf("*********************************");
               printf("**********************************\n"); 
               treePrint(rootPtr, 0);
            }
         #endif  
         
         #ifdef PRINT_SYMBOL_TABLE
            /* *******************************************************
            *                print all symbol tables                 *
            *********************************************************/
            if(rootPtr != NULL) 
               printSymTables();
         #endif	         

         /* perform intermediate code generation */
         //intermediateCodeGenerate();

         /* perform final code generation */
         //finalCodeGenerate();			
      }
   }
	
   return 0;
}


/* ***************************************************************************
**            Function to create and save a new token struct                **
*****************************************************************************/
treeNodePtr saveTokStruct(int category)
{
   /* *****************************************************
   *            allocate memory for yytoken               *
   *******************************************************/
   yytokenStruct = (struct token*)calloc(1, sizeof(struct token));
   yytokenStruct->category = category;
   yytokenStruct->text = strdup(yytext);
   yytokenStruct->lineNumber = yylineno;
   yytokenStruct->fileName = strdup(yyFileName);
   
   if(category == INTEGER)             //if it's an integer constant
       yytokenStruct->ival = atoi(yytext);
   else
       yytokenStruct->ival = '\0';	
   if(category == CHARACTER)           //if it's a character constant
   {
      //copy yytext to sval w/o the single quotation marks  
      char *tmpSval = strdup(strchr(yytext, '\'')+1);
      yytokenStruct->sval = strndup(tmpSval, strlen(tmpSval)-1);
      free(tmpSval);
   }     
   else if(category == STRING)             //if it's a string constant
   {
      //copy yytext to sval w/o the prefix or quotation marks  
      char *tmpSval = strdup(strchr(yytext, '"')+1);
      yytokenStruct->sval = strndup(tmpSval, strlen(tmpSval)-1);
      free(tmpSval);      
   }     
   else
   {
       yytokenStruct->sval = '\0';	
   } 	

   /* *****************************************************
    *  allocate space and assign values for tokenpar    *
    *  (it's only real value will be leafTokenPtr)         *
    *******************************************************/
   treeNodePtr tokenpar = (treeNodePtr)calloc(1, sizeof(struct treeNode));
   tokenpar->rule = strdup("token");
   tokenpar->ruleLabel = 900;
   tokenpar->numChildren = 0;
   tokenpar->leafTokenPtr = yytokenStruct;
   tokenpar->children[0] = NULL;   

   return tokenpar;
}


/* ****************************************************************************
** Function that is called each time a production rule is matched. It        **
** creates a new tree node, which becomes the par node of previously      **
** matched production rules as they are popped off of the parse value stack  **
******************************************************************************/
treeNodePtr allocTree(int ruleLabel, char *rule, int nChildren, ...)
{
   int z;
   va_list args;
   treeNodePtr Par = (treeNodePtr)calloc(1, sizeof(struct treeNode));
   if (Par == NULL) {fprintf(stderr, "allocTree out of memory\n"); exit(1); }
 
   va_start(args, nChildren);
   for(z=0; z<9; z++)
   {
      Par->children[z] = va_arg(args, treeNodePtr);
      if(z < nChildren)
         Par->children[z]->par = Par;
   }
   va_end(args);	  
   Par->numChildren = nChildren;
   Par->ruleLabel = ruleLabel;
   Par->rule = strdup(rule);   
   return Par;
}


/* ***************************************************************************
**         Preorder recursive function to print the parse tree              **
*****************************************************************************/
int treePrint(treeNodePtr t, int depth)
{					
   int i;
   int nChildren = t->numChildren;

   if( strcmp(t->rule, "") )
   {
      for(i=0; i<depth; i++)
         printf(" ");

      if( t->ruleLabel == 900)	
      {
         printf("(%d)TOKEN CATEGORY: %d,  TEXT: %s\n", depth, 
                t->leafTokenPtr->category, t->leafTokenPtr->text);              
      }
      else
      {      
         printf("(%d)%s: %d\n", depth, t->rule, nChildren);          
      }
   }

   depth++;
   for(i=0; i<nChildren; i++)
   {
      treePrint(t->children[i], depth);
   }
}


/* ***************************************************************************
**                      Function to free the parse tree                     **
*****************************************************************************/
int freeTreeMemory(treeNodePtr t)
{					
   int i;
   if(t)
   {
      int nChildren = t->numChildren;
      
      for(i=0; i<nChildren; i++)
      {
         freeTreeMemory(t->children[i]);
      }	
      
      if( !strcmp(t->rule, "token") )	
      {
         free(t->leafTokenPtr->text);
         free(t->leafTokenPtr->fileName);
         free(t->leafTokenPtr->sval);
         free(t->leafTokenPtr);
      }
      free(t->rule);
      free(t);
   }
   return 1;
}


/* ***************************************************************************
**                                 yyerror                                  **
*****************************************************************************/
yyerror(char *s, ...)
{
   treeNodePtr tPtr = NULL;
   va_list args;
   va_start(args, s);
   tPtr = va_arg(args, treeNodePtr);
   va_end(args);	   
   
   fprintf(stderr, "%s: ", s);
   if(yyFileName){
      fprintf(stderr, "file \"%s\", ", yyFileName);		  
   }
	
   /* Exit with the appropriate exit status */
   if(! strncmp(s, "semantic error", 14)){
      if(tPtr != NULL)
         fprintf(stderr, "line %d, token \"%s\" \n", 
            tPtr->leafTokenPtr->lineNumber, tPtr->leafTokenPtr->text);
      else
         fprintf(stderr, "line %d, token \"%s\" \n", yylineno, yytext);
      exit(3);
   }
   else if(! strncmp(s, "syntax error", 12)){
      fprintf(stderr, "line %d, token \"%s\" \n", yylineno, yytext);
      exit(2);
   }
   else{
      fprintf(stderr, "line %d, token \"%s\" \n", yylineno, yytext);
      exit(1);
   }
}
