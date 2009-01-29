/**
 * @file error.c
 *
 * @version 1.14
 * @author Jacques Froment & Sylvain Parrino (1995-2005)
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "libmw-defs.h"

#include "error.h"

/* TODO: move to config.h or definitions.h */
int mwerrcnt = 0;

void mwdebug(char *fmt, ...)
{
    if (mwdbg) {
	va_list marker;
	
	va_start(marker, fmt);
	fprintf(stderr, "<dbg> ");
	vfprintf(stderr, fmt, marker);
	va_end(marker);
    }
}

void mwerror(int code, int exit_code, char *fmt, ...)
{
    va_list marker;
    char message[BUFSIZ];
    
    va_start(marker, fmt);
    
    switch(code) {
    case WARNING:
	fprintf(stderr, "megawave warning (%s) : ", mwname);
	vfprintf(stderr, fmt, marker);
	break;
    case ERROR:
	fprintf(stderr, "megawave error (%s) : ", mwname);
	vfprintf(stderr, fmt, marker);
	mwerrcnt++;
	break;
    case FATAL:
	fprintf(stderr, "megawave fatal (%s) : ", mwname);
	vfprintf(stderr, fmt, marker);
	fprintf(stderr, "Exit.\n");
	if (mwrunmode == 2)
	{ /* Send error message to Xmegawave */
	    (void) vsprintf(message, fmt, marker);
	    if (mwerrormessage) free(mwerrormessage);
	    mwerrormessage = (char *) malloc(strlen(message)+1);
	    strcpy(mwerrormessage,message);
	}
	/* TODO: make clear */
	/* mwexit(exit_code); */
	exit(exit_code);
	break;
    case INTERNAL:
	fprintf(stderr, "megawave internal (%s) : ", mwname);
	vfprintf(stderr, fmt, marker);
	fprintf(stderr, "Exit.\n");
	if (mwrunmode == 2)
	{ /* Send error message to Xmegawave */
	    (void) vsprintf(message, fmt, marker);
	    if (mwerrormessage) free(mwerrormessage);
	    mwerrormessage = (char *) malloc(strlen(message)+1);
	    strcpy(mwerrormessage,message);
	}
	/* TODO: make clear */
	/* mwexit(exit_code); */
	exit(exit_code);
	break;
    case USAGE:
	(void) vsprintf(message, fmt, marker);
	if (mwrunmode == 1)
	    mwicmd[mwind].mwuse(message);
	else
	{ /* Send error message to Xmegawave */
	    if (mwerrormessage) free(mwerrormessage);
	    mwerrormessage = (char *) malloc(strlen(message)+1);
	    strcpy(mwerrormessage,message);
	    /* TODO: make clear */
	    /* longjmp(*_mwienv,1); */	
	    exit(exit_code);
	}
	break;
    default:
	mwerror(FATAL, 1, "Bad usage of mwerror : code %d is unknown\n", code);
	break;
    }
    va_end(marker);
}
