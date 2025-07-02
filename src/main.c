#include <stdio.h>
#include "lexico.h"

void main(){
  FILE *file;
  int pageCount = 0;

  file = fopen("../programa.txt", "r"); 
  char line[256]; 
  
  if  (file == NULL) { 
    printf("Error opening file\n");
    return;
  }


  while (fgets(line, sizeof(line), file) != NULL) {
    verifySymbols(line, pageCount);
    pageCount++;
  }

}
