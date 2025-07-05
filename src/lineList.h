

struct Line{
    char value[256];
    int lineNumber;
};

typedef struct element* List;

List* createList();
void freeList(List* li);
int insertList(List* li, struct Line al);
int removeList(List* li, int lineNumber);
int readLine(List* li,int lineNumber, struct Line *al);

