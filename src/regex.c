#include "hs.h"

hs_error_t build_regex(const char *const *patterns, unsigned int numpats,
                       hs_database_t **db, hs_compile_error_t *err) {
    unsigned int flags[1] = { HS_MODE_BLOCK };
    hs_error_t res = hs_compile_multi(patterns, flags, NULL, numpats, HS_MODE_BLOCK, NULL, db, &err);
    return res;
}

