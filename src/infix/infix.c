#include <stdio.h>
#include <string.h>
#include "../types/routines.h"
#include "../postfix/posteval.h"
#include "../err/error.h"

/* add logical */
char *prior[] = {
	"^",
	"/*%",
	"+-",
	"~", "&", "|",
	NULL
};

/* unexpected chars */
static char *reserved = "~`!@#:\"?";

#define MAXLINE		1000
#define MAXBACK		500

/* if base == NULL, checks are disabled */
char *pair(char *base, char *p)
{
	int par, inc;

	/* some incorrect call */
	if (*p != '(' && *p != ')')
		return NULL;

	/* choose the way to go -- avoid 2 loops code */
	par = inc = (*p == '(') ? 1 : -1;
	do {
		p += inc;
		if (p < base && base != NULL)	/* before start of string */
			return NULL;
		switch (*p) {
			case '(': par++; break;
			case ')': par--; break;
			case '\0':
			case '\n':
				  return NULL;
		}
	} while (par);

	return p;
}

/* inc == 1 -- forward, inc == -1 -- backward;
 * -1 in returning means that it's either first or last
 *  oper, depending on inc;
 *  returns the place of first item need to be out of future
 *  "()", either oper or (/) */

char *skip_lex(char *base, char *p, int inc)
{
	int par;

	par = (inc == 1) ? '(' : ')';

	/* move until next oper */
	while (!(isoper(*(p += inc)) || *p == '\0' || *p == '\n' ||
				*p == par + inc || *p == ',' ||
				*p == '[' || *p == ']')) {
		/* before start */
		if (p <= base)
			return base - 1;

		/* skip included stuff */
		if (*p == par) {
			p = pair(base, p);
			if (p == NULL)		/* mismatch () */
				return NULL;
		}
	}

	return p;	/* return oper addr */
}

/* assuming we have enough space,
 * -1 and strlen(s) can be used, algorithm is the same */
void addpair(char *s, int lp, int rp)
{
	char *b;

	b = s + MAXLINE - MAXBACK;
	strcpy(b, s);
	s[lp + 1] = '(';
	memcpy(s + lp + 2, b + lp + 1, rp - lp - 1);
	s[rp + 1] = ')';
	strcpy(s + rp + 2, b + rp);
}

int isredup(char *base, char *s)
{
	return (*s == '(' && *(s+1) == '(' &&
		pair(base, s) == pair(base, s+1) + 1) ||
		(*s == ')' && *(s+1) == ')' &&
		pair(base, s) == pair(base, s+1) + 1);
}

/* remove redundant parentheses */
void redupars(char *s)
{
	char *b, *bp;

	b = s + MAXLINE - MAXBACK;
	strcpy(b, s);
	for (bp = b; *bp; bp++)
		if (!isredup(b, bp))
			*s++ = *bp;
	*s = '\0';
}

/* add spaces */
void setspace(char *s)
{
	char *b, *t;
	
	b = s + MAXLINE - MAXBACK;
	strcpy(b, s);
	for (; *b; b++)
		if (*b == '(' || *b == ')' || isoper(*b)) {
			/* redo to MAXFUN */
			*s++ = ' ', *s++ = ' ';
			*s++ = *b;
			if (!isunar(b))
				*s++ = ' ', *s++ = ' ';
		} else
			*s++ = *b;
	*s = '\0';
}

void rmeol(char *s)
{
	while (*s != '\n') {
		if (*s == '\0')
			return;
		s++;
	}
	*s = '\0';
}

/* add all commands to parentheses */
void parfun(char *s, char *end)
{
	char *base;
	int lp, rp;

	for (base = s; s < end; s++)
		if (isfun(s)) {
			lp = s - 1 - base;
			while (*s != '(')
				s++;
			rp = pair(base, s) + 1 - base;
			addpair(base, lp, rp);
			end += 2;
			parfun(s + 2, base + rp);
			s = pair(base, s + 1) + 1;
		}
}

/* set all literals to parentheses */
void parlit(char *s)
{
	register char *t, *t2;

	for (t = s; *t; t++)
		if ((t2 = islit(t)) != NULL) {
			addpair(s, t - 1 - s, t2 - s);
			t += 2;
		}
}

/* assuming we have enough space */
int setpars(char *s)
{
	register char **priorp, *prv, *nxt, *t;
	int l;

	rmeol(s);	/* remove newlines */
	l = strlen(s);
	addpair(s, -1, l);		/* garantee (...) */
	parfun(s, l + 2 + s);	/* each f(x) -> (f(x)), length is + () */

	/* parse priority list */
	for (priorp = prior; *priorp; priorp++)
		/* 1. find next oper;
		 * 2. find its borders;
		 * 3. place brackets, goto 1
		 */
		for (t = s; (t = strpbrk(t, *priorp)) != NULL; t += 2) {
			/* check unary, if not -- skip lexem back */
			if (!isunar(t)) {
				prv = skip_lex(s, t, -1);
				if (prv == NULL)
					return -1;
			} else
				prv = t - 1;
			/* next is anyway */
			nxt = skip_lex(s, t, 1);
			if (nxt == NULL)
				return -1;
			/* add (expr1 oper expr2) or (oper expr) */
			addpair(s, prv - s, nxt - s);
		}

	/* remove redundant pairs */
	redupars(s);
	
	/* add more spaces near '()' (optional, it's for 
	 * revpar to avoid usage of multiple additional memcpy's
	 * after each reversing */
	
	setspace(s);

	return 0;
}

/* infix -> postfix */
char *deep(char *p)
{
	while (*p != ')')
		if (*p++ == '\0')
			return NULL;
	/* come back */
	while (*--p != '(')
		if (*p == ')')	/* skip internal */
			p = pair(NULL, p);
	return p;
}

/* check expr in par, return it's type,
 * literal can be without '()' for efficiency (no addpair) */
#define LIT		'1'	/* (12), (-12.24), (  -12), (a), (-a) */
#define EX_OP_EX	'2'	/* ((...) + (...)) */
#define OP_EX		'3'	/* (+ (...)) */
#define FUN_EX		'4'	/* (fun(...)) */
#define FUN_ARG		'5'	/* (12, sin(x), 3), ((-10), 4) */
#define EX		'6'	/* (  (...)   ) */
#define NONE		'7'	/* (,), (\.) */

/* receives pointer after '(' in (EXPR);
 * it's a special type: FUN_ARG */
int expr_type(char *s)
{
	register char *t;

	while (isspace(*s))	/* skip */
		s++;

	/* check LIT */
	if ((t = islit(s)) != NULL) {
		while (isspace(*t))
			t++;
		if (issep(*t) || *t == '\n' || *t == '\0')
			return LIT;
	}

	/* check FUN(EXPR) */
	if (isfun(s))
		return FUN_EX;

	/* check OPER EXPR */
	if (isoper(*s))
		return OP_EX;

	/* check EXPR1 OPER EXPR2 or ARG or EXPR */
	while (!(isoper(*s) || *s == ',' || *s == ')')) {
		if (*s == '(')
			s = pair(NULL, s);
		s++;
	}
	if (*s == ')')
		return EX;
	return isoper(*s) ? EX_OP_EX : FUN_ARG;
}

/* (EXPR1 OPER EXPR2) -> (EXPR1 EXPR2 OPER) */
void op_ex(char *s)
{
	register char *p, *expr_beg;
	register int op, sc;	/* oper and space count */

	while (!(isoper(*s))) {
		if (*s == '(')
			s = pair(NULL, s);
		s++;
	}
	op = *s;
	for (p = s + 1, sc = 0; isspace(*p); p++, sc++)
		;
	expr_beg = p;
	if (*p == '(')
		p = pair(NULL, p);
	else {
		p = islit(p);
		if (p == NULL) {
			slv_errno |= NOT_LIT;

			return;
		}
		p--;
	}
	memmove(s, expr_beg, p - expr_beg + 1);
	memset(s + (p - expr_beg) + 1, ' ', sc);
	s[p - expr_beg + 1 + sc] = op;
}

void fun_ex(char *s)
{
	register char *t;
	int bc;
	char b[100];	/* MAXFUN NAME */

	for (t = s; *t != '('; t++)
		;
	memcpy(b, s, t - s);
	bc = pair(NULL, t) - t + 1;
	memmove(s, t, bc);
	memmove(s + bc, b, t - s);

	/* now remove commas (-> fun arg?) */
	for (; bc; bc--)
		if (s[bc] == ',')
			s[bc] = ' ';
}

void strcut(char *s, int a, int b, int bc)
{
	char *t;

	t = s + MAXLINE - MAXBACK;
	strcpy(t, s);
	memcpy(s + a, t + b, bc);
	memcpy(s + a + bc, t + a, b - a);
	strcpy(s + b + bc, t + b + bc);
}

/* translate to postfix form starting from deepest () */
void revpar(char *s)
{
	register char *p, *t, *t2;
	int bc;

	p = s;
	while ((p = deep(p)) != NULL) {
		p++;
		/*
		 * 6 cases (LIT == VAR or NUM), EXPR:
		 * - (LIT) -> (LIT)
		 * - (EXPR) -> (EXPR)
		 * - (EXPR1 OPER EXPR2) -> (EXPR1 EXPR2 OPER)
		 * - (OPER EXPR) -> (EXPR OPER)
		 * - (FUN(EXPR)) -> (EXPR FUN)
		 * - (ARG) -> (ARG)
		 */
		switch (expr_type(p)) {	/* redo to f(expr_type(p)) with table */
			case LIT:	/* literal/arg/ex -- nothing to do */
			case EX:
			case FUN_ARG:
				break;
			case EX_OP_EX:	/* 2nd and 3rd -- swap 2 last */
			case OP_EX:
				op_ex(p);
				break;
			case FUN_EX:
				fun_ex(p);
				break;
			default:
				printf("revpar: unknown sequence: %s\n", p);
		}
		/* move to next */
		while (*p != ')') {
			if (*p == '(')
				p = pair(NULL, p);
			p++;
		}
		p++;
	}
}

void rmspace(char *s)
{
	char b[MAXLINE];
	register char *bp;

	strcpy(b, s);
	for (bp = b; *bp; bp++)
		if (!(*bp == ' ' && *(bp + 1) == ' '))
			*s++ = *bp;
	*s = '\0';
}

int checkpars(char *s)
{
	register int par, br;

	for (par = br = 0; *s; s++)
		switch (*s) {
			case '(': par++; break;
			case ')': par--; break;
			case '[': br++; break;
			case ']': br--;
		}
	if (par || br) {
		slv_errno |= PAR_MISMATCH;
		return -1;
	}
	return 0;
}

void rmpars(char *s)
{
	for (; *s; s++)
		if (*s == '(' || *s == ')')
			*s = ' ';
}

void addeol(char *s)
{
	int l;

	l = strlen(s);
	s[l] = '\n', s[l + 1] = '\0';
}

int checkch(char *s)
{
	register char *t;

	for (t = s; *t && *t != '\n'; t++)
		if (ishalpha(*t) || (strchr(reserved, *t) != NULL)) {
			slv_errno |= UNKNOWN_CHAR;
			return -1;
		}
	return 0;
}

int checkfun(char *s)
{
	register char *t, *t2;

	for (t = s; *t && *t != '\n'; t++)
		if (isfun(t)) {
			for (t2 = t + 1; islalpha(*t2); t2++)
				;		/* find end of name */
			while (isspace(*t2))
				t2++;		/* skip spaces */
			if (*t2 != '(') {
				slv_errno |= ARG_MISSED;
				return -1;
			}
		}
	return 0;
}

int in2post(char *s)
{
	if (checkpars(s) || checkch(s) || checkfun(s))
		return -1;
	setpars(s);
	revpar(s);
	rmspace(s);
	rmpars(s);
	addeol(s);

	return 0;
}
