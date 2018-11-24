#ifndef COMP_H
#define COMP_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <vector>
#include <map>
#include <string>

//const std::string direct = "", indirect = "*", immediate = "#";
const std::string varOverrideError = "Variable already exist";

extern int line;
extern int labelCounter;
extern int tmpCounter;
extern int SP;

extern std::string prevRelop;

enum varType
{
    labelType = 1,
    intType = 2,
    realType = 3,
    intConstType = 4,
    realConstType = 5,
    noType = 6,
    procedureType = 7,
    functionType = 8,
    intArrayType = 9,
    realArrayType = 10
};

struct entry
{
    std::string name;
    std::string value;
    varType type;
    bool global;
    std::string adress;
    std::vector<varType> args;
    varType funType;
    std::string funAdress;
    int arraySize;
};

extern std::map<std::string, std::string> operators;

extern std::vector<entry> symbolTable;

extern std::string output;
extern bool global;

void error(char *e);
void error(std::string e);
int yyparse();
int yylex();
int yyerror(char*);

//PROCESSOR
int generateLabel();
std::string reverse(std::string relop);
std::string showType(varType type);
void generateCode(
        const std::string& opCode, 
        int index1, 
        const std::string& adressMode1,
        int index2,
        const std::string& adressMode2, 
        int index3, 
        const std::string& adressMode3);
void generateCode(
        const std::string& opCode, 
        const std::string& op1, 
        const std::string& adressMode1,
        const std::string& op2,
        const std::string& adressMode2, 
        const std::string& op3, 
        const std::string& adressMode3);
int addConstant(std::string, varType type);
int addSymbol(std::string, varType type);
int findSymbol(std::string, bool onlyInScope=false);
void printLabel(int);
void printLabel(std::string);
void addType(varType, bool isFunction);
void addType(varType type, bool isFunction, int size);
void printSymbols();
void setAdress();
void setAdress(int size);
void setParametersAdress();

void assign(int index1, int index2, int idx);
int signOp(char op, int index1, int index2);
void rw(char op, int index);
void rw(char op, std::vector<int> args);
void procedure();
void printProcedure();
void eraseLocal();
void call(int index, std::vector<int> args);
void checkIndex(int index);
void startThen(int i, int label);
int relOp(std::string op, int index1, int index2);

int negative(int index);

int intToReal(int index);
int realToInt(int index);
std::string getNewTmpName();
void printEnter();

#endif