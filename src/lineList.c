#include <stdio.h>
#include <stdlib.h>
#include "lineList.h" //inclui os Prot�tipos

//Defini�o do tipo lista
struct element{
    struct Line line;
    struct element *prox;
};
typedef struct element Elem;

List* createList(){
    List* li = (List*) malloc(sizeof(List));
    if(li != NULL)
        *li = NULL;
    return li;
}

void freeList(List* li){
    if(li != NULL){
        Elem* no;
        while((*li) != NULL){
            no = *li;
            *li = (*li)->prox;
            free(no);
        }
        free(li);
    }
}


