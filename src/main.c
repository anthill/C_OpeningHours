
# include <stdio.h>
# include <string.h>
# include <strings.h>
# include <stdlib.h>
# include <ctype.h>

#include "parsing.h"

#ifdef STANDALONE

int main(int ac, char **av) {
	if (ac > 1)
		print_oh(build_opening_hours(av[1]));
	return (0);
}
#endif /* !STANDALONE */

