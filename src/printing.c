#include <string.h>
#include <stdio.h>
#include "opening_hours.h"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 2048
#endif

static char result[BUFFER_SIZE] = {0};

char *strdup(const char *);

void print_weeknum(bitset wn) {
	size_t i = 0,
		   set = 0,
		   ever = 0;

	snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "     Weeknums:");
	do {
		if (!set && GET_BIT(wn, i)) {
			set = 1;
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "%s %lu", ever ? "                 " : "  ", i + 1);
		} else if (set && !(GET_BIT(wn, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(wn, i - 2))
				snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - %lu", i);
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		}
	} while (++i < 54);
	if (set)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - 54\n");
	else if (!ever)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "   none\n");
}

void print_hours(time_selector ts) {
	size_t i = 0,
		   set = 0,
		   ever = 0;

	snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "     Hours:");
	do {
		if (!set && GET_BIT(ts.time_range, i)) {
			set = 1;
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "%s %02lu:%02lu", ever ? "                 " : "      ", i / 60, i % 60);
		} else if (set && !(GET_BIT(ts.time_range, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(ts.time_range, i - 2)) {
				snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - %02lu:%02lu", i / 60, i % 60);
			}
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		}
	} while (++i < 24 * 60);
	if (set)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "+\n");
	else if (!ever)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "       none\n");
}

void print_weekday(weekday_selector wd) {
	size_t i = 0,
		   set = 0,
		   ever = 0;

	snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "     Weekdays:");
	do {
		if (!set && GET_BIT(wd.day, i)) {
			set = 1;
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "%s %s", ever ? "                 " : "   ", WEEKDAY_STR[i]);
		} else if (set && !(GET_BIT(wd.day, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(wd.day, i - 2)) {
				snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - %s", WEEKDAY_STR[(i - 1)]);
			}
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		}
	} while (++i < 7);
	if (set)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - %s\n", WEEKDAY_STR[6]);
	else if (!ever)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "    none\n");
}

void print_months(monthday_range md) {
	size_t i = 0,
	       set = 0,
	       ever = 0,
	       from;

	snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "     Monthdays:");
	do {
		if (!set && GET_BIT(md.days, i)) {
			set = 1;
			from = i;
			if (i % 32)
				snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "%s %lu %s", ever ? "                " : " ", i % 32 + 1, MONTHS_STR[i / 32]);
			else
				snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "%s %s", ever ? "                " : " ", MONTHS_STR[i / 32]);
		} else if (set && !(GET_BIT(md.days, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(md.days, i - 2)) {
				int day = (i - 1) % 32 + 2;
				if (day <= NB_DAYS[i / 32])
					snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - %d %s", day > NB_DAYS[i / 32] ? NB_DAYS[i / 32] : day, MONTHS_STR[(i - 1) / 32]);
				else if (strcmp(MONTHS_STR[(i - 1) / 32], MONTHS_STR[from / 32]))
					snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - %s", MONTHS_STR[(i - 1) / 32]);
			} else if (!(i % 32)) {
				snprintf(result + strlen(result), BUFFER_SIZE, " %lu", i % 32);
			}
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		}
	} while (++i < 32 * 12);
	if (set)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - %s\n", MONTHS_STR[11]);
	else if (!ever)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "  none\n");
}

void print_years(bitset years) {
	size_t i = 0,
		   set = 0,
		   ever = 0;

	snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "     Years:");
	do {
		if (!set && GET_BIT(years, i)) {
			set = 1;
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "%s %lu", ever ? "                " : "     ", i + 1900);
		} else if (set && !(GET_BIT(years, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(years, i - 2))
				snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), " - %lu", i + 1899);
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		}
	} while (++i < 1024);
	if (set)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "+\n");
	else if (!ever)
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "      none\n");
}

char *print_oh(opening_hours oh) {
	opening_hours cur = oh;

	result[0] = 0;
	if (!oh) {
		return (NULL);
	}
	if (oh->to_str)
		return (oh->to_str);

	do {
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "-------- SEPARATOR --------\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "  Separator: %d\n", cur->rule.separator);
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "-------- SELECTORS --------\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "  Anyway: %d\n", cur->rule.selector.anyway);
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "  WIDE_RANGE_SELECTOR -----\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "     Type: %d\n", cur->rule.selector.wide_range.type);
		print_years(cur->rule.selector.wide_range.years);
		print_months(cur->rule.selector.wide_range.monthdays);
		print_weeknum(cur->rule.selector.wide_range.weeks);
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "  SMALL_RANGE_SELECTOR ----\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "     Type: %d\n", cur->rule.selector.wide_range.type);
		print_weekday(cur->rule.selector.small_range.weekday);
		print_hours(cur->rule.selector.small_range.hours);
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "--------   STATE   --------\n");
		snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "   That's %s\n\n", cur->rule.state.type == RULE_OPEN ? "open" : "closed");
		if ((cur = cur->next_item))
			snprintf(result + strlen(result), BUFFER_SIZE - strlen(result), "====================================\n\n");
	} while (cur);
	oh->to_str = strdup(result);
	return (oh->to_str);
}
