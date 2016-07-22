#include <time.h>
#include "opening_hours.h"

/*
           struct tm {
               int tm_sec;    Seconds (0-60)
               int tm_min;    Minutes (0-59)
               int tm_hour;   Hours (0-23)
               int tm_mday;   Day of the month (1-31)
               int tm_mon;    Month (0-11)
               int tm_year;   Year - 1900
               int tm_wday;   Day of the week (0-6, Sunday = 0)
               int tm_yday;   Day in the year (0-365, 1 Jan = 0)
               int tm_isdst;  Daylight saving time
           };
*/

int is_open(opening_hours oh, struct tm date) {
	selector_sequence selector = oh->rule.selector;

	return ((selector.anyway
				|| (GET_BIT(selector.wide_range.years, date.tm_year)
					&& GET_BIT(selector.wide_range.monthdays->days, date.tm_mon * 32 + date.tm_mday - 1)
					&& ((GET_BIT(selector.small_range.weekday.range, date.tm_wday)
						&& GET_BIT(selector.small_range.hours.time_range, date.tm_hour * 60 + date.tm_min))
					|| (GET_BIT(selector.small_range.weekday.range, date.tm_wday - 1 < 0 ? 6 : date.tm_wday - 1)
						&& GET_BIT(selector.small_range.hours.extended_time_range, date.tm_hour * 60 + date.tm_min)))))
			&& oh->rule.state.type == RULE_OPEN);
}

