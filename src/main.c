#include <stdio.h>
#include <stdlib.h>
#include <ws.h>
#include <ws_contrib.h>
#include "util.c"

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#define STDIN_MAX 1000
#define STREAM_BUF_LEN 4096

int main(int argc, char **argv) {
    struct optparse_long longopts[] = {
        {"help", 'h', OPTPARSE_NONE},
        {"version", 'v', OPTPARSE_NONE},
        {"rule", 'r', OPTPARSE_REQUIRED},
        {"ext", 'e', OPTPARSE_REQUIRED},
        {0}
    };

    char *arg;
    int option;
    struct optparse options;

    setvbuf(stderr, NULL, _IOFBF, STREAM_BUF_LEN);
    setvbuf(stdout, NULL, _IOFBF, STREAM_BUF_LEN);

    char *rbuf = NULL;

    lintset_t lintset;
    if (lintset_create(&lintset, 1)) {
        fprintf(stderr, "%s: failed to allocate lintset", argv[0]);
        exit(EXIT_FAILURE);
    }

    int err = 0;

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
        case 'h':
            fprintf(stderr, "todo: help\n");
            err = 2;
            goto free_lintset;
            break;
        case 'v':
            fprintf(stderr, "wordsmith 0.1.0\n");
            goto free_lintset;
            break;
        case 'e':
            switch (register_ext_file(&lintset, options.optarg)) {
            case EXT_OK:
                break;
            case LINTER_NOT_FOUND:
                fprintf(stderr, "%s: extension does not specify init, report, and deinit fns\n", argv[0]);
            default:
                fprintf(stderr, "%s: unable to register extension %s\n", argv[0], options.optarg);
                err = EXIT_FAILURE;
                goto free_lintset;
            }
            break;
        case 'r': ;
            unsigned int len;
            char *temp = read_file(options.optarg, &len);
            if (!temp) {
                fprintf(stderr, "%s: invalid rules file\n", argv[0]);
                err = EXIT_FAILURE;
                goto free_lintset;
            }

            if (!rbuf) {
                rbuf = malloc(len + 1);
                memcpy(rbuf, temp, len + 1);
            } else {
                int rlen = strlen(rbuf);
                rbuf = realloc(rbuf, rlen + len + 1);
                memcpy(rbuf + rlen, temp, len + 1);
            }

            free(temp);
            break;
        case '?':
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            err = EXIT_FAILURE;
            goto free_lintset;
        }
    }

    if (!rbuf) {
        fprintf(stderr, "%s: no rules passed (-r/--rules)\n", argv[0]);
        err = EXIT_FAILURE;
        goto free_lintset;
    }

    rules_t rules;
    rule_error_t res = build_rules(';', rbuf, &rules);

    if (res != RULES_OK) {
        fprintf(stderr, "%s\n", xstr(err));
        err = EXIT_FAILURE;
        goto free_rules;
    }

    int fs = 0;

    sink_t sink = stderr_sink;
    linter_t rlinter = regex_linter();

    if (lintset_add(&lintset, rlinter)) {
        err = EXIT_FAILURE;
    }
    
    if (lintset_init(&lintset, &rules, sink) != 0) {
        err = 1;
        goto free_rules;
    }

    // Deal with remaining files
    while ((arg = optparse_arg(&options))) {
        fs |= 1;
        unsigned int len;
        char *file = read_file(arg, &len);
        if (!file) {
            err = 1;
            goto free_rules;
        }
        prose_t prose = { file, arg };

        int fin = lintset_report(&lintset, prose);
        
        free(file);

        if (fin != 0) {
            err = 1;
            goto free_rules;
        }
    }

    if (fs == 0) {
        char *input = NULL;
        char lbuf[STDIN_MAX];
        char name[] = "stdin";
        while (fgets(lbuf, STDIN_MAX, stdin)) {
            if (!input) {
                input = malloc(strlen(lbuf) + 1);
                memcpy(input, lbuf, strlen(lbuf) + 1);
            } else {
                int ilen = strlen(input);
                input = realloc(input, ilen + strlen(lbuf) + 1);
                memcpy(input + ilen, lbuf, strlen(lbuf) + 1);
            }
        }

        prose_t prose = { input, name };

        int fin = lintset_report(&lintset, prose);

        if (fin != 0) {
            err = 1;
            goto free_rules;
        }
    }


free_rules:
    free_rules(&rules);
free_lintset:
    if (rbuf) {
        free(rbuf);
    }

    lintset_deinit(&lintset);
    exit(err);
}

