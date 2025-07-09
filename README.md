# Compilador

Este projeto é um compilador simples para uma linguagem de programação customizada, desenvolvido em C. O foco principal do código fornecido é a **análise léxica**, que consiste em ler o código-fonte e convertê-lo em uma sequência de tokens.

## Estrutura do Projeto

```
/
├── CompiladorEspecificacao.pdf  # Documento com a especificação da linguagem
├── programa.txt                 # Código de exemplo 1 para o compilador
├── programa2.txt                # Código de exemplo 2 para o compilador
├── build/                         # Diretório para os arquivos compilados
│   └── main                     # Executável do compilador
└── src/
    └── main.c                   # Código-fonte principal do compilador
```

- **`CompiladorEspecificacao.pdf`**: A especificação formal da linguagem e dos requisitos do compilador.
- **`programa.txt` / `programa2.txt`**: Arquivos de texto contendo código-fonte na linguagem customizada para serem processados pelo compilador.
- **`build/`**: Contém o executável do projeto após a compilação.
- **`src/main.c`**: O coração do compilador, onde toda a lógica de análise léxica e execução está implementada.

---

## Funcionalidades do `src/main.c`

O arquivo `main.c` é responsável por ler um arquivo de código-fonte, dividi-lo em "tokens" e verificar a sintaxe léxica.

### Gerenciamento de Memória

Para evitar o consumo excessivo de recursos, foi implementado um sistema simples de controle de memória.

- **`Malloc(size_t size)`**: Uma função que envolve `malloc`. Ela verifica se a alocação de memória ultrapassará o limite de `2MB` definido. Caso o limite seja excedido ou a alocação falhe, o programa é encerrado. Um alerta é emitido se o uso de memória estiver entre 90% e 99%.
- **`Free(void* ptr, size_t size)`**: Envolve `free` e atualiza o contador de memória utilizada.

### Analisador Léxico (Lexer)

Esta é a parte central do programa, que transforma o texto do código-fonte em uma sequência de tokens que o compilador pode entender.

- **`TokenType`**: Um `enum` que define todos os tipos de tokens possíveis, como palavras-chave (`TOKEN_PRINCIPAL`, `TOKEN_SE`), identificadores (`TOKEN_ID_VAR`), literais (`TOKEN_LITERAL_INT`) e operadores (`TOKEN_OP_SOMA`).
- **`Token`**: Uma `struct` que representa um token individual, armazenando seu tipo, valor (o texto real, como o nome de uma variável) e a linha onde foi encontrado.
- **`get_next_token()`**: A função principal do lexer. Ela processa o código-fonte caractere por caractere e identifica o próximo token válido, ignorando espaços em branco. Ela é responsável por:
    - Identificar palavras-chave (ex: `principal`, `se`, `funcao`).
    - Identificar variáveis, que devem começar com `! ` (ex: `!meu_nome`).
    - Identificar funções, que devem começar com `__` (ex: `__minha_funcao`).
    - Reconhecer números (inteiros e decimais), texto (strings entre aspas duplas), operadores e separadores.
    - Reportar erros léxicos, como um caractere inesperado ou uma string que não foi fechada.

### Função Principal (`main`)

A função `main` orquestra todo o processo.

1.  Recebe o caminho do arquivo a ser analisado via argumento de linha de comando. Se nenhum for fornecido, utiliza `programa.txt` como padrão.
2.  Lê todo o conteúdo do arquivo para a memória.
3.  Entra em um loop, chamando `get_next_token()` repetidamente para obter o próximo token.
4.  O loop continua até que um token de erro (`TOKEN_ERRO`) ou o fim do arquivo (`TOKEN_EOF`) seja encontrado.
5.  Se um erro for detectado, uma mensagem é exibida com a linha do erro e o programa para.
6.  Ao final, exibe o pico de memória utilizado durante a execução.

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
# Executa a análise léxica no arquivo programa.txt
./build/main programa.txt

# Executa a análise no arquivo programa2.txt
./build/main programa2.txt
```

Se nenhum arquivo for especificado, o `programa.txt` será usado por padrão.
