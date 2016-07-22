
#include "dprintf.h"
#include "opening_hours.h"

void print_weeknum(bitset wn) {
	size_t i = 0,
		   set = 0,
		   ever = 0;

	dprintf(2, "     Weeknums:");
	do {
		if (!set && GET_BIT(wn, i)) {
			set = 1;
			dprintf(2, "%s %lu", ever ? "                 " : "  ", i + 1);
		} else if (set && !(GET_BIT(wn, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(wn, i - 2))
				dprintf(2, " - %lu", i);
			dprintf(2, "\n");
		}
	} while (++i < 54);
	if (set)
		dprintf(2, " - 54\n");
	dprintf(2, "\n");
}

void print_hours(time_selector ts) {
	size_t i = 0,
		   set = 0,
		   ever = 0;

	if (!ever)
		dprintf(2, "     Hours:");
	do {
		if (!set && GET_BIT(ts.time_range, i)) {
			set = 1;
			dprintf(2, "%s %02lu:%02lu", ever ? "                 " : "      ", i / 60, i % 60);
		} else if (set && !(GET_BIT(ts.time_range, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(ts.time_range, i - 2)) {
				dprintf(2, " - %02lu:%02lu", i / 60, i % 60);
			}
			dprintf(2, "\n");
		}
	} while (++i < 24 * 60);
	if (set)
		dprintf(2, "+\n");
	dprintf(2, "\n");
}

void print_weekday(weekday_selector wd) {
	size_t i = 0,
		   set = 0,
		   ever = 0;

	if (!ever)
		dprintf(2, "     Weekdays:");
	do {
		if (!set && GET_BIT(wd.day, i)) {
			set = 1;
			dprintf(2, "%s %s", ever ? "                 " : "   ", WEEKDAY_STR[i]);
		} else if (set && !(GET_BIT(wd.day, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(wd.day, i - 2)) {
				dprintf(2, " - %s", WEEKDAY_STR[(i - 1)]);
			}
			dprintf(2, "\n");
		}
	} while (++i < 7);
	if (set)
		dprintf(2, " - %s\n", WEEKDAY_STR[6]);
	dprintf(2, "\n");
}

void print_months(monthday_range md, int ever) {
	size_t i = 0,
		   set = 0;

	if (!ever)
		dprintf(2, "     Monthdays:");
	do {
		if (!set && GET_BIT(md.days, i)) {
			set = 1;
			dprintf(2, "%s %lu %s", ever ? "                " : " ", i % 32 + 1, MONTHS_STR[i / 32]);
		} else if (set && !(GET_BIT(md.days, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(md.days, i - 2)) {
				int day = (i - 1) % 32 + 1;
				dprintf(2, " - %d %s", day > NB_DAYS[(i - 1) / 32] ? NB_DAYS[(i - 1) / 32] : day, MONTHS_STR[(i - 1) / 32]);
			}
		}
	} while (++i < 32 * 12);
	if (set)
		dprintf(2, " - %s\n", MONTHS_STR[11]);
	dprintf(2, "\n");
}

void print_years(bitset years) {
	size_t i = 0,
		   set = 0,
		   ever = 0;

	dprintf(2, "     Years:");
	do {
		if (!set && GET_BIT(years, i)) {
			set = 1;
			dprintf(2, "%s %lu", ever ? "                " : "     ", i + 1900);
		} else if (set && !(GET_BIT(years, i))) {
			set = 0;
			ever = 1;
			if (i > 1 && GET_BIT(years, i - 2))
				dprintf(2, " - %lu", i + 1899);
			dprintf(2, "\n");
		}
	} while (++i < 1024);
	if (set)
		dprintf(2, "+\n");
	dprintf(2, "\n");
}

int print_oh(opening_hours oh) {
	monthday_range *cur;

	if (!oh) {
		dprintf(2, "NULL\n");
		return (1);
	}
	cur = oh->rule.selector.wide_range.monthdays;

	dprintf(2, "\n\n-------- SEPARATOR --------\n");
	dprintf(2, "  Separator: %d\n", oh->rule.separator);
	dprintf(2, "\n");
	dprintf(2, "-------- SELECTORS --------\n\n");
	dprintf(2, "  Anyway: %d\n", oh->rule.selector.anyway);
	dprintf(2, "\n");
	dprintf(2, "  WIDE_RANGE_SELECTOR -----\n\n");
	dprintf(2, "     Type: %d\n\n", oh->rule.selector.wide_range.type);
	print_years(oh->rule.selector.wide_range.years);
	do {
		print_months(*cur, cur != oh->rule.selector.wide_range.monthdays);
	} while ((++cur)->days);
	dprintf(2, "\n");
	print_weeknum(oh->rule.selector.wide_range.weeks);
	dprintf(2, "\n");
	dprintf(2, "  SMALL_RANGE_SELECTOR ----\n\n");
	dprintf(2, "     Type: %d\n\n", oh->rule.selector.wide_range.type);
	print_weekday(oh->rule.selector.small_range.weekday);
	print_hours(oh->rule.selector.small_range.hours);
	dprintf(2, "\n");
	dprintf(2, "--------   STATE   --------\n\n");
	dprintf(2, "   That's %s\n", oh->rule.state.type == RULE_OPEN ? "open" : "closed");
	dprintf(2, "\n");
	return (0);
}
