#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sintatico.h"
#include "lexico.h"
#include "semantico.h"

static Token* currentToken;
static int syntax_error = 0;
static char current_scope[256] = "global"; /* Ação Semântica: Controle de escopo */

/* Protótipos das funções de parsing */
static void parse_programa();
static void parse_declaracao_variavel();
static void parse_funcao_def();
static void parse_principal_def();
static void parse_bloco(Simbolo* funcao_atual);
static void parse_comando(Simbolo* funcao_atual);
static TipoDado parse_atribuicao_sem_pv();
static TipoDado parse_tipo();
static void parse_atribuicao();
static void parse_comando_se(Simbolo* funcao_atual);
static void parse_comando_para(Simbolo* funcao_atual);
static void parse_comando_leia();
static void parse_comando_escreva();
static void parse_retorno(Simbolo* funcao_atual);
static TipoDado parse_expressao();
static TipoDado parse_expressao_simples();
static TipoDado parse_termo();
static TipoDado parse_fator();
static TipoDado parse_chamada_funcao();

/* Novos protótipos para o laço 'para' x3 */
static TipoDado parse_expressao_para_x3();
static TipoDado parse_expressao_simples_para_x3();
static TipoDado parse_termo_para_x3();
static TipoDado parse_fator_para_x3();

static void error(const char* message) {
    printf("ERRO SINTÁTICO na linha %d: %s.\n", 
           currentToken->line, message); 
    syntax_error = 1;
    exit(1);
}

static void consume(TokenType type) {
    if (syntax_error) return;
    if (currentToken->type == type) {
        free_token(currentToken);
        currentToken = get_next_token();
    } else {
        char msg[256];
        sprintf(msg, "Esperava-se o token %s, mas foi encontrado %s", 
                token_type_to_string(type), token_type_to_string(currentToken->type));
        error(msg);
    }
}

void parse() {
    inicializar_tabela_simbolos(); /* Ação Semântica: Inicializa tabela */
    currentToken = get_next_token();
    parse_programa();
    if (!syntax_error && currentToken->type != TOKEN_EOF) {
        error("Código extra encontrado após o final do programa");
    }
    checar_funcao_principal(); /* Ação Semântica: Checa se a função principal existe */
    printf("\nAnálise sintática e semântica concluída\n");
    liberar_tabela_simbolos(); /* Ação Semântica: Libera a memória da tabela */
}

static void parse_programa() {
    while (currentToken->type != TOKEN_EOF && !syntax_error) {
        if (currentToken->type == TOKEN_FUNCAO) {
            parse_funcao_def();
        } else if (currentToken->type == TOKEN_PRINCIPAL) {
            parse_principal_def();
        } else {
            error("Esperava-se uma definição de função ou o módulo principal");
            break;
        }
    }
}

static void parse_funcao_def() {
    consume(TOKEN_FUNCAO);
    
    /* Ação Semântica: Inserir função na tabela de símbolos */
    Simbolo* funcao_atual = inserir_simbolo_funcao(currentToken->value, currentToken->line, "global");
    strcpy(current_scope, currentToken->value);

    consume(TOKEN_ID_FUNC);
    consume(TOKEN_LPAREN);

    if (currentToken->type != TOKEN_RPAREN) {
        do {
            if(currentToken->type == TOKEN_VIRGULA) consume(TOKEN_VIRGULA);
            TipoDado tipo_param = parse_tipo();
            
            /* Ação Semântica: Adicionar parâmetro na função e na tabela de símbolos */
            adicionar_parametro(funcao_atual, currentToken->value, tipo_param);
            inserir_simbolo_variavel(currentToken->value, tipo_param, currentToken->line, current_scope);

            consume(TOKEN_ID_VAR);
        } while (currentToken->type == TOKEN_VIRGULA);
    }
    consume(TOKEN_RPAREN);
    parse_bloco(funcao_atual);

    /* Ação Semântica: Se não houver retorno, a função é do tipo vazio (procedimento) */
    if (funcao_atual->tipo == TIPO_DADO_INDEFINIDO) {
        funcao_atual->tipo = TIPO_DADO_VAZIO;
    }

    /* Ação Semântica: Checar se a função tem retorno */
    checar_retorno_funcao(funcao_atual);
    strcpy(current_scope, "global"); /* Retorna ao escopo global */
}

static void parse_principal_def() {
    consume(TOKEN_PRINCIPAL);
    
    /* Ação Semântica: Inserir 'principal' na tabela */
    Simbolo* funcao_atual = inserir_simbolo_funcao("principal", currentToken->line, "global");
    strcpy(current_scope, "principal");

    consume(TOKEN_LPAREN);
    consume(TOKEN_RPAREN);
    parse_bloco(funcao_atual);
    strcpy(current_scope, "global");
}

static void parse_bloco(Simbolo* funcao_atual) {
    consume(TOKEN_LBRACE);
    while (currentToken->type != TOKEN_RBRACE && currentToken->type != TOKEN_EOF && !syntax_error) {
        parse_comando(funcao_atual);
    }
    consume(TOKEN_RBRACE);
}

static void parse_comando(Simbolo* funcao_atual) {
    /* Ação Semântica: Impede declaração em locais proibidos */
    if (strcmp(current_scope, "se") == 0 || strcmp(current_scope, "leia") == 0) {
        if (currentToken->type == TOKEN_INTEIRO || currentToken->type == TOKEN_DECIMAL || currentToken->type == TOKEN_TEXTO) {
            emitir_erro("Declaração de variável não permitida neste escopo.", currentToken->line);
        }
    }

    switch (currentToken->type) {
        case TOKEN_INTEIRO:
        case TOKEN_DECIMAL:
        case TOKEN_TEXTO:
            parse_declaracao_variavel();
            break;
        case TOKEN_ID_VAR:
            parse_atribuicao();
            break;
        case TOKEN_SE:
            parse_comando_se(funcao_atual);
            break;
        case TOKEN_PARA:
            parse_comando_para(funcao_atual);
            break;
        case TOKEN_LEIA:
            parse_comando_leia();
            break;
        case TOKEN_ESCREVA:
            parse_comando_escreva();
            break;
        case TOKEN_RETORNO:
            parse_retorno(funcao_atual);
            break;
        case TOKEN_ID_FUNC:
            parse_chamada_funcao();
            consume(TOKEN_PONTO_VIRGULA);
            break;
        default:
            error("Comando inválido ou inesperado");
            free_token(currentToken);
            currentToken = get_next_token();
            break;
    }
}

static TipoDado parse_tipo() {
    TipoDado tipo = token_type_para_tipo_dado(currentToken->type);
    if (tipo != TIPO_DADO_INDEFINIDO) {
        consume(currentToken->type);
        return tipo;
    } else {
        error("Esperava-se um tipo de dado (inteiro, texto, decimal)");
        return TIPO_DADO_INDEFINIDO;
    }
}

static void parse_declaracao_variavel() {
    TipoDado tipo = parse_tipo();
    do {
        if (currentToken->type == TOKEN_VIRGULA) {
            consume(TOKEN_VIRGULA);
        }
        
        /* Ação Semântica: Inserir variável na tabela */
        char* nome_var = strdup(currentToken->value);
        inserir_simbolo_variavel(nome_var, tipo, currentToken->line, current_scope);
        consume(TOKEN_ID_VAR);

        if (currentToken->type == TOKEN_OP_ATRIB) {
            consume(TOKEN_OP_ATRIB);
            TipoDado tipo_expr = parse_expressao();
            /* Ação Semântica: Checar tipos na atribuição da declaração */
            checar_compatibilidade_tipos_atribuicao(tipo, tipo_expr, currentToken->line);
        }

        if (currentToken->type == TOKEN_LBRACKET) {
            consume(TOKEN_LBRACKET);
            if (currentToken->type == TOKEN_LITERAL_INT || currentToken->type == TOKEN_LITERAL_DEC) {
                consume(currentToken->type);
            } else {
                error("Esperava-se um tamanho para o tipo de dado");
            }
            consume(TOKEN_RBRACKET);
        }
        free(nome_var);
    } while (currentToken->type == TOKEN_VIRGULA);

    consume(TOKEN_PONTO_VIRGULA);
}

static TipoDado parse_atribuicao_sem_pv() {
    /* Ação Semântica: Checar se a variável foi declarada */
    checar_variavel_declarada(currentToken->value, current_scope, currentToken->line);
    Simbolo* var = buscar_simbolo(currentToken->value, current_scope);
    consume(TOKEN_ID_VAR);
    
    consume(TOKEN_OP_ATRIB);
    
    TipoDado tipo_expr = parse_expressao();
    /* Ação Semântica: Checar compatibilidade de tipos */
    if(var) checar_compatibilidade_tipos_atribuicao(var->tipo, tipo_expr, currentToken->line);
    
    return var ? var->tipo : TIPO_DADO_INDEFINIDO;
}

static void parse_atribuicao() {
    parse_atribuicao_sem_pv();
    consume(TOKEN_PONTO_VIRGULA);
}

static void parse_comando_se(Simbolo* funcao_atual) {
    consume(TOKEN_SE);
    consume(TOKEN_LPAREN);
    parse_expressao();
    consume(TOKEN_RPAREN);

    char* previous_scope = strdup(current_scope);
    strcpy(current_scope, "se");

    if (currentToken->type == TOKEN_LBRACE) {
        parse_bloco(funcao_atual);
    } else {
        parse_comando(funcao_atual);
    }

    if (currentToken->type == TOKEN_SENAO) {
        consume(TOKEN_SENAO);
        if (currentToken->type == TOKEN_LBRACE) {
            parse_bloco(funcao_atual);
        } else {
            parse_comando(funcao_atual);
        }
    }
    strcpy(current_scope, previous_scope);
    free(previous_scope);
}

static void parse_comando_para(Simbolo* funcao_atual) {
    consume(TOKEN_PARA);
    consume(TOKEN_LPAREN);

    if (currentToken->type != TOKEN_PONTO_VIRGULA) {
        parse_atribuicao_sem_pv();
    }
    consume(TOKEN_PONTO_VIRGULA);

    if (currentToken->type != TOKEN_PONTO_VIRGULA) {
        parse_expressao();
    }
    consume(TOKEN_PONTO_VIRGULA);

    if (currentToken->type != TOKEN_RPAREN) {
        parse_expressao_para_x3();
    }

    consume(TOKEN_RPAREN);

    if (currentToken->type == TOKEN_LBRACE) {
        parse_bloco(funcao_atual);
    } else {
        parse_comando(funcao_atual);
    }
}

static void parse_comando_leia() {
    consume(TOKEN_LEIA);
    consume(TOKEN_LPAREN);
    
    char* previous_scope = strdup(current_scope);
    strcpy(current_scope, "leia");

    /* Ação Semântica: Checar se variável foi declarada */
    checar_variavel_declarada(currentToken->value, previous_scope, currentToken->line);
    consume(TOKEN_ID_VAR);

    while (currentToken->type == TOKEN_VIRGULA) {
        consume(TOKEN_VIRGULA);
        checar_variavel_declarada(currentToken->value, previous_scope, currentToken->line);
        consume(TOKEN_ID_VAR);
    }
    
    strcpy(current_scope, previous_scope);
    free(previous_scope);

    consume(TOKEN_RPAREN);
    consume(TOKEN_PONTO_VIRGULA);
}

static void parse_comando_escreva() {
    consume(TOKEN_ESCREVA);
    consume(TOKEN_LPAREN);
    if (currentToken->type == TOKEN_ID_VAR) {
        checar_variavel_declarada(currentToken->value, current_scope, currentToken->line);
    }
    parse_expressao();

    while (currentToken->type == TOKEN_VIRGULA) {
        consume(TOKEN_VIRGULA);
        if (currentToken->type == TOKEN_ID_VAR) {
            checar_variavel_declarada(currentToken->value, current_scope, currentToken->line);
        }
        parse_expressao();
    }
    consume(TOKEN_RPAREN);
    consume(TOKEN_PONTO_VIRGULA);
}

static void parse_retorno(Simbolo* funcao_atual) {
    consume(TOKEN_RETORNO);
    if (funcao_atual) {
        if (strcmp(funcao_atual->nome, "principal") == 0) {
            emitir_erro("Módulo 'principal' não pode ter instrução de retorno.", currentToken->line);
        }
        funcao_atual->tem_retorno = 1;
        TipoDado tipo_retorno = parse_expressao();

        if (funcao_atual->tipo == TIPO_DADO_INDEFINIDO) {
            funcao_atual->tipo = tipo_retorno;
        } else {
            /* Se já existe um tipo de retorno, verifica a compatibilidade */
            checar_compatibilidade_tipos_atribuicao(funcao_atual->tipo, tipo_retorno, currentToken->line);
        }
    } else {
        emitir_erro("Instrução de retorno fora de uma função.", currentToken->line);
        parse_expressao(); /* Consome a expressão para evitar erros em cascata */
    }
    consume(TOKEN_PONTO_VIRGULA);
}

static TipoDado parse_chamada_funcao() {
    char* func_name = my_strdup(currentToken->value);
    consume(TOKEN_ID_FUNC);
    consume(TOKEN_LPAREN);

    int arg_count = 0;
    if (currentToken->type != TOKEN_RPAREN) {
        do {
            if (currentToken->type == TOKEN_VIRGULA) {
                consume(TOKEN_VIRGULA);
            }
            parse_expressao();
            arg_count++;
        } while (currentToken->type == TOKEN_VIRGULA);
    }

    consume(TOKEN_RPAREN);

    checar_chamada_funcao(func_name, arg_count, currentToken->line, current_scope);
    Simbolo* func_simbolo = buscar_simbolo(func_name, current_scope);
    free(func_name);

    return func_simbolo ? func_simbolo->tipo : TIPO_DADO_INDEFINIDO;
}

static TipoDado parse_fator() {
    switch (currentToken->type) {
        case TOKEN_LITERAL_INT:
            consume(currentToken->type);
            return TIPO_DADO_INTEIRO;
        case TOKEN_LITERAL_DEC:
            consume(currentToken->type);
            return TIPO_DADO_DECIMAL;
        case TOKEN_LITERAL_TEXTO:
            consume(currentToken->type);
            return TIPO_DADO_TEXTO;
        case TOKEN_ID_VAR:
            /* Ação Semântica: Checar se variável foi declarada */
            checar_variavel_declarada(currentToken->value, current_scope, currentToken->line);
            Simbolo* var = buscar_simbolo(currentToken->value, current_scope);
            consume(currentToken->type);
            return var ? var->tipo : TIPO_DADO_INDEFINIDO;
        case TOKEN_ID_FUNC:
            return parse_chamada_funcao();
        case TOKEN_LPAREN:
            consume(TOKEN_LPAREN);
            TipoDado tipo = parse_expressao();
            consume(TOKEN_RPAREN);
            return tipo;
        default:
            error("Fator inválido em expressão");
            return TIPO_DADO_INDEFINIDO;
    }
}

static TipoDado parse_termo() {
    TipoDado tipo = parse_fator();
    while (currentToken->type == TOKEN_OP_MULT || currentToken->type == TOKEN_OP_DIV || currentToken->type == TOKEN_OP_EXP) {
        TipoDado tipo2 = parse_fator();
        /* Ação Semântica: Checar compatibilidade de tipos na expressão */
        checar_compatibilidade_tipos_expressao(tipo, tipo2, currentToken->line);
        tipo = tipo2; /* Propaga o tipo */
    }
    return tipo;
}

static TipoDado parse_expressao_simples() {
    TipoDado tipo = parse_termo();
    while (currentToken->type == TOKEN_OP_SOMA || currentToken->type == TOKEN_OP_SUB) {
        TipoDado tipo2 = parse_termo();
        /* Ação Semântica: Checar compatibilidade de tipos na expressão */
        checar_compatibilidade_tipos_expressao(tipo, tipo2, currentToken->line);
        tipo = tipo2;
    }
    return tipo;
}

static TipoDado parse_expressao() {
    TipoDado tipo = parse_expressao_simples();
    while (currentToken->type == TOKEN_OP_IGUAL || currentToken->type == TOKEN_OP_DIF ||
           currentToken->type == TOKEN_OP_MENOR || currentToken->type == TOKEN_OP_MENOR_IGUAL ||
           currentToken->type == TOKEN_OP_MAIOR || currentToken->type == TOKEN_OP_MAIOR_IGUAL) {
        TokenType op = currentToken->type;
        consume(op);
        TipoDado tipo2 = parse_expressao_simples();
        /* Ação Semântica: Checar tipos e operações com texto */
        checar_operacao_texto(op, tipo, currentToken->line);
        checar_operacao_texto(op, tipo2, currentToken->line);
        checar_compatibilidade_tipos_expressao(tipo, tipo2, currentToken->line);
        tipo = TIPO_DADO_INTEIRO; /* Resultado de uma expressão relacional é booleano (inteiro) */
    }
    while (currentToken->type == TOKEN_OP_E || currentToken->type == TOKEN_OP_OU) {
        consume(currentToken->type);
        parse_expressao_simples();
        tipo = TIPO_DADO_INTEIRO;
    }
    return tipo;
}

/* Implementações para o laço 'para' x3 (com retorno de tipo) */
static TipoDado parse_fator_para_x3() {
    if (currentToken->type == TOKEN_OP_INC || currentToken->type == TOKEN_OP_DEC) {
        consume(currentToken->type);
        checar_variavel_declarada(currentToken->value, current_scope, currentToken->line);
        Simbolo* var = buscar_simbolo(currentToken->value, current_scope);
        consume(TOKEN_ID_VAR);
        return var ? var->tipo : TIPO_DADO_INDEFINIDO;
    }

    TipoDado tipo = parse_fator();

    if (currentToken->type == TOKEN_OP_INC || currentToken->type == TOKEN_OP_DEC) {
        consume(currentToken->type);
    }
    return tipo;
}

static TipoDado parse_termo_para_x3() {
    TipoDado tipo = parse_fator_para_x3();
    while (currentToken->type == TOKEN_OP_MULT || currentToken->type == TOKEN_OP_DIV || currentToken->type == TOKEN_OP_EXP) {
        consume(currentToken->type);
        TipoDado tipo2 = parse_fator_para_x3();
        checar_compatibilidade_tipos_expressao(tipo, tipo2, currentToken->line);
        tipo = tipo2;
    }
    return tipo;
}

static TipoDado parse_expressao_simples_para_x3() {
    TipoDado tipo = parse_termo_para_x3();
    while (currentToken->type == TOKEN_OP_SOMA || currentToken->type == TOKEN_OP_SUB) {
        consume(currentToken->type);
        TipoDado tipo2 = parse_termo_para_x3();
        checar_compatibilidade_tipos_expressao(tipo, tipo2, currentToken->line);
        tipo = tipo2;
    }
    return tipo;
}

static TipoDado parse_expressao_para_x3() {
    return parse_expressao_simples_para_x3();
}

