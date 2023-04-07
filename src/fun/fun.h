#ifndef _FUN_H
#define _FUN_H

/* number of commands (functions + special commands) */
#define NFUN	(sizeof(fun_table) / sizeof(struct fun))

/* some math constants */
#define PI	3.1415926535897932384

struct fun {		/* one record in fun table */
	const char *name;
	void (*code)(void);
};

extern struct fun fun_table[];
void (*find_fun(const char *s))(void);
static int fun_cmp(const void *s, const void *fun);

static void _cos(void);
static void ap(void);
static void _exp(void);
static void _pow(void);
static void pi(void);
void der(void);
void _int(void);
static void and(void);
static void or(void);
static void not(void);
static void xor(void);
static void neg(void);
static void _sin(void);
static void sum(void);
static void min(void);
static void _div(void);
static void mul(void);
static void mod(void);
static void _sqr(void);
static void _sqrt(void);
static void arrang(void);
static void comb(void);
static void permut(void);
static void _tan(void);

#endif
