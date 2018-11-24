#include "comp.h"
#include <iostream>

void error (char *e) 
{
  fprintf (stderr, "line%d : %s\n", line, e);
  exit (1);
}

void error (std::string e) 
{
    std::cout << "line " << line << ": " << e << std::endl;
    exit (1);
}

