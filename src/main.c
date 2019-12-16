#include <stdio.h>
#include <stdlib.h>
#include "rules.c"

int main(int argc, char **argv) {
    char x[] = "a\\;;b;c;d\ne;f;g;h\ni;j;k;l";
    rules_t y = build_rules(x);

    for (size_t i = 0; i < y.used; i++) {
        rule_t r = y.array[i];
        printf("%s %s %s %s\n", r.name, r.rule, r.message, r.payload);
    }

    free_rules(&y);
    return 0;
}

// test
