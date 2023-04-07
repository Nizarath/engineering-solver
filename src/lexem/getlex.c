#include <stdio.h>
#include <string.h>
#include "../types/routines.h"
#include "../lexem/lexem.h"
#include "../infix/infix.h"

/*
 * *argv will be updated to continue form
 * previous place for the constructs like
 * $ 1a0.001int sin
 * (3 stops inside one argument);
 * lex is a string where lexem is stored, as with getlex();
 *
 * instead of c = getch() each time we check *(*argv)++, until
 * find '\0' or finish another lexem;
 * we parse atmost one CLI argument this way
 */
int getlex(char **argv, char *lex)
{
	int isfrac;
	int bc;
	char *p;

	/* skip empty chars */
	while (isspace(**argv))
		++*argv;

	/* check eval */
	if (**argv == '[') {
		for (p = *argv + 1; *p != ']'; p++)
			;
		bc = p - *argv - 1;
		memcpy(lex, *argv + 1, bc);
		(*argv) += (bc + 2);
		lex[bc] = '\0';

		return EVAL;
	}

	/* first non-empty -- check it */
	if (!(isdigit(**argv) || **argv == '.' || **argv == '-' ||
				**argv == '=' || islalpha(**argv)))
		return *(*argv)++;		/* operator */

	/* check for command or variable or numeral system */
	if (islalpha(**argv)) {
		/* special check for binary */
		if (**argv == 'b') {
			++*argv;
			if (**argv == '-' || isbin(**argv)) {
				do {
					*lex++ = **argv;
				} while (isbin(*++*argv));
				*lex = '\0';

				return BIN;
			} else
				--*argv;	/* fun/var starting from 'b',
						   return to the state when it occured */
		/* special check for hex */
		} else if (**argv == 'x') {
			++*argv;
			if (**argv == '-' || ishex(**argv)) {
				do {
					*lex++ = **argv;
				} while (ishex(*++*argv));
				*lex = '\0';

				return HEX;
			} else
				--*argv;	/* fun/var starting from 'x',
						   return to state when it occured */
		}

		/* 1st letter or next after b/x must be stored anyway */
		*lex++ = *(*argv)++;
		if (!islalpha(**argv)) {	/* var */
			*lex = '\0';

			return VAR;
		} else {			/* fun */
			do {
				*lex++ = **argv;
			} while (islalpha(*++*argv));
			*lex = '\0';

			return FUN;
		}
	}
	
	/* will be number */
	isfrac = 0;

	/* special check for minus */
	if (**argv == '-') {
		*lex++ = *(*argv)++;
		if (!isdigit(**argv)) {		/* oper, not neg num */
			return '-';
		}
		/* if we here -- then neg num -- '-' is already stored */

	/* also may be empty integer part */
	} else if (**argv == '.') {
		isfrac = 1;

	/* set var */
	} else if (**argv == '=') {
		++*argv;
		*lex = islalpha(**argv) ? **argv : 'a';
		*++lex = '\0';

		return '=';
	}

	/* next is only for number */

	do {		/* accum digits */
		*lex++ = **argv;
	} while (isdigit(*++*argv));
	
	/* first non-digit is '.' and there wasn't '.' before --
	 * accum fraction part */
	if (**argv == '.' && !isfrac) {
		*lex++ = '.';
		while (isdigit(*++*argv))
			*lex++ = **argv;
	}
	if (**argv == 'e' || **argv == 'E') {	/* exp */
		*lex++ = *(*argv)++;
		if (**argv == '-' || **argv == '+' || isdigit(**argv)) { /* exp sign */
			do {
				*lex++ = **argv;
			} while (isdigit(*++*argv));
		} else
			--*argv;	/* this and e/E */
	}

	*lex = '\0';

	return NUM;
}
