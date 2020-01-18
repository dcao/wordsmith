#include <libtcc.h>
#include <ws.h>
#include <string.h>
#include "util.c"

ext_error_t register_ext_file(lintset_t *lintset, char *fname) {
    // This just reads the file then passes it to register_ext_str
    unsigned int len;
    char *file = read_file(fname, &len);
    if (!file) {
        return TCC_FILE_ERR;
    }

    ext_error_t res = register_ext_str(lintset, file);

    free(file);

    return res;
}

ext_error_t register_ext_str(lintset_t *lintset, char *str) {
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

    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0) {
        return TCC_RELOC_ERR;  
    }

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

    // // A problem: these functions in the linter become inaccessible after this function returns
    // tcc_delete(s);

    return EXT_OK;
}
