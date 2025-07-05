#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lineList.h"

struct element{
  struct Line line;
  struct element *prox;
};
typedef struct element Elem;

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
           c == ';' || c == '{' || c == '}' || c == '\n' || c == '\0' || c == '\t';
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
int balancing(char opemSymbol, char closeSymbols, List *li, int lineNumber) {

  int count = 0;

  Elem *no = *li;
 
  while(no != NULL) {
    int i = 0;
    while(no->line.value[i] != '\0') {
      if(no->line.value[i] == opemSymbol) {
        count++;
      }else if(no->line.value[i] == closeSymbols) {
        count--;
      }
      i++;
    }
    no = no->prox;
  }

  if(count != 0) {
    printf("Erro de balanceamento: '%c' na linha '%d' nao esta balanceado.\n", opemSymbol, lineNumber);
    return 1;
  }else {
    return 0;
  }
}

int verifySymbols(List* li) {
  int i = 0;
  int j = 0;

  char *tokens;

  int errorFlag = 0;

  tokens = (char *)malloc(256 * sizeof(char));
  Elem *no = *li;
  while (no != NULL) {
    i = 0;
    printf("Linha %d: %s\n", no->line.lineNumber, no->line.value);
    while (no->line.value[i] != '\0') {
      if (is_separator(no->line.value[i]) == 0) {
        j = 0;

        while(is_separator(no->line.value[i]) == 0 && no->line.value[i] != '\0') {
          tokens[j++] = no->line.value[i++];
        }
        tokens[j] = '\0';
        printf("Token: %s\n", tokens);
        if(isReservedWord(tokens) != 0 && isVaribleOrFunction(tokens) != 0 && isOperator(tokens) != 0) {
          errorFlag = 1;
          printf("Erro na linha %d, na palavra '%s'\n", no->line.lineNumber, tokens);
        }
      }else {
        if(no->line.value[i] == '(') {
          errorFlag = balancing('(', ')', li, no->line.lineNumber);
        }else if(no->line.value[i] == '{') {
          errorFlag = balancing('{', '}', li, no->line.lineNumber);
        }
        i++;
      }
      if(errorFlag == 1) {
        break;
      }
    }
    if(errorFlag == 1) {
      break;
    }
    no = no->prox;
  }

  
  
  return errorFlag;
}





