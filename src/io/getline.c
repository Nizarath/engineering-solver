#include <stdio.h>

int getln(char *s, int max)
{
	register char *b;
	register int c;

	for (b = s; (s - b) < (max - 1) &&
			(c = getchar()) != EOF; *s++ = c)
		if (c == '\n') {
			*s++ = c;
			break;
		} else if (c == ';') {
			*s++ = '\n';
			break;
		}
	*s = '\0';

	return s - b;
}
