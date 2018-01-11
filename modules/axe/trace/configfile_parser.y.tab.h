/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YYTRACE_YTRACE_TAB_H_INCLUDED
# define YY_YYTRACE_YTRACE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yytracedebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    STRING = 258,
    EQUAL = 259,
    LBRACE = 260,
    RBRACE = 261,
    LBRACESQ = 262,
    RBRACESQ = 263,
    NEWLINE = 264,
    tokPROJECT_CUSTOM = 265,
    tokPROJECT_TELIMP = 266,
    tokPROJECT_CCM = 267,
    tokLOCALFILENAMEDIR = 268,
    tokTRACEPREFIXNAME = 269,
    tokTRACECONFIGFILENAME = 270,
    tokHEADING = 271,
    tokVARIABLE_MASK = 272,
    tokVARIABLE_NAME = 273,
    tokVARIABLE = 274,
    tokTIMESLICE = 275,
    tokCONVERTBASE = 276,
    tokINDEXTYPE = 277
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 13 "modules/axe/trace/configfile_parser_y" /* yacc.c:1909  */

	char *strVal;

#line 81 "ytrace.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yytracelval;

int yytraceparse (void);

#endif /* !YY_YYTRACE_YTRACE_TAB_H_INCLUDED  */
