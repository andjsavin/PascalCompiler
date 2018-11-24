%option noyywrap

symbol         [a-zA-Z_]
digit          [0-9]
id             {symbol}({symbol}|{digit})*
nl             [\n]
whitespace     [ \t]
rSIGN          (\+|-)
rMULOP         (\*|\/|div|mod|and)
rRELOP         (=|<>|<|>|<=|>=)

%{
    #include "comp.h"
    #include "parser.tab.h"
    #include <iostream>
    std::string prevRelop;
    int line = 1;
    extern bool isArray;
%}

%s EXPECT

%%
{whitespace}                {}
{nl}                        {line++;}
"program"                   {return PROGRAM;}
"var"                       {
                                BEGIN(EXPECT);
                                return VAR;
                            }
"array"                     {return ARRAY;}
"of"                        {return OF;}
"or"                        {
                                yylval = yytext[0];
                                return OR;
                            }
"integer"                   {return INTEGER;}
"real"                      {return REAL;}
"begin"                     {
                                BEGIN(INITIAL);
                                return tBEGIN;
                            }
"end"                       {return tEND;}
"procedure"                 {
                                BEGIN(EXPECT);
                                return PROC;
                            }
"function"                  {
                                BEGIN(EXPECT);
                                return FUNC;
                            }
":="                        {return ASSIGNOP;}
"if"                        {return IF;}
"then"                      {return THEN;}
"else"                      {return ELSE;}
"do"                        {return DO;}
"not"                       {return NOT;}
"while"                     {return WHILE;}
"write"                     {
                                yylval = yytext[0];
                                return RW;
                            }
"read"                      {
                                yylval = yytext[0];
                                return RW;
                            }
{rSIGN}                     {
                                yylval = yytext[0];
                                return SIGN;
                            }
{rMULOP}                    {
                                yylval = yytext[0];
                                return MULOP;
                            }
{rRELOP}                    {
                                prevRelop = yytext;
                                return RELOP;
                            }
<EXPECT>{id}                {
                                yylval = addSymbol(yytext, noType);
                                return ID;
                            }
<INITIAL>{id}               {
                                yylval = findSymbol(yytext);
                                return ID;
                            }
{digit}+                    {
                                if (isArray == true) {
                                    yylval = atoi(strdup(yytext));
                                    return NUM;
                                } else {
                                    yylval = addConstant(yytext, intConstType);
                                    return NUM;
                                }
                            }
{digit}+"."{digit}+         {
                                yylval = addConstant(yytext, realConstType);
                                return NUM;
                            }
.                           {
                                return yytext[0];
                            }
<<EOF>>                     {
                                return COMPLETE;
                            }
%%

int lexan() {
    return yylex();
}