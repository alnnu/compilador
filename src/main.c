#include <stdio.h>


void main(){
  FILE *file;
  
  file = fopen("../programa.txt", "r");

  char line[256];

  if  (file == NULL) {
    printf("Error opening file\n");
    return;
  }

  while (fgets(line, sizeof(line), file) != NULL) {
    printf("%s", line);
  }
}
