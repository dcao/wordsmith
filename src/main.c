#include <stdio.h>
#include <stdlib.h>
#include "rules.c"

int main(int argc, char **argv) {
    char x[] = "a;b;c;d\ne;f;g;h\ni;j;k;l";
    int *size = malloc(sizeof(int));
    rule_t *y = build_rules(x, size);

    for (int i = 0; i < *size; i++) {
        rule_t r = y[i];
        printf("%s %s %s %s\n", r.name, r.rule, r.message, r.payload);
    }

    free_rules(y, *size);
    free(size);
    
    return 0;
}
