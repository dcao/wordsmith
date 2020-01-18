#include <libtcc.h>
#include <ws.h>
#include <string.h>
#include "util.c"

// mems - for dealing with storing memory from tcc states
void init_mems(mems_t *a, unsigned int initial_size) {
    a->array = malloc(initial_size * sizeof(void *));
    a->used = 0;
    a->size = initial_size;
}

void add_mem(mems_t *a, void *ptr) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(void *));
    }
    a->array[a->used++] = ptr;
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

    if (tcc_compile_string(s, str) < 0) {
        return TCC_COMPILE_ERR;
    }

    int size = tcc_relocate(s, NULL);

    if (size < 0) {
        return TCC_RELOC_ERR;  
    }

    void *mem = malloc(size);
    tcc_relocate(s, mem);
    add_mem(mems, mem);

    int (*init)(void **, rules_t *, sink_t) = (int (*)(void **, rules_t *, sink_t)) tcc_get_symbol(s, "init");
    int (*report)(void *, prose_t) = (int (*)(void *, prose_t)) tcc_get_symbol(s, "init");
    void (*deinit)(void *) = (void (*)(void *)) tcc_get_symbol(s, "init");

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
