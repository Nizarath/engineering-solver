/*
 * slv entry point
 * see "main skeleton" part for description
 */

#define SLV_VER		("1.0")

#include <stdio.h>
#include <string.h>
#include "cmd/cmd.h"
#include "err/error.h"
#include "fun/fun.h"
#include "global.h"
#include "hist/history.h"
#include "infix/infix.h"
#include "io/getline.h"
#include "lexem/lexem.h"
#include "postfix/posteval.h"
#include "stack/stack.h"
#include "types/routines.h"

/********************** main skeleton **********************/
/*
 * main algorithm is below
 *
 * calc output is after ">", error is after "!" (showing function sometimes)
 */

void prhelp(void)
{
	register char **p;

	printf("Usage: slv [-e <commands>] [-i] [-h] [-p] [-v]\n"
			"\t-e -- expressions\n"
			"\t-i -- interactive mode on\n"
			"\t-h -- print this help\n"
			"\t-p -- postfix mode on\n"
			"\t-v -- show version\n\n"
		"Operators available:\n");

	for (p = prior; *p != NULL; p++)
		while (**p) {
			printf("%c ", **p);
			(*p)++;
		}

	printf("\n\nFunctions available:\n");

	for (p = (char **)fun_table; p < (char **)&fun_table[25];
			p = (char **)((char *)p + sizeof(struct fun)))
		printf("%s ", ((struct fun *)p)->name);

	printf("\n\nCommands available:\n");

	for (p = (char **)cmd_table; p < (char **)&cmd_table[9];
			p = (char **)((char *)p + sizeof(struct cmd)))
		printf("%s ", ((struct cmd *)p)->name);

	putchar('\n');
}

int main(int argc, char *argv[])
{
	/* CLI options */
	int i, p, c, iscli, tmp;

	/* processing logic */
	char s[ARG_MAX];	/* arg sequence */
	char b[ARG_MAX * 2];	/* arg buf */
	char hist[ARG_MAX * 2];	/* copy for history storage with result */
	register int type;	/* arg type */
	char *bp;

	/* CLI */
	i = p = iscli = 0, *b = '\0';
	if (argc > 1) {
		for (++argv; *argv != NULL; ++argv) {
			if (**argv == '-')
				while (c = *++*argv)
				switch (c) {
				case 'i': i = 1; break;	/* interactive */
				case 'p': p = 1; break;	/* postfix */
				case 'v':		/* version */
					  printf("slv %s\n", SLV_VER);
					  return 0;
				case 'h':		/* help */
					  prhelp();
					  return 0;
				case 'e':	/* options are separated with ; */
					  if (*(*argv + 1) || *++argv == NULL) {
						  printf("! missed arg\n"
							 "-h for help\n");
						  return 1;
					  }
					  iscli = 1;
					  strcat(b, *argv);
					  tmp = strlen(b);
					  b[tmp] = ';', b[tmp + 1] = '\0';
					  goto iter;
				default:
					  printf("! unknown option: %c\n"
						 "-h for help\n", c);
					  return 1;
				}
		iter:	;
		}
		if (iscli && isempty(b)) {	/* blank symbols only */
			printf("! empty line\n-h for help\n");
			return 1;
		}
		if (!p && in2post(b)) {	/* postfix mode off -- treat as infix */
			printerr(slv_errno);
			return 1;
		}
		rmeol(b);		/* remove any '\n'-s */
		for (bp = b; (type = getlex(&bp, s)) != '\0'; )
			postcalc(type, s);
		printerr(slv_errno);	/* show possible errors */
		if (!i && iscli)	/* calculations already were -- no interaction */
			return 0;
	}

	/* no CLI lexems -- interactive editor */
	while (getln(b, ARG_MAX * 2) > 0) {

		/* check and prepare string */
		if (isempty(b)) /* empty line, don't show error \n */
			continue;
		rmeol(b);

		/* prepare for history storage with result
		 * (since will be translated to postfix) */
		strcpy(hist, b);

		/* execute */
		if (*b == ':')		/* special command */
			exec_spec(b);
		else if (*b == '!')	/* execute in history */
			hist_exec(b);
		else if (*b == '/')	/* find in history */
			hist_find(b);
		else {			/* math command */
			if (!p && in2post(b)) {	/* check can be avoided with
						   2 loops */
				printerr(slv_errno);
				goto put_hist;
			}
			for (bp = b; (type = getlex(&bp, s)) != '\0'; )
				postcalc(type, s);
			printerr(slv_errno);	/* show possible errors */
		}

put_hist:	/* -> history and clear global command state */
		puthist(hist, slv_res, slv_errno);
		slv_res = slv_errno = 0;	/* no errors for new cmd */
	}

	return 0;
}
