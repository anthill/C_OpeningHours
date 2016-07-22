#include <strings.h>
#include "parsing.h"

static int get_month_id(char *s) {
	size_t i = 12;

	while (*s == ' ') ++s;
	if (strlen(s) > 2) {
		for (i = 0; i < 12; i++)
			if ((strstr(s, MONTHS_STR[i]) == s) && !isalpha(s[3]))
				break;
	}
	return (i);
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
			if (match(*s + 4, "^ *- *[0-9]{4}([^0-9]|$)")) {
				range[0] = atoi(*s);
				while (isdigit(**s)) ++*s;
				while (**s == ' ') ++*s;
				++*s;
				while (**s == ' ') ++*s;
				range[1] = atoi(*s);
				while (isdigit(**s)) ++*s;
				set_subset(*years, range[0] - 1900, range[1] - 1900, true);
			} else {
				range[1] = range[0] = atoi(*s);
				if (range[1] < 1900) {
					printf("Invalid range: year must be greater than or equal to 1900\n");
					return (ERROR);
				} else if (range[1] > 2923) {
					printf("Invalid range: are you sure somebody still could use your opening hours in %d?\n", range[1]);
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

	*weeks = Bitset(54);
	if (strstr(*s, "week ") != *s) {
		set_subset(*weeks, 0, 52, true);
		return (EMPTY);
	}
	*s += sizeof("week");
	do {
		while (**s == ' ') ++*s;
		if ((weeknum = atoi(*s)) < 1 || weeknum > 54) {
			printf("Invalid syntax: week %d doesn't exist.\n", weeknum);
			return (ERROR);
		}
		SET_BIT(*weeks, weeknum - 1, true);
		while (isdigit(**s)) ++*s;
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

