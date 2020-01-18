#include <stdlib.h>
#include <string.h>
#include <ws.h>

#define RULE_DELIM ';'

int init_rules(rules_t *a, size_t initial_size) {
    a->array = malloc(initial_size * sizeof(rule_t));
    if (!a->array) {
        return 1;
    }
    a->used = 0;
    a->size = initial_size;

    return 0;
}

int insert_rule(rules_t *a, rule_t element) {
    // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
    // Therefore a->used can go up to a->size 
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(rule_t));
        if (!a->array) {
            return 1;
        }
    }
    a->array[a->used++] = element;

    return 0;
}

int field_size(char delim, char *src, char **end) {
    int valid = 0;
    while (*src != delim && *src != '\0' && *src != '\n') {
        switch (*src) {
        case '\\':
            // Possibly escaped character
            // We'll fall into the default case anyway; if it's an escaped
            // char, tho, we fall increment src to avoid adding the escape
            // char
            if (*(src+1) == '\\' || *(src+1) == delim || *(src+1) == '\n') {
                src++;
            }
            // fall through
        default:
            src++;
            valid++;
            break;
        }
    }
    *end = src;
    return valid;
}

void str_esc(char delim, char **src, char *dest) {
    int valid_chars = 0;
    while (**src != delim && **src != '\0' && **src != '\n') {
        switch (**src) {
        case '\\':
            // Possibly escaped character
            // We'll fall into the default case anyway; if it's an escaped
            // char, tho, we fall increment cur to avoid adding the escape
            // char
            if (*(*src+1) == '\\' || *(*src+1) == delim || *(*src+1) == '\n') {
                (*src)++;
            }
            // fall through
        default:
            dest[valid_chars++] = **src;
            break;
        }
        (*src)++;
    }

    dest[valid_chars] = 0;
}

void free_rules(rules_t *x) {
    for (size_t i = 0; i < x->used; i++) {
        rule_t c = x->array[i];
        free(c.name);
        free(c.rule);
        free(c.mesg);
        free(c.payl);
    }
    free(x->array);
    x->used = x->size = 0;
}

rule_error_t build_rules(char delim, char *rules_txt, rules_t *rules) {
    if (init_rules(rules, 16)) {
        return RULE_ALLOC;
    }

    // Test our arguments for null-ity
    if (!rules_txt) {
        return NULL_RULES;
    }

    // After this, attempt to create a rule for each line in our rules_txt
    char *cur = rules_txt;
    rule_error_t err;
    while (*cur != '\0') {
        rule_t cur_rule;

        // Attempt to parse name
        char *end;
        int name_size = field_size(delim, cur, &end);
        // if cur isn't at a delimeter at this point or it's empty, it's invalid
        if (name_size == 0 || *end != delim) {
            err = INVALID_RULE;
            goto free_rules;
        }
        cur_rule.name = malloc((name_size + 1) * sizeof(char));
        if (!cur_rule.name) {
            // Allocation failure
            err = RULE_ALLOC;
            goto free_rules;
        }
        str_esc(delim, &cur, cur_rule.name);
        cur++;

        int rule_size = field_size(delim, cur, &end);
        if (rule_size == 0 || *end != delim) {
            err = INVALID_RULE;
            goto free_name;
        }
        cur_rule.rule = malloc((rule_size + 1) * sizeof(char));
        if (!cur_rule.rule) {
            // Allocation failure
            err = RULE_ALLOC;
            goto free_name;
        }
        str_esc(delim, &cur, cur_rule.rule);
        cur++;

        int message_size = field_size(delim, cur, &end);
        if (message_size == 0 || *end != delim) {
            // Missing a semicolon; too few sections
            // We continue onwards
            err = INVALID_RULE;
            goto free_rule;
        }
        cur_rule.mesg = malloc((message_size + 1) * sizeof(char));
        if (!cur_rule.mesg) {
            // Allocation failure
            err = RULE_ALLOC;
            goto free_rule;
        }
        str_esc(delim, &cur, cur_rule.mesg);
        cur++;

        int payload_size = field_size(delim, cur, &end);
        if (payload_size == 0 || *end == delim) {
            // Extra semicolon; too many sections
            err = INVALID_RULE;
            goto free_mesg;
        }
        cur_rule.payl = malloc((payload_size + 1) * sizeof(char));
        if (!cur_rule.payl) {
            // Allocation failure
            err = RULE_ALLOC;
            goto free_mesg;
        }
        str_esc(delim, &cur, cur_rule.payl);
        cur++;

        if (insert_rule(rules, cur_rule)) {
            err = RULE_ALLOC;
            goto free_rules;
        }
        continue;

    free_mesg:
        free(cur_rule.mesg);
    free_rule:
        free(cur_rule.rule);
    free_name:
        free(cur_rule.name);
        goto free_rules;
    }

    return RULES_OK;

free_rules:
    // Error path: something went wrong
    free_rules(rules);
    return err;
}

