#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd.h"
#include "../err/error.h"
#include "../hist/history.h"
#include "../lexem/lexem.h"
#include "../postfix/posteval.h"
#include "../stack/stack.h"
#include "../types/routines.h"

/* specific functions */
struct cmd cmd_table[] = {
	{ "cl",		&cl		},
	{ "du",		&du		},
	{ "ex",		&ex		},
	{ "exit",	&ex		},
	{ "help",	&help		},
	{ "hist",	&hist		},
	{ "pr",		&pr		},
	{ "q",		&ex		},
	{ "sw",		&sw		}
};

void (*find_cmd(const char *s))(const char *)
{
	struct cmd *cmd;

	cmd = bsearch(s, cmd_table, NCMD, sizeof(struct cmd),
			cmd_cmp);
	if (cmd == NULL) {
		slv_errno |= UNKNOWN_CMD;

		return NULL;
	}

	return cmd->code;	/* pointer to cmd */
}

/* compare the following:
 * || name | &cmd ||  vs  || name || */
int cmd_cmp(const void *cmd_name, const void *cmd_rec)
{
	return strcmp((const char *)cmd_name, ((struct cmd *)cmd_rec)->name);
}

/* execute command of the given format:
 * :<cmd> [args]
 * by calling postcalc
 * (if we just call postcalc, then it will fail
 * due to [args], since it does brutal exact check of name
 * which is essential for math functions)
 */
void exec_spec(char *cmd)
{
	/* skip : and cmd->name:
	 * cmd == ":hist 12",
	 * cmd->name == "hist",
	 * make cmd = " 12" */

	void (*code)(const char *);
	char b[ARG_MAX];
	register char *p;

	/* make separate buffer to avoid changing
	 * cmd in the cases it's record in history table */
	strcpy(b, cmd);

	/* cut name */
	for (p = b + 1; !isspace(*p); p++)
		;
	*p = '\0';
	code = find_cmd(b + 1);
	if (code == NULL) {
		slv_errno |= UNKNOWN_CMD;
		return;
	}
	/* call for args */
	code(p + 1);
}

void pr(const char *s)
{
	register int cnt;
	double tmp;

	if (!(cnt = atoi(s)))
		cnt++;
	while (cnt--) {
		tmp = pop();
		printf("> %lf\n", tmp);
		push(tmp);
	}
}

void du(const char *s)		/* ignores arg */
{
	double tmp;

	tmp = pop();
	push(tmp);
	push(tmp);
}

void sw(const char *s)		/* ignores arg */
{
	double tmp, tmp2;

	tmp = pop();
	tmp2 = pop();
	push(tmp);
	push(tmp2);
}

void ex(const char *s)
{
	exit(0);
}

void help(const char *s)
{
	extern void prhelp(void);
	prhelp();
}

void cl(const char *s)
{
	clear_stack();
}

void hist(const char *s)
{
	showhist();
}

