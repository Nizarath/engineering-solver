#ifndef _CMD_H
#define _CMD_H

/* number of commands (functions + special commands) */
#define NCMD	(sizeof(cmd_table) / sizeof(struct cmd))

struct cmd {		/* one record in fun table */
	const char *name;
	void (*code)(const char *);
};

/* prototypes */
void cl(const char *s);
void du(const char *s);
void ex(const char *s);
void help(const char *s);
void hist(const char *s);
void pr(const char *s);
void sw(const char *s);

extern struct cmd cmd_table[];
void (*find_cmd(const char *s))(const char *);
int cmd_cmp(const void *, const void *);
void exec_spec(char *cmd);

#endif
