#include <ctype.h>
#include <string.h>
#include "parsing.h"

int match(char *s, char *pattern) {
	regex_t reg;
	regmatch_t matched;
	bool res;

	REG_COMPILE(reg, pattern, REG_EXTENDED);
	res = regexec(&reg, s, 1, &matched, 0) != REG_NOMATCH;
	regfree(&reg);
	return (res);
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

opening_hours build_opening_hours(char *s) {
	opening_hours oh = calloc(1, sizeof(*oh)),
		      cur = oh;
	int it = 0;
	char *entire_string = s,
	     cursor_str[strlen(s) * 2 + 1];

	if (!oh) {
		dprintf(2, "FATAL ERROR: Allocation failed for oh.\nMaybe RAM is full?\n");
		exit(2);
	}
	oh->rule.separator = SEP_HEAD;
	do {
		if (it++) {
			cur = (cur->next_item = calloc(1, sizeof(*oh)));
		}
		if (parse_rule_sequence(&cur->rule, &s) == ERROR) {
			printf("\n%s\n%s\n", entire_string, set_cursor(s - entire_string, cursor_str));
			free_oh(oh);
			return (NULL);
		}
	} while (*s && *++s);
	return (oh);
}
