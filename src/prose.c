#include "ws.h"
#include <stdlib.h>

void free_prose(prose_t prose) {
    free(prose.name);
    free(prose.text);
}
