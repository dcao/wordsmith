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
