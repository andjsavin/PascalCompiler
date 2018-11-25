%{
    #include "comp.h"
    #include <iostream>
    #define YYDEBUG 1
    std::vector<int> args;
    int relopTemp;
    int relopOld;
    int relopTempOld;
    int tmp;
    std::vector<int> stack;
    int whileStart;
    int whileEnd;
    bool isFunction;
    bool isNot;
    bool isArray;
    int arraySize;
    int idx = -1;
    extern char* yytext;
%}

%token PROGRAM
%token ID
%token ARRAY
%token NUM
%token OF
%token INTEGER
%token REAL
%token FUNC
%token PROC
%token tBEGIN
%token tEND
%token ASSIGNOP
%token IF
%token THEN
%token ELSE
%token WHILE
%token DO
%token VAR
%token RELOP
%token SIGN
%token OR
%token MULOP
%token NOT
%token RW
%token COMPLETE 1

%%
program: PROGRAM ID
         {
            $2 = generateLabel();
            generateCode("jump.i", symbolTable[$2].name, "#", "", "", "", "");
         } 
         '(' identifier_list ')' ';'
         declarations subprogram_declarations
         {
             printLabel($2);
         }
         compound_statement '.'
         {
            output += "\texit\n";
            return 0;
         }
         ;

identifier_list: ID |
                 identifier_list ',' ID
                 ;

declarations: declarations VAR identifier_list ':' type ';'
              {
                if (isArray == true) {
                    setAdress(arraySize);
                    isArray = false;
                } else {
                    setAdress();
                }
              } |
              /* empty */
              ;

type: standart_type
      {

      } |
      ARRAY
      {
          isArray = true;
      } '[' NUM '.' '.' NUM 
      {
          arraySize = atoi(yytext);
      }
       ']' OF standart_type
      {
      }
      ;

standart_type: INTEGER
               {  
                   if (isArray == true) {
                       addType(intArrayType, isFunction, arraySize);
                   } else {
                       addType(intType, isFunction);
                   }
               } |
               REAL
               {
                   if (isArray == true) {
                       addType(realArrayType, isFunction, arraySize);
                   } else {
                       addType(realType, isFunction);
                   }
               }
               ;

subprogram_declarations: subprogram_declarations subprogram_declaration ';' |
                         /* empty */
                         ;

subprogram_declaration: subprogram_head 
                        {
                            setParametersAdress();
                        }
                        declarations
                        {
                            printProcedure();
                        } 
                        compound_statement
                        {
                            global = true;
                            printSymbols();
                            printEnter();
                            eraseLocal();
                        }
                        ;

 subprogram_head: FUNC ID
                  {
                      symbolTable.back().type = functionType;
                      symbolTable.back().funType = noType;
                      global = false;
                  }
                  arguments ':'
                  {
                      isFunction = true;
                  }
                  standart_type ';'
                  {
                      isFunction = false;
                  } |
                  PROC ID
                  {
                      symbolTable.back().type = procedureType;
                      global = false;
                  }
                  arguments ';'
                  {
                      if (isArray == true)
                        isArray = false;
                      setParametersAdress();
                  }
                  ;

arguments: '(' parameter_list ')' |
           /* empty */
           ;

parameter_list: identifier_list ':' type |
                parameter_list ';' identifier_list ':' type
                ;

compound_statement: tBEGIN optional_statements tEND
                    ;

optional_statements: statement_list |
                     /* empty */
                     ;

statement_list: statement |
                statement_list ';' statement
                ;

statement: variable ASSIGNOP expression
           {
               assign($3, $1, idx);
               if (idx != -1) idx = -1;
           } |
           procedure_statement |
           compound_statement |
           IF expression 
           {
               int index = generateLabel();
               startThen(relopTemp, index);
               $$ = index;
           }
           THEN statement
           {
               int index = generateLabel();
               generateCode("jump.i", symbolTable[index].name, "#", "", "", "", "");
               output += symbolTable[$3].name + ": \n";
               $$ = index;
           }
           ELSE statement
           {
               output += symbolTable[$6].name + ": \n";
           } |
           WHILE
           {
               whileEnd = generateLabel();
               whileStart = generateLabel();
               printLabel(whileStart);
           }
           expression
           {
               entry value = symbolTable[relopTemp];
               entry label = symbolTable[whileEnd];
               generateCode("je.i", value.adress, "", "0", "#", label.name, "#");
           }
           DO statement
           {
                generateCode("jump.i", symbolTable[whileStart].name, "#", "", "", "", "");
                printLabel(whileEnd);
           }
           ;

variable: ID
          {

          } |
          ID '[' expression ']'
          {
              idx = $3;
          }
          ;

procedure_statement: ID
                     {
                         call($1, args);
                     } |
                     RW '(' expression_list ')'
                     {
                         rw((char) $1, args);
                         args.clear();
                     } |
                     ID '(' expression_list ')'
                     {
                         call($1, args);
                         args.clear();   
                     }
                     ;

expression_list: expression
                 {
                     $$ = $1;
                     args.push_back($1);
                 } |
                 expression_list ',' expression
                 {
                     $$ = $3;
                     args.push_back($3);
                 }
                 ;

expression: simple_expression
            {
                $$ = $1;
            } |
            simple_expression RELOP simple_expression
            {
                relopOld = relopTemp;
                if (stack.size() != 0) {
                    if (stack[stack.size() - 1] != relopOld) {
                        stack.push_back(relopOld);
                    }
                } else {
                    stack.push_back(relopOld);
                }
                if (isNot == true) {
                    prevRelop = reverse(prevRelop);
                    isNot = false;
                }
                relopTemp = relOp(prevRelop, $1, $3);
            }
            ;

simple_expression: term
                   {
                       $$ = $1;
                   } |
                   SIGN term
                   {
                       int tempIndex = negative($2);
                       $$ = tempIndex;
                    } |
                   simple_expression SIGN term
                   {
                       $$ = signOp((char) $2, $1, $3);
                   } |
                   simple_expression OR term
                   {
                       relopTemp = signOp((char) $2, relopOld, relopTemp);
                       tmp = relopOld;
                   }
                   ;

term: factor |
      term MULOP factor
      {
          if ((char) $2 == 'a')
            {
                if (tmp == relopOld) {
                    relopOld = stack[stack.size() - 2];
                }
                relopTemp = signOp((char) $2, relopOld, relopTemp);  
            } else {
                $$ = signOp((char) $2, $1, $3);                
            }
      }
      ;

factor: variable
        {
            call($1, args);
        } |
        ID '(' expression_list ')'
        {
            call($1, args);
            args.clear();
        } |
        NUM |
        '(' expression ')'
        {

        } |
        NOT factor
        {
            isNot = true;
        }
        ;
%%

int yyerror(char *e) {
    /*error(e);*/
    return 0;
}