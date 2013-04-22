#include "helper.h"
#include <stdio.h>
#include <errno.h>	/* for definition of errno */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>	/* ISO C variable arguments */
#

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void
err_sys(const char *fmt, ...) {
	va_list		ap;
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Fatal error unrelated to a system call
 * Print a message and terminate.
 */
void err_quit(const char *fmt, ...) {
	va_list	ap;
	va_start(ap,fmt);
	err_doit(0,0, fmt, ap);
	va_end(ap);
	exit(1);
}

static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap) {
	char buf[MAX_LINE];
	vsnprintf(buf, MAX_LINE, fmt, ap);
	if(errnoflag)
		snprintf(buf+strlen(buf), MAX_LINE-strlen(buf), ": %s", strerror(error));
	strcat(buf, "\n");
	// in case stdout and stderr are the same 
	fflush(stdout);	
	fputs(buf, stderr);
	fflush(NULL);
}



