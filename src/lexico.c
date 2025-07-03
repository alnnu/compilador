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
  "decimal",
};

char operators[14][256] = {
  "+",
  "-",
  "*",
  "/",
  "^",
  "=",
  "==",
  "<",
  ">",
  "<=",
  ">=",
  "<>",
  "&&",
  "||"
};

int prefix(const char *pre, const char *str) {
    int returnValue = -1;

    if(strncmp(pre, str, strlen(pre)) == 0 && strlen(str) >= strlen(pre) + 1) {
      
      returnValue = 0;
    } 
  return returnValue;

}
int verifySymbols(char line[256], int lineNumber) {
  char *pch;
  int i = 0;
  int errorFlag = 0;  
  pch = strtok(line, " (){};,\n\0");

  while (pch != NULL) {
    i = 0;
    printf("pch: %s\n", pch);
    if(strcmp(pch, " ") != 0 && strcmp(pch, "\n") != 0) {
      if (prefix("__", pch) != 0 && prefix("!", pch) != 0) {
        for(i; i < 11; i++) {
          if(strcmp(pch, reserveWords[i]) == 0) {
            errorFlag = 0;
            break;
          }else {
            errorFlag = 1;
          }
        }
        
        if (errorFlag == 0) {
          pch = strtok(NULL, " (){};,\n\0");
          continue;
        }

        for(i = 0; i < 14; i++) {
          if(strcmp(pch, operators[i]) == 0) {
            errorFlag = 0;
            break;
          }else {
            errorFlag = 1;
          }
        }
      }
    }
    if (errorFlag == 1) {
      printf("Erro na linha %d, na palavra '%s' \n", lineNumber, pch);
      break;
    } 
    pch = strtok (NULL, " (){};,\n\0");
  }

  return errorFlag;

}


