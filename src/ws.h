#ifndef WS_H
#define WS_H

// Utilities for printing enums
#define str(x) #x
#define xstr(x) str(x)

// Prose
typedef struct {
    char *text;
    char *name;
} prose_t;

void free_prose(prose_t prose);

// Rules
typedef struct {
    char *name;
    char *rule;
    char *mesg;
    char *payl;
} rule_t;

typedef struct {
    rule_t *array;
    unsigned long used;
    unsigned long size;
} rules_t;

typedef enum {
    RULES_OK,
    NULL_RULES,
    RULE_ALLOC,
    INVALID_RULE,
} rule_error_t;

// Lints
typedef struct {
    unsigned long long offset;
    unsigned long line;
    unsigned long col;
    prose_t prose;
    rule_t rule;
} lint_t;

// TODO: Better errors out of the handle fns
// Sinks
typedef struct sink {
    void *ctx;
    int (*handle)(void *ctx, lint_t);
} sink_t;

// Linters
typedef struct linter {
    void *ctx;
    int (*init_ctx)(void **ctx, rules_t *, sink_t);
    int (*report)(void *ctx, prose_t);
    void (*deinit_ctx)(void *ctx);
} linter_t;

#endif
