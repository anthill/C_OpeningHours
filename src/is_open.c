#include "opening_hours.h"

int is_open(opening_hours oh, when date) {
	if (!oh)
		return (0);
	opening_hours cur = oh;

	do {
		selector_sequence selector = cur->rule.selector;

		if (selector.anyway
				|| (GET_BIT(selector.wide_range.years, date.tm_year)
					&& GET_BIT(selector.wide_range.monthdays.days, date.tm_mon * 32 + date.tm_mday - 1)
					&& ((GET_BIT(selector.small_range.weekday.range, date.tm_wday)
						&& GET_BIT(selector.small_range.hours.time_range, date.tm_hour * 60 + date.tm_min))
					|| (GET_BIT(selector.small_range.weekday.range, date.tm_wday - 1 < 0 ? 6 : date.tm_wday - 1)
						&& GET_BIT(selector.small_range.hours.extended_time_range, date.tm_hour * 60 + date.tm_min)))))
			return (cur->rule.state.type == RULE_OPEN);
	} while ((cur = cur->next_item));
	return (0);
}

int is_open_time(opening_hours oh, struct tm date) {
	return (is_open(oh, *((when *)&date.tm_min)));
}

int is_open_expended(opening_hours oh, int min, int hour, int day, int month, int year, int day_of_week) {
	return (is_open(oh, (when){{{min, hour, day, month, year, day_of_week}}}));
}
