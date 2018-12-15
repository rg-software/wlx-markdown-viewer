#ifndef HOEDOWNCOMMON_H
#define HOEDOWNCOMMON_H

#include "common.h"
#include "version.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define str(x) __str(x)
#define __str(x) #x

#define count_of(arr) (sizeof(arr)/sizeof(0[arr]))

#ifdef __cplusplus
extern "C" char* INPUT_STRING;
extern "C" char* SP_INPUT_STRING;
extern "C" char* OUTPUT_STRING;
extern "C" char* SP_OUTPUT_STRING;
#else
extern char* INPUT_STRING;
extern char* SP_INPUT_STRING;
extern char* OUTPUT_STRING;
extern char* SP_OUTPUT_STRING;
#endif

int parseint(const char *string, long *result);
const char * strprefix(const char *str, const char *prefix);
void print_option(char short_opt, const char *long_opt, const char *description);
void print_version();
int parse_options(int argc, char **argv, int(*parse_short_option)(char opt, char *next, void *opaque), int(*parse_long_option)(char *opt, char *next, void *opaque),
	int(*parse_argument)(int argn, char *arg, int is_forced, void *opaque), void *opaque);

#endif // HOEDOWNCOMMON_H
