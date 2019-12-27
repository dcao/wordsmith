#include "hs.h"
#include "../sink.c"
#include <string.h>

typedef struct {
    unsigned long l;
    unsigned long c;
} linum;

typedef struct {
    rules_t *rules;
    prose_t *prose;
    sink_t *sink;
    linum *linums;
} regex_linter_ctx_t;

int handle_match(unsigned id, unsigned long long from, unsigned long long to,
                 unsigned flags, void *ctx) {
    regex_linter_ctx_t *self = (regex_linter_ctx_t *)ctx;
    lint_t l = { to, self->linums[to].l, self->linums[to].c, *self->prose, self->rules->array[id] };
    sink_handle(self->sink, l);

    return 0;
}

int build_linums(regex_linter_ctx_t *self) {
    self->linums = malloc(strlen(self->prose->text) * sizeof(linum));
    if (!self->linums) return 1;
    unsigned long long cur = 0;
    unsigned long l = 1;
    unsigned long c = 0;

    while (self->prose->text[cur] != '\0') {
        if (self->prose->text[cur] == '\n') {
            l += 1;
            c = 1;
        } else {
            c += 1;
        }
        linum t = { l, c };
        self->linums[cur++] = t;
    }

    return 0;
}

int regex_report(void *ctx, rules_t *rules, prose_t prose, sink_t sink) {
    int res = 0;
    regex_linter_ctx_t *self = (regex_linter_ctx_t *)ctx;
    self->rules = rules;

    // Compile our regexes
    char **regexes = calloc(rules->used, sizeof(char *));
    if (!regexes) {
        res = 1;
        goto free_nil;
    }
    unsigned *rids = calloc(rules->used, sizeof(unsigned));
    if (!rids) {
        res = 1;
        goto free_rxs;
    }
    int cur = 0;
    for (int i = 0; i < rules->used; i++) {
        if (strcmp(rules->array[i].rule, "regex") == 0) {
            rids[cur] = i;
            regexes[cur++] = rules->array[i].payl;
        }
    }

    const char *const *crs = (const char *const *)regexes;
    const unsigned *crids = (const unsigned *)rids;

    self->prose = &prose;
    self->sink = &sink;

    hs_database_t *db;
    hs_compile_error_t *comp_err;
    if (hs_compile_multi(crs, NULL, crids, cur, HS_MODE_BLOCK, NULL,
                         &db, &comp_err) != HS_SUCCESS) {
        hs_free_compile_error(comp_err);
        res = 1;
        goto free_arrs;
    }

    hs_scratch_t *s = NULL;
    if (hs_alloc_scratch(db, &s) != HS_SUCCESS) {
        res = 1;
        goto free_db;
    }

    if (build_linums(self) != 0) {
        res = 1;
        goto free_scr;
    }

    if (hs_scan(db, prose.text, strlen(prose.text), 0, s, handle_match,
                ctx) != HS_SUCCESS) {
        res = 1;
        goto free_linums;
    }

free_linums:
    free(self->linums);
free_scr:
    hs_free_scratch(s);
free_db:
    hs_free_database(db);
free_arrs:
    free(rids);
free_rxs:
    free(regexes);
free_nil:
    return res;
}

hs_error_t build_regex(const char *const *patterns, unsigned int numpats,
                       hs_database_t **db, hs_compile_error_t *err) {
    unsigned int flags[1] = { HS_MODE_BLOCK };
    hs_error_t res = hs_compile_multi(patterns, flags, NULL, numpats, HS_MODE_BLOCK, NULL, db, &err);
    return res;
}

linter_t regex_linter() {
    regex_linter_ctx_t *ctx = calloc(1, sizeof(regex_linter_ctx_t));
    linter_t linter = { ctx, regex_report };

    return linter;
}

