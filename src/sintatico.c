#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sintatico.h" 
#include "lexico.h"

static Token* currentToken;
static int syntax_error = 0;
static int in_se_senao_block = 0;



static void parse_programa();
static void parse_declaracao_variavel();
static void parse_funcao_def();
static void parse_principal_def();
static void parse_bloco();
static void parse_comando();
static void parse_atribuicao_sem_pv();
static void parse_tipo();
static void parse_atribuicao();
static void parse_comando_se();
static void parse_comando_para();
static void parse_comando_leia();
static void parse_comando_escreva();
static void parse_retorno();
static void parse_expressao();
static void parse_expressao_simples();
static void parse_termo();
static void parse_fator();
static void parse_expressao_para_x3();
static void parse_expressao_simples_para_x3();
static void parse_termo_para_x3();
static void parse_fator_para_x3();

static void error(const char* message) {
    printf("ERRO SINTÁTICO na linha %d: %s. Token inesperado: '%s' (%s)\n", 
           currentToken->line, message, currentToken->value ? currentToken->value : "", 
           token_type_to_string(currentToken->type));
    syntax_error = 1;
    /* Tenta continuar para encontrar mais erros, mas para em caso de erro grave. */
    /* Em um compilador real, a recuperação de erros seria mais sofisticada. */
    exit(1); /* Finaliza para evitar erros em cascata */
}

/* --- Tabela de Símbolos --- */

#define MAX_SYMBOLS 1024

typedef struct {
    char* name;
    TokenType type;
} Symbol;

Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

Symbol* find_symbol(const char* name);

void add_symbol(const char* name, TokenType type) {
    if (find_symbol(name) != NULL) {
        char msg[256];
        sprintf(msg, "Variável '%s' já declarada.", name);
        error(msg);
    }
    if (symbol_count < MAX_SYMBOLS) {
        symbol_table[symbol_count].name = my_strdup(name);
        symbol_table[symbol_count].type = type;
        symbol_count++;
    } else {
        error("Estouro da tabela de símbolos.");
    }
}

Symbol* find_symbol(const char* name) {
    int i;
    for (i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return &symbol_table[i];
        }
    }
    return NULL;
}

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

void parse() {
    currentToken = get_next_token();
    parse_programa();
    if (!syntax_error && currentToken->type != TOKEN_EOF) {
        error("Código extra encontrado após o final do programa.");
    }
    printf("\nAnálise sintática concluída.\n");
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
                return; /* Sai para evitar loop infinito */
        }
    }
    if (!principal_found) {
        error("Módulo Principal Inexistente.");
    }
}

static void parse_funcao_def() {
    consume(TOKEN_FUNCAO);
    consume(TOKEN_ID_FUNC);
    consume(TOKEN_LPAREN);
    /* Lógica para parâmetros */
    if (currentToken->type != TOKEN_RPAREN) {
        TokenType type = currentToken->type;
        parse_tipo(); /* First parameter type */
        add_symbol(currentToken->value, type);
        consume(TOKEN_ID_VAR); /* First parameter name */
        while (currentToken->type == TOKEN_VIRGULA) {
            consume(TOKEN_VIRGULA);
            type = currentToken->type;
            parse_tipo(); /* Subsequent parameter type */
            add_symbol(currentToken->value, type);
            consume(TOKEN_ID_VAR); /* Subsequent parameter name */
        }
    }
    consume(TOKEN_RPAREN);
    parse_bloco();
}

static void parse_principal_def() {
    consume(TOKEN_PRINCIPAL);
    consume(TOKEN_LPAREN);
    consume(TOKEN_RPAREN);
    parse_bloco();
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
        case TOKEN_LITERAL_INT:
        case TOKEN_LITERAL_DEC:
        case TOKEN_LITERAL_TEXTO:
        case TOKEN_LPAREN: /* For expressions like (a + b) */
            /* Check for assignment or increment/decrement */
            if (currentToken->type == TOKEN_ID_VAR && (peek_token()->type == TOKEN_OP_ATRIB || peek_token()->type == TOKEN_OP_INC || peek_token()->type == TOKEN_OP_DEC)) {
                if (peek_token()->type == TOKEN_OP_ATRIB) {
                    parse_atribuicao();
                } else { /* Increment or decrement */
                    consume(currentToken->type); /* Consume the ID_VAR */
                    /* At this point, currentToken is the OP_INC or OP_DEC */
                    consume(currentToken->type); /* Consume ++ or -- */
                    consume(TOKEN_PONTO_VIRGULA);
                }
            } else { /* It's a general expression */
                parse_expressao();
                consume(TOKEN_PONTO_VIRGULA);
            }
            break;
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
            /* Avança para tentar recuperar, mas pode causar problemas */
            free_token(currentToken);
            currentToken = get_next_token();
            break;
    }
}

/* Implementações vazias para o resto das funções de parsing (a serem completadas) */
static void parse_tipo() {
    if (currentToken->type == TOKEN_INTEIRO || currentToken->type == TOKEN_TEXTO || currentToken->type == TOKEN_DECIMAL) {
        consume(currentToken->type);
    } else {
        error("Esperava-se um tipo de dado (inteiro, texto, decimal).");
    }
}

static void parse_declaracao_variavel() {
    if (in_se_senao_block) {
        error("Declaração de variável não permitida dentro de um bloco se/senao.");
    }
    TokenType type = currentToken->type;
    parse_tipo();
    do {
        if (currentToken->type == TOKEN_VIRGULA) {
            consume(TOKEN_VIRGULA);
        }
        add_symbol(currentToken->value, type);
        consume(TOKEN_ID_VAR);
        if (currentToken->type == TOKEN_OP_ATRIB) {
            consume(TOKEN_OP_ATRIB);
            parse_expressao();
        }
        if (currentToken->type == TOKEN_LBRACKET) {
            consume(TOKEN_LBRACKET);
            if (currentToken->type == TOKEN_LITERAL_INT || currentToken->type == TOKEN_LITERAL_DEC) {
                consume(currentToken->type);
            } else {
                error("Esperava-se um tamanho para o tipo de dado.");
            }
            consume(TOKEN_RBRACKET);
        }
    } while (currentToken->type == TOKEN_VIRGULA);

    consume(TOKEN_PONTO_VIRGULA);
}
static void parse_atribuicao_sem_pv() {
    consume(TOKEN_ID_VAR);
    consume(TOKEN_OP_ATRIB);
    parse_expressao();
}

static void parse_atribuicao() {
    parse_atribuicao_sem_pv();
    consume(TOKEN_PONTO_VIRGULA);
}
static void parse_comando_se() {

    consume(TOKEN_SE);
    consume(TOKEN_LPAREN);
    parse_expressao();
    consume(TOKEN_RPAREN);

    in_se_senao_block = 1;
    if (currentToken->type == TOKEN_LBRACE) {
        parse_bloco();
    } else {
        parse_comando();
    }
    in_se_senao_block = 0;

    if (currentToken->type == TOKEN_SENAO) {
        consume(TOKEN_SENAO);
        in_se_senao_block = 1;
        if (currentToken->type == TOKEN_LBRACE) {
            parse_bloco();
        } else {
            parse_comando();
        }
        in_se_senao_block = 0;
    }
}
static void parse_comando_para() {
    consume(TOKEN_PARA);
    consume(TOKEN_LPAREN);

    /* x1 - Inicialização */
    if (currentToken->type != TOKEN_PONTO_VIRGULA) {
        parse_atribuicao_sem_pv();
        while (currentToken->type == TOKEN_VIRGULA) {
            consume(TOKEN_VIRGULA);
            parse_atribuicao_sem_pv();
        }
    }
    consume(TOKEN_PONTO_VIRGULA);

    /* x2 - Condição */
    if (currentToken->type != TOKEN_PONTO_VIRGULA) {
        parse_expressao();
    }
    consume(TOKEN_PONTO_VIRGULA);

    /* x3 - Incremento */
    if (currentToken->type != TOKEN_RPAREN) {
        parse_expressao_para_x3(); /* Usa a nova função para x3 */
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
    consume(TOKEN_ID_VAR);
    while (currentToken->type == TOKEN_VIRGULA) {
        consume(TOKEN_VIRGULA);
        consume(TOKEN_ID_VAR);
    }
    consume(TOKEN_RPAREN);
    consume(TOKEN_PONTO_VIRGULA);
}
static void parse_comando_escreva() {
    consume(TOKEN_ESCREVA);
    consume(TOKEN_LPAREN);
    if (currentToken->type == TOKEN_LITERAL_TEXTO) {
        consume(currentToken->type);
    } else if (currentToken->type == TOKEN_ID_VAR) {
        if (find_symbol(currentToken->value) == NULL) {
            printf("ALERTA SEMÂNTICO na linha %d: Variável '%s' não declarada.\n", currentToken->line, currentToken->value);
        }
        consume(currentToken->type);
    } else {
        parse_expressao();
    }
    while (currentToken->type == TOKEN_VIRGULA) {
        consume(TOKEN_VIRGULA);
        if (currentToken->type == TOKEN_LITERAL_TEXTO) {
            consume(currentToken->type);
        }
        else if (currentToken->type == TOKEN_ID_VAR) {
            if (find_symbol(currentToken->value) == NULL) {
                printf("ALERTA SEMÂNTICO na linha %d: Variável '%s' não declarada.\n", currentToken->line, currentToken->value);
            }
            consume(currentToken->type);
        } else {
            parse_expressao();
        }
    }
    consume(TOKEN_RPAREN);
    consume(TOKEN_PONTO_VIRGULA);
}
static void parse_retorno() {
    consume(TOKEN_RETORNO);
    parse_expressao();
    consume(TOKEN_PONTO_VIRGULA);
}
static void parse_fator() {
    switch (currentToken->type) {
        case TOKEN_LITERAL_INT:
        case TOKEN_LITERAL_DEC:
        case TOKEN_LITERAL_TEXTO:
            consume(currentToken->type);
            break;
        case TOKEN_ID_VAR:
            if (find_symbol(currentToken->value) == NULL) {
                printf("ALERTA SEMÂNTICO na linha %d: Variável '%s' não declarada.\n", currentToken->line, currentToken->value);
            }
            consume(currentToken->type);
            break;
        case TOKEN_ID_FUNC:
            consume(TOKEN_ID_FUNC);
            consume(TOKEN_LPAREN);
            if (currentToken->type != TOKEN_RPAREN) {
                parse_expressao();
                while (currentToken->type == TOKEN_VIRGULA) {
                    consume(TOKEN_VIRGULA);
                    parse_expressao();
                }
            }
            consume(TOKEN_RPAREN);
            break;
        case TOKEN_LPAREN:
            consume(TOKEN_LPAREN);
            parse_expressao();
            consume(TOKEN_RPAREN);
            break;
        default:
            error("Fator inválido em expressão.");
    }
}

static void parse_termo() {
    parse_fator();
    while (currentToken->type == TOKEN_OP_MULT || currentToken->type == TOKEN_OP_DIV || currentToken->type == TOKEN_OP_EXP) {
        consume(currentToken->type);
        parse_fator();
    }
}

static void parse_expressao_simples() {
    parse_termo();
    while (currentToken->type == TOKEN_OP_SOMA || currentToken->type == TOKEN_OP_SUB) {
        consume(currentToken->type);
        parse_termo();
    }
}

static void parse_expressao() {
    parse_expressao_simples();

    while (currentToken->type == TOKEN_OP_IGUAL || currentToken->type == TOKEN_OP_DIF ||
           currentToken->type == TOKEN_OP_MENOR || currentToken->type == TOKEN_OP_MENOR_IGUAL ||
           currentToken->type == TOKEN_OP_MAIOR || currentToken->type == TOKEN_OP_MAIOR_IGUAL) {
        consume(currentToken->type);
        parse_expressao_simples();
    }
    while (currentToken->type == TOKEN_OP_E || currentToken->type == TOKEN_OP_OU) {
        consume(currentToken->type);
        parse_expressao_simples();
    }
}

/* Implementações para o laço 'para' x3 */
static void parse_fator_para_x3() {
    /* Trata operadores de pré-incremento/decremento */
    if (currentToken->type == TOKEN_OP_INC || currentToken->type == TOKEN_OP_DEC) {
        consume(currentToken->type);
        consume(TOKEN_ID_VAR);
        return; /* Retorna pois isso é um fator completo */
    }

    switch (currentToken->type) {
        case TOKEN_LITERAL_INT:
        case TOKEN_LITERAL_DEC:
        case TOKEN_LITERAL_TEXTO:
            consume(currentToken->type);
            break;
        case TOKEN_ID_VAR:
            if (find_symbol(currentToken->value) == NULL) {
                printf("ALERTA SEMÂNTICO na linha %d: Variável '%s' não declarada.\n", currentToken->line, currentToken->value);
            }
            consume(currentToken->type);
            break;
        case TOKEN_ID_FUNC:
            consume(TOKEN_ID_FUNC);
            consume(TOKEN_LPAREN);
            if (currentToken->type != TOKEN_RPAREN) {
                parse_expressao();
                while (currentToken->type == TOKEN_VIRGULA) {
                    consume(TOKEN_VIRGULA);
                    parse_expressao();
                }
            }
            consume(TOKEN_RPAREN);
            break;
        case TOKEN_LPAREN:
            consume(TOKEN_LPAREN);
            parse_expressao_para_x3();
            consume(TOKEN_RPAREN);
            break;
        default:
            error("Fator inválido em expressão para x3 do para.");
    }

    /* Trata operadores de pós-incremento/decremento */
    if (currentToken->type == TOKEN_OP_INC || currentToken->type == TOKEN_OP_DEC) {
        consume(currentToken->type);
    }
}

static void parse_termo_para_x3() {
    parse_fator_para_x3();
    while (currentToken->type == TOKEN_OP_MULT || currentToken->type == TOKEN_OP_DIV || currentToken->type == TOKEN_OP_EXP) {
        consume(currentToken->type);
        parse_fator_para_x3();
    }
}

static void parse_expressao_simples_para_x3() {
    parse_termo_para_x3();
    while (currentToken->type == TOKEN_OP_SOMA || currentToken->type == TOKEN_OP_SUB) {
        consume(currentToken->type);
        parse_termo_para_x3();
    }
}

static void parse_expressao_para_x3() {
    parse_expressao_simples_para_x3();
    while (currentToken->type == TOKEN_OP_IGUAL || currentToken->type == TOKEN_OP_DIF ||
           currentToken->type == TOKEN_OP_MENOR || currentToken->type == TOKEN_OP_MENOR_IGUAL ||
           currentToken->type == TOKEN_OP_MAIOR || currentToken->type == TOKEN_OP_MAIOR_IGUAL) {
        consume(currentToken->type);
        parse_expressao_simples_para_x3();
    }
    while (currentToken->type == TOKEN_OP_E || currentToken->type == TOKEN_OP_OU) {
        consume(currentToken->type);
        parse_expressao_simples_para_x3();
    }
}

