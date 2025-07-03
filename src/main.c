#include <stdio.h>
#include <stdlib.h>
#include "lexico.h"
#include <string.h>

void main(){
  printf("Compilador iniciado\n"); 
  FILE *file;
  int pageCount = 0;
  
  file = fopen("../programa.txt", "r"); 
  
  char line[256]; 
  

  if  (file == NULL) { 
    printf("Error opening file\n");
    return;
  }

  while (fgets(line, sizeof(line), file) != NULL) {
    if(verifySymbols(line, pageCount) == 1) {
      break;
    }
    pageCount++;
  }


}
