#include <stdlib.h>
#include <stdio.h>
#include "minunit.h"
#include "../src/rules.c"

int compare_rules(rules_t fst, rules_t snd) {
    for (size_t i = 0; i < fst.used; i++) {
        rule_t *a = &fst.array[i];
        rule_t *b = &snd.array[i];
        if (strcmp(a->name, b->name) || strcmp(a->rule, b->rule)
            || strcmp(a->mesg, b->mesg) || strcmp(a->payl, b->payl)) {
            printf("%s %s %s %s\n", a->name, a->rule, a->mesg, a->payl);
            printf("%s %s %s %s\n", b->name, b->rule, b->mesg, b->payl);
            return 0;
        }
    }
    return 1;
}

static char *test_rule_parse_basic() {
    char x[] = "a;b;c;d\ne;f;g;h\ni;j;k;l\0";
    rules_t y;
    rule_error_t res = build_rules(x, &y);

    rule_t corrules[] = {
        { "a", "b", "c", "d" },
        { "e", "f", "g", "h" },
        { "i", "j", "k", "l" }
    };

    rules_t correct = { corrules, 3, 3 };

    mu_assert("test_rule_parse_basic failed", res == RULES_OK);
    mu_assert("test_rule_parse_basic failed", compare_rules(y, correct));
    
    free_rules(&y);
    
    return 0;
}

static char *test_rule_parse_esc() {
    char x[] = "a;b;c\\;;d\ne;f;g;h\ni;j;k;l\0";
    rules_t y;
    rule_error_t res = build_rules(x, &y);

    rule_t corrules[] = {
        { "a", "b", "c;", "d" },
        { "e", "f", "g",  "h" },
        { "i", "j", "k",  "l" }
    };

    rules_t correct = { corrules, 3, 3 };

    mu_assert("test_rule_parse_esc failed", res == RULES_OK);
    mu_assert("test_rule_parse_esc failed", compare_rules(y, correct));
    
    free_rules(&y);
    
    return 0;
}

static char *test_rule_parse_empty() {
    char x[] = "a;;c\\;;d\ne;f;g;h\ni;j;k;l\0";
    rules_t y;
    rule_error_t res = build_rules(x, &y);

    mu_assert("test_rule_parse_empty failed", res == INVALID_RULE);
    
    return 0;
}

static char *test_rule_parse_extra() {
    char x[] = "this\\;b;c;d;\ne;f;g;h\ni;j;k;l\0";
    rules_t y;
    rule_error_t res = build_rules(x, &y);

    mu_assert("test_rule_parse_extra failed", res == INVALID_RULE);
    
    return 0;
}
