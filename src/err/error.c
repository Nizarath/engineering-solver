#include <stdio.h>
#include "error.h"

struct err errs[] = {
	{ UNKNOWN_FUN, "! unknown function\n" },
	{ UNKNOWN_OPER, "! unknown operator\n" },
	{ MOD_ZERO, "! % 0\n" },
	{ DIV_ZERO, "! / 0\n" },
	{ PUSH_FILL, "! push fill\n" },
	{ POP_EMPTY, "! pop empty\n" },
	{ EVPUSH_FILL, "! eval push fill\n" },
	{ EVPOP_EMPTY, "! eval pop empty\n" },
	{ UNKNOWN_CMD, "! unknown command\n" },
	{ NOT_LIT, "! not a literal\n" },
	{ PAR_MISMATCH, "! parentheses mismatch\n" },
	{ ARG_MISSED, "! function argument missed\n" },
	{ UNKNOWN_CHAR, "! unknown character\n" }
};

void printerr(int err_mask)
{
	struct err *errp;

	for (errp = errs; errp < &errs[NERR]; errp++)
		if (err_mask & errp->code)
			fputs(errp->msg, stderr);
}
