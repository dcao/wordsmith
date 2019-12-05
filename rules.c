#include "hs.h"

typedef struct {
    char *name;
    char *rule;
    char *message;
    char *payload;
} rule_t;

hs_error_t build_regex(const char *const *patterns, unsigned int numpats, hs_database_t **db, hs_compile_error_t *err) {
     unsigned int *flags = malloc(1 * sizeof(int));
     flags[0] = HS_MODE_BLOCK;
     return hs_compile_multi(patterns, flags, NULL, numpats, HS_MODE_BLOCK, NULL, db, &err);
}



