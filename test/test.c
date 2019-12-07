#include <stdio.h>
#include "minunit.h"
#include "rules.c"

int tests_run = 0;

static char *all_tests() {
    mu_run_test(test_rule_parse_basic);
    // mu_run_test(test_rule_parse_esc);
    mu_run_test(test_rule_parse_empty);
    mu_run_test(test_rule_parse_err);
    return 0;
}

int main() {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
