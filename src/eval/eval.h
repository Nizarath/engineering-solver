#ifndef _EVAL_H
#define _EVAL_H

#include "../lexem/lexem.h"	/* types */

/* buffer which stores functions and
 * data, prepared for higher order
 * functions */

#define EVAL_MAX	100

/* element in eval buf */
struct lex {
	int type;	/* NUM, FUN, OPER, VAR */
	union {
		void (*fun_code)(void);
		double num;
		char oper;
		char var;
	} val;
};

void putev(int type, void *val);
struct lex *getev(void);
void clean_ev(void);
int parse_eval(char *e);
void reset_ev(void);
void eval(double x);

#endif
