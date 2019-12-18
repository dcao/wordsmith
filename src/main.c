#include <stdio.h>
#include <stdlib.h>
#include "rules.c"

int main(int argc, char **argv) {
    char x[] = "a\\;;b;c;d\ne;f;g;h\ni;j;k;l\0";
    rules_t y;
    rule_error_t res = build_rules(x, &y);

    if (res == RULES_OK) {
        for (size_t i = 0; i < y.used; i++) {
            rule_t r = y.array[i];
            printf("%s %s %s %s\n", r.name, r.rule, r.mesg, r.payl);
        }

        free_rules(&y);
    } else if (res == INVALID_RULE) {
        printf("err");
    }
    return 0;
}

