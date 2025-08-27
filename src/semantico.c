#include <stdio.h>
#include "semantico.h"
#include "lexico.h"

void semantic_alert(const char* message) {
    printf("ALERTA SEMÂNTICO na linha %d: %s\n", current_line, message);
}