#include <stdlib.h>
#include <string.h>

#define RULE_DELIM ';'

typedef struct {
    char *name;
    char *rule;
    char *message;
    char *payload;
} rule_t;

/**
 * Checks, whether a given string is empty or not.
 * A string is empty if it only contains white space
 * characters.
 * 
 * Returns 1 if given string is empty otherwise 0.
 */
int is_empty(const char *str) {
    char ch;

    do {
        ch = *(str++);

        // Check non whitespace character
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' && ch != '\0')
            return 0;

    } while (ch != '\0');

    return 1;
}

int field_size(char *src, char **end) {
    int valid = 0;
    while (*src != ';' && *src != '\0' && *src != '\n') {
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

char *fast_forward(char *src) {
    while (*src != '\0' && *src != '\n') {
        src++;
    }
    return ++src;
}

char *str_esc(char *src, char *dest) {
    int valid_chars = 0;
    while (*src != ';' && *src != '\0' && *src != '\n') {
        switch (*src) {
        case '\\':
            // Possibly escaped character
            // We'll fall into the default case anyway; if it's an escaped
            // char, tho, we fall increment cur to avoid adding the escape
            // char
            if (*(src+1) == '\\' || *(src+1) == RULE_DELIM || *(src+1) == '\n') {
                src++;
            }
            // fall through
        default:
            dest[valid_chars++] = *src;
            break;
        }
        src++;
    }
    return src;
}

rule_t *build_rules(char *rules_txt, int *size) {
    // Test our arguments for null-ity
    if (!rules_txt || !size) {
        return NULL;
    }
    
    // We first want to count the number of lines in the text; this corresponds
    // to how many rules we'll have. We filter out blank lines.
    unsigned int lines = 0;

    char *cur_line = rules_txt;
    while (cur_line) {
        char *next_line = strchr(cur_line, '\n');
        if (next_line) *next_line = '\0';
        if (!is_empty(cur_line)) lines++;
        if (next_line) *next_line = '\n';
        cur_line = next_line ? (next_line + 1) : NULL;
    }

    // We then create our array of rules.
    rule_t *rules = calloc(lines, sizeof(rule_t));
    if (!rules) {
        // Allocation failure
        return NULL;
    }

    // After this, attempt to create a rule for each line in our rules_txt
    char *cur = NULL;
    cur = rules_txt;
    int valid  = 0;
    while (*cur != '\0') {
        rule_t *cur_rule = &rules[valid];

        // Attempt to parse name
        char *end;
        int name_size = field_size(cur, &end);
        // if cur isn't at a delimeter at this point or it's empty, it's invalid
        if (name_size == 0 || *end != RULE_DELIM) {
            cur = fast_forward(cur);
            continue;
        }
        cur_rule->name = calloc(name_size + 1, sizeof(char));
        if (!cur_rule->name) {
            // Allocation failure
            return NULL;
        }
        cur = str_esc(cur, cur_rule->name);
        cur++;

        int rule_size = field_size(cur, &end);
        if (rule_size == 0 || *end != RULE_DELIM) {
            free(cur_rule->name);
            cur = fast_forward(cur);
            continue;
        }
        cur_rule->rule = calloc(rule_size + 1, sizeof(char));
        if (!cur_rule->rule) {
            // Allocation failure
            return NULL;
        }
        cur = str_esc(cur, cur_rule->rule);
        cur++;

        int message_size = field_size(cur, &end);
        if (message_size == 0 || *end != RULE_DELIM) {
            // Missing a semicolon; too few sections
            // We continue onwards
            free(cur_rule->rule);
            free(cur_rule->name);
            cur = fast_forward(cur);
            continue;
        }
        cur_rule->message = calloc(message_size + 1, sizeof(char));
        if (!cur_rule->message) {
            // Allocation failure
            return NULL;
        }
        cur = str_esc(cur, cur_rule->message);
        cur++;

        int payload_size = field_size(cur, &end);
        if (payload_size == 0 || *end == RULE_DELIM) {
            // Extra semicolon; too many sections
            free(cur_rule->message);
            free(cur_rule->rule);
            free(cur_rule->name);
            cur = fast_forward(cur);
            continue;
        }
        cur_rule->payload = calloc(payload_size + 1, sizeof(char));
        if (!cur_rule->payload) {
            // Allocation failure
            return NULL;
        }
        cur = str_esc(cur, cur_rule->payload);
        cur++;

        valid++;
    }

    *size = valid;

    // We skip reallocing because we've already passed on the correct size of
    // the array

    return rules;
}

void free_rules(rule_t *x, int cnt) {
    // TODO
    for (int i = 0; i < cnt; i++) {
        rule_t *c = &x[i];
        free(c->name);
        free(c->rule);
        free(c->message);
        free(c->payload);
    }
    free(x);
}
