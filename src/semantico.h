#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "token.h"

#include "sintatico.h" /* For Symbol struct definition */ 

void semantico_check_function_call(Symbol* func_symbol, TokenType* arg_types, int num_args);



#endif
