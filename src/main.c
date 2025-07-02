#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// --- Configuração de Memória ---
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

// --- Analisador Léxico (Lexer) ---

typedef enum {
    TOKEN_EOF, TOKEN_ERRO,
    // Palavras Reservadas
    TOKEN_PRINCIPAL, TOKEN_FUNCAO, TOKEN_RETORNO, TOKEN_LEIA, TOKEN_ESCREVA,
    TOKEN_SE, TOKEN_SENAO, TOKEN_PARA, TOKEN_INTEIRO, TOKEN_TEXTO, TOKEN_DECIMAL,
    // Identificadores
    TOKEN_ID_VAR, TOKEN_ID_FUNC,
    // Literais
    TOKEN_LITERAL_INT, TOKEN_LITERAL_DEC, TOKEN_LITERAL_TEXTO,
    // Operadores
    TOKEN_OP_SOMA, TOKEN_OP_SUB, TOKEN_OP_MULT, TOKEN_OP_DIV, TOKEN_OP_EXP,
    TOKEN_OP_ATRIB, TOKEN_OP_IGUAL, TOKEN_OP_DIF, TOKEN_OP_MENOR, TOKEN_OP_MENOR_IGUAL,
    TOKEN_OP_MAIOR, TOKEN_OP_MAIOR_IGUAL, TOKEN_OP_E, TOKEN_OP_OU,
    // Separadores
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_LBRACKET, TOKEN_RBRACKET, TOKEN_VIRGULA, TOKEN_PONTO_VIRGULA
} TokenType;

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
    token->value = value ? strdup(value) : NULL;
    token->line = current_line;
    if (value) free(value);
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

Token* get_next_token() {
    while (current_file_content[current_pos] != '\0') {
        char current_char = current_file_content[current_pos];

        if (isspace(current_char)) {
            if (current_char == '\n') current_line++;
            current_pos++;
            continue;
        }

        if (current_char == '!') { // Variável
            int start = current_pos;
            current_pos++;
            if (islower(current_file_content[current_pos])) {
                current_pos++;
                while (isalnum(current_file_content[current_pos])) {
                    current_pos++;
                }
                char* value = strndup(&current_file_content[start], current_pos - start);
                return create_token(TOKEN_ID_VAR, value);
            }
            return create_token(TOKEN_ERRO, strdup("Nome de variável inválido"));
        }

        if (current_char == '_') { // Função
            if (current_file_content[current_pos + 1] == '_') {
                int start = current_pos;
                current_pos += 2;
                while (isalnum(current_file_content[current_pos])) {
                    current_pos++;
                }
                char* value = strndup(&current_file_content[start], current_pos - start);
                return create_token(TOKEN_ID_FUNC, value);
            }
        }

        if (isalpha(current_char)) { // Palavra reservada ou ID (não deveria acontecer pela regra)
            int start = current_pos;
            while (isalnum(current_file_content[current_pos])) {
                current_pos++;
            }
            char* value = strndup(&current_file_content[start], current_pos - start);
            int keyword_token = is_keyword(value);
            if (keyword_token) {
                return create_token(keyword_token, value);
            }
            free(value);
            return create_token(TOKEN_ERRO, strdup("Identificador inválido (deve começar com '!' ou '__')"));
        }

        if (isdigit(current_char)) { // Número
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
            char* value = strndup(&current_file_content[start], current_pos - start);
            return create_token(is_decimal ? TOKEN_LITERAL_DEC : TOKEN_LITERAL_INT, value);
        }

        if (current_char == '"') { // String literal
            current_pos++;
            int start = current_pos;
            while (current_file_content[current_pos] != '"' && current_file_content[current_pos] != '\0') {
                current_pos++;
            }
            if (current_file_content[current_pos] == '"') {
                char* value = strndup(&current_file_content[start], current_pos - start);
                current_pos++;
                return create_token(TOKEN_LITERAL_TEXTO, value);
            }
            return create_token(TOKEN_ERRO, strdup("String não terminada"));
        }

        // Operadores e Separadores (Lógica Refatorada e Final)
        char next_char = current_file_content[current_pos + 1];

        // Prioriza operadores de 2 caracteres (válidos e inválidos)
        if (next_char != '\0') { // Garante que há um próximo caractere para checar
            if (current_char == '=' && next_char == '=') { current_pos += 2; return create_token(TOKEN_OP_IGUAL, strdup("==")); }
            if (current_char == '<' && next_char == '>') { current_pos += 2; return create_token(TOKEN_OP_DIF, strdup("<>")); }
            if (current_char == '<' && next_char == '=') { current_pos += 2; return create_token(TOKEN_OP_MENOR_IGUAL, strdup("<=")); }
            if (current_char == '>' && next_char == '=') { current_pos += 2; return create_token(TOKEN_OP_MAIOR_IGUAL, strdup(">=")); }
            if (current_char == '&' && next_char == '&') { current_pos += 2; return create_token(TOKEN_OP_E, strdup("&&")); }
            if (current_char == '|' && next_char == '|') { current_pos += 2; return create_token(TOKEN_OP_OU, strdup("||")); }

            // Checagem de operadores inválidos conhecidos
            if ((current_char == '=' && next_char == '>') || (current_char == '=' && next_char == '<') || (current_char == '!' && next_char == '=')) {
                current_pos += 2;
                char err_msg[50];
                sprintf(err_msg, "Operador inválido: %c%c", current_char, next_char);
                return create_token(TOKEN_ERRO, strdup(err_msg));
            }
        }

        // Se não for um operador de 2 caracteres, processa como um de 1 caractere
        if (current_char == '+') { current_pos++; return create_token(TOKEN_OP_SOMA, strdup("+")); }
        if (current_char == '-') { current_pos++; return create_token(TOKEN_OP_SUB, strdup("-")); }
        if (current_char == '*') { current_pos++; return create_token(TOKEN_OP_MULT, strdup("*")); }
        if (current_char == '/') { current_pos++; return create_token(TOKEN_OP_DIV, strdup("/")); }
        if (current_char == '^') { current_pos++; return create_token(TOKEN_OP_EXP, strdup("^")); }
        if (current_char == '=') { current_pos++; return create_token(TOKEN_OP_ATRIB, strdup("=")); }
        if (current_char == '<') { current_pos++; return create_token(TOKEN_OP_MENOR, strdup("<")); }
        if (current_char == '>') { current_pos++; return create_token(TOKEN_OP_MAIOR, strdup(">")); }
        if (current_char == '(') { current_pos++; return create_token(TOKEN_LPAREN, strdup("(")); }
        if (current_char == ')') { current_pos++; return create_token(TOKEN_RPAREN, strdup(")")); }
        if (current_char == '{') { current_pos++; return create_token(TOKEN_LBRACE, strdup("{")); }
        if (current_char == '}') { current_pos++; return create_token(TOKEN_RBRACE, strdup("}")); }
        if (current_char == '[') { current_pos++; return create_token(TOKEN_LBRACKET, strdup("[")); }
        if (current_char == ']') { current_pos++; return create_token(TOKEN_RBRACKET, strdup("]")); }
        if (current_char == ',') { current_pos++; return create_token(TOKEN_VIRGULA, strdup(",")); }
        if (current_char == ';') { current_pos++; return create_token(TOKEN_PONTO_VIRGULA, strdup(";")); }

        // Se chegou aqui, é um caractere inválido
        current_pos++;
        char err_msg[50];
        sprintf(err_msg, "Caractere inesperado: %c", current_char);
        return create_token(TOKEN_ERRO, strdup(err_msg));
    }
    return create_token(TOKEN_EOF, NULL);
}

// --- Parser e Analisador Semântico ---

Token* current_token;
Token* peek_token;

void advance() {
    free_token(current_token);
    current_token = peek_token;
    peek_token = get_next_token();
    if (current_token->type == TOKEN_ERRO) {
        printf("ERRO Léxico (Linha %d): %s\n", current_token->line, current_token->value);
        exit(1);
    }
}

void consume(TokenType type, const char* error_message) {
    if (current_token->type == type) {
        advance();
    } else {
        printf("ERRO Sintático (Linha %d): %s. Encontrado %s (%d).\n", current_token->line, error_message, current_token->value, current_token->type);
        exit(1);
    }
}

// Protótipos
void program();
void statement();

void expression();

void primary_expression() {
    if (current_token->type == TOKEN_ID_VAR || 
        current_token->type == TOKEN_LITERAL_INT || 
        current_token->type == TOKEN_LITERAL_DEC || 
        current_token->type == TOKEN_LITERAL_TEXTO) {
        advance();
    } else if (current_token->type == TOKEN_ID_FUNC) {
        // Function call
        advance();
        consume(TOKEN_LPAREN, "Esperado '(' para chamada de função");
        if (current_token->type != TOKEN_RPAREN) {
            do {
                expression(); // Parse arguments
                if (current_token->type == TOKEN_VIRGULA) {
                    advance();
                } else {
                    break;
                }
            } while(1);
        }
        consume(TOKEN_RPAREN, "Esperado ')' para fechar chamada de função");
    } else if (current_token->type == TOKEN_LPAREN) {
        advance();
        expression();
        consume(TOKEN_RPAREN, "Esperado ')' para fechar expressão em parênteses");
    } else {
        printf("ERRO Sintático (Linha %d): Expressão inesperada, token %s.\n", current_token->line, current_token->value);
        exit(1);
    }
}

void expression() {
    primary_expression();
    while (current_token->type >= TOKEN_OP_SOMA && current_token->type <= TOKEN_OP_OU) {
        advance(); // consume operator
        primary_expression();
    }
}

void block() {
    consume(TOKEN_LBRACE, "Esperado '{' para iniciar o bloco");
    while (current_token->type != TOKEN_RBRACE && current_token->type != TOKEN_EOF) {
        statement();
    }
    consume(TOKEN_RBRACE, "Esperado '}' para finalizar o bloco");
}

void statement() {
    switch (current_token->type) {
        case TOKEN_INTEIRO:
        case TOKEN_DECIMAL:
        case TOKEN_TEXTO:
            advance(); // Consome o tipo
            do {
                consume(TOKEN_ID_VAR, "Esperado nome da variável");
                if (current_token->type == TOKEN_LBRACKET) { // Array declaration for decimal
                    advance();
                    expression();
                    consume(TOKEN_RBRACKET, "Esperado ']' no final da declaração de array");
                }
                if (current_token->type == TOKEN_OP_ATRIB) {
                    advance();
                    expression();
                }
                if (current_token->type != TOKEN_VIRGULA) {
                    break;
                }
                advance(); // Consome a vírgula
            } while (current_token->type == TOKEN_ID_VAR);
            consume(TOKEN_PONTO_VIRGULA, "Esperado ';' no final da declaração");
            break;
        case TOKEN_RETORNO:
            advance(); // Consome 'retorno'
            expression();
            consume(TOKEN_PONTO_VIRGULA, "Esperado ';' após a expressão de retorno");
            break;

        case TOKEN_ID_VAR:
            advance();
            consume(TOKEN_OP_ATRIB, "Esperado '=' na atribuição");
            expression();
            consume(TOKEN_PONTO_VIRGULA, "Esperado ';' no final da atribuição");
            break;
        case TOKEN_ESCREVA:
            advance();
            consume(TOKEN_LPAREN, "Esperado '(' depois de 'escreva'");
            while (current_token->type != TOKEN_RPAREN && current_token->type != TOKEN_EOF) {
                if (current_token->type == TOKEN_LITERAL_TEXTO || current_token->type == TOKEN_ID_VAR) {
                    printf("%s ", current_token->value);
                }
                advance();
                if (current_token->type == TOKEN_VIRGULA) {
                    advance();
                }
            }
            printf("\n");
            consume(TOKEN_RPAREN, "Esperado ')' depois dos argumentos de 'escreva'");
            consume(TOKEN_PONTO_VIRGULA, "Esperado ';' no final do comando 'escreva'");
            break;
        case TOKEN_LEIA:
            advance();
            consume(TOKEN_LPAREN, "Esperado '(' depois de 'leia'");
            consume(TOKEN_ID_VAR, "Esperado variável em 'leia'");
            consume(TOKEN_RPAREN, "Esperado ')' depois dos argumentos de 'leia'");
            consume(TOKEN_PONTO_VIRGULA, "Esperado ';' no final do comando 'leia'");
            break;
        case TOKEN_SE:
            advance();
            consume(TOKEN_LPAREN, "Esperado '(' na condição do 'se'");
            expression();
            consume(TOKEN_RPAREN, "Esperado ')' na condição do 'se'");
            if (current_token->type == TOKEN_LBRACE) {
                block();
            } else {
                statement();
            }
            if (current_token->type == TOKEN_SENAO) {
                advance();
                if (current_token->type == TOKEN_LBRACE) {
                    block();
                } else {
                    statement();
                }
            }
            break;
        case TOKEN_PARA:
            advance();
            consume(TOKEN_LPAREN, "Esperado '(' no laço 'para'");
            expression();
            consume(TOKEN_PONTO_VIRGULA, "Esperado ';' após a inicialização do 'para'");
            expression();
            consume(TOKEN_PONTO_VIRGULA, "Esperado ';' após a condição do 'para'");
            expression();
            consume(TOKEN_RPAREN, "Esperado ')' no laço 'para'");
            block();
            break;
        default:
            printf("ERRO Sintático (Linha %d): Comando inesperado '%s'.\n", current_token->line, current_token->value);
            exit(1);
    }
}

void function_declaration() {
    consume(TOKEN_FUNCAO, "Esperado 'funcao'");
    consume(TOKEN_ID_FUNC, "Esperado nome da função");
    consume(TOKEN_LPAREN, "Esperado '(' após o nome da função");

    // Análise de Parâmetros
    if (current_token->type != TOKEN_RPAREN) { // Se não for uma função vazia
        do {
            // Consome o tipo do parâmetro (inteiro, decimal, texto)
            if (current_token->type == TOKEN_INTEIRO || current_token->type == TOKEN_DECIMAL || current_token->type == TOKEN_TEXTO) {
                advance();
            } else {
                printf("ERRO Sintático (Linha %d): Esperado tipo de dado para o parâmetro.\n", current_token->line);
                exit(1);
            }
            consume(TOKEN_ID_VAR, "Esperado nome da variável para o parâmetro");

            // Se o próximo token for uma vírgula, consome e continua o loop
            if (current_token->type == TOKEN_VIRGULA) {
                advance();
            } else {
                break; // Sai do loop se não houver mais vírgulas
            }
        } while (1);
    }

    consume(TOKEN_RPAREN, "Esperado ')' para fechar a declaração de parâmetros");
    block();
}

void program() {
    int has_principal = 0;
    while (current_token->type != TOKEN_EOF) {
        if (current_token->type == TOKEN_PRINCIPAL) {
            has_principal = 1;
            advance();
            consume(TOKEN_LPAREN, "Esperado '(' após 'principal'");
            consume(TOKEN_RPAREN, "Esperado ')' após 'principal()'");
            block();
        } else if (current_token->type == TOKEN_FUNCAO) {
            function_declaration();
        } else if (current_token->type != TOKEN_EOF) {
            statement();
        }
    }
    if (!has_principal) {
        printf("ERRO Semântico: Módulo Principal Inexistente.\n");
        exit(1);
    }
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

    peek_token = get_next_token();
    advance();
    
    program();

    printf("\nAnálise concluída com sucesso.\n");
    printf("Valor máximo de memória utilizada: %ld bytes.\n", max_memory_used);

    free(current_file_content);
    free_token(current_token);
    free_token(peek_token);

    return 0;
}
