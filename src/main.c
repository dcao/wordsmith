#include <stdio.h>
#include <stdlib.h>
#include "util.c"
#include "rule.c"
#include "lint.c"
#include "lint/regex.c"

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#define STDIN_MAX 1000

int main(int argc, char **argv) {
    struct optparse_long longopts[] = {
        {"help", 'h', OPTPARSE_NONE},
        {"version", 'v', OPTPARSE_NONE},
        {"rule", 'r', OPTPARSE_REQUIRED},
        {0}
    };

    char *arg;
    int option;
    struct optparse options;

    char *rbuf = NULL;

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
        case 'h':
            fprintf(stderr, "todo: help\n");
            break;
        case 'v':
            fprintf(stderr, "wordsmith 0.1.0\n");
            break;
        case 'r': ;
            // bleh
            unsigned int len;
            char *temp = read_file(options.optarg, &len);
            if (!temp) {
                exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
        }
    }

    if (!rbuf) {
        fprintf(stderr, "%s: no rules passed (-r/--rules)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    rules_t rules;
    rule_error_t res = build_rules(';', rbuf, &rules);

    int err = 0;
    if (res != RULES_OK) {
        fprintf(stderr, "%s\n", xstr(err));
        err = 1;
        goto free_rules;
    }

    int fs = 0;

    sink_t sink = stderr_sink;
    linter_t linter = regex_linter();
    if (linter_init(&linter, &rules, sink) != 0) {
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
            goto linter_deinit;
        }
        prose_t prose = { file, arg };

        int fin = linter_report(&linter, prose);
        
        free(file);

        if (fin != 0) {
            err = 1;
            goto linter_deinit;
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

        int fin = linter_report(&linter, prose);

        if (fin != 0) {
            err = 1;
            goto linter_deinit;
        }
    }


linter_deinit:
    linter_deinit(&linter);
free_rules:
    free_rules(&rules);
    if (err == 0) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}

