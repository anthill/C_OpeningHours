#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "opening_hours.h"

# define REG_COMPILE(regex, pattern, opt)  ({ \
		if (regcomp(&regex, pattern, opt) < 0) \
			printf("%s failed in %s:%d", __func__, __FILE__, __LINE__); \
	})

# define ERROR false
# define SUCCESS true
# define EMPTY -1

# define MONTHS_STR      ((char [][4]){"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"})
# define MONTHS_FULLSTR  ((char [][10]){\
		"january", \
		"february", \
		"march", \
		"april", \
		"may", \
		"june", \
		"july", \
		"august", \
		"september", \
		"october", \
		"november", \
		"december" \
	})
# define NB_DAYS         ((int []){31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31})

opening_hours build(char *);

#ifdef STANDALONE
int main(int ac, char **av) {
	if (ac > 1)
		build(av[1]);
	return (0);
}
#endif /* !STANDALONE */

static char *pos;

int get_month_id(char *s) {
	size_t i = 12;
	if (strlen(s) > 2) {
		for (i = 0; i < 12; i++)
			if ((strstr(s, MONTHS_STR[i]) == s) && !isalpha(s[3]))
				break;
	}
	return (i);
}

int match(char *s, char *pattern) {
	regex_t reg;
	regmatch_t matched;
	bool res;

	REG_COMPILE(reg, pattern, REG_EXTENDED);
	res = regexec(&reg, s, 1, &matched, 0) != REG_NOMATCH;
	regfree(&reg);
	return (res);
}

int parse_year_range(bitset *years, char **s) {
	u_int range[2] = {1900, 2923};

	while (**s == ' ') ++*s;

	if (strstr(*s, ",") == *s) {
		printf("Syntax error: empty element at list of ranges. Expected value before coma.\n");
		return (ERROR);
	}

	*years = Bitset(1024);
	do {
		if (match(*s, "^[0-9]{4}[^0-9]")) {
			if (match(*s + 4, "^.*-.*[0-9]{4}[^0-9]")) {
				range[0] = atoi(*s);
				while (**s == ' ') ++*s;
				++*s;
				while (**s == ' ') ++*s;
				range[1] = atoi(*s);
				set_subset(*years, range[0] - 1900, range[1] - 1900, true);
			} else {
				range[1] = range[0] = atoi(*s);
				*s += 4;
				SET_BIT(*years, range[0] - 1900, true);
			}
		} else {
			set_subset(*years, range[0] - 1900, range[1] - 1900, true);
			break;
		}
	} while (strstr(*s, ",") == *s && *(++*s));
	return (SUCCESS);
}

int parse_monthday_range(monthday_range **monthdays, char **s) {
	monthday_range* current;
	size_t nb_ranges = 0;
	int month_id, month_to,
	    daynum = 0, dayto = 0;

	while (**s == ' ') ++*s;

	if (!isdigit(**s) && get_month_id(*s) == 12)
		return (EMPTY);
	do {
		if (!(*monthdays = realloc(*monthdays, ++nb_ranges * sizeof(*current)))) {
			printf("FATAL ERROR: cannot allocate %lu bytes!\nMaybe memory is full?", nb_ranges * sizeof(*current));
			exit(2);
		}
		current = *monthdays + nb_ranges - 1;
		bzero(current, sizeof(*current));
		current->days = Bitset(12 * 32);
		while (**s == ' ') ++*s;
		if (strstr(*s, "easter") == *s) {
			*s += sizeof("easter");
			while (**s == ' ') ++*s;
			current->easter = true;
			if (**s == '-') {
				printf("Unsupported syntax: ranges including easter aren't allowed here, aborting.\n");
				return (ERROR);
			}
			continue;
		}
		month_id = get_month_id(*s);
		if (month_id == 12) {
			printf("Syntax error: expected month in the monthday_range.\n");
			return (ERROR);
		}
		*s += 4;
		while (**s == ' ') ++*s;
		if ((dayto = daynum = atoi(*s))) {
			if (daynum > NB_DAYS[month_id]) {
				printf("Syntax error: day %d doesn't exist for %s.\n", daynum, MONTHS_FULLSTR[month_id]);
				return (ERROR);
			}
			while (isdigit(**s)) ++*s;
		}
		while (**s == ' ') ++*s;
		if (**s == '-') {
			++*s;
			while (**s == ' ') ++*s;
			if (strstr(*s, "easter") == *s) {
				printf("Unsupported syntax: ranges including easter aren't allowed here, aborting.\n");
				return (ERROR);
			}
			if ((month_to = get_month_id(*s)) == 12) {
				printf("Syntax error: month range enclosed without new month. Aborting.\n");
				return (ERROR);
			}
			*s += 4;
			while (**s == ' ') ++*s;
			if ((dayto = atoi(*s))) {
				if (dayto > NB_DAYS[month_to]) {
					printf("Syntax error: day %d doesn't exist for %s.\n", daynum, MONTHS_FULLSTR[month_id]);
					return (ERROR);
				}
				while (isdigit(**s)) ++*s;
			} else {
				dayto = 31;
			}
			daynum = !daynum ? 1 : daynum;
			if (month_to > month_id) {
				set_subset(current->days, month_id * 32 + daynum - 1, month_to * 32 + dayto - 1, true);
			} else if (month_to == month_id && dayto < daynum) {
				set_subset(current->days, 0, 12 * 32, true);
				set_subset(current->days, month_to * 32 + dayto, month_id * 32 + daynum - 2, false);
			}
		} else {
			if (!daynum)
				set_subset(current->days, month_id * 32, month_id * 32 + 30, true);
			else
				SET_BIT(current->days, month_id * 32 + daynum - 1, true);
		}
	} while (strstr(*s, ",") == *s && *(++*s));
	if (!(*monthdays = realloc(*monthdays, (nb_ranges + 1) * sizeof(*current)))) {
		printf("FATAL ERROR: cannot allocate %lu bytes!\nMaybe memory is full?", nb_ranges * sizeof(*current));
		exit(2);
	}
	bzero(*monthdays + nb_ranges, sizeof(*current));
	return (SUCCESS);
}

int parse_week_selector(bitset *weeks, char **s) {
	int weeknum;

	while (**s == ' ') ++*s;

	*weeks = Bitset(53);
	if (strstr(*s, "week ") != *s)
		return (EMPTY);
	do {
		while (**s == ' ') ++*s;
		if ((weeknum = atoi(*s)) < 1 || weeknum > 53) {
			printf("Syntax error: week %d doesn't exist.\n", weeknum);
			return (ERROR);
		}
		SET_BIT(*weeks, weeknum - 1, true);
	} while (strstr(*s, ",") == *s && *(++*s));
	return (SUCCESS);
}

int parse_wide_range_selector(wide_range_selector *selector, char **s) {
	while (**s == ' ') ++*s;

	if (**s == '"') {
		selector->type = WIDE_RANGE_COMMENT;
		if (!strstr(*s + 1, "\"")) {
			printf("Invalid syntax: unclosed quote for comment as selector.\n");
			return (ERROR);
		} else if (strstr(*s + 1, "\"")[1] != ':') {
			printf("Invalid syntax: missing colon right after enclosing quote for the selector.\n");
			return (ERROR);
		}
		strncpy(selector->comment, *s + 1, strstr(*s + 1, "\":") - *s - 2);
		return (SUCCESS);
	}
	parse_year_range(&selector->years, s);
	parse_monthday_range(&selector->monthdays, s);
	parse_week_selector(&selector->weeks, s);
	return (SUCCESS);
}

int parse_small_range_selector(small_range_selector *selector, char **s) {
	(void)selector;
	(void)s;
	return (SUCCESS);
}

int parse_selector_sequence(selector_sequence *seq, char **s) {
	int wide_res, small_res;

	while (**s == ' ') ++*s;

	if (strstr(*s, "24/7 ") == *s) {
		seq->anyway = true;
		*s += sizeof("24/7 ");
		return (SUCCESS);
	}

	wide_res  = parse_wide_range_selector(&seq->wide_range, s);
	small_res = parse_small_range_selector(&seq->small_range, s);
	if (wide_res == small_res && wide_res == EMPTY)
		seq->anyway = true;
	return (SUCCESS);
}

int parse_rule_modifier(rule_modifier *rule, char **s) {
	regex_t reg_comment;
	regmatch_t match_comment;

	while (**s == ' ') ++*s;

	if (strstr(*s, "open") == *s) rule->type = RULE_OPEN, *s += sizeof("open");
	else if (strstr(*s, "closed") == *s) rule->type = RULE_CLOSED, *s += sizeof("closed");
	else if (strstr(*s, "off") == *s) rule->type = RULE_CLOSED, *s += sizeof("off");
	else if (strstr(*s, "unknown") == *s) rule->type = RULE_UNKNOWN, *s += sizeof("unknown");

	while (**s == ' ') ++*s;
	REG_COMPILE(reg_comment, "\"[^\"]+\"", REG_EXTENDED);
	if (regexec(&reg_comment, *s, 1, &match_comment, 0) != REG_NOMATCH) {
		strncpy(rule->comment, *s + match_comment.rm_so + 1, match_comment.rm_eo - match_comment.rm_so - 2);
	} else if (rule->type == RULE_COMMENT) {
		regfree(&reg_comment);
		printf("Invalid syntax: expected at least a comment for the rule modifier.\n");
		pos = *s;
		return (ERROR);
	}
	regfree(&reg_comment);
	return (SUCCESS);
}

int parse_rule_sequence(rule_sequence *seq, char **s) {
	while (**s == ' ') ++*s;

	if (!seq->separator) {
		if (strstr(*s, ",") == *s)  seq->separator = SEP_COMA, ++*s;
		if (strstr(*s, ";") == *s)  seq->separator = SEP_SEMICOLON, ++*s;
		if (strstr(*s, "||") == *s) seq->separator = SEP_FALLBACK, *s += 2;
	}
	if (parse_selector_sequence(&seq->selector, s) == ERROR)
		return (ERROR);
	if (parse_rule_modifier(&seq->state, s) == ERROR)
		return (ERROR);
	return (SUCCESS);
}

void free_oh(opening_hours oh) {
	free(oh);
}

opening_hours build(char *s) {
	opening_hours oh = calloc(1, sizeof(*oh)),
		      cur = oh,
		      prev = NULL;
	int it = 0;

	pos = s;
	oh->rule.separator = SEP_HEAD;
	do {
		if (it++) {
			prev = cur;
			cur = (cur->next_item = calloc(1, sizeof(*oh)));
		}
		if (parse_rule_sequence(&cur->rule, &s) == ERROR) {
			free_oh(oh);
			if (prev)
				prev->next_item = NULL;
			break;
		}
	} while (!*s);
	return (oh);
}
