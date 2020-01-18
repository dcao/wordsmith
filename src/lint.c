#include <ws.h>
#include <stdlib.h>

// Generic linter functions
int linter_init(linter_t *linter, rules_t *rules, sink_t sink) {
    return linter->init_ctx(&linter->ctx, rules, sink);
}

int linter_report(linter_t *linter, prose_t prose) {
    return linter->report(linter->ctx, prose);
}

void linter_deinit(linter_t *linter) {
    if (linter->ctx) {
        linter->deinit_ctx(linter->ctx);
    }
}

// Lintsets
int lintset_create(lintset_t *a, unsigned int initial_size) {
    a->array = malloc(initial_size * sizeof(linter_t));

    if (!a->array) {
        return 1;
    }
    
    a->used = 0;
    a->size = initial_size;

    return 0;
}

int lintset_add(lintset_t *a, linter_t element) {
    // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
    // Therefore a->used can go up to a->size 
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(rule_t));

        if (!a->array) {
            return 1;
        }
    }
    a->array[a->used++] = element;

    return 0;
}

int lintset_init(lintset_t *lintset, rules_t *rules, sink_t sink) {
    for (int i = 0; i < lintset->used; i++) {
        if (linter_init(&lintset->array[i], rules, sink)) {
            return 1;
        }
    }
    return 0;
}

int lintset_report(lintset_t *lintset, prose_t prose) {
    for (int i = 0; i < lintset->used; i++) {
        if (linter_report(&lintset->array[i], prose)) {
            return 1;
        }
    }
    return 0;
}

void lintset_deinit(lintset_t *lintset) {
    for (int i = 0; i < lintset->used; i++) {
        linter_deinit(&lintset->array[i]);
    }

    free(lintset->array);
    lintset->used = lintset->size = 0;
}
