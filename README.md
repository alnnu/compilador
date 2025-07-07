# Compilador Simples em C

Este projeto é um compilador simples, escrito em C, para uma linguagem de programação educacional fictícia. O compilador realiza a análise léxica e sintática de um código-fonte fornecido em um arquivo de texto e verifica se ele está de acordo com a gramática da linguagem.

## Estrutura do Projeto

```
/
├── build/
│   └── main           # Arquivo executável (após compilação)
├── src/
│   └── main.c         # Código-fonte principal do compilador
├── programa.txt       # Exemplo de código na linguagem do compilador
├── programa2.txt      # Outro exemplo de código
└── README.md          # Este arquivo
```

## Como Compilar e Executar

### Pré-requisitos
- Um compilador C, como o `gcc`.

### Compilação
Para compilar o projeto, navegue até o diretório raiz e execute o seguinte comando. Ele irá compilar o `src/main.c` e criar um executável chamado `main` dentro da pasta `build`.

```bash
gcc src/main.c -o build/main -lm
```

### Execução
Para executar o compilador, utilize o executável gerado, passando como argumento o caminho para o arquivo de código que você deseja analisar.

```bash
./build/main programa.txt
```
Se nenhum arquivo for especificado, ele tentará ler o `programa2.txt` por padrão.

## Estrutura do Código (`src/main.c`)

O código-fonte é dividido em quatro seções principais:

### 1. Gerenciamento de Memória

Para evitar o uso excessivo de memória, foram criadas funções customizadas que encapsulam `malloc` e `free`.

- `void* Malloc(size_t size)`: Aloca memória e verifica se o limite (`MAX_MEMORY_KB`) foi excedido. Também monitora o pico de uso de memória.
- `void Free(void* ptr, size_t size)`: Libera a memória alocada e decrementa o contador de memória em uso.

### 2. Analisador Léxico (Lexer)

Esta seção é responsável por ler o código-fonte e dividi-lo em pequenas unidades chamadas **tokens**.

- **`TokenType` (enum)**: Define todos os tipos de tokens que a linguagem reconhece, como palavras-chave (`TOKEN_PRINCIPAL`, `TOKEN_SE`), operadores (`TOKEN_OP_SOMA`), identificadores (`TOKEN_ID_VAR`), etc.
- **`Token` (struct)**: Representa um token individual, armazenando seu tipo, o valor (o texto do token, ex: "+") e a linha onde foi encontrado.
- **`get_next_token()`**: É a função central do lexer. A cada chamada, ela lê o código a partir da posição atual e retorna o próximo token válido. Ela ignora espaços em branco e identifica os diferentes componentes da linguagem (variáveis, funções, números, operadores).

### 3. Analisador Sintático (Parser)

O parser recebe a sequência de tokens do lexer e verifica se eles formam uma estrutura gramaticalmente válida. Ele constrói a lógica do programa, validando declarações, expressões e comandos.

- **`advance()`**: Avança para o próximo token.
- **`consume(TokenType type, ...)`**: Verifica se o token atual é do tipo esperado. Se não for, lança um erro sintático e encerra o programa.
- **Funções da Gramática (`program`, `block`, `statement`, `expression`)**: Cada função corresponde a uma regra da gramática da linguagem. Por exemplo, a função `statement()` sabe como analisar uma declaração de variável, um comando `se`, um laço `para`, etc. Elas se chamam recursivamente para analisar a estrutura aninhada do código.

### 4. Função `main`

É o ponto de entrada do programa. Suas responsabilidades são:
1.  Identificar o arquivo de código-fonte a ser analisado (seja pelo argumento da linha de comando ou o padrão).
2.  Ler todo o conteúdo do arquivo para a memória.
3.  Inicializar o processo de análise chamando a função `program()`.
4.  Imprimir uma mensagem de sucesso se a análise for concluída sem erros.
5.  Liberar toda a memória alocada ao final da execução.

## A Linguagem de Programação

O compilador foi projetado para uma linguagem com as seguintes características:

- **Variáveis**: Devem começar com `!` seguido por uma letra minúscula. Ex: `!minhaVariavel`.
- **Funções**: Devem começar com `__` (dois underscores). Ex: `__minhaFuncao`.
- **Tipos de Dados**: `inteiro`, `decimal`, `texto`.
- **Estrutura Principal**: Todo programa deve ter um bloco `principal() { ... }`.
- **Comandos**:
    - `leia(!variavel);`: Lê um valor do usuário.
    - `escreva("Olá", !variavel);`: Imprime valores na tela.
    - `se (condição) { ... } senao { ... }`: Estrutura condicional.
    - `para (inicialização; condição; incremento) { ... }`: Laço de repetição.
    - `retorno expressao;`: Retorna um valor de uma função.
- **Comentários**: A especificação atual não define sintaxe para comentários.

## Funções de Bibliotecas Padrão do C Utilizadas

Como solicitado, aqui está uma explicação das principais funções de bibliotecas do C usadas no projeto:

### `<ctype.h>`
Esta biblioteca contém funções para classificar e converter caracteres.

- **`isspace(char c)`**: Verifica se o caractere `c` é um espaço em branco (espaço, tab, nova linha `
`, etc.). Usada no lexer para pular partes irrelevantes do código.
- **`isalnum(char c)`**: Verifica se `c` é um caractere alfanumérico (letra de 'a' a 'z', 'A' a 'Z' ou dígito de '0' a '9'). Usada para identificar nomes de variáveis e funções.
- **`isalpha(char c)`**: Verifica se `c` é uma letra do alfabeto.
- **`islower(char c)`**: Verifica se `c` é uma letra minúscula. Usada para validar a regra de que nomes de variáveis devem começar com `!` seguido de uma letra minúscula.
- **`isdigit(char c)`**: Verifica se `c` é um dígito numérico ('0' a '9'). Usada para identificar literais numéricos (inteiros e decimais).

### `<string.h>`
Funções para manipulação de strings (sequências de caracteres).

- **`strcmp(const char* s1, const char* s2)`**: Compara duas strings, `s1` e `s2`. Retorna `0` se forem iguais. Usada para verificar se um identificador é uma palavra-chave (como "se", "para", "inteiro").
- **`strdup(const char* s)`**: Duplica uma string. Ela aloca memória para uma nova string e copia o conteúdo da original para ela. É útil para armazenar o valor de um token sem se preocupar se o buffer original será modificado.
- **`strndup(const char* s, size_t n)`**: Similar a `strdup`, mas copia no máximo `n` caracteres. Usada no lexer para extrair exatamente o texto de um token do buffer do arquivo.

### `<stdio.h>`
Funções de entrada e saída padrão (Standard Input/Output).

- **`printf(...)`**: Imprime texto formatado na tela. Usada para exibir mensagens de erro, sucesso e os resultados do comando `escreva`.
- **`sprintf(char* buffer, ...)`**: Funciona como `printf`, mas em vez de imprimir na tela, armazena o texto formatado em uma variável (um `buffer` de caracteres).
- **`fopen(const char* path, const char* mode)`**: Abre um arquivo. Usada para abrir o código-fonte a ser compilado.
- **`fseek(...)`**: Move o ponteiro de posição dentro de um arquivo. Usada para ir até o final do arquivo para descobrir seu tamanho.
- **`ftell(FILE* stream)`**: Retorna a posição atual do ponteiro no arquivo. Usada em conjunto com `fseek` para obter o tamanho total do arquivo.
- **`fread(...)`**: Lê dados de um arquivo para uma variável.
- **`fclose(FILE* stream)`**: Fecha um arquivo aberto, liberando os recursos do sistema.

### `<stdlib.h>`
Funções de utilidade geral.

- **`malloc(size_t size)`**: Aloca um bloco de memória de `size` bytes. É a base para a função `Malloc` customizada.
- **`free(void* ptr)`**: Libera um bloco de memória previamente alocado. É a base para a função `Free` customizada.
- **`exit(int status)`**: Termina a execução do programa imediatamente. Usada para parar o compilador quando um erro fatal (léxico, sintático ou semântico) é encontrado.

### `<math.h>`
Incluída no código, mas nenhuma de suas funções (como `pow`, `sqrt`, etc.) parece ser utilizada na versão atual do compilador. A operação de exponenciação (`^`) é reconhecida pelo lexer, mas a sua implementação matemática não foi feita no parser.

---

## Análise Detalhada das Funções do Compilador

Para entender como o compilador funciona, é fundamental separar as funções pelas fases da compilação. Neste código, temos principalmente funções para a **Análise Léxica** e para a **Análise Sintática**. A **Análise Semântica** (que verifica o *significado* e a lógica do código) é feita de forma simples e misturada com as funções da análise sintática.

### 1. Funções de Análise Léxica (Lexer)
O objetivo do analisador léxico é converter a sequência de caracteres do código-fonte em uma sequência de **tokens**.

| Função | Explicação |
| :--- | :--- |
| `Token* get_next_token()` | É o coração do analisador léxico. A cada chamada, ele avança pelo `current_file_content` e identifica o próximo token. Ele é responsável por ignorar espaços em branco, identificar palavras-chave, identificadores, literais, operadores e retornar um `TOKEN_ERRO` se encontrar algo inválido. |
| `int is_keyword(const char* str)` | Função auxiliar que verifica se uma string corresponde a uma das palavras-chave da linguagem. |
| `Token* create_token(...)` | Aloca memória e cria uma nova estrutura `Token`. |
| `void free_token(Token* token)` | Libera a memória alocada para um token. |

### 2. Funções de Análise Sintática (Parser)
O analisador sintático recebe os tokens e verifica se eles formam uma estrutura gramaticalmente correta.

| Função | Explicação |
| :--- | :--- |
| `void program()` | Função de mais alto nível que analisa a estrutura geral do arquivo (declarações de função e um bloco `principal`). |
| `void function_declaration()` | Analisa a sintaxe de uma declaração de função: `funcao __nome() { ... }`. |
| `void block()` | Analisa um bloco de código delimitado por chaves `{ ... }`. |
| `void statement()` | Analisa e valida os diferentes tipos de comandos da linguagem (declarações, atribuições, `se`, `para`, `leia`, `escreva`). |
| `void expression()` | Analisa uma expressão, como `!a + 10 * !b`. |
| `void primary_expression()` | Analisa os componentes básicos de uma expressão: um número, uma variável, uma chamada de função ou outra expressão entre parênteses. |
| `void advance()` | Utilitário que consome o token atual e avança para o próximo. |
| `void consume(...)` | Valida se o token atual é do tipo esperado, lançando um erro sintático caso contrário. |

### 3. Funções com Papel Misto (Sintático, Semântico e Execução)
Neste compilador, não há uma separação estrita entre as fases. Algumas funções do parser realizam também verificações semânticas ou até mesmo executam o código (característica de um **interpretador**).

| Função | Análise da Mistura de Papéis |
| :--- | :--- |
| `void program()` | **Sintático**: Garante a estrutura geral do programa. <br> **Semântico**: Verifica se existe um bloco `principal`, emitindo um erro semântico se não houver. |
| `void function_declaration()` | **Sintático**: Analisa a estrutura da declaração. <br> **Semântico**: Realiza uma verificação de tipos simples ao validar os tipos de dados dos parâmetros. |
| `void statement()` | **Sintático**: Valida a estrutura dos comandos. <br> **Execução/Interpretação**: No caso do `TOKEN_ESCREVA`, a função **executa** a ação de escrever na tela com `printf()`, agindo como um interpretador. |
