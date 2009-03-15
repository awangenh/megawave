/** @file mwp_cmdline.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.1
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef MWP_CMDLINE_H
#define MWP_CMDLINE_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MW_CMDLINE_PACKAGE
/** @brief the program name */
#define MW_CMDLINE_PACKAGE "mwp"
#endif

#ifndef MW_CMDLINE_VERSION
/** @brief the program version */
#define MW_CMDLINE_VERSION "1.01"
#endif

/** @brief Where the command line options are stored */
struct mw_args_info
{
  char * source_arg;	/**< @brief module source file name (default='-').  */
  char * source_orig;	/**< @brief module source file name original value given at command line.  */
  const char *source_help; /**< @brief module source file name help description.  */
  char * executable_arg;	/**< @brief executable code file name.  */
  char * executable_orig;	/**< @brief executable code file name original value given at command line.  */
  const char *executable_help; /**< @brief executable code file name help description.  */
  char * library_arg;	/**< @brief library code file name.  */
  char * library_orig;	/**< @brief library code file name original value given at command line.  */
  const char *library_help; /**< @brief library code file name help description.  */
  char * documentation_arg;	/**< @brief documentation file name.  */
  char * documentation_orig;	/**< @brief documentation file name original value given at command line.  */
  const char *documentation_help; /**< @brief documentation file name help description.  */
  char * name_arg;	/**< @brief name file name.  */
  char * name_orig;	/**< @brief name file name original value given at command line.  */
  const char *name_help; /**< @brief name file name help description.  */
  char * module_name_arg;	/**< @brief module name (default='unknown').  */
  char * module_name_orig;	/**< @brief module name original value given at command line.  */
  const char *module_name_help; /**< @brief module name help description.  */
  char * group_name_arg;	/**< @brief module group name (default='unknown').  */
  char * group_name_orig;	/**< @brief module group name original value given at command line.  */
  const char *group_name_help; /**< @brief module group name help description.  */
  int help_flag;	/**< @brief print help and exit (default=off).  */
  const char *help_help; /**< @brief print help and exit help description.  */
  int version_flag;	/**< @brief print version and exit (default=off).  */
  const char *version_help; /**< @brief print version and exit help description.  */
  int debug_flag;	/**< @brief debug flag (default=off).  */
  const char *debug_help; /**< @brief debug flag help description.  */
  
  unsigned int source_given ;	/**< @brief Whether source was given.  */
  unsigned int executable_given ;	/**< @brief Whether executable was given.  */
  unsigned int library_given ;	/**< @brief Whether library was given.  */
  unsigned int documentation_given ;	/**< @brief Whether documentation was given.  */
  unsigned int name_given ;	/**< @brief Whether name was given.  */
  unsigned int module_name_given ;	/**< @brief Whether module-name was given.  */
  unsigned int group_name_given ;	/**< @brief Whether group-name was given.  */
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int debug_given ;	/**< @brief Whether debug was given.  */

} ;

/** @brief The additional parameters to pass to parser functions */
struct mw_cmdline_params
{
  int override; /**< @brief whether to override possibly already present options (default 0) */
  int initialize; /**< @brief whether to initialize the option structure mw_args_info (default 1) */
  int check_required; /**< @brief whether to check that all required options were provided (default 1) */
  int check_ambiguity; /**< @brief whether to check for options already specified in the option structure mw_args_info (default 0) */
  int print_errors; /**< @brief whether getopt_long should print an error message for a bad option (default 1) */
} ;

/** @brief the purpose string of the program */
extern const char *mw_args_info_purpose;
/** @brief the usage string of the program */
extern const char *mw_args_info_usage;
/** @brief all the lines making the help output */
extern const char *mw_args_info_help[];

/**
 * The command line parser
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int mw_cmdline (int argc, char * const *argv,
  struct mw_args_info *args_info);

/**
 * The command line parser (version with additional parameters - deprecated)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use mw_cmdline_ext() instead
 */
int mw_cmdline2 (int argc, char * const *argv,
  struct mw_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The command line parser (version with additional parameters)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int mw_cmdline_ext (int argc, char * const *argv,
  struct mw_args_info *args_info,
  struct mw_cmdline_params *params);

/**
 * Save the contents of the option struct into an already open FILE stream.
 * @param outfile the stream where to dump options
 * @param args_info the option struct to dump
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int mw_cmdline_dump(FILE *outfile,
  struct mw_args_info *args_info);

/**
 * Save the contents of the option struct into a (text) file.
 * This file can be read by the config file parser (if generated by gengetopt)
 * @param filename the file where to save
 * @param args_info the option struct to save
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int mw_cmdline_file_save(const char *filename,
  struct mw_args_info *args_info);

/**
 * Print the help
 */
void mw_cmdline_print_help(void);
/**
 * Print the version
 */
void mw_cmdline_print_version(void);

/**
 * Initializes all the fields a mw_cmdline_params structure 
 * to their default values
 * @param params the structure to initialize
 */
void mw_cmdline_params_init(struct mw_cmdline_params *params);

/**
 * Allocates dynamically a mw_cmdline_params structure and initializes
 * all its fields to their default values
 * @return the created and initialized mw_cmdline_params structure
 */
struct mw_cmdline_params *mw_cmdline_params_create(void);

/**
 * Initializes the passed mw_args_info structure's fields
 * (also set default values for options that have a default)
 * @param args_info the structure to initialize
 */
void mw_cmdline_init (struct mw_args_info *args_info);
/**
 * Deallocates the string fields of the mw_args_info structure
 * (but does not deallocate the structure itself)
 * @param args_info the structure to deallocate
 */
void mw_cmdline_free (struct mw_args_info *args_info);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int mw_cmdline_required (struct mw_args_info *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* MWP_CMDLINE_H */
