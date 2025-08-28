#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> /* Added for isdigit */
#include "sintatico.h" 
#include "lexico.h"
#include "semantico.h"

/* --- Protótipos --- */
static void parse_programa();
static void parse_declaracao_variavel();
static void parse_funcao_def();
static void parse_principal_def();
static void parse_bloco();
static void parse_comando();
static void parse_atribuicao();
static void parse_comando_se();
static void parse_comando_para();
static void parse_comando_leia();
static void parse_comando_escreva();
static void parse_retorno();
static TokenType parse_expressao();
static TokenType parse_logical_and_expression();
static TokenType parse_relational_expression();
static TokenType parse_expressao_simples();
static TokenType parse_termo();
static TokenType parse_fator();


static Token* currentToken;
static int syntax_error = 0;
static int in_se_senao_block = 0;
static char* current_scope = "global";

/* --- Funções de erro e Tabela de Símbolos --- */

static void error(const char* message) {
    printf("ERRO SINTÁTICO na linha %d: %s. Token inesperado: '%s' (%s)\n", 
           currentToken->line, message, currentToken->value ? currentToken->value : "", 
           token_type_to_string(currentToken->type));
    syntax_error = 1;
    exit(1);
}

void semantic_alert(const char* message) {
    printf("ALERTA SEMÂNTICO na linha %d: %s\n", currentToken->line, message);
}




#define MAX_SYMBOLS 1024



Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

static Symbol* current_function_symbol = NULL; /* Points to the symbol table entry of the function currently being parsed */

Symbol* find_symbol(const char* name) {
    int i;
    for (i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            if (strcmp(symbol_table[i].scope, current_scope) == 0 || strcmp(symbol_table[i].scope, "global") == 0) {
                return &symbol_table[i];
            }
        }
    }
    return NULL;
}

void add_symbol(const char* name, TokenType type, const char* scope) {
    if (symbol_count < MAX_SYMBOLS) {
        symbol_table[symbol_count].name = my_strdup(name);
        symbol_table[symbol_count].type = type;
        symbol_table[symbol_count].scope = my_strdup(scope);
        symbol_table[symbol_count].return_type = TOKEN_ERRO; /* Initialize for functions */
        symbol_table[symbol_count].has_return_statement = 0; /* Initialize for functions */
        symbol_table[symbol_count].initial_value_str = NULL; /* Initialize new field */
        /* Initialize new fields for function parameters */
        symbol_table[symbol_count].num_params = 0;
        int i;
        for (i = 0; i < MAX_PARAMS; i++) {
            symbol_table[symbol_count].param_types[i] = TOKEN_ERRO; /* Or some other default invalid type */
        }
        symbol_count++;
    } else {
        error("Estouro da tabela de símbolos.");
    }
}

/* --- Funções de Parsing --- */

static void consume(TokenType type) {
    if (syntax_error) return;
    if (currentToken->type == type) {
        free_token(currentToken);
        currentToken = get_next_token();
    } else {
        char msg[256];
        sprintf(msg, "Esperava-se o token %s", token_type_to_string(type));
        error(msg);
    }
}

/* Helper function to validate decimal size format [int.int] */
static int validate_decimal_size_format(const char* value) {
    int dot_count = 0;
    int i;
    for (i = 0; value[i] != '\0'; i++) {
        if (value[i] == '.') {
            dot_count++;
        } else if (!isdigit(value[i])) {
            return 0; /* Not a digit or dot */
        }
    }
    if (dot_count != 1) {
        return 0; /* Must have exactly one dot */
    }
    /* Further check: ensure there are digits before and after the dot */
    char* dot_pos = strchr(value, '.');
    if (dot_pos == value || *(dot_pos + 1) == '\0') {
        return 0; /* No digits before or after dot */
    }
    return 1; /* Format seems valid */
}

static void parse_declaracao_variavel() {
    if (in_se_senao_block) {
        error("Declaração de variável não permitida dentro de um bloco se/senao.");
    }
    TokenType type = currentToken->type; /* This 'type' should be TOKEN_TEXTO */
    consume(type); /* Consumes TOKEN_TEXTO */

    do {
        if (currentToken->type == TOKEN_VIRGULA) {
            consume(TOKEN_VIRGULA);
        }

        char* var_name = my_strdup(currentToken->value);
        Symbol* existing_symbol = find_symbol(var_name);
        if (existing_symbol != NULL) {
            if (strcmp(existing_symbol->scope, current_scope) == 0) {
                char msg[256];
                sprintf(msg, "Variável '%s' já declarada neste escopo.", var_name);
                semantic_alert(msg);
            } else {
                char msg[256];
                sprintf(msg, "Variável '%s' declarada em escopo diferente ('%s'), sombreando declaração anterior em '%s'.",
                        var_name, current_scope, existing_symbol->scope);
                semantic_alert(msg);
            }
        }
        add_symbol(var_name, type, current_scope); /* Add symbol first */

        Symbol* added_var_symbol = find_symbol(var_name); /* Retrieve the newly added symbol */

        consume(TOKEN_ID_VAR); /* Consumes !a or !b */
        
        if (currentToken->type == TOKEN_OP_ATRIB) {
            consume(TOKEN_OP_ATRIB);
            char* assigned_literal_value = NULL;
            /* Check if the next token is a literal before parsing the expression */
            if (currentToken->type == TOKEN_LITERAL_INT ||
                currentToken->type == TOKEN_LITERAL_DEC ||
                currentToken->type == TOKEN_LITERAL_TEXTO) {
                assigned_literal_value = my_strdup(currentToken->value);
            }

            TokenType expr_type = parse_expressao(); /* This will consume the literal if it was one */

            if (type != expr_type) {
                char msg[256];
                sprintf(msg, "Atribuição de tipo incompatível para a variável '%s'. Esperado %s, obteve %s.", 
                        var_name, token_type_to_string(type), token_type_to_string(expr_type));
                semantic_alert(msg);
            }

            /* Store the initial value string in the symbol table entry */
            if (added_var_symbol != NULL) {
                added_var_symbol->initial_value_str = assigned_literal_value;
            } else {
                /* This case should ideally not happen if add_symbol worked correctly */
                if (assigned_literal_value) free(assigned_literal_value);
            }
        }

        if (currentToken->type == TOKEN_LBRACKET) {
            consume(TOKEN_LBRACKET);
            if (currentToken->type == TOKEN_LITERAL_INT || currentToken->type == TOKEN_LITERAL_DEC) {
                if (type == TOKEN_TEXTO) {
                    int size_val = atoi(currentToken->value); /* Assuming integer literal for size */
                    if (size_val < 1) {
                        semantic_alert("Tamanho de texto deve ser maior ou igual a um.");
                    }
                } else if (type == TOKEN_DECIMAL) {
                    if (!validate_decimal_size_format(currentToken->value)) {
                        semantic_alert("Formato de tamanho para decimal inválido. Esperado [inteiro.inteiro].");
                    }
                }
                consume(currentToken->type);
            } else {
                error("Esperava-se um tamanho para o tipo de dado.");
            }
            consume(TOKEN_RBRACKET);
        }
        free(var_name);
    } while (currentToken->type == TOKEN_VIRGULA);

    consume(TOKEN_PONTO_VIRGULA);
}

static void parse_atribuicao() {
    Symbol* var = find_symbol(currentToken->value);
    if (var == NULL) {
        char msg[256];
        sprintf(msg, "Variável '%s' não declarada.", currentToken->value);
        semantic_alert(msg);
        consume(TOKEN_ID_VAR);
        consume(TOKEN_OP_ATRIB);
        parse_expressao();
        consume(TOKEN_PONTO_VIRGULA);
        return;
    }
    
    consume(TOKEN_ID_VAR);
    consume(TOKEN_OP_ATRIB);
    TokenType expr_type = parse_expressao();

    if (var->type != expr_type) {
        char msg[256];
        sprintf(msg, "Atribuição de tipo incompatível para a variável '%s'. Esperado %s, obteve %s.", 
                var->name, token_type_to_string(var->type), token_type_to_string(expr_type));
        semantic_alert(msg);
    }
    consume(TOKEN_PONTO_VIRGULA);
}

static TokenType parse_fator() {
    TokenType type = TOKEN_ERRO;
    switch (currentToken->type) {
        case TOKEN_LITERAL_INT:
            type = TOKEN_INTEIRO;
            consume(TOKEN_LITERAL_INT);
            break;
        case TOKEN_LITERAL_DEC:
            type = TOKEN_DECIMAL;
            consume(TOKEN_LITERAL_DEC);
            break;
        case TOKEN_LITERAL_TEXTO:
            type = TOKEN_TEXTO;
            consume(TOKEN_LITERAL_TEXTO);
            break;
        case TOKEN_ID_VAR: {
            Symbol* var = find_symbol(currentToken->value);
            if (var == NULL) {
                char msg[256];
                sprintf(msg, "Variável '%s' não declarada.", currentToken->value);
                semantic_alert(msg);
                type = TOKEN_ERRO; /* Assume erro se não declarada */
            } else {
                type = var->type;
            }
            consume(TOKEN_ID_VAR);
            break;
        }
        case TOKEN_ID_FUNC: {
            Symbol* func_symbol = find_symbol(currentToken->value);
            if (func_symbol == NULL || func_symbol->type != TOKEN_FUNCAO) {
                char msg[256];
                sprintf(msg, "Função '%s' não declarada.", currentToken->value);
                semantic_alert(msg);
                type = TOKEN_ERRO; /* Assume erro se não declarada */
            } else {
                type = func_symbol->return_type; /* Use the function's declared return type */
            }
            consume(TOKEN_ID_FUNC);
            consume(TOKEN_LPAREN);
            TokenType arg_types[MAX_PARAMS];
            int num_args = 0;
            if (currentToken->type != TOKEN_RPAREN) {
                arg_types[num_args++] = parse_expressao();
                while (currentToken->type == TOKEN_VIRGULA) {
                    consume(TOKEN_VIRGULA);
                    if (num_args < MAX_PARAMS) {
                        arg_types[num_args++] = parse_expressao();
                    } else {
                        semantic_alert("Número máximo de argumentos excedido na chamada da função.");
                        parse_expressao(); /* Consume the extra argument */
                    }
                }
            }
            consume(TOKEN_RPAREN);
            if (func_symbol != NULL && func_symbol->type == TOKEN_FUNCAO) {
                semantico_check_function_call(func_symbol, arg_types, num_args);
            }
            break;
        }
        case TOKEN_LPAREN:
            consume(TOKEN_LPAREN);
            type = parse_expressao();
            consume(TOKEN_RPAREN);
            break;
        default:
            error("Fator inválido em expressão.");
    }
    return type;
}

static TokenType parse_termo() {
    TokenType type = parse_fator();
    while (currentToken->type == TOKEN_OP_MULT || currentToken->type == TOKEN_OP_DIV || currentToken->type == TOKEN_OP_EXP) {
        Token* op = currentToken;
        consume(op->type);
        TokenType right_type = parse_fator();
        if (type != right_type) {
            semantic_alert("Operação com tipos incompatíveis.");
            type = TOKEN_ERRO;
        }
        /* Lógica adicional para determinar o tipo resultante (ex: int * dec -> dec) */
    }
    return type;
}

static TokenType parse_expressao_simples() {
    TokenType type = parse_termo();
    while (currentToken->type == TOKEN_OP_SOMA || currentToken->type == TOKEN_OP_SUB) {
        Token* op = currentToken;
        consume(op->type);
        TokenType right_type = parse_termo();
        if (type != right_type) {
            semantic_alert("Operação com tipos incompatíveis.");
            type = TOKEN_ERRO;
        }
        /* Lógica adicional para determinar o tipo resultante (ex: int * dec -> dec) */
    }
    return type;
}

/* New function for relational expressions */
static TokenType parse_relational_expression() {
    TokenType type = parse_expressao_simples(); /* Left operand of relational expression */

    /* Check for a relational operator */
    if (currentToken->type == TOKEN_OP_IGUAL || currentToken->type == TOKEN_OP_DIF ||
        currentToken->type == TOKEN_OP_MENOR || currentToken->type == TOKEN_OP_MENOR_IGUAL ||
        currentToken->type == TOKEN_OP_MAIOR || currentToken->type == TOKEN_OP_MAIOR_IGUAL) {

        TokenType op_type = currentToken->type; /* Store the type of the operator */
        consume(op_type); /* Consume using the stored type */
        TokenType right_type = parse_expressao_simples(); /* Right operand of relational expression */

        /* Perform semantic checks */
        if (type == TOKEN_TEXTO && (op_type == TOKEN_OP_MENOR || op_type == TOKEN_OP_MENOR_IGUAL || op_type == TOKEN_OP_MAIOR || op_type == TOKEN_OP_MAIOR_IGUAL)) {
            semantic_alert("Operador relacional inválido para o tipo texto.");
        }
        if (type != right_type) {
            semantic_alert("Comparação com tipos incompatíveis.");
        }
        type = TOKEN_INTEIRO; /* Relational expressions result in booleano (inteiro 0 or 1) */
    }
    return type;
}

/* New function for logical AND expressions */
static TokenType parse_logical_and_expression() {
    TokenType type = parse_relational_expression(); /* Left operand of logical AND expression */
    while (currentToken->type == TOKEN_OP_E) { /* && */
        if (type != TOKEN_INTEIRO) {
            semantic_alert("Operador lógico '&&' usado com tipo não booleano.");
        }
        consume(TOKEN_OP_E);
        TokenType right_type = parse_relational_expression(); /* Right operand of logical AND expression */
        if (right_type != TOKEN_INTEIRO) {
            semantic_alert("Operador lógico '&&' usado com tipo não booleano na expressão lógica.");
        }
        type = TOKEN_INTEIRO; /* Result is boolean */
    }
    return type;
}

/* parse_expressao will now handle logical OR (lowest precedence) */
static TokenType parse_expressao() {
    TokenType type = parse_logical_and_expression(); /* Left operand of logical OR expression */

    /* Handle logical OR operators (||) */
    while (currentToken->type == TOKEN_OP_OU) { /* || */
        if (type != TOKEN_INTEIRO) {
            semantic_alert("Operador lógico '||' usado com tipo não booleano.");
        }
        consume(TOKEN_OP_OU);
        TokenType right_expr_type = parse_logical_and_expression(); /* Right operand of logical OR expression */
        if (right_expr_type != TOKEN_INTEIRO) {
            semantic_alert("Operador lógico '||' usado com tipo não booleano na expressão lógica.");
        }
        type = TOKEN_INTEIRO; /* Result of logical expression is booleano (inteiro) */
    }

    return type;
}


static void parse_comando_se() {
    consume(TOKEN_SE);
    consume(TOKEN_LPAREN);
    parse_expressao();
    consume(TOKEN_RPAREN);

    in_se_senao_block = 1;
    if (currentToken->type == TOKEN_LBRACE) {
        parse_bloco();
    }
    else {
        parse_comando();
    }
    in_se_senao_block = 0;

    if (currentToken->type == TOKEN_SENAO) {
        consume(TOKEN_SENAO);
        in_se_senao_block = 1;
        if (currentToken->type == TOKEN_LBRACE) {
            parse_bloco();
        }
        else {
            parse_comando();
        }
        in_se_senao_block = 0;
    }
}

static void parse_comando_para() {
    consume(TOKEN_PARA);
    consume(TOKEN_LPAREN);

    if (currentToken->type != TOKEN_PONTO_VIRGULA) {
        parse_atribuicao();
    } else {
        consume(TOKEN_PONTO_VIRGULA);
    }

    if (currentToken->type != TOKEN_PONTO_VIRGULA) {
        parse_expressao();
    }
    consume(TOKEN_PONTO_VIRGULA);

    if (currentToken->type != TOKEN_RPAREN) {
        TokenType next_type = peek_token()->type;
        if (currentToken->type == TOKEN_ID_VAR && (next_type == TOKEN_OP_INC || next_type == TOKEN_OP_DEC)) {
            Symbol* var = find_symbol(currentToken->value);
            if (var == NULL) {
                char msg[256];
                sprintf(msg, "Variável '%s' não declarada.", currentToken->value);
                semantic_alert(msg);
            } else if (var->type != TOKEN_INTEIRO) {
                semantic_alert("Operador de incremento/decremento só pode ser usado com variáveis do tipo inteiro.");
            }
            consume(TOKEN_ID_VAR);
            consume(next_type);
        } 
        else if ((currentToken->type == TOKEN_OP_INC || currentToken->type == TOKEN_OP_DEC) && next_type == TOKEN_ID_VAR) {
            TokenType op_type = currentToken->type;
            consume(op_type);
            Symbol* var = find_symbol(currentToken->value);
             if (var == NULL) {
                char msg[256];
                sprintf(msg, "Variável '%s' não declarada.", currentToken->value);
                semantic_alert(msg);
            } else if (var->type != TOKEN_INTEIRO) {
                semantic_alert("Operador de incremento/decremento só pode ser usado com variáveis do tipo inteiro.");
            }
            consume(TOKEN_ID_VAR);
        }
        else {
            parse_expressao();
        }
    }
    consume(TOKEN_RPAREN);

    if (currentToken->type == TOKEN_LBRACE) {
        parse_bloco();
    } else {
        parse_comando();
    }
}

static void parse_comando_leia() {
    consume(TOKEN_LEIA);
    consume(TOKEN_LPAREN);
    do {
        if (currentToken->type == TOKEN_VIRGULA) consume(TOKEN_VIRGULA);
        if (find_symbol(currentToken->value) == NULL) {
            char msg[256];
            sprintf(msg, "Variável '%s' não declarada.", currentToken->value);
            semantic_alert(msg);
        }
        consume(TOKEN_ID_VAR);
    } while (currentToken->type == TOKEN_VIRGULA);
    consume(TOKEN_RPAREN);
    consume(TOKEN_PONTO_VIRGULA);
}

static void parse_comando_escreva() {
    consume(TOKEN_ESCREVA);
    consume(TOKEN_LPAREN);
    do {
        if (currentToken->type == TOKEN_VIRGULA) consume(TOKEN_VIRGULA);
        if (currentToken->type == TOKEN_LITERAL_TEXTO) {
            consume(TOKEN_LITERAL_TEXTO);
        }
        else {
            parse_expressao();
        }
    } while (currentToken->type == TOKEN_VIRGULA);
    consume(TOKEN_RPAREN);
    consume(TOKEN_PONTO_VIRGULA);
}

static void parse_retorno() {
    consume(TOKEN_RETORNO);
    TokenType expr_type = parse_expressao(); /* O tipo de retorno deve ser verificado contra o tipo da função */

    if (current_function_symbol != NULL) {
        current_function_symbol->has_return_statement = 1;
        if (current_function_symbol->return_type == TOKEN_ERRO) {
            current_function_symbol->return_type = expr_type;
        } else if (current_function_symbol->return_type != expr_type) {
            semantic_alert("Tipo de retorno inconsistente com retornos anteriores da função.");
        }
    }
    consume(TOKEN_PONTO_VIRGULA);
}

static void parse_bloco() {
    consume(TOKEN_LBRACE);
    while (currentToken->type != TOKEN_RBRACE && currentToken->type != TOKEN_EOF && !syntax_error) {
        parse_comando();
    }
    consume(TOKEN_RBRACE);
}

static void parse_comando() {
    switch (currentToken->type) {
        case TOKEN_INTEIRO:
        case TOKEN_DECIMAL:
        case TOKEN_TEXTO:
            parse_declaracao_variavel();
            break;
        case TOKEN_ID_VAR:
            if (peek_token()->type == TOKEN_OP_ATRIB) {
                parse_atribuicao();
            } else if (peek_token()->type == TOKEN_OP_INC || peek_token()->type == TOKEN_OP_DEC) {
                consume(TOKEN_ID_VAR); /* Consume !x */
                consume(peek_token()->type); /* Consume ++ or -- */
                consume(TOKEN_PONTO_VIRGULA); /* Expect semicolon */
            }
            else { /* It's a general expression */
                parse_expressao();
                consume(TOKEN_PONTO_VIRGULA);
            }
            break;
        case TOKEN_ID_FUNC: { /* New case for function calls as commands */
            Symbol* func_symbol = find_symbol(currentToken->value);
            if (func_symbol == NULL || func_symbol->type != TOKEN_FUNCAO) {
                char msg[256];
                sprintf(msg, "Função '%s' não declarada.", currentToken->value);
                semantic_alert(msg);
            }
            consume(TOKEN_ID_FUNC);
            consume(TOKEN_LPAREN);
            TokenType arg_types[MAX_PARAMS];
            int num_args = 0;
            if (currentToken->type != TOKEN_RPAREN) {
                arg_types[num_args++] = parse_expressao(); /* Parse first argument */
                while (currentToken->type == TOKEN_VIRGULA) {
                    consume(TOKEN_VIRGULA);
                    if (num_args < MAX_PARAMS) {
                        arg_types[num_args++] = parse_expressao(); /* Parse subsequent arguments */
                    } else {
                        semantic_alert("Número máximo de argumentos excedido na chamada da função.");
                        parse_expressao(); /* Consume the extra argument */
                    }
                }
            }
            consume(TOKEN_RPAREN);
            if (func_symbol != NULL && func_symbol->type == TOKEN_FUNCAO) {
                semantico_check_function_call(func_symbol, arg_types, num_args);
            }
            consume(TOKEN_PONTO_VIRGULA);
            break;
        }
        case TOKEN_SE:
            parse_comando_se();
            break;
        case TOKEN_PARA:
            parse_comando_para();
            break;
        case TOKEN_LEIA:
            parse_comando_leia();
            break;
        case TOKEN_ESCREVA:
            parse_comando_escreva();
            break;
        case TOKEN_RETORNO:
            parse_retorno();
            break;
        default:
            error("Comando inválido ou inesperado.");
            free_token(currentToken);
            currentToken = get_next_token();
            break;
    }
}

static void parse_funcao_def() {
    char* previous_scope = current_scope;
    Symbol* previous_function_symbol = current_function_symbol; /* Save previous function context */

    consume(TOKEN_FUNCAO);
    current_scope = my_strdup(currentToken->value);
    
    /* Find or add function to symbol table */
    Symbol* func_symbol = find_symbol(current_scope); /* Check if function already exists (forward declaration) */
    if (func_symbol == NULL) {
        add_symbol(current_scope, TOKEN_FUNCAO, "global"); /* Functions are global scope */
        func_symbol = find_symbol(current_scope); /* Retrieve the newly added symbol */
    }
    current_function_symbol = func_symbol; /* Set current function context */

    consume(TOKEN_ID_FUNC);
    consume(TOKEN_LPAREN);
    if (currentToken->type != TOKEN_RPAREN) {
        TokenType type = currentToken->type;
        consume(type);
        add_symbol(currentToken->value, type, current_scope);
        if (func_symbol->num_params < MAX_PARAMS) {
            func_symbol->param_types[func_symbol->num_params] = type;
            func_symbol->num_params++;
        } else {
            semantic_alert("Número máximo de parâmetros excedido para a função.");
        }
        consume(TOKEN_ID_VAR);
        while (currentToken->type == TOKEN_VIRGULA) {
            consume(TOKEN_VIRGULA);
            type = currentToken->type;
            consume(type);
            add_symbol(currentToken->value, type, current_scope);
            if (func_symbol->num_params < MAX_PARAMS) {
                func_symbol->param_types[func_symbol->num_params] = type;
                func_symbol->num_params++;
            } else {
                semantic_alert("Número máximo de parâmetros excedido para a função.");
            }
            consume(TOKEN_ID_VAR);
        }
    }
    consume(TOKEN_RPAREN);
    parse_bloco();

    /* Check for mandatory return statement */
    if (current_function_symbol->has_return_statement == 0) {
        semantic_alert("Função não possui uma declaração de retorno.");
    }

    free(current_scope);
    current_scope = previous_scope;
    current_function_symbol = previous_function_symbol; /* Restore previous function context */
}

static void parse_principal_def() {
    char* previous_scope = current_scope;
    Symbol* previous_function_symbol = current_function_symbol; /* Save previous function context */

    current_scope = "principal";
    /* Principal is a special function, no need to add to symbol table as a regular function */
    current_function_symbol = NULL; /* No return type check for principal */

    consume(TOKEN_PRINCIPAL);
    consume(TOKEN_LPAREN);
    consume(TOKEN_RPAREN);
    parse_bloco();
    current_scope = previous_scope;
    current_function_symbol = previous_function_symbol; /* Restore previous function context */
}

static void parse_programa() {
    int principal_found = 0;
    while (currentToken->type != TOKEN_EOF && !syntax_error) {
        switch (currentToken->type) {
            case TOKEN_FUNCAO:
                parse_funcao_def();
                break;
            case TOKEN_PRINCIPAL:
                if (principal_found) {
                    error("Múltiplas definições do módulo principal.");
                }
                parse_principal_def();
                principal_found = 1;
                break;
            default:
                error("Esperava-se uma definição de função ou o módulo principal.");
                return;
        }
    }
    if (!principal_found) {
        error("Módulo Principal Inexistente.");
    }
}

void parse() {
    currentToken = get_next_token();
    parse_programa();
    if (!syntax_error && currentToken->type != TOKEN_EOF) {
        error("Código extra encontrado após o final do programa.");
    }
    printf("\nAnálise sintática e semântica concluída.\n");
}