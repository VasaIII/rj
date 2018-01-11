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

#ifndef YY_YYMML_YMML_TAB_H_INCLUDED
# define YY_YYMML_YMML_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yymmldebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    STRING = 258,
    TRACE = 259,
    COMMENTED = 260,
    EQUAL = 261,
    LBRACE = 262,
    RBRACE = 263,
    NEWLINE = 264,
    DOTDOT = 265,
    DOTCOMMA = 266,
    COMMA = 267,
    MINUS = 268,
    tokHELP = 269,
    tokDUMP = 270,
    tokMMLCNF = 271,
    tokSTATS = 272,
    tokTRN = 273,
    tokINFO = 274,
    tokSS7LOC = 275,
    tokSS7CON = 276,
    tokOWNSP = 277,
    tokSP = 278,
    tokSPTYPE = 279,
    tokNET = 280,
    tokSI = 281,
    tokNI = 282,
    tokSLS = 283,
    tokTRNLOC = 284,
    tokTRNCON = 285,
    tokEPID = 286,
    tokSAID = 287,
    tokLIP = 288,
    tokLPN = 289,
    tokMODE = 290,
    tokRIP = 291,
    tokRPN = 292,
    tokUSER = 293,
    tokM3ACON = 294,
    tokDEST = 295,
    tokTRNID = 296,
    tokBMODE = 297,
    tokM3LOOP = 298,
    tokTRNID1 = 299,
    tokDEST1 = 300,
    tokOWNSP1 = 301,
    tokTRNID2 = 302,
    tokDEST2 = 303,
    tokOWNSP2 = 304,
    tokDIR = 305,
    tokCICINC1 = 306,
    tokCICINC2 = 307,
    tokSCCPCF = 308,
    tokSSN = 309,
    tokBSCI = 310,
    tokBTSI = 311,
    tokBSC = 312,
    tokMSCSP = 313,
    tokCELL = 314,
    tokCGI = 315,
    tokMEI = 316,
    tokME = 317,
    tokIMSI = 318,
    tokRANGE = 319,
    tokPHONEI = 320,
    tokPHONE = 321,
    tokANUM = 322,
    tokBNUM = 323,
    tokBDEST = 324,
    tokCALL = 325,
    tokSTART = 326,
    tokSTOP = 327,
    tokDELETE = 328,
    tokPRINT = 329,
    tokID = 330,
    tokCALLER = 331,
    tokCALLED = 332,
    tokTYPE = 333,
    tokCPS = 334,
    tokDURATION = 335,
    tokCIC = 336,
    tokCMD = 337
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 198 "modules/axe/mml/parser_y" /* yacc.c:1909  */

	char *strVal;

#line 141 "ymml.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yymmllval;

int yymmlparse (void);

#endif /* !YY_YYMML_YMML_TAB_H_INCLUDED  */
