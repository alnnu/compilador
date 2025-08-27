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
    ├── lexico.c                 # Código-fonte do analisador léxico
    ├── lexico.h                 # Cabeçalho do analisador léxico
    ├── sintatico.c              # Código-fonte do analisador sintático
    ├── sintatico.h              # Cabeçalho do analisador sintático
    ├── token.h                  # Definições de tokens
    └── main.c                   # Código-fonte principal do compilador
```

- **`programa.txt` / `programa2.txt` / `programa3.txt`**: Arquivos de texto contendo código-fonte na linguagem customizada para serem processados pelo compilador. `programa3.txt` é usado para testar regras de erro específicas.
- **`build/`**: Contém o executável do projeto após a compilação.
- **`src/main.c`**: O coração do compilador, onde toda a lógica de análise léxica, verificação de balanceamento e gerenciamento de memória está implementada.

---

## Funcionalidades Implementadas

O compilador é dividido em três componentes principais: o analisador léxico, o analisador sintático e o programa principal que os orquestra.

### `src/lexico.c` - Analisador Léxico

Este componente é responsável por ler o código-fonte e convertê-lo em uma sequência de tokens.

- **`get_next_token()`**: A função principal do lexer. Ela processa o código-fonte caractere por caractere e identifica o próximo token válido.
- **Verificação de Balanceamento de Símbolos**: Garante que todos os parênteses `()`, chaves `{}` e colchetes `[]` no código-fonte estejam corretamente abertos e fechados, incluindo aspas duplas `""`.
- **Gerenciamento de Memória Customizado**: Para um controle mais granular e para evitar o consumo excessivo de recursos, foi implementado um sistema simples de gerenciamento de memória.

### `src/sintatico.c` - Analisador Sintático

Este componente é responsável por analisar a sequência de tokens gerada pelo analisador léxico e verificar se ela segue as regras gramaticais da linguagem.

- **`parse()`**: A função principal do analisador sintático. Ela consome os tokens do analisador léxico e constrói a árvore sintática.
- **Tabela de Símbolos**: Uma tabela de símbolos é usada para armazenar informações sobre as variáveis declaradas, como nome e tipo. Isso permite a verificação de declarações duplicadas e o uso de variáveis não declaradas.
- **Tratamento de Erros Sintáticos**: O analisador sintático é capaz de identificar e relatar erros de sintaxe, como a falta de um ponto e vírgula ou o uso de um token inesperado.

### `src/main.c` - Programa Principal

A função `main` orquestra todo o processo de compilação.

1.  Define o caminho do arquivo de entrada.
2.  Inicializa o analisador léxico com o conteúdo do arquivo.
3.  Chama o analisador sintático para iniciar o processo de parsing.
4.  Libera a memória alocada.

---

## Como Compilar e Executar

Você pode compilar o projeto usando um compilador C padrão como o GCC.

**1. Compilar o código:**

```bash
# Cria o diretório de build se ele não existir
mkdir -p build

# Compila o código-fonte e gera o executável em build/main
gcc -std=c90 -o ./build/main src/main.c src/lexico.c src/sintatico.c
```

**2. Executar o compilador:**

Você pode executar o programa passando o caminho do arquivo de código como argumento.

```bash
# Executa a análise no arquivo programa.txt
./build/main programa.txt

# Executa a análise no arquivo programa2.txt
./build/main programa2.txt

# Executa a análise no arquivo programa3.txt
./build/main programa3.txt
```

Se nenhum arquivo for especificado, o `programa2.txt` será usado por padrão.
