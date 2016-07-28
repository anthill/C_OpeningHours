#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <time.h>
#include <unistd.h>

#include "opening_hours.h"

#define ADD_TEST(test) CU_add_test(suite, #test, test)

#define standard_output 1
#define error_output 2

typedef enum output_match_opts output_match_opts;

enum output_match_opts {
	EXACT_MATCH = 1,
	JUST_CONTAINS = 2,
	INVERT_MATCH = 4,
	BEGIN_WITH = 8
};

typedef struct open_params open_params;

struct open_params {
	char oh[128];
	struct tm date;
};

#define output_match(func, av, fd, opt, str) ({ \
	ssize_t _len = strlen(str); \
	char _fd_content[_len]; \
	int _pipes[2]; \
	int _last_fd = dup(fd); \
	int _res; \
 \
	pipe(_pipes); \
	dup2(_pipes[1], fd); \
	if (func(av)) \
		write(1, "ok", 2); \
	_res = read(_pipes[0], _fd_content, _len + 1) == _len; \
	close(_pipes[0]); \
	dup2(_last_fd, fd); \
	(!(opt & EXACT_MATCH) || _res) && !strncmp(_fd_content, str, _len); \
})

void write_toto(char *s) {
	write(1, s, strlen(s));
}

void wide_ranges(void) {
	CU_ASSERT(output_match(build_opening_hours, "off", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "2016 Mar: off", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "2016 Mar off", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "2016 Mar 06 off", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "Mar 06-Jan 19 off", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "Mar 06-Jan 19 off", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "Mar 06-Jan 19: closed", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "Jan 06-Jan 19: closed", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "Jan-Feb: closed", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "2016 Feb 29", standard_output, BEGIN_WITH, "ok"));
}

void small_ranges(void) {
	CU_ASSERT(output_match(build_opening_hours, "Tu-Sa 09:00-12:00,14:00-18:00", standard_output, BEGIN_WITH, "ok"));
}

void wide_and_small_ranges(void) {
	CU_ASSERT(output_match(build_opening_hours, "2016 Feb 29: Tu -Mo", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "2016 Tu-Sa 09:00-12:00,14:00-18:00", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "2016: Tu-Sa 09:00-12:00,14:00-18:00", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "Mar: Tu-Sa 09:00-12:00,14:00-18:00", standard_output, BEGIN_WITH, "ok"));
	CU_ASSERT(output_match(build_opening_hours, "Mar-Apr: Tu-Sa 09:00-12:00,14:00-18:00", standard_output, BEGIN_WITH, "ok"));
}

void syntax_errors(void)
{
	CU_ASSERT(output_match(build_opening_hours, "toto", standard_output, BEGIN_WITH, "Invalid syntax: "));
	CU_ASSERT(output_match(build_opening_hours, ":", standard_output, BEGIN_WITH, "Invalid syntax: "));
	CU_ASSERT(output_match(build_opening_hours, "\"comment\"", standard_output, BEGIN_WITH, "Invalid syntax: missing colon"));
	CU_ASSERT(output_match(build_opening_hours, "\"\":", standard_output, BEGIN_WITH, "Invalid syntax: empty comment"));
	CU_ASSERT(output_match(build_opening_hours, "1800", standard_output, BEGIN_WITH, "Invalid range:"));
	CU_ASSERT(output_match(build_opening_hours, "3800", standard_output, BEGIN_WITH, "Invalid range:"));
	CU_ASSERT(output_match(build_opening_hours, "2016 Feb 30", standard_output, BEGIN_WITH, "Invalid range:"));
	CU_ASSERT(output_match(build_opening_hours, "2016 Feb 29: Tu -Ma", standard_output, BEGIN_WITH, "Invalid range:"));
	CU_ASSERT(output_match(build_opening_hours, "2016 Feb 29: Ta -Mo", standard_output, BEGIN_WITH, "Invalid syntax:"));
}

int open(open_params params) {
	int pipes[2];
	int last_fd = dup(1);

	pipe(pipes);
	dup2(pipes[1], 1);
	opening_hours oh = build_opening_hours(params.oh);
	close(pipes[0]);
	dup2(last_fd, 1);
	return (is_open_time(oh, params.date));
}

void opening_tests(void) {
	CU_ASSERT(open((open_params){"2016 Mar-Dec: Mo-Fr 09:00-19:00", (struct tm){.tm_min = 24, .tm_hour = 12, .tm_mday = 21, .tm_wday = 3, .tm_year = 2016 - 1900, .tm_mon = 6}}));
	CU_ASSERT(open((open_params){"2016 Mar-Dec: Mo-Fr 09:00-19:00", (struct tm){.tm_min = 24, .tm_hour = 12, .tm_mday = 21, .tm_wday = 3, .tm_year = 2016 - 1900, .tm_mon = 6}}));
	CU_ASSERT(!open((open_params){"2016 Mar-Dec: Mo-Fr 14:00-19:00", (struct tm){.tm_min = 24, .tm_hour = 12, .tm_mday = 21, .tm_wday = 3, .tm_year = 2016 - 1900, .tm_mon = 6}}));
	CU_ASSERT(!open((open_params){"2016 Mar-Dec: Mo-Fr 09:00-19:00", (struct tm){.tm_min = 24, .tm_hour = 12, .tm_mday = 21, .tm_wday = 3, .tm_year = 2015 - 1900, .tm_mon = 6}}));
	CU_ASSERT(!open((open_params){"2016 Mar-Dec: Mo-Fr 09:00-19:00", (struct tm){.tm_min = 24, .tm_hour = 20, .tm_mday = 21, .tm_wday = 3, .tm_year = 2015 - 1900, .tm_mon = 6}}));
	CU_ASSERT(!open((open_params){"2016 Mar-Dec: Mo-Fr 09:00-19:00", (struct tm){.tm_min = 24, .tm_hour = 12, .tm_mday = 21, .tm_wday = 3, .tm_year = 2016 - 1900, .tm_mon = 0}}));
}

int main() {
	CU_initialize_registry();
	CU_pSuite suite = CU_add_suite("Tests fonctionnels", 0, 0);

	ADD_TEST(syntax_errors);
	ADD_TEST(wide_ranges);
	ADD_TEST(small_ranges);
	ADD_TEST(wide_and_small_ranges);
	ADD_TEST(wide_and_small_ranges);
	ADD_TEST(opening_tests);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

    return 0;
}
