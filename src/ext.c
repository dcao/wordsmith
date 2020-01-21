#include <libtcc.h>
#include <ws.h>
#include <string.h>
#include "util.c"

// mems - for dealing with storing memory from tcc states
int init_mems(mems_t *a, unsigned int initial_size) {
    a->array = malloc(initial_size * sizeof(void *));
    if (!a->array) {
        return 1;
    }
    a->used = 0;
    a->size = initial_size;

    return 0;
}

int add_mem(mems_t *a, void *ptr) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(void *));
        if (!a->array) {
            return 1;
        }
    }
    a->array[a->used++] = ptr;
    return 0;
}

void free_mems(mems_t *x) {
    for (size_t i = 0; i < x->used; i++) {
        free(x->array[i]);
    }
    free(x->array);
    x->used = x->size = 0;
}


ext_error_t register_ext_file(lintset_t *lintset, char *fname, mems_t *mems) {
    // This just reads the file then passes it to register_ext_str
    unsigned int len;
    char *file = read_file(fname, &len);
    if (!file) {
        return TCC_FILE_ERR;
    }

    ext_error_t res = register_ext_str(lintset, file, mems);

    free(file);

    return res;
}

ext_error_t register_ext_str(lintset_t *lintset, char *str, mems_t *mems) {
    TCCState *s = tcc_new();
    if (!s) {
        return TCC_STATE_ERR;
    }

    // To account for the fact that we might want to pass flags to tcc to add
    // libraries or library paths, we allow for passing
    // #pragma ws tcc <flags>
    // to pass all additional flags to the tcc compiler.
    char *flags = strstr(str, "#pragma ws ");
    while (flags) {
        flags += 11;

        char *endl = strchr(flags, '\n');
        *endl = 0;

        // We check the next word
        char *word = strchr(flags, ' ');

        if (word) {
            *word = 0;

            if (strcmp(flags, "tcc") == 0) {
                flags = word + 1;
                tcc_set_options(s, flags);
            }

            *word = ' ';
        }
        
        *endl = '\n';
        flags = endl + 1;
        flags = strstr(flags, "#pragma ws ");
    }

    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    // To avoid having to force extension authors to include a wordsmith header, we simply
    // reproduce the needed API that extensions need through defined symbols.
    tcc_define_symbol(s, "rule_t", "struct { char *name; char *rule; char *mesg; char *payl; }");
    tcc_define_symbol(s, "rules_t", "struct { rule_t *array; unsigned long used; unsigned long size; }");
    tcc_define_symbol(s, "prose_t", "struct { char *text; char *name; }");
    tcc_define_symbol(s, "lint_t", "struct { unsigned long long offset; unsigned long line; unsigned long col; prose_t prose; rule_t rule; }");
    tcc_define_symbol(s, "sink_t", "struct { void *ctx; int (*handle)(void *ctx, lint_t); }");
    tcc_define_symbol(s, "sink_handle(sink, lint)", "sink->handle(sink->ctx, lint)");

    if (tcc_compile_string(s, str) < 0) {
        return TCC_COMPILE_ERR;
    }

    int size = tcc_relocate(s, NULL);

    if (size < 0) {
        return TCC_RELOC_ERR;  
    }

    void *mem = malloc(size);
    tcc_relocate(s, mem);

    if (add_mem(mems, mem)) {
        return MEM_ALLOC_ERR;
    }

    int (*init)(void **, rules_t *, sink_t) = (int (*)(void **, rules_t *, sink_t)) tcc_get_symbol(s, "init");
    int (*report)(void *, prose_t) = (int (*)(void *, prose_t)) tcc_get_symbol(s, "report");
    void (*deinit)(void *) = (void (*)(void *)) tcc_get_symbol(s, "deinit");

    if (!init || !report || !deinit) {
        return LINTER_NOT_FOUND;
    }

    linter_t l = { NULL, init, report, deinit };

    if (lintset_add(lintset, l)) {
        return LINTSET_ERR;
    }

    tcc_delete(s);

    return EXT_OK;
}
