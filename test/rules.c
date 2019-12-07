#include <stdlib.h>
#include <stdio.h>
#include "minunit.h"
#include "../src/rules.c"

int compare_arrs(rule_t *a, rule_t *b, int cnt) {
    for (int i = 0; i < cnt; i++) {
        if (strcmp(a->name, b->name) || strcmp(a->rule, b->rule)
            || strcmp(a->message, b->message) || strcmp(a->payload, b->payload)) {
            printf("%s %s %s %s\n", a->name, a->rule, a->message, a->payload);
            printf("%s %s %s %s\n", b->name, b->rule, b->message, b->payload);
            return 0;
        }
    }
  return 1;
}

static char *test_rule_parse_basic() {
    char x[] = "a;b;c;d\ne;f;g;h\ni;j;k;l";
    int *cnt = malloc(sizeof(int));
    rule_t *y = build_rules(x, cnt);

    rule_t correct[] = {
        { "a", "b", "c", "d" },
        { "e", "f", "g", "h" },
        { "i", "j", "k", "l" }
    };

    mu_assert("test_rule_parse_basic failed", compare_arrs(y, correct, *cnt));
    
    free_rules(y, *cnt);
    free(cnt);
    
    return 0;
}

static char *test_rule_parse_esc() {
    char x[] = "a;b;c\\;;d\ne;f;g;h\ni;j;k;l";
    int *cnt = malloc(sizeof(int));
    rule_t *y = build_rules(x, cnt);

    rule_t correct[] = {
        { "a", "b", "c;", "d" },
        { "e", "f", "g",  "h" },
        { "i", "j", "k",  "l" }
    };

    mu_assert("test_rule_parse_esc failed", compare_arrs(y, correct, *cnt));
    
    free_rules(y, *cnt);
    free(cnt);
    
    return 0;
}

static char *test_rule_parse_err() {
    char x[] = "this\\;b;c;d\ne;f;g;h\ni;j;k;l";
    int *cnt = malloc(sizeof(int));
    rule_t *y = build_rules(x, cnt);

    rule_t correct[] = {
        { "e", "f", "g",  "h" },
        { "i", "j", "k",  "l" }
    };

    mu_assert("test_rule_parse_err failed", compare_arrs(y, correct, *cnt));
    
    free_rules(y, *cnt);
    free(cnt);
    
    return 0;
}
