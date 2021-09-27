#ifndef ONEGIN_TOOLKIT_H
#define ONEGIN_TOOLKIT_H

#include <stdio.h>
#include <stdlib.h>

/*
 * A number of keys to change the behavior of the program. They are set via command line parameter.
 *
 * For more informaton, run program with -h key.
 */
#define KEYS_HELP		0x00000001
#define KEYS_ERROR		0x00000002
#define KEYS_FILENAME_TO	0x00000010
#define KEYS_FILENAME_FROM	0x00000020
#define KEYS_ENUMERATE		0x00000040
#define KEYS_NO_SORT		0x00000100
#define KEYS_REVERSE		0x00000200
#define KEYS_SORT_FROM_END	0x00000400

/*
 * A struct for holding all the data for the sorting
 *
 */
struct text
{
	char *text = NULL;
	size_t text_len = 0;
	char **index = NULL;
	size_t index_len = 0;
	int keys = KEYS_HELP;
	char *filename_from = NULL;
	char *filename_to = NULL;
};

int parse_arguments (const int argc, char *argv[], struct text *hamlet);
void print_help (char *prog_name);
int get_text (struct text *hamlet);
int index_text (struct text *hamlet);
void sort_text (struct text *hamlet);
int print_text (struct text *hamlet);

#endif // ONEGIN_TOOLKIT_H
