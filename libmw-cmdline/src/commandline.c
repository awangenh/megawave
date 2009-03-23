/**
 * @mainpage libmw-cmdline
 *
 * @section Introduction
 *
 * This library provides all the tools used for command-line parsing
 * and automated structures allocation/deallocation and input/output
 * in the executable versions of the megawave modules.
 */

/**
 * @file commandline.c
 *
 * @version 1.14
 * @author Jacques Froment (1995 - 2005)
 * @author Sylvain Parrino (1995 - 2005)
 * @author Nicolas Limare (2009)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <setjmp.h>

#include "mw.h"
#include "definitions.h"
#include "commandline.h"

/*
 * global variables
 */

int verbose_flg = FALSE;

/**
 * default option buffer
 */
char _mwdefoptbuf[BUFSIZ]="";

char type_force[mw_ftype_size+1] = "?";

int help_flg = FALSE;
int vers_flg = FALSE;

extern int help_flg;
extern int vers_flg;

char _mwoptlist[BUFSIZ] = "";

int   _mwoptind = 1;
char *_mwoptarg = NULL;

Mwiline mwicmd[]={{NULL, NULL, NULL, NULL, NULL, NULL}};
int mwind=0;


/*
 * structures
 */

/* TODO : move to definitions.h */

/**
 *  data structure for the default megawave options
 */
struct mwargs {
     char *name;
     char *arg;
     int   argsiz;
     char *argtexname;
     void (*action)(void);
};

/**
 * system options
 */
/* TODO: explanations */

static void call_debug(void);
static void call_help(void);
static void call_verbose(void);
static void call_vers(void);
static void call_ftypelist(void);
static void call_ftype(void);

struct mwargs mwargs[] = { 
    {"-debug",     NULL, 0, NULL, call_debug}, 
    {"-help",      NULL, 0, NULL, call_help}, 
    {"-verbose",   NULL, 0, NULL, call_verbose}, 
    {"-vers",      NULL, 0, NULL, call_vers}, 
    {"-ftypelist", NULL, 0, NULL, call_ftypelist},
    {"-ftype",     type_force, sizeof(type_force), "<image type>", call_ftype}, 
    {NULL,         NULL, 0, NULL, NULL}
};

/*
 * local functions
 */

/**
 * emulate a simple isdigit() without handling localization
 * to avoid glibc dependencies
 */
static int simple_isdigit(int c)
{
     return ((c >= '0') && (c <= '9'));
}

/**
 * change, if wanted, stdout and/or stderr
 */
/* TODO: drop, use shell redirection instead? */
static void setnewout(void)
{
     char *fname;

     if (NULL != (fname = getenv("MW_STDOUT")) 
	 && verbose_flg == FALSE)
     {
	 if (NULL == freopen(fname, "w", stdout))
	     fprintf(stderr,						\
		     "Cannot redirect standard output to \"%s\"\n", fname);
     }

     if (NULL != (fname = getenv("MW_STDERR"))
	 && verbose_flg == FALSE)
     {
	 if (NULL == freopen(fname, "w", stderr))
	     fprintf(stderr,						\
		     "Cannot redirect standard error to \"%s\"\n", fname);
     }
}

/*
 * actions for default megawave options
 */

static void call_debug(void)
{
     mwdbg = TRUE;
     return;
}

static void call_help(void)
{
     help_flg = TRUE;
     return;
}

static void call_verbose(void)
{
     verbose_flg = TRUE;
     return;
}


static void call_ftype(void)
{
     return;
}

static void call_vers(void)
{
     vers_flg = TRUE;
     return;
}

static void call_ftypelist(void)
{
     char **A;
     int i;
 
     A = (char **)mw_ftypes_description;

     for (i = 0; (A[i] != NULL) && (A[i+1] != NULL); i += 2)
     {
	 printf("%s \t\t %s\n", A[i], A[i+1]);
     }
     exit(0);
}

static struct mwargs *lookup(char *s)
{
     struct mwargs *p;
     for (p=mwargs; p->name != NULL; p++)
	  if (!strcmp(p->name, s))
	       return p;
     return NULL;
}

/**
 * main run-time function
 */
int _mw_main(int argc, char *argv[], char *envp[])
{ 
    char *userargv[BUFSIZ];
    int i, userargc, flg;
    struct mwargs *p;
    char command[BUFSIZ],*chm;
    int retcommand;
    
    /* FIXME: unused parameter */
    envp = envp;
    
    /* Name of module */
    if ((mwname = strrchr(argv[0], '/')) != NULL)
	mwname = mwname + 1;
    else
	mwname = argv[0];
    
    /* Group */
    mwgroup = mwicmd[mwind].group;
    
    /* If MW_CHECK_HIDDEN set, check for hidden module */
    chm=getenv("MW_CHECK_HIDDEN");
    if ((chm) && (chm[0]=='1'))
    {
	sprintf(command,"mwwhere -bin %s > /dev/null",mwname);
	retcommand=system(command) >> 8;
	if (retcommand == 2)
	    mwerror(WARNING,1,"Module of same name hidden by this one !\n");
    }
  
     /* make default option buf for usage */
     for (p=mwargs; p->name != NULL; p++) {
	  strcat(_mwdefoptbuf, " [");
	  strcat(_mwdefoptbuf, p->name);
	  if (p->arg != NULL) {
	       strcat(_mwdefoptbuf, " ");
	       strcat(_mwdefoptbuf, p->argtexname);
	  }
	  strcat(_mwdefoptbuf, "]");
     }

     /*
      * sort user options and MegaWave default options;
      * for the lasts, do corresponding action
      */
     for (i=1, userargc=1, userargv[0]=argv[0], flg = FALSE; i<argc; i++) {
	  if (flg == TRUE) {
	       if (strlen(argv[i]) > (size_t) (p->argsiz -1))
		    *(argv[i] + p->argsiz - 1) = '\0';
	       strcpy(p->arg, argv[i]);
	       (*p->action)();
	       flg = FALSE;
	  }
	  else if ((p=lookup(argv[i])) != NULL) {
	       if (p->arg != NULL)
		    flg = TRUE;
	       else
		    (*p->action)();
	  }
	  else
	       userargv[userargc++]=argv[i];
     }

     setnewout();
     mwicmd[mwind].mwarg(userargc, userargv);
     exit(0);
     return 0;
}

/**
 * megawave default options actions
 */
void MegaWaveDefOpt(char *vers)
{
     /* Version flag */
     if (vers_flg == TRUE)
     {
	  printf("%s\n",vers);
	  exit(0);
     }

     /* Help flag */
     if (help_flg == TRUE)
	  mwicmd[mwind].mwuse(NULL);

}

int mw_opt_used(char c)
{
     return (strchr(_mwoptlist, c) != NULL) ? TRUE : FALSE;
}

/*
 * i/o conversion functions called by mwp (data_io.c)
 */

char *_mw_ctoa_(char c)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%c", c);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_uctoa_(unsigned char uc)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%c", (char)uc);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_stoa_(short s)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%hd", s);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_ustoa_(unsigned short us)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%hu", us);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_itoa_(int i)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%d", i);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_uitoa_(unsigned int ui)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%u", ui);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_ltoa_(long l)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%ld", l);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_ultoa_(unsigned long ul)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%lu", ul);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

/*
 * 17/3/95: Erreur inexpliquee sur Sun Solaris 5.3 (Grib1):
 *          la variable f n'est pas mise a la bonne valeur
 *          lors de l'appel de _mw_ftoa_(xx) si cette
 *          fonction n'est pas declaree suivant la norme ANSI !!
 *  A enlever dans une future version (tester par exemple
 *  la valeur retournee par le module fentropy)
 */

char *_mw_ftoa_(float f)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%g", (double)f);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_dtoa_(double d)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%g", d);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

int _mwis_open(char *s, char *rw)
{
     char fname[BUFSIZ];

     if (*rw == 'r') /* read */
     {
	 strcpy(fname,s);          /* do not change the value of s */
	 return(_search_filename(fname));
     }
     return(TRUE); /* checking diseable for writing */
}


int _mwgetopt(int argc, char **argv, char *optstring)
{
     if (_mwoptind < argc) {
	  if (*argv[_mwoptind] == '-') {
	       if (*(argv[_mwoptind] + 1) != '.' 
		   && *(argv[_mwoptind] + 1) != '-' &&
		   !simple_isdigit(*(argv[_mwoptind] + 1))) {
		    char *opt;
		    opt = strchr(optstring, *(argv[_mwoptind] + 1));
		    if (opt == NULL)
			 return '?';
		    else {
			 if (*(opt+1) == ':') {
			      if (++_mwoptind < argc)
				   _mwoptarg = argv[_mwoptind];
			      else {
				   _mwoptarg = NULL;
				   return '?';
			      }
			 }
			 _mwoptind++;
			 return *opt;
		    }
	       }
	       else
		    return -1;
	  }
	  else
	       return -1;
     }
     else
	  return -1;
}
