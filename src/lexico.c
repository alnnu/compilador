#include <stdio.h>
#include <string.h>

char reserveWords[11][256] = { 
  "pricipal",
  "funcao",
  "retorno",
  "leia",
  "escreva",
  "se",
  "senao",
  "para",
  "inteiro",
  "texto",
  "decimal"
};

void verifySymbols(char line[256], int lineNumber) {
  char * pch;
  pch = strtok (line," ");
  while (pch != NULL)
  {
    printf ("%s\n",pch);
    pch = strtok (NULL, " ");
  }
}


