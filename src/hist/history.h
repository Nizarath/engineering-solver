#ifndef _HIST_H
#define _HIST_H

/* count of commands cached */
#define NHIST		20

/* one record in the history */
struct hist_rec {
	char *cmd;	/* command itself */
	double res;	/* cached result */
	int err;	/* any errors */
};

/* functions */
void puthist(const char *cmd, double res, int err);
void showhist(void);
void hist_exec(const char *num);
void hist_find(const char *arg);

#endif
