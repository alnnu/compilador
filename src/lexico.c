#include <stdlib.h>
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


int is_separator(char c) {
    return c == ' ' || c == ',' || c == '(' || c == ')' || 
           c == ';' || c == '{' || c == '}' || c == '\n' || c == '\0';
}

int isVaribleOrFunction(char *token) {
  if (prefix("__", token) == 0 || prefix("!", token) == 0) {
    return 0; 
  }else {
    return -1;
  }
}

int isReservedWord(char *token) {
  int i = 0;
  for(i = 0; i < 11; i++) {
    if(strcmp(token, reserveWords[i]) == 0) {
      return 0; 
    }
  }
  return -1; 
}

int isOperator(char *token) {
  int i = 0;
  for(i = 0; i < 14; i++) {
    if(strcmp(token, operators[i]) == 0) {
      return 0;   
    }
  }
  return -1;
}

int verifySymbols(char line[256], int listNumberd) {
  int i = 0;
  int j = 0;

  char *tokens;

  int errorFlag = 0;

  tokens = (char *)malloc(256 * sizeof(char));

  
  
  while (line[i] != '\0') {
    if (is_separator(line[i]) == 0) {
      j = 0;

      while(is_separator(line[i]) == 0 && line[i] != '\0') {
        tokens[j++] = line[i++];
      }
      tokens[j] = '\0';
      printf("Token: %s\n", tokens);
      if(isReservedWord(tokens) != 0 && isVaribleOrFunction(tokens) != 0 && isOperator(tokens) != 0) {
        errorFlag = 1;
        printf("Erro na linha %d, na palavra '%s'\n", listNumberd, tokens);
        break;
      }
    }else {
      i++;
    }
  }

  return errorFlag;
}



