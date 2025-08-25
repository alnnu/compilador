#ifndef LEXICO_H
#define LEXICO_H

#include <stdio.h>
#include "token.h"

/* Variáveis globais do analisador léxico */
extern char* current_file_content;
extern int current_line;

/* Funções do analisador léxico */
void init_lexico(const char* filepath);
Token* get_next_token();
void free_token(Token* token);
const char* token_type_to_string(TokenType type);
char* my_strdup(const char* s);

#endif