
# include <stdio.h>
# include <string.h>
# include <strings.h>
# include <stdlib.h>
# include <ctype.h>

#include "parsing.h"

#ifdef STANDALONE

int main(int ac, char **av) {
	char *printed;
	opening_hours oh;

	if (ac > 1) {
		oh = build_opening_hours(av[1]);
		printed = print_oh(oh);
		printf("%s", printed);
		free_oh(oh);
	}
	return (0);
}
#endif /* !STANDALONE */

