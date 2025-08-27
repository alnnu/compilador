#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "lexico.h"
#include "sintatico.h"

int main(int argc, char *argv[]) {
    const char* filepath = "programa2.txt";
    if (argc > 1) {
        filepath = argv[1];
    }

    init_lexico(filepath);
    
    parse();

    /* Libera a memória do conteúdo do arquivo, que é uma variável global em lexico.c */
    free(current_file_content);

    return 0;
}