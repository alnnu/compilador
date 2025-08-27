#ifndef SINTATICO_H
#define SINTATICO_H

#include "token.h"

#define MAX_PARAMS 10 /* Define a reasonable max number of parameters */

typedef struct Symbol {
    char* name;
    TokenType type;
    char* scope;
    TokenType return_type; /* For functions: stores the inferred return type */
    int has_return_statement; /* For functions: flag if a return statement was found */
    char* initial_value_str; /* New field for "Possible Value" */
    /* New fields for function parameters */
    int num_params;
    TokenType param_types[MAX_PARAMS];
} Symbol;


void semantic_alert(const char* message); /* Prototype for semantic_alert */
void parse();

#endif
