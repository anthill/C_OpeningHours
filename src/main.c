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

# define ERROR 0
# define SUCCESS 1
# define EMPTY 2

# define WEEKDAY_STR      ((char [][3]){"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"})
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

int get_weekday_id(char *s) {
	size_t i = 7;

	while (*s == ' ') ++s;
	if (s[1]) {
		for (i = 0; i < 7; i++)
			if ((strstr(s, WEEKDAY_STR[i]) == s) && !isalpha(s[2]))
				break;
	}
	return (i);
}

int get_month_id(char *s) {
	size_t i = 12;

	while (*s == ' ') ++s;
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
		printf("Invalid syntax: empty element at list of ranges. Expected value before coma.\n");
		return (ERROR);
	}

	*years = Bitset(1024);
	do {
		if (match(*s, "^[0-9]{4}([^0-9]|$)")) {
			if (match(*s + 4, "^.*-.*[0-9]{4}([^0-9]|$)")) {
				range[0] = atoi(*s);
				while (**s == ' ') ++*s;
				++*s;
				while (**s == ' ') ++*s;
				range[1] = atoi(*s);
				set_subset(*years, range[0] - 1900, range[1] - 1900, true);
			} else {
				range[1] = range[0] = atoi(*s);
				if (range[1] < 1900) {
					printf("Invalid range: year must be greater than or equal to 1900\n");
					return (ERROR);
				} else if (range[1] > 2923) {
					printf("Invalid range: are you sure somebody still could use your opening hours\n              in the year %d?\n", range[1]);
					return (ERROR);
				}
				*s += 4;
				SET_BIT(*years, range[0] - 1900, true);
			}
		} else {
			set_subset(*years, range[0] - 1900, range[1] - 1900, true);
			return (EMPTY);
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

	if (!(*monthdays = calloc(++nb_ranges, sizeof(*current)))) {
		printf("FATAL ERROR: cannot allocate %lu bytes!\nMaybe memory is full?", nb_ranges * sizeof(*current));
		exit(2);
	}
	if (get_month_id(*s) == 12) {
		if (!(*monthdays = realloc(*monthdays, (nb_ranges + 1) * sizeof(*current)))) {
			printf("FATAL ERROR: cannot allocate %lu bytes!\nMaybe memory is full?", nb_ranges * sizeof(*current));
			exit(2);
		}
		(*monthdays)->days = Bitset(12 * 32);
		set_subset((*monthdays)->days, 0, 12 * 32, true);
		bzero(*monthdays + nb_ranges, sizeof(*current));
		return (EMPTY);
	}
	do {
		current = *monthdays + nb_ranges - 1;
		bzero(current, sizeof(*current));
		current->days = Bitset(12 * 32);
		while (**s == ' ') ++*s;
		if (strstr(*s, "easter ") == *s) {
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
			printf("Invalid syntax: expected month in the monthday_range.\n");
			return (ERROR);
		}
		*s += 3;
		while (**s == ' ') ++*s;
		if ((dayto = daynum = atoi(*s))) {
			if (daynum > NB_DAYS[month_id]) {
				printf("Invalid range: day %d doesn't exist for %s.\n", daynum, MONTHS_FULLSTR[month_id]);
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
				printf("Invalid syntax: month range enclosed without new month. Aborting.\n");
				return (ERROR);
			}
			*s += 3;
			while (**s == ' ') ++*s;
			if ((dayto = atoi(*s))) {
				if (dayto > NB_DAYS[month_to]) {
					printf("Invalid range: day %d doesn't exist for %s.\n", daynum, MONTHS_FULLSTR[month_id]);
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
			while (isdigit(**s)) ++*s;
		} else {
			if (!daynum)
				set_subset(current->days, month_id * 32, month_id * 32 + 30, true);
			else
				SET_BIT(current->days, month_id * 32 + daynum - 1, true);
		}
		if (!(*monthdays = realloc(*monthdays, ++nb_ranges * sizeof(*current)))) {
			printf("FATAL ERROR: cannot allocate %lu bytes!\nMaybe memory is full?", nb_ranges * sizeof(*current));
			exit(2);
		}

	} while (strstr(*s, ",") == *s && *(++*s));
	bzero(*monthdays + nb_ranges - 1, sizeof(*current));
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
			printf("Invalid syntax: week %d doesn't exist.\n", weeknum);
			return (ERROR);
		}
		SET_BIT(*weeks, weeknum - 1, true);
	} while (strstr(*s, ",") == *s && *(++*s));
	return (SUCCESS);
}

int parse_wide_range_selector(wide_range_selector *selector, char **s) {
	int year_res, monthday_res, week_res;

	while (**s == ' ') ++*s;

	if (**s == '"') {
		selector->type = WIDE_RANGE_COMMENT;
		if (!strstr(*s + 1, "\"")) {
			printf("Invalid syntax: unclosed quote for comment as selector.\n");
			return (ERROR);
		} else if (!match(*s, "[^\"]*\" *:")) {
			*s = strstr(*s + 1, "\"") + 1;
			printf("Invalid syntax: missing colon right after enclosing quote for the selector.\n");
			return (ERROR);
		} else if ((*s)[1] == '"') {
			++*s;
			printf("Invalid syntax: empty comment.\n");
			return (ERROR);
		}
		strncpy(selector->comment, *s + 1, strstr(*s + 1, "\"") - *s - 1);
		*s = strstr(*s + 1, ":") + 1;
		return (SUCCESS);
	}
	if ((year_res = parse_year_range(&selector->years, s)) == ERROR)
		return (ERROR);
	if ((monthday_res = parse_monthday_range(&selector->monthdays, s)) == ERROR)
		return (ERROR);
	if ((week_res = parse_week_selector(&selector->weeks, s)) == ERROR)
		return (ERROR);
	if (year_res == EMPTY
			&& monthday_res == EMPTY
			&& week_res == EMPTY) {
		while (**s == ' ') ++*s;
		if (strstr(*s, ":") == *s) {
			printf("Invalid syntax: empty wide range selector.\n");
			return (ERROR);
		}
		return (EMPTY);
	}
	if (strstr(*s, ":") == *s) ++*s;
	return (SUCCESS);
}

int parse_weekday_selector(weekday_selector *selector, char **s) {
	char sep_char = 0,
		 weekday_id, weekday_to;

	selector->range = Bitset(7);

	do {
		while (**s == ' ') ++*s;
		if (strstr(*s, "SH ") == *s) {
			*s += sizeof("SH");
			if (**s != ' ' && **s != ',' && **s) {
				printf("Invalid syntax: if you want to select a single day holiday, you need\n                to put a space or a coma.\n");
				return (ERROR);
			}
			if ((sep_char = **s))
				++*s;
			selector->single_day_holiday = true;
		}
		while (**s == ' ') ++*s;
		if (strstr(*s, "PH ") == *s) {
			*s += sizeof("PH");
			if (**s != ' ' && **s != ',' && **s) {
				printf("Invalid syntax: if you want to select a plural day holiday, you need\n                to put a space or a coma.\n");
				return (ERROR);
			}
			if ((sep_char = **s))
				++*s;
			selector->plural_day_holiday = true;
		}
		if ((weekday_id = get_weekday_id(*s)) == 7) {
			if (sep_char == ',') {
				--*s;
				printf("Invalid selector: expected weekday.\n");
				return (ERROR);
			}
			set_subset(selector->range, 0, 6, true);
			return (EMPTY);
		}
		while (**s == ' ') ++*s;
		*s += 2;
		if (**s == '-') {
			++*s;
			while (**s == ' ') ++*s;
			if ((weekday_to = get_weekday_id(*s)) == 7) {
				printf("Invalid range: weekday range not enclosed by another weekday.\n");
				return (ERROR);
			}
			if (weekday_id < weekday_to)
				set_subset(selector->range, weekday_id, weekday_to, true);
			else {
				set_subset(selector->range, 0, 6, true);
				set_subset(selector->range, weekday_to + 1, weekday_id - 1, false);
			}
			*s += 2;
		} else {
			SET_BIT(selector->day, weekday_id, true);
			while (**s == ' ') ++*s;
			if (**s == '[') {
				while (**s == ' ') ++*s;
				if (**s < '1' || **s > '5') {
					printf("Invalid syntax: expected value between 1 and 5 included.\n               Expected nth of month selector.\n");
					return (ERROR);
				}
				selector->type = WD_NTH_OF_MONTH;
				selector->nth_of_month = **s - '0';
				++*s;
				while (**s == ' ') ++*s;
				if (**s != ']') {
					printf("Invalid syntax: unenclosed bracket. Expected ']' to enclose nth of month selector.\n");
					return (ERROR);
				}
			}
			while (**s == ' ') ++*s;
			if (**s == '-') {
				printf("Invalid syntax: unexpected token '-'. Cannot set a range involving nth of month.\n");
				return (ERROR);
			}
		}
		while (**s == ' ') ++*s;
	} while (**s == ',' && *(++*s));
	return (SUCCESS);
}

int parse_time_selector(time_selector *selector, char **s) {
	int hours_from = 0, hours_to = 0,
		mins_from = 0, mins_to = 0,
		extended_hour;
	char hourmin_sep;

	selector->time_range = Bitset(60 * 24);
	selector->extended_time_range = Bitset(60 * 24);

	do {
		while (**s == ' ') ++*s;
		if (!isdigit(**s)) {
			if (!(hours_from | hours_to | mins_from | mins_to))
				return (EMPTY);
			printf("Invalid syntax: unexpected token.\n");
			return (ERROR);
		}
		if ((hours_from = atoi(*s)) > 23) {
			printf("Invalid range: are you really sure that such an hour does exist?\n");
			return (ERROR);
		}
		while (isdigit(**s)) ++*s;
		if ((hourmin_sep = **s) != ':' && **s != 'h') {
			printf("Invalid syntax: unexpected token '%c'.\n                Only ':' and 'h' are allowed to separate hours from their minutes.\n", **s);
			return (ERROR);
		}
		++*s;
		while (**s == ' ') ++*s;
		if (!isdigit(**s) && hourmin_sep != 'h') {
			printf("Invalid syntax: expected number of minutes.\n");
			return (ERROR);
		}
		if ((mins_from = atoi(*s)) > 59) {
			printf("Invalid range: are you really sure that such a minute does exist in an hour?\n");
			return (ERROR);
		}
		while (isdigit(**s)) ++*s;
		while (**s == ' ') ++*s;
		if (**s != '-') {
			printf("Invalid syntax: expected range, separated by '-' token.\n");
			return (ERROR);
		}
		++*s;
		while (**s == ' ') ++*s;
		if (!isdigit(**s)) {
			printf("Invalid syntax: expected enclosing range hour.\n");
			return (ERROR);
		}
		if ((hours_to = atoi(*s)) > 47) {
			printf("Invalid range: the enclosing range hour need to be less than 48 (extended time).\n");
			return (ERROR);
		}
		while (isdigit(**s)) ++*s;
		if ((hourmin_sep = **s) != ':' && **s != 'h') {
			printf("Invalid syntax: unexpected token '%c'.\n                Only ':' and 'h' are allowed to separate hours from their minutes.", **s);
			return (ERROR);
		}
		++*s;
		while (**s == ' ') ++*s;
		if (!isdigit(**s) && hourmin_sep != 'h') {
			printf("Invalid syntax: expected number of minutes.\n");
			return (ERROR);
		}
		if ((mins_to = atoi(*s)) > 59) {
			printf("Invalid range: are you really sure that such a minute does exist in an hour?\n");
			return (ERROR);
		}
		if (hours_to < hours_from || (hours_to == hours_from && mins_to <= mins_from)) {
			printf("Invalid range: the enclosing range hour needs to be greater than the opening hour.\n               If you want to mean the tomorrow's hour, please use the extended time syntax.\n               For this purpose, you can specify an enclosing range hour greater than 23.\n");
			while (**s != '-') --*s;
			while (!isdigit(**s)) ++*s;
			return (ERROR);
		}
		set_subset(selector->time_range, hours_from * 60 + mins_from, hours_to * 60 + mins_to - 1, true);
		if ((extended_hour = hours_to * 60 + mins_to - 24 * 60) > 0)
			set_subset(selector->extended_time_range, 0, extended_hour, true);
		while (isdigit(**s)) ++*s;
		while (**s == ' ') ++*s;
	} while (**s == ',' && *(++*s));
	return (SUCCESS);
}

int parse_small_range_selector(small_range_selector *selector, char **s) {
	int res_weekday, res_time;

	while (**s == ' ') ++*s;

	if ((res_weekday = parse_weekday_selector(&selector->weekday, s)) == ERROR)
		return (ERROR);
	if ((res_time = parse_time_selector(&selector->hours, s)) == ERROR)
		return (ERROR);
	return (res_weekday == res_time && res_weekday == EMPTY ? EMPTY : SUCCESS);
}

int parse_selector_sequence(selector_sequence *seq, char **s) {
	int wide_res, small_res;

	while (**s == ' ') ++*s;

	if (strstr(*s, "24/7 ") == *s) {
		seq->anyway = true;
		*s += sizeof("24/7");
		return (SUCCESS);
	}

	wide_res  = parse_wide_range_selector(&seq->wide_range, s);
	if (wide_res == ERROR)
		return (ERROR);
	small_res = parse_small_range_selector(&seq->small_range, s);
	if (small_res == ERROR)
		return (ERROR);
	if (wide_res == small_res && wide_res == EMPTY) {
		seq->anyway = true;
		return (EMPTY);
	}
	return (SUCCESS);
}

int parse_rule_modifier(rule_modifier *rule, char **s) {
	regex_t reg_comment;
	regmatch_t match_comment;

	while (**s == ' ') ++*s;

	REG_COMPILE(reg_comment, "^\"[^\"]*\"", REG_EXTENDED);

	if (strstr(*s, "open") == *s) rule->type = RULE_OPEN, *s += sizeof("open") - 1;
	else if (strstr(*s, "closed") == *s) rule->type = RULE_CLOSED, *s += sizeof("closed") - 1;
	else if (strstr(*s, "off") == *s) rule->type = RULE_CLOSED, *s += sizeof("off") - 1;
	else if (strstr(*s, "unknown") == *s) rule->type = RULE_UNKNOWN, *s += sizeof("unknown") - 1;
	else if (regexec(&reg_comment, *s, 1, &match_comment, 0) != REG_NOMATCH) {
		if ((*s)[1] == '"') {
			++*s;
			printf("Invalid syntax: empty comment.\n");
			return (ERROR);
		}
		strncpy(rule->comment, *s + match_comment.rm_so + 1, match_comment.rm_eo - match_comment.rm_so - 2);
		*s += match_comment.rm_so;
	} else if (isalpha(**s)) {
		printf("Invalid syntax: invalid rule modifier.\n");
		return (ERROR);
	}
	regfree(&reg_comment);
	while (**s == ' ') ++*s;
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

char *set_cursor(int pos, char *str) {
	memset(str, ' ', pos);
	str[pos] = '^';
	str[pos + 1] = 0;
	return (str);
}

opening_hours build(char *s) {
	opening_hours oh = calloc(1, sizeof(*oh)),
		      cur = oh,
		      prev = NULL;
	int it = 0;
	char *entire_string = s,
	     cursor_str[strlen(s) * 2 + 1];

	pos = s;
	oh->rule.separator = SEP_HEAD;
	do {
		if (it++) {
			prev = cur;
			cur = (cur->next_item = calloc(1, sizeof(*oh)));
		}
		if (parse_rule_sequence(&cur->rule, &s) == ERROR) {
			printf("\n%s\n%s\n", entire_string, set_cursor(s - entire_string, cursor_str));
			free_oh(oh);
			if (prev)
				prev->next_item = NULL;
			break;
		} else {
			printf("ok");
		}
	} while (*s && *++s);
	return (oh);
}
