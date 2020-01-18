#ifndef WS_H
#define WS_H

// Utilities for printing enums
#define str(x) #x
#define xstr(x) str(x)

// Utilities for bit vectors
#define setbit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )         
#define clearbit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )         
#define testbit(A,k)    ( A[(k/32)] & (1 << (k%32)) )               

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

rule_error_t build_rules(char delim, char *rules_txt, rules_t *rules);
void free_rules (rules_t *rules);

// Lints
typedef struct {
    unsigned long long offset;
    unsigned long line;
    unsigned long col;
    prose_t prose;
    rule_t rule;
} lint_t;

// TODO: Better errors out of the handle fns
//       Maybe have a void *err param?
// Sinks
typedef struct sink {
    void *ctx;
    int (*handle)(void *ctx, lint_t);
} sink_t;

int sink_handle(sink_t *sink, lint_t lint);

// Linters
typedef struct linter {
    void *ctx;
    int (*init_ctx)(void **ctx, rules_t *, sink_t);
    int (*report)(void *ctx, prose_t);
    void (*deinit_ctx)(void *ctx);
} linter_t;

// Lintsets

// A lintset is an array of linters
typedef struct {
    linter_t *array;
    unsigned long used;
    unsigned long size;
} lintset_t;

int lintset_create(lintset_t *lintset, unsigned int initial_size);
int lintset_add(lintset_t *lintset, linter_t linter);
int lintset_init(lintset_t *lintset, rules_t *rules, sink_t sink);
int lintset_report(lintset_t *lintset, prose_t prose);
void lintset_deinit(lintset_t *lintset);

// Extensions
typedef struct {
    void **array;
    unsigned long used;
    unsigned long size;
} mems_t;

void init_mems(mems_t *a, unsigned int initial_size);
void add_mem(mems_t *a, void *ptr);
void free_mems(mems_t *a);

typedef enum {
    EXT_OK,
    TCC_STATE_ERR,
    TCC_FILE_ERR,
    TCC_COMPILE_ERR,
    TCC_RELOC_ERR,
    LINTER_NOT_FOUND,
    LINTSET_ERR,
} ext_error_t;

ext_error_t register_ext_file(lintset_t *lintset, char *fname, mems_t *mems);
ext_error_t register_ext_str(lintset_t *lintset, char *str, mems_t *mems);

#endif
