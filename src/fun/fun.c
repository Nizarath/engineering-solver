#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../fun/fun.h"
#include "../eval/eval.h"
#include "../err/error.h"
#include "../hist/history.h"
#include "../postfix/posteval.h"
#include "../lexem/lexem.h"
#include "../stack/stack.h"

struct fun fun_table[] = {
	{ "and",	&and		},
	{ "ap",		&ap		},
	{ "arr",	&arrang		},
	{ "comb",	&comb		},
	{ "cos",	&_cos		},
	{ "der",	&der		},
	{ "div",	&_div		},
	{ "exp",	&_exp		},
	{ "fac",	&permut		},
	{ "int",	&_int		},
	{ "min",	&min		},
	{ "mod",	&mod		},
	{ "mul",	&mul		},
	{ "neg",	&neg		},
	{ "not",	&not		},
	{ "or",		&or		},
	{ "per",	&permut		},
	{ "pi",		&pi		},
	{ "pow",	&_pow		},
	{ "sin",	&_sin		},
	{ "sqr",	&_sqr		},
	{ "sqrt",	&_sqrt		},
	{ "sum",	&sum		},
	{ "tan",	&_tan		},
	{ "xor",	&xor		}
};

/* main interface */
void (*find_fun(const char *s))(void)
{
	struct fun *fun;

	fun = bsearch(s, fun_table, NFUN, sizeof(struct fun),
			fun_cmp);
	if (fun == NULL) {
		slv_errno |= UNKNOWN_FUN;

		return NULL;
	}

	return fun->code;	/* pointer to fun */
}

/* compare the following:
 * || "fun"  |  &fun  ||    with     || "fun" ||
 */
static int fun_cmp(const void *fun_name, const void *fun_rec)
{
	return strcmp((char *)fun_name, ((struct fun *)fun_rec)->name);
}

/* math functions */

/* apply given function to argument */
void ap(void)
{
	eval(pop());
	clean_ev();
}

/* integral calculation */
void _int(void)
{
	double a, b, dx, x, sum;

	/* get args */

	dx = pop();
	b = pop();
	a = pop();

	/* calculate integral with trapecion method */
	for (sum = 0.0, x = a; x < b; x += dx) {
		eval(x);
		sum += pop() * dx;
	}
	
	/* push to stack */
	push(sum);

	/* clear eval stack */
	clean_ev();
}

/* derivative calculation */
void der(void)
{
	double dx, x;

	/* get args */
	dx = pop();
	x = pop();

	/* calc derivative */
	eval(x + dx);
	eval(x);
	x = pop();

	/* push to stack */
	push((pop() - x) / dx);

	/* clean eval stack */
	clean_ev();
}

static void _cos(void)
{
	push(cos(pop()));
}

static void _sin(void)
{
	push(sin(pop()));
}

static void _tan(void)
{
	push(tan(pop()));
}

static void _exp(void)
{
	push(exp(pop()));
}

static void _pow(void)
{
	double t;

	t = pop();
	push(pow(pop(), t));
}

static void _sqr(void)
{
	double t;

	t = pop();
	push(t * t);
}

static void _sqrt(void)
{
	push(sqrt(pop()));
}

static void sum(void)
{
	push(pop() + pop());
}

static void min(void)
{
	double t;

	t = pop();
	push(pop() - t);
}

static void _div(void)
{
	double t;

	t = pop();
	if (t == 0.0) {
		slv_errno |= DIV_ZERO;
		return;
	}
	push(pop() / t);
}

static void mul(void)
{
	push(pop() * pop());
}

static void mod(void)
{
	int t;

	t = (int)pop();
	if (!t) {
		slv_errno |= MOD_ZERO;
		return;
	}
	push((int)pop() % t);
}

static void permut(void)
{
	int n;
	double p;	/* just for size */

	n = pop();
	for (p = 1.0; n > 0; n--)
		p *= n;
	push(p);
}

static void and(void)
{
	push((unsigned long)pop() & (unsigned long)pop());
}

static void or(void)
{
	push((unsigned long)pop() | (unsigned long)pop());
}

static void not(void)
{
	push(~(unsigned long)pop());
}

static void xor(void)
{
	push((unsigned long)pop() ^ (unsigned long)pop());
}

static void pi(void)
{
	push(PI);
}

static void neg(void)
{
	push(pop() * (-1));
}

static void comb(void)
{
	int m, n, i;
	double p, t;	/* just for size */

	n = pop();
	m = pop();
	for (p = 1.0, i = n-m+1; i <= n; i++)
		p *= i;
	for (t = 1.0, i = 2; i <= m; i++)
		t *= i;
	push(p / t);
}

static void arrang(void)
{
	int m, n, i;
	double p;	/* just for size */

	n = pop();
	m = pop();
	for (p = 1.0, i = n-m+1; i <= n; i++)
		p *= i;
	push(p);	
}
