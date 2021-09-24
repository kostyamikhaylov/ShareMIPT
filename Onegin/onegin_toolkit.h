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
#define KEYS_FILENAME_TO	0x00000010
#define KEYS_FILENAME_FROM	0x00000020
#define KEYS_ENUMERATE		0x00000040
#define KEYS_NO_SORT		0x00000100
#define KEYS_REVERSE		0x00000200
#define KEYS_SORT_FROM_END	0x00000400


int parse_arguments (const int argc, char *argv[], char **filename_from, char **filename_to);
void print_help (char *prog_name);
char *get_text (const char *filename_from, size_t *text_len);
char **index_text (char *text, const size_t text_len, size_t *index_len);
void sort_text (char **index, const size_t index_len, const int keys);
int print_text (char **index, const size_t index_len, const char *filename_to, const int keys);

#endif // ONEGIN_TOOLKIT_H
