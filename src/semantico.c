#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantico.h"
#include "lexico.h"

/* Cabeça da lista ligada que representa a tabela de símbolos */
static Simbolo* tabela_simbolos = NULL;
static int tem_funcao_principal = 0;

/* --- Funções de Alerta e Erro --- */

void emitir_alerta(const char* mensagem, int linha) {
    fprintf(stderr, "ALERTA SEMÂNTICO na linha %d: %s\n", linha, mensagem);
}

void emitir_erro(const char* mensagem, int linha) {
    fprintf(stderr, "ERRO SEMÂNTICO na linha %d: %s\n", linha, mensagem);
    exit(EXIT_FAILURE); /* Finaliza a compilação */
}

/* --- Funções de Gerenciamento da Tabela de Símbolos --- */

void inicializar_tabela_simbolos() {
    tabela_simbolos = NULL;
    tem_funcao_principal = 0;
}

void liberar_tabela_simbolos() {
    Simbolo* atual = tabela_simbolos;
    while (atual != NULL) {
        Simbolo* proximo = atual->proximo;
        free(atual->nome);
        free(atual->escopo);
        if (atual->categoria == SIMBOLO_FUNCAO) {
            Parametro* param_atual = atual->parametros;
            while (param_atual != NULL) {
                Parametro* param_proximo = param_atual->proximo;
                free(param_atual->nome);
                free(param_atual);
                param_atual = param_proximo;
            }
        }
        free(atual);
        atual = proximo;
    }
    tabela_simbolos = NULL;
}

Simbolo* inserir_simbolo_funcao(const char* nome, int linha, const char* escopo) {
    checar_declaracao_duplicada(nome, "global", linha);
    Simbolo* novo = (Simbolo*) malloc(sizeof(Simbolo));
    novo->nome = my_strdup(nome);
    novo->categoria = SIMBOLO_FUNCAO;
    novo->tipo = TIPO_DADO_INDEFINIDO; /* O tipo de retorno será definido depois */
    novo->linha_declaracao = linha;
    novo->escopo = my_strdup(escopo);
    novo->parametros = NULL;
    novo->contagem_parametros = 0;
    novo->tem_retorno = 0;
    novo->proximo = tabela_simbolos;
    tabela_simbolos = novo;
    if (strcmp(nome, "principal") == 0) {
        tem_funcao_principal++;
    }
    return novo;
}

Simbolo* inserir_simbolo_variavel(const char* nome, TipoDado tipo, int linha, const char* escopo) {
    checar_declaracao_duplicada(nome, escopo, linha);
    Simbolo* novo = (Simbolo*) malloc(sizeof(Simbolo));
    novo->nome = my_strdup(nome);
    novo->categoria = SIMBOLO_VARIAVEL;
    novo->tipo = tipo;
    novo->linha_declaracao = linha;
    novo->escopo = my_strdup(escopo);
    novo->parametros = NULL;
    novo->contagem_parametros = 0;
    novo->tem_retorno = 0;
    novo->proximo = tabela_simbolos;
    tabela_simbolos = novo;
    return novo;
}

Simbolo* buscar_simbolo(const char* nome, const char* escopo) {
    Simbolo* atual = tabela_simbolos;
    while (atual != NULL) {
        /* Checa no escopo local ou no escopo global */
        if (strcmp(atual->nome, nome) == 0 && 
            (strcmp(atual->escopo, escopo) == 0 || strcmp(atual->escopo, "global") == 0)) {
            return atual;
        }
        atual = atual->proximo;
    }
    return NULL;
}

void adicionar_parametro(Simbolo* funcao, const char* nome_param, TipoDado tipo_param) {
    if (funcao == NULL || funcao->categoria != SIMBOLO_FUNCAO) return;

    Parametro* novo_param = (Parametro*) malloc(sizeof(Parametro));
    novo_param->nome = my_strdup(nome_param);
    novo_param->tipo = tipo_param;
    novo_param->proximo = NULL;

    /* Insere no final da lista de parâmetros */
    if (funcao->parametros == NULL) {
        funcao->parametros = novo_param;
    } else {
        Parametro* atual = funcao->parametros;
        while (atual->proximo != NULL) {
            atual = atual->proximo;
        }
        atual->proximo = novo_param;
    }
    funcao->contagem_parametros++;
}

/* --- Funções de Checagem Semântica --- */

void checar_declaracao_duplicada(const char* nome, const char* escopo, int linha) {
    Simbolo* s = buscar_simbolo(nome, escopo);
    /* Permite shadowing, mas não no mesmo escopo */
    if (s != NULL && strcmp(s->escopo, escopo) == 0) {
        char msg[256];
        sprintf(msg, "Redeclaração do símbolo '%s' no mesmo escopo.", nome);
        emitir_erro(msg, linha);
    }
}

void checar_variavel_declarada(const char* nome, const char* escopo, int linha) {
    if (buscar_simbolo(nome, escopo) == NULL) {
        char msg[256];
        sprintf(msg, "Variável ou função '%s' não declarada.", nome);
        emitir_erro(msg, linha);
    }
}

void checar_compatibilidade_tipos_atribuicao(TipoDado tipo_var, TipoDado tipo_expr, int linha) {
    if (tipo_var != tipo_expr) {
        emitir_alerta("Tipos incompatíveis na atribuição.", linha);
    }
}

void checar_compatibilidade_tipos_expressao(TipoDado tipo1, TipoDado tipo2, int linha) {
    if (tipo1 != tipo2) {
        emitir_alerta("Tipos incompatíveis em expressão.", linha);
    }
}

void checar_operacao_texto(TokenType op, TipoDado tipo, int linha) {
    if (tipo == TIPO_DADO_TEXTO) {
        if (op != TOKEN_OP_IGUAL && op != TOKEN_OP_DIF) {
            emitir_alerta("Operador relacional inválido para o tipo texto. Use apenas '==' ou '<>'.", linha);
        }
    }
}

void checar_funcao_principal() {
    if (tem_funcao_principal == 0) {
        emitir_erro("Módulo Principal Inexistente.", 0);
    } else if (tem_funcao_principal > 1) {
        emitir_erro("Múltiplas definições do módulo principal.", 0);
    }
}

void checar_chamada_funcao(const char* nome, int num_args, int linha, const char* escopo) {
    Simbolo* s = buscar_simbolo(nome, escopo);
    if (s == NULL || s->categoria != SIMBOLO_FUNCAO) {
        char msg[256];
        sprintf(msg, "Função '%s' não declarada.", nome);
        emitir_erro(msg, linha);
        return;
    }
    if (s->contagem_parametros != num_args) {
        char msg[256];
        sprintf(msg, "Número incorreto de argumentos para a função '%s'. Esperado: %d, Recebido: %d.", 
                nome, s->contagem_parametros, num_args);
        emitir_erro(msg, linha);
    }
    /* A checagem de tipo dos argumentos precisaria de mais infraestrutura no parser */
}

void checar_retorno_funcao(Simbolo* funcao) {
    if (funcao != NULL && strcmp(funcao->nome, "principal") != 0) {
        /* A lógica de verificação de retorno foi movida para o analisador sintático
         para permitir funções sem retorno (procedimentos).
         Se uma função não tiver instrução de retorno, seu tipo será TIPO_DADO_VAZIO. */
    }
}

/* --- Funções Auxiliares --- */

TipoDado token_type_para_tipo_dado(TokenType tipo_token) {
    switch (tipo_token) {
        case TOKEN_INTEIRO:
            return TIPO_DADO_INTEIRO;
        case TOKEN_DECIMAL:
            return TIPO_DADO_DECIMAL;
        case TOKEN_TEXTO:
            return TIPO_DADO_TEXTO;
        default:
            return TIPO_DADO_INDEFINIDO;
    }
}
