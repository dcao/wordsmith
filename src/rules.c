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

char *str_esc(char *src, char *dest) {
    int valid_chars = 0;
    while (*src != ';' && *src != '\0') {
        switch (*src) {
        case '\\':
            // Possibly escaped character
            // We'll fall into the default case anyway; if it's an escaped
            // char, tho, we fall increment cur to avoid adding the escape
            // char
            if (*(src+1) == '\\' || *(src+1) == RULE_DELIM) {
                src++;
            }
            // fall through
        default:
            if (*src != '\0') {
                dest[valid_chars++] = *src;
            } else {
                src--;
            }
            break;
        }
        src++;
    }
    return src;
}

// TODO: Regexes can have spaces tho
// TODO: Regexes can have any delimiter tho
// TODO: Custom parser I guess: https://stackoverflow.com/questions/12568643/strtok-and-escape-characters
// TODO: Escaping things
// TODO: Dealing with failure cases
rule_t *build_rules(char *rules_txt, int *size) {
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
    rule_t *rules = malloc(lines * sizeof(rule_t));

    // After this, attempt to create a rule for each line in our rules_txt

    // pstate_t is the state of the parser; either it's parsing the name, rule,
    // message, or payload.
    char *cur  = rules_txt;
    int valid  = 0;
    int eos = 0;
    while (*cur != '\0') {
        char *endl = strchr(cur, '\n');
        rule_t *cur_rule = &rules[valid];
        if (!endl) {
            endl = strchr(cur, '\0');
            eos = 1;
        }
        // The end of the line is now the end of the string, for parsing
        // purposes
        *endl = '\0';

        // Attempt to parse name
        char *end = strchr(cur, RULE_DELIM);
        if (!end) {
            // Missing a semicolon; too few sections
            // We continue onwards
            *endl = '\n';
            cur = endl + 1;
            if (eos) {
                break;
            }
            continue;
        }
        cur_rule->name = malloc((end - cur) * sizeof(char));
        cur = str_esc(cur, cur_rule->name) + 1;

        end = strchr(cur, RULE_DELIM);
        if (!end) {
            // Missing a semicolon; too few sections
            // We continue onwards
            *endl = '\n';
            cur = endl + 1;
            free(cur_rule->name);
            if (eos) {
                break;
            }
            continue;
        }
        cur_rule->rule = malloc((end - cur) * sizeof(char));
        cur = str_esc(cur, cur_rule->rule) + 1;

        // Attempt to parse mesg
        end = strchr(cur, RULE_DELIM);
        if (!end) {
            // Missing a semicolon; too few sections
            // We continue onwards
            *endl = '\n';
            cur = endl + 1;
            free(cur_rule->rule);
            free(cur_rule->name);
            if (eos) {
                break;
            }
            continue;
        }
        cur_rule->message = malloc((end - cur) * sizeof(char));
        cur = str_esc(cur, cur_rule->message) + 1;

        // Attempt to parse payl
        end = strchr(cur, RULE_DELIM);
        // This time, we don't want too many fields, so if the
        // delimeter exists, it's now an error case.
        if (end) {
            // Missing a semicolon; too few sections
            // We continue onwards
            *endl = '\n';
            cur = endl + 1;
            free(cur_rule->message);
            free(cur_rule->rule);
            free(cur_rule->name);
            if (eos) {
                break;
            }
            continue;
        }
        // We use endl instead of end, since we're going to the end of the line,
        // not to a delim
        cur_rule->payload = malloc((endl - cur) * sizeof(char));
        str_esc(cur, cur_rule->payload);
        valid++;
        if (eos) {
            break;
        } else {
            *endl = '\n';
            cur = endl + 1;
        }
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
