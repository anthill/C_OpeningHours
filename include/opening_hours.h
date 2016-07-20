#ifndef OPENING_HOURS_H_
# define OPENING_HOURS_H_

# define COMMENT_SIZE 128

# include <bitset.h>

/*
 * Typedefs:
 */

typedef struct monthday_range monthday_range;
typedef struct opening_hours* opening_hours;
typedef struct rule_sequence rule_sequence;
typedef struct selector_sequence selector_sequence;
typedef struct small_range_selector small_range_selector;
typedef struct time_selector time_selector;
typedef struct week_selector week_selector;
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
	RULE_COMMENT = 0,
	RULE_CLOSED = 1,
	RULE_OPEN = 2,
	RULE_UNKNOWN
};

enum weekday_selector_type {
	WD_NTH_OF_MONTH,
	WD_RANGE
};

enum rule_separator {
	SEP_NOT_SET = 0,
	SEP_HEAD,
	SEP_SEMICOLON,
	SEP_COMA,
	SEP_FALLBACK
	// <space>
};

enum wide_range_selector_type {
	WIDE_RANGE_DATE = 0,
	WIDE_RANGE_COMMENT
};

/*
 * Structures:
 */

struct monthday_range {
	int year; // Optionnal
	bitset days;
	bool easter;
};

struct week_selector {
	week_selector* next_item;
	bool aligned;
	bitset weeknum;
};

struct wide_range_selector {
	wide_range_selector_type type;
	union {
		struct {
			bitset years;
			monthday_range* monthdays;
			week_selector weeks;
		};
		char comment[COMMENT_SIZE];
	};
};

struct weekday_selector {
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
	bool anyway; // "24/7"
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
	// <space>
};

struct opening_hours {
	opening_hours next_item;
	bool aligned;
	rule_sequence rule;
};

opening_hours build_opening_hours(char *);


#endif /* !OPENING_HOURS_H_ */
