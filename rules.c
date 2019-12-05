#include <string.h>
#include "hs.h"

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
        if(ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' && ch != '\0')
            return 0;

    } while (ch != '\0');

    return 1;
}

// TODO: Regexes can have spaces tho
// TODO: Regexes can have any delimiter tho
// TODO: Escaping things
// TODO: Dealing with failure cases
rule_t *build_rules(char *rules_txt) {
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

     // After this, we loop through each line, attempting to create a rule for each.
     cur_line = rules_txt;
     unsigned int valid = 0;
     while (cur_line) {
          char *next_line = strchr(cur_line, '\n');
          if (next_line) *next_line = '\0';

          char *name = strtok(cur_line," ");
          char *rule = strtok(NULL," ");
          char *mesg = strtok(NULL," ");
          char *payl = strtok(NULL," ");

          if (payl) {
               rule_t r = {name, rule, mesg, payl};
               rules[valid++] = r;
          }
          
          // We don't set the \0 back to \n so that the payloads are properly
          // terminated.

          cur_line = next_line ? (next_line + 1) : NULL;
     }

     // realloc to correct size
     return rules;
}

hs_error_t build_regex(const char *const *patterns, unsigned int numpats,
                       hs_database_t **db, hs_compile_error_t *err) {
     unsigned int flags[1] = { HS_MODE_BLOCK };
     hs_error_t res = hs_compile_multi(patterns, flags, NULL, numpats, HS_MODE_BLOCK, NULL, db, &err);
     return res;
}



