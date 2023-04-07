#ifndef _ERROR_H
#define _ERROR_H

extern int slv_errno;	/* errors from last exec */

/* error types */
#define UNKNOWN_FUN	(1 << 0)
#define UNKNOWN_OPER	(1 << 1)
#define DIV_ZERO	(1 << 2)
#define MOD_ZERO	(1 << 3)
#define PUSH_FILL	(1 << 4)
#define POP_EMPTY	(1 << 5)
#define EVPUSH_FILL	(1 << 6)
#define EVPOP_EMPTY	(1 << 7)
#define UNKNOWN_CMD	(1 << 8)
#define NOT_LIT		(1 << 9)
#define PAR_MISMATCH	(1 << 10)
#define UNKNOWN_CHAR	(1 << 11)
#define ARG_MISSED	(1 << 12)

struct err {		/* error record */
	const int code;		/* error code, see below */
	const char *msg;	/* error message, starts with ! */
};

extern struct err errs[];

#define NERR	(sizeof(errs) / sizeof(struct err))

void printerr(int err_mask);

#endif
