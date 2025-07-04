struct Line{
    string value[256];
    int lineNumber;
};

typedef struct elemento* List;

Lista* createList();
void freeList(List* li);
int insertList(List* li, struct Line al);
int removeList(Lista* li, int lineNumber);
int readLine(Lista* li,int lineNumber, struct aluno *al);
