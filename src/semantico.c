#include <stdio.h>
#include <string.h> /* For strcmp */
#include "semantico.h"
#include "lexico.h"
#include "sintatico.h" /* For Symbol struct and semantic_alert */

/* Implementation of semantico_check_function_call */
void semantico_check_function_call(Symbol* func_symbol, TokenType* arg_types, int num_args) {
    if (func_symbol == NULL) {
        /* This case should ideally be caught by sintatico.c before calling this */
        /* semantic_alert("Função não declarada."); */ 
        return;
    }

    /* Check number of arguments */
    if (func_symbol->num_params != num_args) {
        char msg[256];
        sprintf(msg, "Número incorreto de argumentos para a função '%s'. Esperado %d, obteve %d.",
                func_symbol->name, func_symbol->num_params, num_args);
        semantic_alert(msg);
        return;
    }

    /* Check types of arguments */
    int i;
    for (i = 0; i < num_args; i++) {
        if (func_symbol->param_types[i] != arg_types[i]) {
            char msg[256];
            sprintf(msg, "Tipo de argumento incompatível para o parâmetro %d da função '%s'. Esperado %s, obteve %s.",
                    i + 1, func_symbol->name, token_type_to_string(func_symbol->param_types[i]), token_type_to_string(arg_types[i]));
            semantic_alert(msg);
            return;
        }
    }
}

