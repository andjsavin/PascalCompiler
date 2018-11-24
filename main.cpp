#include "comp.h"

#include <iostream>
#include <fstream>

std::string output;
std::map<std::string, std::string> operators;
bool global;
extern FILE* yyin;
//extern int yydebug;

void init();

int main (int argc, char* argv[]) 
{    
    init();
    
    
    FILE* f;
	
    if (argc > 1) {
	f = fopen(argv[1],"r");
	if (!f) {
            std::cerr << "Input file cannot be opened\n";
            return 0;
	}
    }
    else {
	std::cerr << "Input file not specified\n";
	return 0;
    }
    
    yyin = f;
    
    std::cout << "Starting parsing" << std::endl;
    //yydebug = 1;
    yyparse ();
    
    fclose(f);
    
    std::cout << "Parsing completed." << std::endl;
    std::cout << "Writing result to result.asm" << std::endl;

    std::ofstream result;
	result.open("result.asm");
	result << output;
	result.close();
        
    std::cout << "Compilation finished" << std::endl;
    std::cout << "Symbols: " << std::endl;
    printSymbols();
    exit (0);
}

void init() 
{
    global = true; 
    operators[":="] = "mov";
    operators["+"] = "add";
    operators["-"] = "sub";
    operators["*"] = "mul";
    operators["/"] = "div";
    operators["m"] = "mod";
    operators["d"] = "div";
    operators["a"] = "and";
    operators["o"] = "or";
    operators["n"] = "not";
    operators["="] = "je";
    operators["<"] = "jl";
    operators["<="] = "jle";
    operators[">"] = "jg";
    operators[">="] = "jge";
    operators["<>"] = "jne";
    operators["r"] = "read";
    operators["w"] = "write";
}