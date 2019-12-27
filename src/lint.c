#include "ws.h"

int linter_report(linter_t *linter, rules_t *rules, prose_t prose, sink_t sink) {
    return linter->report(linter->ctx, rules, prose, sink);
}
