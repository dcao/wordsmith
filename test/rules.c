#include <stdlib.h>
#include <stdio.h>
#include "minunit.h"
#include "../src/rules.c"

int compare_rules(rules_t fst, rules_t snd) {
    for (size_t i = 0; i < fst.used; i++) {
        rule_t *a = &fst.array[i];
        rule_t *b = &snd.array[i];
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
    rules_t y = build_rules(x);

    rule_t corrules[] = {
        { "a", "b", "c", "d" },
        { "e", "f", "g", "h" },
        { "i", "j", "k", "l" }
    };

    rules_t correct = { corrules, 3, 3 };

    mu_assert("test_rule_parse_basic failed", compare_rules(y, correct));
    
    free_rules(&y);
    
    return 0;
}

static char *test_rule_parse_esc() {
    char x[] = "a;b;c\\;;d\ne;f;g;h\ni;j;k;l";
    rules_t y = build_rules(x);

    rule_t corrules[] = {
        { "a", "b", "c;", "d" },
        { "e", "f", "g",  "h" },
        { "i", "j", "k",  "l" }
    };

    rules_t correct = { corrules, 3, 3 };

    mu_assert("test_rule_parse_esc failed", compare_rules(y, correct));
    
    free_rules(&y);
    
    return 0;
}

static char *test_rule_parse_empty() {
    char x[] = "a;;c\\;;d\ne;f;g;h\ni;j;k;l";
    rules_t y = build_rules(x);

    rule_t corrules[] = {
        { "e", "f", "g",  "h" },
        { "i", "j", "k",  "l" }
    };

    rules_t correct = { corrules, 3, 3 };

    mu_assert("test_rule_parse_empty failed", compare_rules(y, correct));
    
    free_rules(&y);
    
    return 0;
}

/*

static char *test_rule_parse_err() {
    char x[] = "this\\;b;c;d\ne;f;g;h\ni;j;k;l";
    int *cnt = calloc(1, sizeof(int));
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

*/
