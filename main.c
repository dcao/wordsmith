#include <stdio.h>
#include "rules.c"

int main(int argc, char **argv) {
    char x[] = "a b c d\ne f g h\ni j k l";
    rule_t *y = build_rules(x);

    for (int i = 0; i < 3; i++) {
        rule_t r = y[i];
        printf("%s %s %s %s\n", r.name, r.rule, r.message, r.payload);
    }
    
    return 0;
}
