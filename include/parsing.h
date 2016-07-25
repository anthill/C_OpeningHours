#ifndef PARSING_H_

# include <ctype.h>
# include <string.h>
# include <stdio.h>
# include <regex.h>
# include "dprintf.h"
# include "opening_hours.h"

/*
 * Defining possible return codes of parsing functions
 *
 * A parsing function, which should return an int, must:
 *   - return ERROR only where the pattern isn't what we expected.
 *   - return SUCCESS when the function parsed something correct.
 *   - return EMPTY when the concerned parse sequence could be empty, and is empty.
 *
 */

# define ERROR 0
# define SUCCESS 1
# define EMPTY 2

/* Error check for compiling regexs: */
# define REG_COMPILE(regex, pattern, opt)  ({ \
		if (regcomp(&regex, pattern, opt) < 0) { \
			dprintf(2, "%s failed in %s:%d", __func__, __FILE__, __LINE__); \
			dprintf(2, "Regex %s cannot be compiled; aborting.\n", pattern); \
			exit(2); \
		} \
	})

/*
 * Functions:
 */

char *set_cursor(int, char *);
char *set_cursor(int, char *);
int match(char *, char *);
int parse_monthday_range(monthday_range *, char **);
int parse_rule_modifier(rule_modifier *, char **);
int parse_rule_sequence(rule_sequence *, char **);
int parse_selector_sequence(selector_sequence *, char **);
int parse_small_range_selector(small_range_selector *, char **);
int parse_time_selector(time_selector *, char **);
int parse_week_selector(bitset *, char **);
int parse_weekday_selector(weekday_selector *, char **);
int parse_wide_range_selector(wide_range_selector *, char **);
int parse_year_range(bitset *, char **);

#endif /* PARSING_H_ */
