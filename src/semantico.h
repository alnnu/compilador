#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "token.h"

/* Enumeração para os tipos de dados da linguagem */
typedef enum {
    TIPO_DADO_INTEIRO,
    TIPO_DADO_DECIMAL,
    TIPO_DADO_TEXTO,
    TIPO_DADO_VAZIO,
    TIPO_DADO_INDEFINIDO /* Para casos de erro ou antes da checagem */
} TipoDado;

/* Enumeração para a categoria do símbolo (variável, função, etc.) */
typedef enum {
    SIMBOLO_VARIAVEL,
    SIMBOLO_FUNCAO
} CategoriaSimbolo;

/* Estrutura para um parâmetro de função */
typedef struct Parametro {
    TipoDado tipo;
    char* nome;
    struct Parametro* proximo;
} Parametro;

/* Estrutura para uma entrada na Tabela de Símbolos */
typedef struct Simbolo {
    char* nome;
    CategoriaSimbolo categoria;
    TipoDado tipo;
    int linha_declaracao;
    char* escopo; /* Nome da função ou "global" */
    
    /* Específico para funções */
    Parametro* parametros;
    int contagem_parametros;
    int tem_retorno;

    struct Simbolo* proximo;
} Simbolo;

/* Funções de inicialização e finalização */
void inicializar_tabela_simbolos();
void liberar_tabela_simbolos();

/* Funções para manipulação da tabela de símbolos */
Simbolo* inserir_simbolo_funcao(const char* nome, int linha, const char* escopo);
Simbolo* inserir_simbolo_variavel(const char* nome, TipoDado tipo, int linha, const char* escopo);
Simbolo* buscar_simbolo(const char* nome, const char* escopo);
void adicionar_parametro(Simbolo* funcao, const char* nome_param, TipoDado tipo_param);

/* Funções de checagem semântica */
void checar_declaracao_duplicada(const char* nome, const char* escopo, int linha);
void checar_variavel_declarada(const char* nome, const char* escopo, int linha);
void checar_compatibilidade_tipos_atribuicao(TipoDado tipo_var, TipoDado tipo_expr, int linha);
void checar_compatibilidade_tipos_expressao(TipoDado tipo1, TipoDado tipo2, int linha);
void checar_operacao_texto(TokenType op, TipoDado tipo, int linha);
void checar_funcao_principal();
void checar_chamada_funcao(const char* nome, int num_args, int linha, const char* escopo);
void checar_retorno_funcao(Simbolo* funcao);

/* Funções auxiliares */
TipoDado token_type_para_tipo_dado(TokenType tipo_token);
void emitir_alerta(const char* mensagem, int linha);
void emitir_erro(const char* mensagem, int linha);

#endif /* SEMANTICO_H */
