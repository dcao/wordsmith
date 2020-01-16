#include "ws.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

int stderr_handle(void *ctx, lint_t lint) {
    fprintf(stderr, "%s:%ld:%ld %s:%s\n", lint.prose.name, lint.line, lint.col,
           lint.rule.name, lint.rule.mesg);

    return 0;
}

sink_t stderr_sink = { NULL, stderr_handle };

int sink_handle(sink_t *sink, lint_t lint) {
    return sink->handle(sink->ctx, lint);
}
