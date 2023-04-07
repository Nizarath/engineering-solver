#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../cmd/cmd.h"
#include "../hist/history.h"
#include "../postfix/posteval.h"
#include "../err/error.h"

static struct hist_rec histab[NHIST];
/* type is NONE, since zeroed */
static int top = 0;

void puthist(const char *s, double res, int err)
{
	int pos;
	char *cmd;

	/* try alloc first, may fail */
	cmd = strdup(s);
	if (cmd == NULL)
		return;

	/* calc pos, free if the last used */
	pos = top++ % NHIST;
	if (top > NHIST)
		free(histab[pos].cmd);

	/* place */
	histab[pos].cmd = cmd;	/* cmd itself */
	histab[pos].res = res;	/* cmd result */
	histab[pos].err = err;	/* cmd errors */
}

void showhist(void)
{
	register struct hist_rec *hp;
	register int pos, num;

	num = 1;
	if (top >= NHIST) {	/* reverse */
		pos = top % NHIST;
		for (hp = histab + pos; hp < &histab[NHIST]; hp++, num++)
			printf("%d: %s\n", num, hp->cmd);
		for (hp = histab; hp < &histab[pos]; hp++, num++)
			printf("%d: %s\n", num, hp->cmd);
	} else		/* normal loop */
		for (hp = histab; hp < &histab[top]; hp++, num++)
			printf("%d: %s\n", num, hp->cmd);
}

/* / <cmd>;
 * it doesn't loop in print order since in most cases after
 * beginning it's not the fastest way, it just loops over all
 * table and then calculates matched cmd num if it's present
 */
void hist_find(const char *arg)
{
	register struct hist_rec *hp;
	int num;

	for (hp = histab; hp < &histab[NHIST]; hp++)
		/* first check to is for if record is in use */
		if (hp->cmd != NULL && strstr(hp->cmd, arg + 1) != NULL) {
			if (top <= NHIST)		/* calc position */
				num = hp - histab + 1;
			else if (hp >= histab + (top % NHIST))
				num = 1 + hp - (histab + (top % NHIST));
			else
				num = NHIST - (histab + (top % NHIST) - hp - 1);
			printf("%d: %s\n", num, hp->cmd);
		}
}

/* ! [cmd] */
void hist_exec(const char *arg)
{
	int pos;
	struct hist_rec *recp;
	struct err *errp;

	/* determine command, skipping ! */
	pos = atoi(++arg);
	if (pos >= top) 	/* not in history yet */
		return;
	else if (!pos)	/* ! -- last command */
		recp = histab + (top - 1) % NHIST;
	else if (top <= NHIST)	/* no overlap */
		recp = histab + pos - 1;
	else			/* overlap */
		recp = histab + (top % NHIST - 1 + pos) % NHIST;

	if (recp->err)			/* error, no answer */
		printerr(recp->err);
	else {				/* give an answer */
		switch (recp->cmd[0]) {
			case ':':	/* special command */
				exec_spec(recp->cmd);
				break;
			case '!':	/* history, recursion */
				hist_exec(recp->cmd);
				break;
			case '/':	/* search */
				hist_find(recp->cmd);
				break;
			default:	/* math case, no recall, cached */
				printf("> %lf\n", recp->res);
		}
	}
}
