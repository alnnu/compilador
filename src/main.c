#include <stdio.h>
#include <stdlib.h>
#include "lexico.h"
#include <string.h>

void main(){
  printf("Compilador iniciado\n");
  FILE *file;
  int pageCount = 0;
  List *lineList = createList();
  
  file = fopen("../programa.txt", "r"); 
  
  char line[256]; 
  

  if  (file == NULL) { 
    printf("Error opening file\n");
    return;
  }

  while (fgets(line, sizeof(line), file) != NULL) {
    struct Line al;
    al.lineNumber = pageCount;
    strcpy(al.value, line);
    insertList(lineList, al);
    pageCount++;
  }

  verifySymbols(lineList);

}
