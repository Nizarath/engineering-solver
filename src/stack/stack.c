#include <stdio.h>
#include "../lexem/lexem.h"
#include "../stack/stack.h"
#include "../err/error.h"

/* normal stack, used only for nums */

double stack[STACK_MAX];
int sp = 0;		/* next FREE position */

void push(double d)
{
	if (sp < STACK_MAX)
		stack[sp++] = d;
	else
		slv_errno |= PUSH_FILL;
}

double pop(void)
{
	if (sp > 0)
		return stack[--sp];
	slv_errno |= POP_EMPTY;

	return 0.0;
}

void clear_stack(void)
{
	sp = 0;
}
