#include <stdio.h>
#include <stdlib.h>
#include "lineList.h" 


struct element{
    struct Line line;
    struct element *prox;
};
typedef struct element Elem;

List* createList(){
    List* li = (List*)malloc(sizeof(List));
    if(li != NULL)
        *li = NULL;
    return li;
}

/*void freeList(List* li){
    if(li != NULL){
        Elem* no;
        while((*li) != NULL){
            no = *li;
            *li = (*li)->prox;
            free(no);
        }
        free(li);
    }
}*/
int insertList(List* li, struct Line al) {
    if(li == NULL)
        return 0;
    Elem *no;
    no = (Elem*) malloc(sizeof(Elem));
    if(no == NULL)
        return 0;
    no->line = al;
    no->prox = NULL;
    if((*li) == NULL) {
        *li = no;
    }else{
        Elem *aux;
        aux = *li;
        while(aux->prox != NULL){
            aux = aux->prox;
        }
        aux->prox = no;
    }
    return 1;

}
   
int removeList(List* li, int lineNumber) {
  if(li == NULL)
    return 0;
  if((*li) == NULL)
    return 0;
  Elem *ant, *no = *li;
  while(no != NULL && no->line.lineNumber != lineNumber) {
    ant = no;
    no = no->prox;
  }
  if(no == NULL)
    return 0;

  if(no == *li)
    *li = no->prox;
  else
    ant->prox = no->prox;
  free(no);
  return 1;
}

int readLine(List* li,int lineNumber, struct Line *al) {

  if(li == NULL)
    return 0;
  Elem *no = *li;
  while(no != NULL && no->line.lineNumber != lineNumber) {
    no = no->prox;
  }
  if(no == NULL)
    return 0;
  else{
    *al = no->line;
    return 1;
  }
}



