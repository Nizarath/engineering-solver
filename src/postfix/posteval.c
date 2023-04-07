#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../fun/fun.h"
#include "../err/error.h"
#include "../eval/eval.h"
#include "../lexem/lexem.h"
#include "../postfix/posteval.h"
#include "../stack/stack.h"
#include "../types/routines.h"

/* vars a-z */
double vars[26];

double slv_res;
int slv_errno;

void postcalc(int type, char *s)
{
	double tmp, tmp2; /* temp for some ops */
	void (*fun)(void);
	char **sp;

	switch (type) {
		case EVAL:	/* eval mode -- parse it, place to postfix ebuf */
			clean_ev();
			parse_eval(s);
			break;
		case VAR:	/* variable -> place to stack */
			push(vars[*s - 'a']);
			break;
		case FUN:	/* function -> find in table and execute */
			fun = find_fun(s);
			if (fun != NULL)
				fun();
			else
				slv_errno |= UNKNOWN_FUN;	/* ERROR! */
			break;
		case NUM:	/* number -> place to stack */
			push(atof(s));
			break;
		case BIN:	/* bin num */
			push(atob(s));
			break;
		case HEX:	/* hex num */
			push(atox(s));
			break;
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
				slv_errno |= DIV_ZERO;	/* ERROR! */
			else
				push(pop() / tmp);
			break;
		case '%':	/* a b % -> (a % b) */
			tmp = (int)pop();
			if (!tmp)
				slv_errno |= MOD_ZERO;	/* ERROR */
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
		case '$':	/* integral */
			_int();
			break;
		case '\'':	/* derivative */
			der();
			break;
		case '=':	/* set var */
			tmp = pop();
			vars[*s - 'a'] = tmp;
			push(tmp);
			break;
		case ';':	/* show result */
		case '\n':
			slv_res = tmp = pop();
			printf("> %lf\n", tmp);
			break;
		default:	/* some unexpected */
			slv_errno |= UNKNOWN_OPER;	/* ERROR! */
	}
}
