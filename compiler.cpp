#include "comp.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

int labelCounter = 0;
int tmpCounter = 0;
std::vector<entry> symbolTable;
int SP = 0;
int BP = 0;
int generateLabel()
{
    std::string label = "lab" + std::to_string(labelCounter);
    labelCounter++;
    return addSymbol(label, labelType);
};

void generateCode(
        const std::string& opCode, 
        int index1, 
        const std::string& adressMode1,
        int index2,
        const std::string& adressMode2, 
        int index3, 
        const std::string& adressMode3)
{
    auto op = operators.find(opCode);
    if (op != operators.end()) 
    {
        if (op->second == "mov") {
            output += operators.find(opCode)->second + " " +  symbolTable.at(index1).adress + "," + std::to_string(index2) + ";\n"; 
        }
    } else 
        output += opCode + " " +  symbolTable.at(index1).name + ";\n"; 
}

int addConstant(std::string value, varType type)
{
    int index = findSymbol(value);
    if (index != -1)
        return index;
    
    entry entry = {
        value, 
        value, 
        type, 
        global,
        ""};
    
    symbolTable.push_back(entry); 
    
    return symbolTable.size() - 1;
}

int addSymbol(std::string symbol, varType type)
{
    int index = findSymbol(symbol, true);
    if (index != -1)
    {
        error(varOverrideError + ": " + symbol);
        return index;
    }    
    
    entry entry = {
        symbol, 
        "", 
        type, 
        global,
        ""};
    
    symbolTable.push_back(entry); 
    return symbolTable.size() - 1;
}

int findSymbol(std::string symbol, bool onlyInScope)
{    
    for (auto it = symbolTable.begin(); it != symbolTable.end(); ++it) {    
        if (it->name.compare(symbol) == 0 && it->global == global)
            return it - symbolTable.begin();
    }
    
    if (!onlyInScope)
    {
        for (auto it = symbolTable.begin(); it != symbolTable.end(); ++it) {    
            if (it->name.compare(symbol) == 0)
                return it - symbolTable.begin();
        }
    }
    
    return -1;
}

void printLabel(int index)
{
    printLabel(symbolTable[index].name);
}


void printLabel(std::string label)
{
    output += label + ":\n";
}

void addType(varType type, bool isFunction)
{
    
    auto it = symbolTable.end() - 1;
    if (isFunction)
    {    
        while(it->type != functionType && it->funType != noType)
        {
            --it;
        }
        it->funType = type;
        return;
    }
    
    while(it->type == noType)
    {
        it->type = type;
        --it;
    }
}

void addType(varType type, bool isFunction, int size)
{
    
    auto it = symbolTable.end() - 1;
    if (isFunction)
    {    
        while(it->type != functionType && it->funType != noType)
        {
            --it;
        }
        it->funType = type;
        it->arraySize = size;
        return;
    }
    
    while(it->type == noType)
    {
        it->type = type;
        it->arraySize = size;
        --it;
    }
}

void printSymbols() {
    for (auto it = symbolTable.begin(); it != symbolTable.end(); ++it)
    {     
        std::cout << "name: " << it->name << "(" << 
                "type: " << showType(it->type) << ", " <<
                "size(array): " << it->arraySize << ", " << 
                "funType: " << showType(it->funType) << ", " << 
                "value: " << it->value << ", " <<
                "global: " << it->global << ", " <<
                "adress: " << it->adress << ", " <<
                "args: ";
        std::for_each(it->args.begin(), it->args.end(), [](varType &t){ std::cout << " " << showType(t); });
        std::cout << ", funAdress: " << it->funAdress << ")" << std::endl;
    }
}

std::string showType(varType type) {
    switch (type) {
        case labelType:
            return "label";
        case intType:
            return "int";
        case realType:
            return "real";
        case intConstType:
            return "int const";
        case realConstType:
            return "real const";
        case noType:
            return "type not applied yet";
        case procedureType:
            return "procedure";
        case functionType:
            return "function";
        case intArrayType:
            return "array of integer";
        case realArrayType:
            return "array of real";
        default:
            return "-";
    }
}

void setAdress() {
    for (auto it = symbolTable.begin(); it != symbolTable.end(); ++it)
    {
        if (it->type == intType && it->adress == "")
        {
            if (it->global) 
            {
                it->adress = std::to_string(SP);
                SP += 4;
            } else {
                BP -= 4;
                it->adress = "BP" + std::to_string(BP);
                printSymbols();
            }
        } else if (it->type == realType && it->adress == "")
        {
            if (it->global) 
            {
                it->adress = std::to_string(SP);
                SP += 8;
            } else {
                BP -= 8;
                it->adress = "BP" + std::to_string(BP);
                printSymbols();
            }
        }
    }
}

void setAdress(int size) {
    for (auto it = symbolTable.begin(); it != symbolTable.end(); ++it)
    {
        if (it->type == intArrayType && it->adress == "")
        {
            if (it->global) 
            {
                it->adress = std::to_string(SP);
                SP += 4*size;
            } else {
                BP -= 4*size;
                it->adress = "BP" + std::to_string(BP);
                printSymbols();
            }
        } else if (it->type == realArrayType && it->adress == "")
        {
            if (it->global) 
            {
                it->adress = std::to_string(SP);
                SP += 8*size;
            } else {
                BP -= 8*size;
                it->adress = "BP" + std::to_string(BP);
                printSymbols();
            }
        }
    }
}

void setParametersAdress()
{
    
    std::vector<varType> args;
    
    int incsp_count = 0;
    BP = 4;
        
    for (auto it = symbolTable.end(); it != symbolTable.begin(); --it)
    {
        if (it->type == functionType)
        {
            it->adress = "*BP+8";
            BP += 4;
            break;
        }
    }
    
    for (auto it = symbolTable.end(); it != symbolTable.begin(); --it)
    {
        if (!it->global) {

            if (it != symbolTable.end()) {
                it->adress = "*BP+" + std::to_string(BP);
            }

            BP += 4;
            
            incsp_count += 4;    
            
            args.push_back(it->type);
        } else {
            break;
        }
        
    }
    
    for (auto it = symbolTable.end(); it != symbolTable.begin(); --it )
    {
        if (it->type == procedureType || it->type == functionType)
        {            
            if (it->type == functionType)
            {
                it->funAdress = std::to_string(SP);
                if (it->funType == intType) {
                    SP += 4;
                } else {
                    SP += 8;
                }
            }
            if (it->type == procedureType) incsp_count -=4;
            it->value = std::to_string(incsp_count);
            args.erase(args.begin());
            it->args = args;
            break;
        }
    }
    
    BP = 0;
}
void printEnter() {
    auto it = symbolTable.end() - 1;
    int enter = 0;
    while ( it->type != procedureType && it->type != functionType)
    {
        if (it->adress.find("BP-") != std::string::npos)
        {
            if (it->type == intType || it->type == intConstType)
            {
                enter += 4;
            } else 
            {
                enter += 8;
            }
        }
        --it;
    }
    int idx = output.find("enter.i #\n");
    output.insert(idx + 9, std::to_string(enter));

}
int signOp(char op, int index1, int index2)
{    
    checkIndex(index1);
    checkIndex(index2);
    entry e1 = symbolTable.at(index1);
    entry e2 = symbolTable.at(index2);    
    
    std::string operation = operators.at(std::string(1, op));
    std::string operand1;
    std::string adress1;
    std::string operand2;
    std::string adress2;

    int tmp_index = -1;
    entry tmp_e;
        
    if ((e1.type == intType || e1.type == intConstType) 
                && (e2.type == realType || e2.type == realConstType) )
    {
        index1 = intToReal(index1);
        e1 = symbolTable.at(index1);
    } else if ((e2.type == intType || e2.type == intConstType) 
                && (e1.type == realType || e1.type == realConstType) )
    {
        index2 = intToReal(index2);
        e2 = symbolTable.at(index2);
    }
    else if ((e1.type == realType || e1.type == realConstType) 
                && (e2.type == intType || e2.type == intConstType))
    {
        index1 = realToInt(index1);
        e1 = symbolTable.at(index1);
    }  
    
    if (    ((e1.type == intType || e1.type == intConstType) 
                && (e2.type == intType || e2.type == intConstType)) 
            ||  
                ((e1.type == realType || e1.type == realConstType) 
                && (e2.type == realType || e2.type == realConstType))
            )       
    {
        
        if (e1.type == intType || e1.type == intConstType) 
        {
            operation += ".i";
            tmp_index = addSymbol(getNewTmpName(), intType);
        }
        else
        {
            operation += ".r";
            tmp_index = addSymbol(getNewTmpName(), realType);
        }
        
        setAdress();
        tmp_e = symbolTable.at(tmp_index);
        
        switch (e1.type)
        {
            case realConstType:
            case intConstType:
                operand1 = e1.value;
                adress1 = "#";
                break;
            case realType:
            case intType:
                operand1 = e1.adress;
                adress1 = "";
                break;
            default:
                break;
        }
        switch (e2.type)
        {
            case realConstType:
            case intConstType:
                operand2 = e2.value;
                adress2 = "#";
                break;
            case realType:
            case intType:
                operand2 = e2.adress;
                adress2 = "";
                break;
            default:
                break;
        }
    }
    else 
    {
        std::string err = "Error with types 2";
        error(err);
    }
    
    generateCode(operation, 
            operand1, adress1,
            operand2, adress2,
            tmp_e.adress, ""
            );
    return tmp_index;
}

std::string reverse(std::string relop) {
    if (relop == "=") return "<>";
    if (relop == "<>") return "=";
    if (relop == ">") return "<=";
    if (relop == "<") return ">=";
    if (relop == ">=") return "<";
    if (relop == "<=") return ">";
}

void assign(int index1, int index2, int idx)
{
    if (idx != -1) {
        addSymbol(getNewTmpName(), intType);
        setAdress();
        std::string adr1 = symbolTable.back().adress;
        output += "\tsub.i #" + std::to_string(idx) + ", #1, " + adr1 + "\n";
        output += "\tmul.i " + adr1 + ", #4, " + adr1 + "\n";
        addSymbol(getNewTmpName(), intType);
        setAdress();
        std::string adr2 = symbolTable.at(index2).adress;
        if (adr2[0] == '*') adr2.erase(0, 1);
        else adr2 = "#" + adr2;
        output += "\tadd.i " + adr2 + ", " + adr1 + ", " + symbolTable.back().adress + "\n";
        output += "\tmov.i #" + std::to_string(index1) + ", *" + symbolTable.back().adress + "\n";
        return;
    }
    checkIndex(index1);
    checkIndex(index2);
    
    entry e1 = symbolTable.at(index1);
    entry e2 = symbolTable.at(index2);
    
    std::string operation;
    std::string operand1;
    std::string adress1;
    std::string operand2;
    std::string adress2;
    
    
    if ((e1.type == intType || e1.type == intConstType || e1.funType == intType) 
                && (e2.type == realType || e2.type == realConstType || e2.funType == realType) )
    {
        index1 = intToReal(index1);
        e1 = symbolTable.at(index1);
    } else if ((e1.type == realType || e1.type == realConstType || e1.funType == realType) 
                && (e2.type == intType || e2.type == intConstType || e2.funType == intType ))
    {
        index1 = realToInt(index1);
        e1 = symbolTable.at(index1);
    }
    
    if (    ((e1.type == intType || e1.type == intConstType || e1.funType == intType) 
                && (e2.type == intType || e2.type == intConstType  || e2.funType == intType)) 
            ||  
                ((e1.type == realType || e1.type == realConstType  || e1.funType == realType) 
                && (e2.type == realType || e2.type == realConstType  || e2.funType == realType))
            )       
    {
        if (e1.type == intType || e1.type == intConstType || e1.funType == intType)
            operation = "mov.i";
        else
            operation = "mov.r";
        
        switch (e1.type)
        {
            case realConstType:
            case intConstType:
                operand1 = e1.value;
                adress1 = "#";
                break;
            case realType:
            case intType:
                operand1 = e1.adress;
                adress1 = "";
                break;
            default:
                break;
        }
        switch (e2.type)
        {
            case realConstType:
            case intConstType:
                operand2 = e2.value;
                adress2 = "#";
                break;
            case realType:
            case intType:
                operand2 = e2.adress;
                adress2 = "";
                break;
            default:
                break;
        }
        switch (e1.funType)
        {
            case realConstType:
            case intConstType:
                if (global) {
                    operand1 = e1.funAdress;
                } else {
                    operand1 = e1.adress;
                }
                adress1 = "#";
                break;
            case realType:
            case intType:
                if (global) {
                    operand1 = e1.funAdress;
                } else {
                    operand1 = e1.adress;
                }
                adress1 = "";
                break;
            default:
                break;
        }
        switch (e2.funType)
        {
            case realConstType:
            case intConstType:
                if (global) {
                    operand2 = e2.funAdress;
                } else {
                    operand2 = e2.adress;
                }
                adress2 = "#";
                break;
            case realType:
            case intType:
                if (global) {
                    operand2 = e2.funAdress;
                } else {
                    operand2 = e2.adress;
                }
                adress2 = "";
                break;
            default:
                break;
        }
    }
    else 
    {
        std::string err = "error with types";
        error(err);
    }
    
    generateCode(operation, 
            operand1, adress1,
            operand2, adress2,
            "",       "");
}

void rw(char op, std::vector<int> args)
{
    for (int i=0; i < args.size(); ++i)
    {
        rw(op, args[i]);
    }
}

void rw(char op, int index)
{
    checkIndex(index);
    entry e1 = symbolTable.at(index);   

    std::string operation = operators.at( std::string(1, op) );
    std::string operand1;
    std::string adress1;
    
    if (e1.type == intType || e1.type == intConstType || e1.funType == intType)
    {
        operation += ".i";
    } else {
        operation += ".r";
    }
    
    switch (e1.type) {
        case realConstType:
        case intConstType:
            operand1 = e1.value;
            adress1 = "#";
            break;
        case realType:
        case intType:
            operand1 = e1.adress;
            adress1 = "";
            break;
        default:
            switch (e1.funType) {
                case intType:
                case realType:
                    operand1 = e1.funAdress;
                    adress1 = "";
                default:
                    break;
            }
        break;
    }
    
    generateCode(operation, 
        operand1, adress1,
        "", "",
        "", "");
    
}

void generateCode(
        const std::string& opCode, 
        const std::string& operand1, 
        const std::string& adressMode1,
        const std::string& operand2,
        const std::string& adressMode2, 
        const std::string& operand3, 
        const std::string& adressMode3)
{
    output += "\t" + opCode + " " + adressMode1 + operand1;
    if (operand2 != "")
            output += "," + adressMode2 + operand2;
    if (operand3 != "") 
            output += "," + adressMode3 + operand3;
    
    output +=  "\n";
}

int intToReal(int index)
{
    entry e = symbolTable.at(index);
    int tmp_index = addSymbol(getNewTmpName(), realType);
    symbolTable.at(tmp_index).value = e.value;
    setAdress();
    entry tmp_e = symbolTable.at(tmp_index);
   
    if (e.adress == "")
    {
        generateCode("inttoreal.i", 
            e.value, "#", 
            tmp_e.adress, "", 
            "", "");
    } else 
    {
    generateCode("inttoreal.i", 
            e.adress, "", 
            tmp_e.adress, "", 
            "", "");
    }
    
    return tmp_index;
}

int realToInt(int index)
{
    entry e = symbolTable.at(index);
    int tmp_index = addSymbol(getNewTmpName(), intType);
    symbolTable.at(tmp_index).value = e.value;
    setAdress();
    entry tmp_e = symbolTable.at(tmp_index);
   
    if (e.adress == "")
    {
        generateCode("realtoint.r", 
            e.value, "#", 
            tmp_e.adress, "", 
            "", "");
    } else 
    {
    generateCode("realtoint.r", 
            e.adress, "", 
            tmp_e.adress, "", 
            "", "");
    }
    
    return tmp_index;
}

std::string getNewTmpName()
{
    std::string tmp_name = "tmp" + std::to_string(tmpCounter);
    ++tmpCounter;
    return tmp_name;
}

void printProcedure()
{  
    auto it = symbolTable.end() - 1;
    int enter = 0;
    while ( it->type != procedureType && it->type != functionType)
    {
        if (it->adress.find("BP-") != std::string::npos)
        {
            if (it->type == intType || it->type == intConstType)
            {
                enter += 4;
            } else 
            {
                enter += 8;
            }
        }
        --it;
    }
        
    printLabel(it->name);
    generateCode("enter.i", 
            "", "#",
            "", "",
            "", "");
}

void eraseLocal()
{
    generateCode("leave", "", "", "", "", "", "");
    generateCode("return", "", "", "", "", "", "");
    
    BP = 0;
    for (auto it=symbolTable.begin(); it != symbolTable.end(); )
    {
        if (it->global == false) 
        {
            it = symbolTable.erase(it);
        } else 
        {
            ++it;
        }
    }
}

void call(int index, std::vector<int> parameters)
{     
    checkIndex(index);
    entry e = symbolTable[index];
        
    if (e.type != functionType && e.type != procedureType) {
        return;
    }
    

    
    if (parameters.size() != e.args.size())
    {
        std::cout << parameters.size() << " found but " << e.args.size() << " was expected " << std::endl;
        error(std::string("Invalid count of parameters in call of: ") + e.name);
    };
    
    std::string operand1;
    std::string operation;
    std::string tmp_operation;

    int tmp_index;
    entry tmp_e;
    
    int i = 0;
    for (auto it = e.args.end(); it != e.args.begin(); --it)
    {
        varType funParamType = e.args[e.args.size() - 1 - i];
        entry par = symbolTable[parameters[i]];
        if (par.type != funParamType && par.type != funParamType + 2)
        {
            int newIndex;
            switch (par.type) {
            case realConstType:
            case realType:
                newIndex = realToInt(parameters[i]);
                break;
            case intConstType:
            case intType:
                newIndex = intToReal(parameters[i]);
                break;
            default:
                break;
            }
            par = symbolTable[newIndex];
        }
        
        switch (par.type) {
            case realConstType:
            case intConstType:
                tmp_index = addSymbol(getNewTmpName(), par.type == intConstType ? intType : realType);
                symbolTable[tmp_index].value = e.value;
                setAdress();
                tmp_e = symbolTable[tmp_index];
                operand1 = tmp_e.adress;
                if (par.type == intConstType) {
                    tmp_operation = "mov.i";
                } else {
                    tmp_operation = "mov.r";
                }
                generateCode(tmp_operation, par.value, "#", operand1, "", "", "");
                break;
            case realType:
            case intType:
                operand1 = par.adress;
                break;
            case realArrayType:
            case intArrayType:
                operand1 = par.adress;
                break;
            default:
                break;
        }
        
        if (operand1.find("*") == 0) {
            generateCode("push.i", operand1.erase(0, 1), "", "", "", "", "");
        } else {
            generateCode("push.i", operand1, "#", "", "", "", "");
        }
        ++i;
    }
    if (e.type == functionType) {
        generateCode("push.i", e.funAdress, "#", "", "", "", "");
    }
    generateCode("call.i", e.name, "#", "", "", "", "");
    
    if (e.type == functionType) {
        generateCode("incsp.i", e.value, "#", "", "", "", "");
    }
    if (e.type == procedureType) {
        if (e.value != "0") {
            generateCode("incsp.i", e.value, "#", "", "", "", "");
        }
    }

}

void checkIndex(int index)
{
    if (index == -1)
    {
        error(std::string("ERROR: No such ID in symbolTable"));
        exit(0);
    }
}


void startThen(int v_index, int label_index)
{
    checkIndex(label_index);
    entry value = symbolTable[v_index];
    entry label = symbolTable[label_index]; 
    generateCode("je.i", value.adress, "", "0", "#", label.name, "#");
}


int relOp(std::string op, int index1, int index2)
{
    checkIndex(index1);
    checkIndex(index2);
    entry e1 = symbolTable.at(index1);
    entry e2 = symbolTable.at(index2);    

    std::string operation = operators.at( op );
    std::string operand1;
    std::string adress1;
    std::string operand2;
    std::string adress2;

    int tmp_index = -1;
    entry tmp_e;
    
    if ((e1.type == intType || e1.type == intConstType) 
                && (e2.type == realType || e2.type == realConstType) )
    {
        index1 = intToReal(index1);
        e1 = symbolTable.at(index1);
    } else if ((e1.type == realType || e1.type == realConstType) 
                && (e2.type == intType || e2.type == intConstType) )
    {
        index1 = realToInt(index1);
        e1 = symbolTable.at(index1);
    }  
    
    if (    (e1.type == intType || e1.type == intConstType) 
                && (e2.type == intType || e2.type == intConstType) 
            ||  
                (e1.type == realType || e1.type == realConstType) 
                && (e2.type == realType || e2.type == realConstType)
            )       
    {
        
        if (e1.type == intType || e1.type == intConstType) 
        {
            operation += ".i";
            tmp_index = addSymbol(getNewTmpName(), intType);
        }
        else
        {
            operation += ".r";
            tmp_index = addSymbol(getNewTmpName(), realType);
        }
        
        setAdress();
        tmp_e = symbolTable.at(tmp_index);
        
        switch (e1.type)
        {
            case realConstType:
            case intConstType:
                operand1 = e1.value;
                adress1 = "#";
                break;
            case realType:
            case intType:
                operand1 = e1.adress;
                adress1 = "";
                break;
            default:
                break;
        }
        switch (e2.type)
        {
            case realConstType:
            case intConstType:
                operand2 = e2.value;
                adress2 = "#";
                break;
            case realType:
            case intType:
                operand2 = e2.adress;
                adress2 = "";
                break;
            default:
                break;
        }
    }
    else 
    {
        std::string err = "Error with types 3";
        error(err);
    }
    
    entry label1 = symbolTable[generateLabel()];
    entry label2 = symbolTable[generateLabel()];
    
    generateCode(operation, 
            operand1, adress1,
            operand2, adress2,
            label1.name, "#"
            );
    
    
    generateCode("mov.i", 
            "0", "#",
            tmp_e.adress, "",
            "", ""
            );
    
    generateCode("jump.i", 
            label2.name, "#",
            "", "",
            "", ""
            );
    
    output += label1.name + ":\n";
    
    generateCode("mov.i", 
            "1", "#",
            tmp_e.adress, "",
            "", ""
            );
    
    output += label2.name + ":\n";
    return tmp_index;   
}

int negative (int index)
{
    entry e = symbolTable[index];
    
    std::string op;
    std::string val;
    std::string add;
    int tmp_index;
    
    if (e.type == intConstType || e.type == intType) 
    {
        op = "sub.i";
        tmp_index = addSymbol(getNewTmpName(), intType);
    } else if (e.type == realConstType || e.type == realType)
    {
        op = "sub.r";
        tmp_index = addSymbol(getNewTmpName(), realType);
    }  else 
    {
        error(std::string("Error with types 1"));
    }
    
    setAdress();
    entry tmp_e = symbolTable.at(tmp_index);
    
    if (e.type == intConstType || e.type == realConstType) 
    {
        val = e.value;
        add = "#";
    } else if (e.type == intType || e.type == realType)
    {
        val = e.adress;
        add = "";
    }
    
    generateCode(op, "0", "#", val, add, tmp_e.adress, "");
    
    return tmp_index;
}