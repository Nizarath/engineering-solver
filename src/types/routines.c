#include <stdio.h>
#include <math.h>
#include "routines.h"

int isempty(const char *s)
{
	for (; *s; s++)
		if (!isspace(*s) && *s != '\n')
			return 0;
	return 1;
}

int isoper(int c)
{
	return c == '+' || c == '-' || c == '/'
		|| c == '*' || c == '%' || c == '^'
		|| c == '&' || c == '|' || c == '~';
}

int issep(int c)
{
	return c == '(' || c == ')' || c == '[' || c == ']' ||
		c == ',' || c == ';';
}

int islpar(int c)
{
	return c == '(' || c == '[';
}

int isrpar(int c)
{
	return c == ')' || c == ']';
}

/*
 * ~EXPR -- unar
 * (-12 -- unar
 * ,-12 -- unar
 * a-b -- binar
 * 12-12 -- binar
 * a-12 -- binar
 */
int isunar(char *p)	/* redo */
{
	if (*p == '~')
		return 1;
	if (*p != '-')
		return 0;
	while (isspace(*--p))
		;
	if (*p == '(' || *p == ',')
		return 1;
	return 0;
}

char *isnum(char *p)	/* redo this shit */
{
	int c;

	for (c = 2; c; p++, c--) {
		if (*p == '-' || *p == '+')
			p++;
		if (isdigit(*p)) {
			while (isdigit(*++p))
				;
			if (*p == '.')
				while (isdigit(*++p))
					;
		} else if (*p == '.')
			while (isdigit(*++p))
				;
		else 
			return NULL;
		if (isspace(*p) || *p == '\0' || *p == '\n' || issep(*p))
			return p;
		if (!(*p == 'e' || *p == 'E'))
			return NULL;
	}

	return NULL;
}

char *isvar(char *p)
{
	if (*p == '-' || *p == '+')
		p++;
	return (islalpha(*p) && !islalpha(*(p+1))) ? p + 1 : NULL;
}

char *islit(char *p)
{
	char *t;

	t = isvar(p);
	if (t != NULL)
		return t;
	return isnum(p);
}

int isfun(char *p)
{
	return (islalpha(*p) && islalpha(*(p + 1))) || *p == '$' || *p == '\'';
}

int isbin(int c)
{
	return c == '0' || c == '1';
}

int ishex(int c)
{
	return (c >= '0' && c <= '9') ||
		(c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F');
}

int xdigit(int c)
{
	if (isdigit(c))
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
}

int isdigit(int c)
{
	return c >= '0' && c <= '9';
}

int isspace(int c)
{
	return c == ' ' || c == '\t';
}

int isalpha(int c)
{
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z');
}

int ishalpha(int c)
{
	return c >= 'A' && c <= 'Z';
}

int islalpha(int c)
{
	return c >= 'a' && c <= 'z';
}

double atof(const char *s)
{
	double d, p, exp;
	int sign, exp_sign;

	/* skip spaces */
	while (isspace(*s))
		s++;

	/* check sign */
	sign = 1;
	if (*s == '-') {
		sign = -1;
		s++;
	} else if (*s == '+')
		s++;

	/* main loop */
	for (d = 0.0; isdigit(*s); s++)
		d = 10 * d + *s - '0';

	/* check . */
	p = 1.0;	/* here for correct end */
	if (*s == '.') {
		for (s++; isdigit(*s); s++) {
			d = 10 * d + *s - '0';
			p *= 10;
		}
	}

	/* check exp, independently of . */
	if (*s == 'e' || *s == 'E') {
		s++;

		/* get exponent sign */
		exp_sign = 1;
		if (*s == '-') {
			exp_sign = -1;
			s++;
		} else if (*s == '+')
			s++;

		/* accum exp (cann't call atoi -- since
		 * cann't skip spaces etc. */
		for (exp = 0.0; isdigit(*s); s++)
			exp = 10.0 * exp + *s - '0';

		/* calc 10^exp */
		exp = pow(10.0, exp);
		d = (exp_sign == 1) ? (d * exp) : (d / exp);
	}

	/* res = f(d, sign, pow); */
	return sign * (double)d / p;
}

unsigned atob(const char *s)
{
	int sign;
	double b;

	while (isspace(*s))	/* skip spaces */
		s++;

	/* sign */
	sign = 1;
	if (*s == '-') {
		sign = -1;
		s++;
	} else if (*s == '+')
		s++;

	/* accum num */
	for (b = 0; isbin(*s); s++)
		b = 2 * b + *s - '0';
	return sign * b;
}

unsigned atox(const char *s)
{
	int sign;
	register unsigned x;

	while (isspace(*s))	/* skip spaces */
		s++;

	/* sign */
	sign = 1;
	if (*s == '-') {
		sign = -1;
		s++;
	} else if (*s == '+')
		s++;

	/* accum num */
	for (x = 0; ishex(*s); s++)
		x = 16 * x + xdigit(*s);

	return sign * x;
}

