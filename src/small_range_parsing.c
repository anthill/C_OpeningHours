#include "parsing.h"

static int get_weekday_id(char *s) {
	size_t i = 7;

	while (*s == ' ') ++s;
	if (*s && s[1]) {
		for (i = 0; i < 7; i++)
			if ((strstr(s, WEEKDAY_STR[i]) == s) && !isalpha(s[2]))
				break;
	}
	return (i);
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
		while (**s == ' ') ++*s;
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
			if (!(hours_from | hours_to | mins_from | mins_to)) {
				set_subset(selector->time_range, 0, 24 * 60, true);
				return (EMPTY);
			}
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
		if (**s == '+') {
			++*s;
			hours_to = 24;
			mins_to = 0;
		} else {
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

