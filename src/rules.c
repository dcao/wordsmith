#include <stdlib.h>
#include <string.h>

#define RULE_DELIM ';'

typedef struct {
    char *name;
    char *rule;
    char *message;
    char *payload;
} rule_t;

typedef struct {
  rule_t *array;
  size_t used;
  size_t size;
} rules_t;

void init_rules(rules_t *a, size_t initial_size) {
  a->array = malloc(initial_size * sizeof(rule_t));
  a->used = 0;
  a->size = initial_size;
}

void insert_rule(rules_t *a, rule_t element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 
  if (a->used == a->size) {
    a->size *= 3 / 2;
    a->array = realloc(a->array, a->size * sizeof(rule_t));
  }
  a->array[a->used++] = element;
}

int field_size(const char *src, char **end) {
    int valid = 0;
    while (*src != RULE_DELIM && *src != '\0' && *src != '\n') {
        switch (*src) {
        case '\\':
            // Possibly escaped character
            // We'll fall into the default case anyway; if it's an escaped
            // char, tho, we fall increment src to avoid adding the escape
            // char
            if (*(src+1) == '\\' || *(src+1) == RULE_DELIM || *(src+1) == '\n') {
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

void fast_forward(char **src) {
    while (**src != '\0' && **src != '\n') {
        (*src)++;
    }
    (*src)++;
}

void str_esc(char **src, char *dest) {
    int valid_chars = 0;
    while (**src != RULE_DELIM && **src != '\0' && **src != '\n') {
        switch (**src) {
        case '\\':
            // Possibly escaped character
            // We'll fall into the default case anyway; if it's an escaped
            // char, tho, we fall increment cur to avoid adding the escape
            // char
            if (*(*src+1) == '\\' || *(*src+1) == RULE_DELIM || *(*src+1) == '\n') {
                (*src)++;
            }
            // fall through
        default:
            dest[valid_chars++] = **src;
            break;
        }
        (*src)++;
    }
}

rules_t build_rules(char *rules_txt) {
    rules_t rules;
    init_rules(&rules, 16);

    // Test our arguments for null-ity
    if (!rules_txt) {
        return rules;
    }
    if (!rules.array) {
        // Allocation failure
        return rules;
    }

    // After this, attempt to create a rule for each line in our rules_txt
    char *cur = rules_txt;
    while (*cur != '\0') {
        rule_t cur_rule;

        // Attempt to parse name
        char *end;
        int name_size = field_size(cur, &end);
        // if cur isn't at a delimeter at this point or it's empty, it's invalid
        if (name_size == 0 || *end != RULE_DELIM) {
            fast_forward(&cur);
            continue;
        }
        cur_rule.name = calloc(name_size + 1, sizeof(char));
        if (!cur_rule.name) {
            // Allocation failure
            return rules;
        }
        str_esc(&cur, cur_rule.name);
        cur++;

        int rule_size = field_size(cur, &end);
        if (rule_size == 0 || *end != RULE_DELIM) {
            free(cur_rule.name);
            fast_forward(&cur);
            continue;
        }
        cur_rule.rule = calloc(rule_size + 1, sizeof(char));
        if (!cur_rule.rule) {
            // Allocation failure
            free(cur_rule.name);
            return rules;
        }
        str_esc(&cur, cur_rule.rule);
        cur++;

        int message_size = field_size(cur, &end);
        if (message_size == 0 || *end != RULE_DELIM) {
            // Missing a semicolon; too few sections
            // We continue onwards
            free(cur_rule.rule);
            free(cur_rule.name);
            fast_forward(&cur);
            continue;
        }
        cur_rule.message = calloc(message_size + 1, sizeof(char));
        if (!cur_rule.message) {
            // Allocation failure
            free(cur_rule.rule);
            free(cur_rule.name);
            return rules;
        }
        str_esc(&cur, cur_rule.message);
        cur++;

        int payload_size = field_size(cur, &end);
        if (payload_size == 0 || *end == RULE_DELIM) {
            // Extra semicolon; too many sections
            free(cur_rule.message);
            free(cur_rule.rule);
            free(cur_rule.name);
            fast_forward(&cur);
            continue;
        }
        cur_rule.payload = calloc(payload_size + 1, sizeof(char));
        if (!cur_rule.payload) {
            // Allocation failure
            free(cur_rule.message);
            free(cur_rule.rule);
            free(cur_rule.name);
            return rules;
        }
        str_esc(&cur, cur_rule.payload);
        cur++;

        insert_rule(&rules, cur_rule);
    }

    return rules;
}

void free_rules(rules_t *x) {
    // TODO
    for (size_t i = 0; i < x->used; i++) {
        rule_t c = x->array[i];
        free(c.name);
        free(c.rule);
        free(c.message);
        free(c.payload);
    }
    free(x->array);
    x->used = x->size = 0;
}
