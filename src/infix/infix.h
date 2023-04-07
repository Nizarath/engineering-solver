#ifndef _INFIX_H
#define _INFIX_H

extern char *prior[];

char *pair(char *base, char *p);
int in2post(char *s);
void rmeol(char *s);

#endif
