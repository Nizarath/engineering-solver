#ifndef _ROUTINES_H
#define _ROUTINES_H

/* common parsing */
char *islit(char *p);
char *isnum(char *p);
char *isvar(char *p);
double atof(const char *s);
int isalpha(int c);
int isbin(int c);
int isdigit(int c);	/* shouldn't be macro: isdigit(c++) etc. will fail */
int isempty(const char *s);
int isfun(char *p);
int ishalpha(int c);
int ishex(int c);
int islalpha(int c);
int isoper(int c);
int issep(int c);
int isspace(int c);
int isunar(char *p);
unsigned atob(const char *s);
unsigned atox(const char *s);

#endif
