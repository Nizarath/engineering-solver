#include <stdio.h>
#include <math.h>
#include "../fun/fun.h"
#include "../eval/eval.h"
#include "../err/error.h"
#include "../global.h"
#include "../postfix/posteval.h"
#include "../types/routines.h"
#include "../stack/stack.h"

/* buffer isn't cyclic since it never fills with new lexems before finishing processing
 * old expression, after first processing it can be reset to start of expression
 * without need to clean buffer (to process the same) and resetted to clear state */

struct lex evbuf[EVAL_MAX];
struct lex *evbeg = evbuf - 1;	/* next place to read lexem */
struct lex *evend = evbuf;	/* next place to write lexem */

#define EV_FILL		(evend >= evbuf + EVAL_MAX)
#define EV_EMPTY	(evbeg >= evend || evbeg < evbuf)

void putev(int type, void *lex)
{
	if (EV_FILL) {
		slv_errno |= EVPUSH_FILL;

		return;
	}
	/* ok, can place */
	evend->type = type;
	switch (type) {
		case NUM:
			evend->val.num = *(double *)lex;
			break;
		case OPER:
			evend->val.oper = *(char *)lex;
			break;
		case FUN:
			evend->val.fun_code = *(void (**)(void))lex;
			break;
		case VAR:
			evend->val.var = *(char *)lex;
			break;
	}
	evend++;	/* points to next */
	if (evbeg < evbuf)	/* was the first lexem in buf */
		evbeg = evbuf;
}

/* doesn't copy anything, just returns the pointer,
 * since normally values pointed to are immediatly placed
 * in the stack for calculations -- without need in intermediate
 * buffers */

struct lex *getev(void)
{
	return !EV_EMPTY ? evbeg++ : NULL;
}

void reset_ev(void)
{
	evbeg = evbuf;
}

void clean_ev(void)
{
	evbeg = evbuf - 1;
	evend = evbuf;
}

/* evaluate an expression -- add val returning? */
void eval(double x)
{
	register struct lex *lp;
	double tmp;

	reset_ev();

	/* get next lexem */
	while ((lp = getev()) != NULL) {
		switch (lp->type) {
			case VAR:
				push(x);	/* REDO for actual var val */
				break;
			case NUM:
				push(lp->val.num);
				break;
			case OPER:
				switch (lp->val.oper) {
				case '^':	/* a b ^ -> (a ^ b) */
					tmp = pop();
					push(pow(pop(), tmp));
					break;
				case '+':	/* a b + -> (a + b) */
					push(pop() + pop());
					break;
				case '-':	/* a b - -> (a - b), careful with order */
					tmp = pop();
					push(pop() - tmp);
					break;
				case '*':	/* a b * -> (a * b) */
					push(pop() * pop());
					break;
				case '/':	/* a b / -> (a / b) */
					tmp = pop();
					if (!tmp)	/* / 0 */
						slv_errno |= DIV_ZERO;
					else
						push(pop() / tmp);
					break;
				case '%':	/* a b % -> (a % b) */
					tmp = (int)pop();
					if (!tmp)
						slv_errno |= MOD_ZERO;
					else
						push((int)pop() % (int)tmp);
					break;
				case '&':	/* logical and */
					push((unsigned long)pop() & (unsigned long)pop());
					break;
				case '|':	/* logical or */
					push((unsigned long)pop() | (unsigned long)pop());
					break;
				case '~':	/* logical not */
					push(~(unsigned long)pop());
					break;
				}
				break;
			case FUN:
				lp->val.fun_code();
				break;
		}
	}
}

int parse_eval(char *e)
{
	static char sp[100];
	char *s;
	int c, isfrac;
	double tmp;
	void (*fun)(void);


	for (;;e++) {

	/* skip empty chars */
	while (isspace(*e))
		e++;

	/* check end */
	if (*e == '\0')
		return 0;
	
	/* oper */
	if (!(isdigit(*e) || *e == '.' || *e == '-' || *e == '=' || islalpha(*e) || 
				*e == '\\')) {
		putev(OPER, e);

		continue;
	}

	s = sp;		/* store to convert later */

	/* check for command or variable or numeral system */
	if (*e == '\\') {		/* \var, const from var array, not real var */
		e++;
		if (islalpha(*e)) {
			tmp = vars[*e - 'a'];
			putev(NUM, &tmp);

			continue;
		} else 
			e--;
	} else if (islalpha(*e)) {
		/* special check for binary */
		if (*e == 'b') {
			e++;
			if (*e == '-' || isbin(*e)) {
				do {
					*s++ = *e;
				} while (isbin(*++e));
				e--;
				*s = '\0';

				tmp = (double)atob(sp);
				putev(NUM, &tmp);

				continue;
			} else {
				e--;
					/* command/var starting with 'b',
						   return to state when read 'b' */
			}

		/* special check for hex */
		} else if (*e == 'x') {
			e++;
			if (*e == '-' || ishex(*e)) {
				do {
					*s++ = *e;
				} while (ishex(*++e));
				e--;
				*s = '\0';

				tmp = (double)atox(sp);
				putev(NUM, &tmp);

				continue;
			} else {
				e--;
					/* fun/var starting with 'x', return to
						   state when read 'x' */
			}
		}

		/* 1st letter or next after b/x must be stored anyway */
		*s++ = *e;
		e++;
		if (!islalpha(*e)) {	/* variable */
			e--;
			*s-- = '\0';

			c = *s;		/* var code in int */
			putev(VAR, &c);

			continue;
		} else {		/* command */
			do {
				*s++ = *e;
			} while (islalpha(*++e));
			e--;
			*s = '\0';

			fun = find_fun(sp);
			if (fun == NULL)
				slv_errno |= UNKNOWN_FUN;
			else
				putev(FUN, &fun);

			continue;
		}
	}

	/* will be number */
	isfrac = 0;

	/* special check for minus */
	if (*e == '-') {
		*s++ = *e;
		e++;
		if (!isdigit(*e)) {	/* operation, not negative num */
			e--;

			c = '-';
			putev(OPER, &c);

			continue;
		}
		/* if we here -- then negative num  -- '-' is already 
		 * stored */

	/* also may be empty integer part */
	} else if (*e == '.')
		isfrac = 1;

	/* next is only for number */

	do {		/* accum digits */
		*s++ = *e;
	} while (isdigit(*++e));

	/* first non-digit is '.' and there wasn't '.' before -- 
	 * accum fraction part */
	if (*e == '.' && !isfrac) {
		*s++ = '.'; 
		while (isdigit(*++e))
			*s++ = *e;
	}
	if (*e == 'e' || *e == 'E') {	/* exp */
		*s++ = *e;
		e++;
		if (*e == '-' || *e == '+' || isdigit(*e))	{ /* exp sign */
			do {
				*s++ = *e;
			} while (isdigit(*++e));
			e--;
		} else {
			e -= 2;	/* this and e/E */
		}
	} else
		e--;

	*s = '\0';

	tmp = atof(sp);
	putev(NUM, &tmp);

	}
}
