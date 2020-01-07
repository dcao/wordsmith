#include "ws.h"

// Generic linter functions
int linter_init(linter_t *linter, rules_t *rules, sink_t sink) {
    return linter->init_ctx(&linter->ctx, rules, sink);
}

int linter_report(linter_t *linter, prose_t prose) {
    return linter->report(linter->ctx, prose);
}

void linter_deinit(linter_t *linter) {
    return linter->deinit_ctx(linter->ctx);
}

// Lintsets
// A lintset is an array of pointers to linters
typedef linter_t *lintset_t[];

int lintset_init(lintset_t lintset, int size, rules_t *rules, sink_t sink) {
    for (int i = 0; i < size; i++) {
        if (linter_init(lintset[i], rules, sink)) {
            return 1;
        }
    }
    return 0;
}

int lintset_report(lintset_t lintset, int size, prose_t prose) {
    for (int i = 0; i < size; i++) {
        if (linter_report(lintset[i], prose)) {
            return 1;
        }
    }
    return 0;
}

void lintset_deinit(lintset_t lintset, int size) {
    for (int i = 0; i < size; i++) {
        linter_deinit(lintset[i]);
    }
}
