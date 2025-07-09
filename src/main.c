#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- Implementações de Funções (para compatibilidade com C90) --- */

char* my_strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char* new_s = malloc(len);
    if (new_s == NULL) return NULL;
    memcpy(new_s, s, len);
    return new_s;
}

char* my_strndup(const char* s, size_t n) {
    if (s == NULL) return NULL;
    size_t len = strnlen(s, n);
    char* new_s = malloc(len + 1);
    if (new_s == NULL) return NULL;
    memcpy(new_s, s, len);
    new_s[len] = '\0';
    return new_s;
}

/* --- Configuração de Memória --- */
#define MAX_MEMORY_KB 2048
long current_memory_used = 0;
long max_memory_used = 0;

void* Malloc(size_t size) {
    if (current_memory_used + size > MAX_MEMORY_KB * 1024) {
        printf("ERRO: Memória Insuficiente.\n");
        exit(1);
    }
    void* ptr = malloc(size);
    if (ptr == NULL) {
        printf("ERRO: Falha ao alocar memória.\n");
        exit(1);
    }
    current_memory_used += size;
    if (current_memory_used > max_memory_used) {
        max_memory_used = current_memory_used;
    }
    if (current_memory_used > (MAX_MEMORY_KB * 1024 * 0.9) && current_memory_used < (MAX_MEMORY_KB * 1024)) {
        printf("ALERTA: Memória utilizada entre 90%% e 99%% do total disponível.\n");
    }
    return ptr;
}

void Free(void* ptr, size_t size) {
    if (ptr != NULL) {
        free(ptr);
        current_memory_used -= size;
    }
}

/* --- Analisador Léxico (Lexer) --- */

typedef enum {
    TOKEN_EOF, TOKEN_ERRO,
    /* Palavras Reservadas */
    TOKEN_PRINCIPAL, TOKEN_FUNCAO, TOKEN_RETORNO, TOKEN_LEIA, TOKEN_ESCREVA,
    TOKEN_SE, TOKEN_SENAO, TOKEN_PARA, TOKEN_INTEIRO, TOKEN_TEXTO, TOKEN_DECIMAL,
    /* Identificadores */
    TOKEN_ID_VAR, TOKEN_ID_FUNC,
    /* Literais */
    TOKEN_LITERAL_INT, TOKEN_LITERAL_DEC, TOKEN_LITERAL_TEXTO,
    /* Operadores */
    TOKEN_OP_SOMA, TOKEN_OP_SUB, TOKEN_OP_MULT, TOKEN_OP_DIV, TOKEN_OP_EXP,
    TOKEN_OP_ATRIB, TOKEN_OP_IGUAL, TOKEN_OP_DIF, TOKEN_OP_MENOR, TOKEN_OP_MENOR_IGUAL,
    TOKEN_OP_MAIOR, TOKEN_OP_MAIOR_IGUAL, TOKEN_OP_E, TOKEN_OP_OU,
    /* Separadores */
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_LBRACKET, TOKEN_RBRACKET, TOKEN_VIRGULA, TOKEN_PONTO_VIRGULA
} TokenType;

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERRO: return "ERRO";
        case TOKEN_PRINCIPAL: return "PRINCIPAL";
        case TOKEN_FUNCAO: return "FUNCAO";
        case TOKEN_RETORNO: return "RETORNO";
        case TOKEN_LEIA: return "LEIA";
        case TOKEN_ESCREVA: return "ESCREVA";
        case TOKEN_SE: return "SE";
        case TOKEN_SENAO: return "SENAO";
        case TOKEN_PARA: return "PARA";
        case TOKEN_INTEIRO: return "INTEIRO";
        case TOKEN_TEXTO: return "TEXTO";
        case TOKEN_DECIMAL: return "DECIMAL";
        case TOKEN_ID_VAR: return "ID_VAR";
        case TOKEN_ID_FUNC: return "ID_FUNC";
        case TOKEN_LITERAL_INT: return "LITERAL_INT";
        case TOKEN_LITERAL_DEC: return "LITERAL_DEC";
        case TOKEN_LITERAL_TEXTO: return "LITERAL_TEXTO";
        case TOKEN_OP_SOMA: return "OP_SOMA";
        case TOKEN_OP_SUB: return "OP_SUB";
        case TOKEN_OP_MULT: return "OP_MULT";
        case TOKEN_OP_DIV: return "OP_DIV";
        case TOKEN_OP_EXP: return "OP_EXP";
        case TOKEN_OP_ATRIB: return "OP_ATRIB";
        case TOKEN_OP_IGUAL: return "OP_IGUAL";
        case TOKEN_OP_DIF: return "OP_DIF";
        case TOKEN_OP_MENOR: return "OP_MENOR";
        case TOKEN_OP_MENOR_IGUAL: return "OP_MENOR_IGUAL";
        case TOKEN_OP_MAIOR: return "OP_MAIOR";
        case TOKEN_OP_MAIOR_IGUAL: return "OP_MAIOR_IGUAL";
        case TOKEN_OP_E: return "OP_E";
        case TOKEN_OP_OU: return "OP_OU";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_VIRGULA: return "VIRGULA";
        case TOKEN_PONTO_VIRGULA: return "PONTO_VIRGULA";
        default: return "DESCONHECIDO";
    }
}

typedef struct {
    TokenType type;
    char* value;
    int line;
} Token;

char* current_file_content;
int current_pos = 0;
int current_line = 1;

Token* create_token(TokenType type, char* value) {
    Token* token = (Token*)Malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->line = current_line;
    return token;
}

void free_token(Token* token) {
    if (token) {
        if (token->value) free(token->value);
        Free(token, sizeof(Token));
    }
}

int is_keyword(const char* str) {
    if (strcmp(str, "principal") == 0) return TOKEN_PRINCIPAL;
    if (strcmp(str, "funcao") == 0) return TOKEN_FUNCAO;
    if (strcmp(str, "retorno") == 0) return TOKEN_RETORNO;
    if (strcmp(str, "leia") == 0) return TOKEN_LEIA;
    if (strcmp(str, "escreva") == 0) return TOKEN_ESCREVA;
    if (strcmp(str, "se") == 0) return TOKEN_SE;
    if (strcmp(str, "senao") == 0) return TOKEN_SENAO;
    if (strcmp(str, "para") == 0) return TOKEN_PARA;
    if (strcmp(str, "inteiro") == 0) return TOKEN_INTEIRO;
    if (strcmp(str, "texto") == 0) return TOKEN_TEXTO;
    if (strcmp(str, "decimal") == 0) return TOKEN_DECIMAL;
    return 0;
}

int is_operator(const char *str) {
    return (strcmp(str, "+") == 0 || strcmp(str, "-") == 0 || strcmp(str, "*") == 0 ||
            strcmp(str, "/") == 0 || strcmp(str, "^") == 0 || strcmp(str, "=") == 0 ||
            strcmp(str, "==") == 0 || strcmp(str, "<>") == 0 || strcmp(str, "<") == 0 ||
            strcmp(str, "<=") == 0 || strcmp(str, ">") == 0 || strcmp(str, ">=") == 0 ||
            strcmp(str, "&&") == 0 || strcmp(str, "||") == 0);
}


Token* get_next_token() {
    while (current_file_content[current_pos] != '\0') {
        char current_char = current_file_content[current_pos];

        if (isspace(current_char)) {
            if (current_char == '\n') current_line++;
            current_pos++;
            continue;
        }

        if (current_char == '!') { /* Variável */
            int start = current_pos;
            current_pos++;
            if (islower(current_file_content[current_pos])) {
                current_pos++;
                while (isalnum(current_file_content[current_pos])) {
                    current_pos++;
                }
                char* value = my_strndup(&current_file_content[start], current_pos - start);
                return create_token(TOKEN_ID_VAR, value);
            }
            return create_token(TOKEN_ERRO, my_strdup("Nome de variável inválido"));
        }

        if (current_char == '_') { /* Função */
            if (current_file_content[current_pos + 1] == '_') {
                int start = current_pos;
                current_pos += 2;
                while (isalnum(current_file_content[current_pos])) {
                    current_pos++;
                }
                char* value = my_strndup(&current_file_content[start], current_pos - start);
                return create_token(TOKEN_ID_FUNC, value);
            }
        }

        if (isalpha(current_char)) { /* Palavra reservada */
            int start = current_pos;
            while (isalnum(current_file_content[current_pos])) {
                current_pos++;
            }
            char* value = my_strndup(&current_file_content[start], current_pos - start);
            int keyword_token = is_keyword(value);
            if (keyword_token) {
                return create_token(keyword_token, value);
            }
            free(value); /* Libera o valor se não for uma palavra-chave */
            return create_token(TOKEN_ERRO, my_strdup("Identificador inválido (deve começar com '!' ou '__')"));
        }

        if (isdigit(current_char)) { /* Número */
            int start = current_pos;
            int is_decimal = 0;
            while (isdigit(current_file_content[current_pos])) {
                current_pos++;
            }
            if (current_file_content[current_pos] == '.') {
                is_decimal = 1;
                current_pos++;
                while (isdigit(current_file_content[current_pos])) {
                    current_pos++;
                }
            }
            char* value = my_strndup(&current_file_content[start], current_pos - start);
            return create_token(is_decimal ? TOKEN_LITERAL_DEC : TOKEN_LITERAL_INT, value);
        }

        if (current_char == '"') { /* String literal */
            current_pos++;
            int start = current_pos;
            while (current_file_content[current_pos] != '"' && current_file_content[current_pos] != '\0') {
                current_pos++;
            }
            if (current_file_content[current_pos] == '"') {
                char* value = my_strndup(&current_file_content[start], current_pos - start);
                current_pos++;
                return create_token(TOKEN_LITERAL_TEXTO, value);
            }
            return create_token(TOKEN_ERRO, my_strdup("String não terminada"));
        }

        /* Operadores e Separadores */
        char next_char = current_file_content[current_pos + 1];

        /* ver se tem mais de 2 caracteres */
        if(current_char == '<' || current_char == '>' || current_char == '=' || current_char == '&' || current_char == '|' || current_char == '+' || current_char == '-' ) {
          int i = 1;      
          char *str = (char*)Malloc(3 * sizeof(char));
          str[0] = current_char;
          str[1] = '\0';
          

          do {
            i++;
              if(i <= 2) {
                str[i - 1] = current_file_content[current_pos + i];
                str[i] = '\0';
              }else {
                return create_token(TOKEN_ERRO, my_strdup("Operador inválido"));
              }
          } while (is_operator(str));

          Free(str, 3 * sizeof(char));
        }

            if (current_char == '<' && next_char == '>') { current_pos += 2; return create_token(TOKEN_OP_DIF, my_strdup("<>")); }
            else if (current_char == '=' && next_char == '=') { current_pos += 2; return create_token(TOKEN_OP_IGUAL, my_strdup("==")); }
            else if (current_char == '<' && next_char == '=') { current_pos += 2; return create_token(TOKEN_OP_MENOR_IGUAL, my_strdup("<=")); }
            else if (current_char == '>' && next_char == '=') { current_pos += 2; return create_token(TOKEN_OP_MAIOR_IGUAL, my_strdup(">=")); }
            else if (current_char == '&' && next_char == '&') { current_pos += 2; return create_token(TOKEN_OP_E, my_strdup("&&")); }
            else if (current_char == '|' && next_char == '|') { current_pos += 2; return create_token(TOKEN_OP_OU, my_strdup("||")); }
            else if (current_char == '+') { current_pos++; return create_token(TOKEN_OP_SOMA, my_strdup("+")); }
            else if (current_char == '-') { current_pos++; return create_token(TOKEN_OP_SUB, my_strdup("-")); }
            else if (current_char == '*') { current_pos++; return create_token(TOKEN_OP_MULT, my_strdup("*")); }
            else if (current_char == '/') { current_pos++; return create_token(TOKEN_OP_DIV, my_strdup("/")); }
            else if (current_char == '^') { current_pos++; return create_token(TOKEN_OP_EXP, my_strdup("^")); }
            else if (current_char == '=') { current_pos++; return create_token(TOKEN_OP_ATRIB, my_strdup("=")); }
            else if (current_char == '<') { current_pos++; return create_token(TOKEN_OP_MENOR, my_strdup("<")); }
            else if (current_char == '>') { current_pos++; return create_token(TOKEN_OP_MAIOR, my_strdup(">")); }
            else if (current_char == '(') { current_pos++; return create_token(TOKEN_LPAREN, my_strdup("(")); }
            else if (current_char == ')') { current_pos++; return create_token(TOKEN_RPAREN, my_strdup(")")); }
            else if (current_char == '{') { current_pos++; return create_token(TOKEN_LBRACE, my_strdup("{")); }
            else if (current_char == '}') { current_pos++; return create_token(TOKEN_RBRACE, my_strdup("}")); }
            else if (current_char == '[') { current_pos++; return create_token(TOKEN_LBRACKET, my_strdup("[")); }
            else if (current_char == ']') { current_pos++; return create_token(TOKEN_RBRACKET, my_strdup("]")); }
            else if (current_char == ',') { current_pos++; return create_token(TOKEN_VIRGULA, my_strdup(",")); }
            else if (current_char == ';') { current_pos++; return create_token(TOKEN_PONTO_VIRGULA, my_strdup(";")); }
             else {
              current_pos += 2;
              return create_token(TOKEN_ERRO, my_strdup("Operador inválido"));
            }

        /* Se chegou aqui, é um caractere inválido */
        current_pos++;
        char err_msg[50];
        sprintf(err_msg, "Caractere inesperado: %c", current_char);
        return create_token(TOKEN_ERRO, my_strdup(err_msg));
    }
    return create_token(TOKEN_EOF, NULL);
}

char* read_file_content(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Erro ao abrir o arquivo %s\n", filepath);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    const char* filepath = "programa.txt";
    if (argc > 1) {
        filepath = argv[1];
    }

    current_file_content = read_file_content(filepath);

    Token* token;
    while (1) {
        token = get_next_token();

        TokenType type = token->type;
        if (type == TOKEN_ERRO) {
            printf("Erro na linha %d: %s\n", token->line, token->value);
            break;
        }

        free_token(token);

        if (type == TOKEN_EOF || type == TOKEN_ERRO) {
            break;
        }
    }

    printf("\nAnálise léxica concluída.\n");
    printf("Valor máximo de memória utilizada: %ld bytes.\n", max_memory_used);

    free(current_file_content);

    return 0;
}

