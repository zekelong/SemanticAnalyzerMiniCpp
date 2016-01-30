#****************************************************************#
#****************************************************************#
#  Zeke Long                                                     #
#  CS 445                                                        #
#  HW3                                                           #
#  10/14/2014                                                    #
#                                                                #
#          Makefile for Lex and Yacc 120++ parser                #
#****************************************************************#
#****************************************************************#

all: 120

120: lex.yy.o 120++.tab.o semantic.o symtab.o
	cc -o 120 lex.yy.o 120++.tab.o semantic.o symtab.o -ll 
	
semantic.o: semantic.c semantic.h
	cc -c -g -w semantic.c

symtab.o: symtab.c symtab.h 
	cc -c -g -w symtab.c

lex.yy.o: lex.yy.c
	cc -c -g -w lex.yy.c

lex.yy.c: 120++.l token.h 120++.tab.h 
	flex 120++.l

120++.tab.o: 120++.tab.c
	cc -c -g 120++.tab.c

120++.tab.c: 120++.y
	bison --debug --verbose 120++.y

120++.tab.h: 120++.y
	bison -d --debug --verbose 120++.y

clean:
	rm  lex.yy.* *.tab.* *.output *.o



