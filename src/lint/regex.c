#include "hs.h"
#include <ws.h>
#include <string.h>

typedef struct {
    unsigned long l;
    unsigned long c;
} linum;

typedef struct {
    rules_t *rules;
    prose_t *prose;
    sink_t sink;
    linum *linums;
    int *consi_map;
    // occur is a bit vector, recording if a regex has been seen yet.
    int *occur;
    int occur_sz;
    hs_database_t *db;
} regex_linter_ctx_t;

int handle_match(unsigned id, unsigned long long from, unsigned long long to,
                 unsigned flags, void *ctx) {
    regex_linter_ctx_t *self = (regex_linter_ctx_t *)ctx;
    setbit(self->occur, id);

    if (self->consi_map[id] != -1) {
        if (testbit(self->occur, self->consi_map[id])) {
            goto report_lint;
        }
    } else {
        goto report_lint;
    }

    return 0;

report_lint: ;
    lint_t l = { to, self->linums[to].l, self->linums[to].c, *self->prose, self->rules->array[id] };
    sink_handle(&self->sink, l);

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

// TODO: Consistency, conditional checks
int regex_init(void **ctx, rules_t *rules, sink_t sink) {
    *ctx = malloc(sizeof(regex_linter_ctx_t));

    int res = 0;
    regex_linter_ctx_t *self = (regex_linter_ctx_t *)*ctx;
    self->rules = rules;

    // Compile our regexes
    char **regexes = malloc(rules->used * sizeof(char *));
    if (!regexes) {
        res = 1;
        goto free_nil;
    }

    unsigned *rids = malloc(rules->used * sizeof(unsigned));
    if (!rids) {
        res = 1;
        goto free_rxs;
    }

    self->occur = malloc((rules->used / 32 + 1) * sizeof(int));
    if (!self->occur) {
        res = 1;
        goto free_arrs;
    }
    self->occur_sz = rules->used / 32 + 1;

    self->consi_map = malloc(rules->used * sizeof(int));
    if (!self->consi_map) {
        res = 1;
        goto free_arrs;
    }

    int cur = 0;
    int consi_ptr = -1;
    for (int i = 0; i < rules->used; i++) {
        if (strcmp(rules->array[i].rule, "regex") == 0) {
            self->consi_map[cur] = -1;

            rids[cur] = i;
            regexes[cur++] = rules->array[i].payl;
        } else if (strcmp(rules->array[i].rule, "consi") == 0) {
            // We do this in case there's an odd number of consi rules
            self->consi_map[cur] = -1;
            if (consi_ptr != -1) {
                self->consi_map[consi_ptr] = cur;
                self->consi_map[cur] = consi_ptr;
            } else {
                consi_ptr = cur;
            }
            
            rids[cur] = i;
            regexes[cur++] = rules->array[i].payl;
        }
    }

    const char *const *crs = (const char *const *)regexes;
    const unsigned *crids = (const unsigned *)rids;

    self->sink = sink;

    hs_compile_error_t *comp_err;
    if (hs_compile_multi(crs, NULL, crids, cur, HS_MODE_BLOCK, NULL,
                         &self->db, &comp_err) != HS_SUCCESS) {
        hs_free_compile_error(comp_err);
        res = 1;
        goto free_arrs;
    }

free_arrs:
    free(rids);
free_rxs:
    free(regexes);
free_nil:
    return res;
}

int regex_report(void *ctx, prose_t prose) {
    regex_linter_ctx_t *self = (regex_linter_ctx_t *)ctx;
    int res = 0;
    self->prose = &prose;

    // We have to zero our occurrence array.
    memset(self->occur, 0, self->occur_sz * sizeof(int));

    hs_scratch_t *s = NULL;
    if (hs_alloc_scratch(self->db, &s) != HS_SUCCESS) {
        res = 1;
        goto free_db;
    }

    if (build_linums(self) != 0) {
        res = 1;
        goto free_scr;
    }

    if (hs_scan(self->db, prose.text, strlen(prose.text), 0, s, handle_match,
                ctx) != HS_SUCCESS) {
        res = 1;
        goto free_linums;
    }

free_linums:
    free(self->linums);
free_scr:
    hs_free_scratch(s);
free_db:
    return res;
}

void regex_deinit(void *ctx) {
    regex_linter_ctx_t *self = (regex_linter_ctx_t *)ctx;

    if (self->consi_map) {
        free(self->consi_map);
    }

    if (self->occur) {
        free(self->occur);
    }

    hs_free_database(self->db);
    free(self);
}

linter_t regex_linter() {
    linter_t l = { NULL, regex_init, regex_report, regex_deinit };
    return l;
}

