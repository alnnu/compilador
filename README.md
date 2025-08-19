# Compilador

Este projeto é um compilador simples para uma linguagem de programação customizada, desenvolvido em C. Atualmente, o foco principal da implementação está na **análise léxica** e na **verificação de balanceamento de símbolos**, que consistem em ler o código-fonte e convertê-lo em uma sequência de tokens válidos, além de garantir a correta abertura e fechamento de parênteses, chaves e colchetes.

## Estrutura do Projeto

```
/
├── programa.txt                 # Código de exemplo 1 para o compilador
├── programa2.txt                # Código de exemplo 2 para o compilador
├── programa3.txt                # Código de exemplo 3 para o compilador (para testes de erro)
├── build/                         # Diretório para os arquivos compilados
│   └── main                     # Executável do compilador
└── src/
    └── main.c                   # Código-fonte principal do compilador
```

- **`programa.txt` / `programa2.txt` / `programa3.txt`**: Arquivos de texto contendo código-fonte na linguagem customizada para serem processados pelo compilador. `programa3.txt` é usado para testar regras de erro específicas.
- **`build/`**: Contém o executável do projeto após a compilação.
- **`src/main.c`**: O coração do compilador, onde toda a lógica de análise léxica, verificação de balanceamento e gerenciamento de memória está implementada.

---

## Funcionalidades Implementadas (`src/main.c`)

O arquivo `main.c` é responsável por ler um arquivo de código-fonte, realizar a verificação de balanceamento de símbolos e, em seguida, dividi-lo em "tokens" (análise léxica).

### Bibliotecas Padrão Utilizadas

- **`stdio.h`**: Fornece funções para operações de entrada e saída padrão, como leitura e escrita de arquivos (`fopen`, `fclose`, `fread`, `printf`).
- **`stdlib.h`**: Contém funções para alocação de memória (`malloc`, `free`) e controle de programa (`exit`).
- **`string.h`**: Oferece funções para manipulação de strings, como cálculo de comprimento (`strlen`), cópia (`memcpy`) e comparação (`strcmp`).
- **`ctype.h`**: Inclui funções para classificação de caracteres, essenciais para a análise léxica:
    *   **`isspace(int c)`**: Verifica se o caractere `c` é um caractere de espaço em branco (espaço, tabulação, nova linha, etc.).
    *   **`isalnum(int c)`**: Verifica se o caractere `c` é alfanumérico (letra ou dígito).
    *   **`isalpha(int c)`**: Verifica se o caractere `c` é uma letra (maiúscula ou minúscula).
    *   **`isdigit(int c)`**: Verifica se o caractere `c` é um dígito decimal (0-9).
    *   **`islower(int c)`**: Verifica se o caractere `c` é uma letra minúscula.

### Gerenciamento de Memória Customizado

Para um controle mais granular e para evitar o consumo excessivo de recursos, foi implementado um sistema simples de gerenciamento de memória.

- **`Malloc(size_t size)`**: Uma função de alocação de memória que envolve a `malloc` padrão. Ela monitora o uso de memória, verifica se a alocação ultrapassará um limite definido (`2MB`), e emite alertas se o uso de memória estiver próximo do limite. Em caso de falha na alocação ou estouro do limite, o programa é encerrado.
- **`Free(void* ptr, size_t size)`**: Uma função de liberação de memória que envolve a `free` padrão. Ela atualiza o contador de memória utilizada, garantindo que o rastreamento do uso de memória seja preciso.

### Funções Utilitárias de String

- **`my_strdup(const char* s)`**: Duplica uma string alocando nova memória para ela. É uma versão segura de `strdup` que utiliza a função `Malloc` customizada.
- **`my_strnlen(const char *s, size_t maxlen)`**: Calcula o comprimento de uma string, limitado por um tamanho máximo. Útil para evitar leitura além dos limites de um buffer.
- **`my_strndup(const char* s, size_t n)`**: Duplica uma porção de uma string, copiando no máximo `n` caracteres. Também utiliza `Malloc` para a alocação.

### Verificação de Balanceamento de Símbolos

Este módulo garante que todos os parênteses `()`, chaves `{}` e colchetes `[]` no código-fonte estejam corretamente abertos e fechados, incluindo aspas duplas `""`.

- **`balance_stack`**: Um array de caracteres que atua como uma pilha para armazenar os símbolos de abertura encontrados.
- **`stack_top`**: Um índice que aponta para o topo da `balance_stack`.
- **`balance_error_line`**: Armazena o número da linha onde um erro de balanceamento foi detectado.
- **`push(char c)`**: Adiciona um caractere (símbolo de abertura) ao topo da pilha. Verifica por estouro de pilha.
- **`pop()`**: Remove e retorna o caractere do topo da pilha. Retorna `\0` se a pilha estiver vazia.
- **`is_empty()`**: Verifica se a pilha de balanceamento está vazia.
- **`check_balance(const char* content)`**: A função principal de verificação de balanceamento. Percorre o conteúdo do arquivo, usando `push` e `pop` para validar o emparelhamento de símbolos. Retorna `1` se balanceado, `0` caso contrário, e define `balance_error_line` em caso de erro.

### Analisador Léxico (Lexer)

Esta é a parte central do programa que transforma o texto do código-fonte em uma sequência de tokens que o compilador pode entender.

- **`TokenType`**: Um `enum` que define todos os tipos de tokens possíveis reconhecidos pelo lexer, incluindo palavras-chave (ex: `TOKEN_PRINCIPAL`, `TOKEN_SE`), identificadores (variáveis `TOKEN_ID_VAR`, funções `TOKEN_ID_FUNC`), literais (inteiros, decimais, texto), operadores (aritméticos, relacionais, lógicos, incremento/decremento) e separadores.
- **`token_type_to_string(TokenType type)`**: Uma função auxiliar que retorna a representação em string de um `TokenType`, útil para depuração e exibição.
- **`Token`**: Uma `struct` que representa um token individual, armazenando seu `TokenType`, o `value` (o texto real do token, como o nome de uma variável ou um literal) e a `line` onde foi encontrado no código-fonte.
- **`create_token(TokenType type, char* value)`**: Aloca memória e inicializa uma nova estrutura `Token`.
- **`free_token(Token* token)`**: Libera a memória alocada para um `Token` e seu valor.
- **`is_keyword(const char* str)`**: Verifica se uma dada string corresponde a uma palavra-chave reservada da linguagem e retorna o `TokenType` correspondente, ou `0` se não for uma palavra-chave.
- **`is_special_char(char c)`**: Uma função auxiliar que verifica se um caractere é considerado um 'caractere especial' para a regra de detecção de sequências inválidas (ex: `!`, `=`, `+`, `-`, `*`, `/`, `^`, `<`, `>`, `&`, `|`).
- **`get_next_token()`**: A função principal do lexer. Ela processa o código-fonte caractere por caractere e identifica o próximo token válido. É responsável por:
  - Ignorar espaços em branco e contar linhas.
  - **Detectar sequências inválidas de 3 ou mais caracteres especiais** (ex: `===`, `+++`, `---`, `!!!`, `>>=`, `<<=`, `!!!`, `>>>`, `<<<`, `&&&`, `|||`). Se encontrada, gera um `TOKEN_ERRO`.
  - Identificar palavras-chave (ex: `principal`, `se`, `funcao`).
  - Identificar variáveis, que devem começar com `!` seguido por letras minúsculas e alfanuméricos (ex: `!meu_nome`).
  - Identificar funções, que devem começar com `__` seguido por alfanuméricos (ex: `__minha_funcao`).
  - Reconhecer números (inteiros e decimais) e literais de texto (strings entre aspas duplas).
  - Identificar operadores aritméticos (`+`, `-`, `*`, `/`, `^`), de atribuição (`=`), relacionais (`==`, `<>`, `<`, `<=`, `>`, `>=`), lógicos (`&&`, `||`), e de incremento/decremento (`++`, `--`).
  - Identificar separadores (`(`, `)`, `{`, `}`, `[`, `]`, `,`, `;`, `.`).
  - Reportar erros léxicos para caracteres ou sequências inesperadas, ou strings não terminadas.

### Leitura de Arquivo

- **`read_file_content(const char* filepath)`**: Lê todo o conteúdo de um arquivo especificado para a memória. Em caso de erro na abertura ou alocação, o programa é encerrado.

### Função Principal (`main`)

A função `main` orquestra todo o processo de análise do compilador.

1.  Define o caminho do arquivo de entrada (padrão para `programa.txt`, mas pode ser especificado via argumento de linha de comando).
2.  Lê todo o conteúdo do arquivo para a memória usando `read_file_content`.
3.  Executa a `check_balance` para verificar o balanceamento de símbolos. Se houver um erro, exibe uma mensagem e encerra.
4.  Entra em um loop, chamando `get_next_token()` repetidamente para obter e exibir o próximo token.
5.  O loop continua até que um token de erro (`TOKEN_ERRO`) ou o fim do arquivo (`TOKEN_EOF`) seja encontrado.
6.  Se um `TOKEN_ERRO` for detectado, uma mensagem detalhada é exibida com a linha do erro e o programa para.
7.  Ao final da análise léxica, exibe o valor máximo de memória utilizada durante a execução.
8.  Libera a memória alocada para o conteúdo do arquivo.

---

## Como Compilar e Executar

Você pode compilar o projeto usando um compilador C padrão como o GCC.

**1. Compilar o código:**

```bash
# Cria o diretório de build se ele não existir
mkdir -p build

# Compila o código-fonte e gera o executável em build/main
gcc src/main.c -o build/main
```

**2. Executar o compilador:**

Você pode executar o programa passando o caminho do arquivo de código como argumento.

```bash
# Executa a análise léxica no arquivo programa.txt (padrão)
./build/main programa.txt

# Executa a análise no arquivo programa2.txt
./build/main programa2.txt

# Executa a análise no arquivo programa3.txt (para testar erros de 3+ caracteres especiais)
./build/main programa3.txt
```

Se nenhum arquivo for especificado, o `programa.txt` será usado por padrão.
