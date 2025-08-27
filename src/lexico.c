#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"
#include "lexico.h"

/* --- Protótipos --- */
void* Malloc(size_t size);
void Free(void* ptr, size_t size);

char* my_strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char* new_s = (char*)Malloc(len);
    if (new_s == NULL) return NULL;
    memcpy(new_s, s, len);
    return new_s;
}

size_t my_strnlen(const char *s, size_t maxlen) {
    size_t len = 0;
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    return len;
}

char* my_strndup(const char* s, size_t n) {
    if (s == NULL) return NULL;
    size_t len = my_strnlen(s, n);
    char* new_s = (char*)Malloc(len + 1);
    if (new_s == NULL) return NULL;
    memcpy(new_s, s, len);
    new_s[len] = '\0';
    return new_s;
}

int is_special_char(char c) {
    return (c == '!' || c == '=' || c == '+' || c == '-'
            || c == '*' || c == '/' || c == '^' || c == '<'
            || c == '>' || c == '&' || c == '|');
}

/* --- Configuração de Memória --- */
#define MAX_MEMORY_KB 2048
long current_memory_used = 0;
long max_memory_used = 0;

static void error(const char* message) {
    printf("ERRO LEXICO na linha %d: %s", current_line, message);
    exit(1); /* Finaliza para evitar erros em cascata */
}

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

/* --- Verificação de Balanceamento --- */
#define STACK_SIZE 1024

typedef struct {
    char character;
    int line;
} StackItem;

StackItem balance_stack[STACK_SIZE];
int stack_top = -1;

void push(char c, int line) {
    if (stack_top < STACK_SIZE - 1) {
        stack_top++;
        balance_stack[stack_top].character = c;
        balance_stack[stack_top].line = line;
    } else {
        printf("ERRO na linha %d: Estouro da pilha de balanceamento.\n", line);
        exit(1);
    }
}

StackItem pop() {
    if (stack_top >= 0) {
        return balance_stack[stack_top--];
    }
    StackItem empty_item = {'\0', -1};
    return empty_item;
}

int is_empty() {
    return stack_top == -1;
}

int check_balance(const char* content) {
    int line = 1;
    int i = 0;
    char c;
    int in_string = 0;

    while ((c = content[i++]) != '\0') {
        if (c == '\n') {
            line++;
            continue;
        }

        if (c == '"') {
            if (in_string) {
                StackItem popped = pop();
                if (popped.character != '"') {
                    printf("ERRO: Aspas duplas abertas na linha %d não foram fechadas.\n", popped.line);
                    return 0;
                }
                in_string = 0;
            } else {
                push('"', line);
                in_string = 1;
            }
        } else if (!in_string) {
            if (c == '(' || c == '{' || c == '[') {
                push(c, line);
            } else if (c == ')') {
                StackItem popped = pop();
                if (popped.character != '(') {
                    printf("ERRO: ')' na linha %d não tem '(' correspondente. O '(' que abriu o escopo foi na linha %d\n", line, popped.line);
                    return 0;
                }
            } else if (c == '}') {
                StackItem popped = pop();
                if (popped.character != '{') {
                    printf("ERRO: '}' na linha %d não tem '{' correspondente. O '{' que abriu o escopo foi na linha %d\n", line, popped.line);
                    return 0;
                }
            } else if (c == ']') {
                StackItem popped = pop();
                if (popped.character != '[') {
                    printf("ERRO: ']' na linha %d não tem '[' correspondente. O '[' que abriu o escopo foi na linha %d\n", line, popped.line);
                    return 0;
                }
            }
        }
    }

    if (!is_empty()) {
        StackItem remaining = pop();
        printf("ERRO: Símbolo '%c' aberto na linha %d não foi fechado.\n", remaining.character, remaining.line);
        return 0;
    }

    return 1;
}

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
        case TOKEN_OP_INC: return "OP_INC";
        case TOKEN_OP_DEC: return "OP_DEC";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_VIRGULA: return "VIRGULA";
        case TOKEN_PONTO_VIRGULA: return "PONTO_VIRGULA";
        case TOKEN_PONTO: return "PONTO";
        default: return "DESCONHECIDO";
    }
}

char* current_file_content;
int current_pos = 0;
int current_line = 1;

static Token* ungot_token = NULL;

Token* create_token(TokenType type, char* value) {
    Token* token = (Token*)Malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->line = current_line;
    return token;
}

void free_token(Token* token) {
    if (token) {
        if (token->value) Free(token->value, strlen(token->value) + 1);
        Free(token, sizeof(Token));
    }
}

void unget_token(Token* token) {
    ungot_token = token;
}

Token* peek_token() {
    if (ungot_token == NULL) {
        ungot_token = get_next_token();
    }
    return ungot_token;
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

Token* get_next_token() {
    if (ungot_token != NULL) {
        Token* token = ungot_token;
        ungot_token = NULL;
        return token;
    }
    while (current_file_content[current_pos] != '\0') {
        char current_char = current_file_content[current_pos];

        if (isspace(current_char)) { /* Removed || current_char == '\\' */
            if (current_char == '\n') current_line++;
            current_pos++;
            continue;
        }

        

        if (current_char == '+' && current_file_content[current_pos + 1] == '+' && current_file_content[current_pos + 2] == '+') {
            current_pos += 3;
            error("Operador triplo inválido: +++");
        }
        if (current_char == '-' && current_file_content[current_pos + 1] == '-' && current_file_content[current_pos + 2] == '-') {
            current_pos += 3;
            error("Operador triplo inválido: ---");
        }
        if (current_char == '<' && current_file_content[current_pos + 1] == '>' && current_file_content[current_pos + 2] == '<') {
            current_pos += 3;
            error("Operador duplicado inválido: <><>");
        }

        if (current_char == '_') { /* Função */
            if (current_file_content[current_pos + 1] == '_') {
                int start = current_pos;
                current_pos += 2; /* Pula o '__' */

                if (!isalnum(current_file_content[current_pos])) {
                  error("Nome de função inválido: '__' deve ser seguido por um caractere alfanumérico.");
                }

                while (isalnum(current_file_content[current_pos])) {
                    current_pos++;
                }
                char* value = my_strndup(&current_file_content[start], current_pos - start);
                Token* token = create_token(TOKEN_ID_FUNC, value);
                return token;
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
                Token* token = create_token(keyword_token, value);
                return token;
            }
            Free(value, strlen(value) + 1);
            error("Identificador inválido (deve começar com '!' ou '__')");
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
            Token* token = create_token(is_decimal ? TOKEN_LITERAL_DEC : TOKEN_LITERAL_INT, value);
            return token;
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
                Token* token = create_token(TOKEN_LITERAL_TEXTO, value);
                return token;
            }
            error("string não terminada");
        }

        /* Operadores, Separadores e Variáveis */
        switch (current_char) {
            case '!':
                if (current_file_content[current_pos + 1] == '=') {
                    current_pos += 2;
                
                    error("Operador inválido: !=");
                }
                int start = current_pos;
                current_pos++;
                if (islower(current_file_content[current_pos])) {
                    current_pos++;
                    while (isalnum(current_file_content[current_pos])) {
                        current_pos++;
                    }
                    char* value = my_strndup(&current_file_content[start], current_pos - start);
                    Token* token = create_token(TOKEN_ID_VAR, value);
                    return token;
                }
                error("Nome de variável inválido");
            case '+':
                if (current_file_content[current_pos + 1] == '+') {
                    current_pos += 2;
                    Token* token = create_token(TOKEN_OP_INC, my_strdup("++"));
                    return token;
                }
                current_pos++;
                Token* token_soma = create_token(TOKEN_OP_SOMA, my_strdup("+"));
                return token_soma;
            case '-':
                if (current_file_content[current_pos + 1] == '-') {
                    current_pos += 2;
                    Token* token = create_token(TOKEN_OP_DEC, my_strdup("--"));
                    return token;
                }
                current_pos++;
                Token* token_sub = create_token(TOKEN_OP_SUB, my_strdup("-"));
                return token_sub;
            case '=':
                if (current_file_content[current_pos + 1] == '=') {
                    current_pos += 2;
                    Token* token = create_token(TOKEN_OP_IGUAL, my_strdup("=="));
                    return token;
                }
                 if (current_file_content[current_pos + 1] == '<' || current_file_content[current_pos + 1] == '>') {
                    current_pos += 2;
                    error("Operador invertido inválido");
                }
                current_pos++;
                Token* token_atrib = create_token(TOKEN_OP_ATRIB, my_strdup("="));
                return token_atrib;
            case '<':
                if (current_file_content[current_pos + 1] == '=') {
                    current_pos += 2;
                    Token* token = create_token(TOKEN_OP_MENOR_IGUAL, my_strdup("<="));
                    return token;
                }
                if (current_file_content[current_pos + 1] == '>') {
                    current_pos += 2;
                    Token* token = create_token(TOKEN_OP_DIF, my_strdup("<>"));
                    return token;
                }
                 if (current_file_content[current_pos + 1] == '<') {
                    current_pos += 2;
                    error("Operador duplicado inválido: <<");
                }
                current_pos++;
                Token* token_menor = create_token(TOKEN_OP_MENOR, my_strdup("<"));
                return token_menor;
            case '>':
                if (current_file_content[current_pos + 1] == '=') {
                    current_pos += 2;
                    Token* token = create_token(TOKEN_OP_MAIOR_IGUAL, my_strdup(">="));
                    return token;
                }
                if (current_file_content[current_pos + 1] == '>' || current_file_content[current_pos + 1] == '<') {
                    current_pos += 2;
                    error("Operador duplicado/invertido inválido");
                }
                current_pos++;
                Token* token_maior = create_token(TOKEN_OP_MAIOR, my_strdup(">"));
                return token_maior;
            case '&':
                if (current_file_content[current_pos + 1] == '&') {
                    if (!isspace(current_file_content[current_pos - 1]) || !isspace(current_file_content[current_pos + 2])) {
                        error("Operador lógico '&&' deve ser cercado por espaços.");
                    }
                    current_pos += 2;
                    Token* token = create_token(TOKEN_OP_E, my_strdup("&&"));
                    return token;
                }
                break;
            case '|':
                if (current_file_content[current_pos + 1] == '|') {
                    if (!isspace(current_file_content[current_pos - 1]) || !isspace(current_file_content[current_pos + 2])) {
                        error("Operador lógico '||' deve ser cercado por espaços.");
                    }
                    current_pos += 2;
                    Token* token = create_token(TOKEN_OP_OU, my_strdup("||"));
                    return token;
                }
                break;
            case '*': current_pos++; Token* token_mult = create_token(TOKEN_OP_MULT, my_strdup("*")); return token_mult;
            case '/': current_pos++; Token* token_div = create_token(TOKEN_OP_DIV, my_strdup("/")); return token_div;
            case '^': current_pos++; Token* token_exp = create_token(TOKEN_OP_EXP, my_strdup("^")); return token_exp;
            case '(': current_pos++; Token* token_lparen = create_token(TOKEN_LPAREN, my_strdup("(")); return token_lparen;
            case ')': current_pos++; Token* token_rparen = create_token(TOKEN_RPAREN, my_strdup(")")); return token_rparen;
            case '{': current_pos++; Token* token_lbrace = create_token(TOKEN_LBRACE, my_strdup("{")); return token_lbrace;
            case '}': current_pos++; Token* token_rbrace = create_token(TOKEN_RBRACE, my_strdup("}")); return token_rbrace;
            case '[': current_pos++; Token* token_lbracket = create_token(TOKEN_LBRACKET, my_strdup("[")); return token_lbracket;
            case ']': current_pos++; Token* token_rbracket = create_token(TOKEN_RBRACKET, my_strdup("]")); return token_rbracket;
            case ',': current_pos++; Token* token_virgula = create_token(TOKEN_VIRGULA, my_strdup(",")); return token_virgula;
            case ';': current_pos++; Token* token_ponto_virgula = create_token(TOKEN_PONTO_VIRGULA, my_strdup(";")); return token_ponto_virgula;
            case '.': current_pos++; Token* token_ponto = create_token(TOKEN_PONTO, my_strdup(".")); return token_ponto;
        }

        /* Se chegou aqui, é um caractere inválido */
        char err_msg[100];
        sprintf(err_msg, "Caractere ou sequência inesperada começando com: %c", current_char);
        current_pos++;
        error(err_msg);
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
    char* buffer = (char*)Malloc(length + 1);
    if (buffer == NULL) {
        printf("Erro ao alocar memória para o conteúdo do arquivo.\n");
        fclose(file);
        exit(1);
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

void init_lexico(const char* filepath) {
    current_file_content = read_file_content(filepath);
    if (!check_balance(current_file_content)) {
        exit(1);
    }
    printf("Verificação de balanceamento concluída com sucesso.\n\n");
}
