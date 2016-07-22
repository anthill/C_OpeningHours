#ifndef OPENING_HOURS_H_
# define OPENING_HOURS_H_

# include "bitset.h"

/*
 * Defines needed for parsing and restitution of the format:
 */

# define WEEKDAY_STR      ((char [][3]){"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"})
# define MONTHS_STR      ((char [][4]){"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"})
# define NB_DAYS         ((int []){31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31})
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


# define COMMENT_SIZE 128

/*
 * Typedefs:
 */

typedef struct monthday_range monthday_range;
typedef struct opening_hours* opening_hours;
typedef struct rule_sequence rule_sequence;
typedef struct selector_sequence selector_sequence;
typedef struct small_range_selector small_range_selector;
typedef struct time_selector time_selector;
typedef struct weekday_selector weekday_selector;
typedef struct wide_range_selector wide_range_selector;
typedef struct year_range year_range;
typedef struct year_selector year_selector;
typedef struct rule_modifier rule_modifier;

typedef enum rule_separator rule_separator;
typedef enum rule_modifier_type rule_modifier_type;
typedef enum wide_range_selector_type wide_range_selector_type;
typedef enum weekday_selector_type weekday_selector_type;

/*
 * Types declarations:
 */

enum rule_modifier_type {
	RULE_OPEN = 0,
	RULE_CLOSED = 1,
	RULE_COMMENT = 2,
	RULE_UNKNOWN
};

enum weekday_selector_type {
	WD_RANGE = 0,
	WD_NTH_OF_MONTH
};

enum rule_separator {
	SEP_NOT_SET = 0,
	SEP_HEAD,
	SEP_SEMICOLON,
	SEP_COMA,
	SEP_FALLBACK
};

enum wide_range_selector_type {
	WIDE_RANGE_DATE = 0,
	WIDE_RANGE_COMMENT
};

/*
 * Structures:
 */

struct monthday_range {
	bitset days;
	bool easter;
};

struct wide_range_selector {
	wide_range_selector_type type;
	union {
		struct {
			bitset years;
			monthday_range* monthdays;
			bitset weeks;
		};
		char comment[COMMENT_SIZE];
	};
};

struct weekday_selector {
	bool plural_day_holiday;
	bool single_day_holiday;
	weekday_selector_type type;
	union {
		bitset range;
		struct {
			bitset day;
			int nth_of_month;
		};
	};
};

struct time_selector {
	bitset time_range;
	bitset extended_time_range;
};

struct small_range_selector {
	weekday_selector weekday;
	time_selector hours;
};

struct selector_sequence {
	bool anyway;
	wide_range_selector wide_range;
	small_range_selector small_range;
};

struct rule_modifier {
	rule_modifier_type type;
	char comment[COMMENT_SIZE];
};

struct rule_sequence {
	rule_separator separator;
	selector_sequence selector;
	rule_modifier state;
};

struct opening_hours {
	opening_hours next_item;
	bool aligned;
	rule_sequence rule;
};

opening_hours build_opening_hours(char *);

#endif /* !OPENING_HOURS_H_ */
