#ifndef _LEXEM_H
#define _LEXEM_H

#define ARG_MAX		200

#define NUM		'0'	/* num type */
#define FUN		'1'	/* function type */
#define VAR		'2'	/* variable type */
#define BIN		'3'	/* bin num */
#define HEX		'4'	/* hex num */
#define OPER		'5'	/* operator type */
#define EVAL		'6'	/* (...) expression */

int getlex(char **argv, char *lex);

#endif
