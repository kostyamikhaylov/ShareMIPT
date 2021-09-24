#include "onegin_toolkit.h"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdint.h>


char default_filename_from[] = "Shakespeare.txt";	// Default source file
char default_filename_to[] = "Sorted.txt";		// Default output file

static int get_keys (char *str);
static int compare (const void *first, const void *second);
static int compare_reverse (const void *first, const void *second);
static const char *process_punctuation (const char *str, const bool reverse);
static int strcicmp (const char *a, const char *b);

/**
 * Parses command line arguments
 *
 * \param [in]	argc		number of arguments
 * \param [in]	argv		arguments vector
 * \param [out]	filename_from	name of source file
 * \param [out]	filename_to	name of file for output
 *
 * \return	keys value (OR'ed keys, see header)
 */
int parse_arguments (const int argc, char *argv[], char **filename_from, char **filename_to)
{
	int keys = 0;
	*filename_from = NULL;
	*filename_to = NULL;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			break;
		} else if (!strcmp (argv[i], "-o")) {
			if (++i < argc) {
				*filename_to = argv[i];
				keys |= KEYS_FILENAME_TO;
			} else {
				printf ("parce_arguments () error: ");
				printf ("no filename after -o key\n");
				keys = KEYS_HELP;
			}
		} else if (!strcmp (argv[i], "-i")) {
			if (++i < argc) {
				*filename_from = argv[i];
				keys |= KEYS_FILENAME_FROM;
			} else {
				printf ("parce_arguments () error: ");
				printf ("no filename after -i key\n");
				keys = KEYS_HELP;
			}
		} else {
			keys |= get_keys (argv[i] + 1);
		}
	}

	if (!*filename_from)
		*filename_from = default_filename_from;
	if (!*filename_to)
		*filename_to = default_filename_to;

	return keys;
}

/**
 * Parses keys
 *
 * \param [in]	str		string with keys
 *
 * \return	keys value from the string str (OR'ed keys, see header)
 */
static int get_keys (char *str)
{
	int keys = 0, i = 0;

	while (str[i] != '\0') {
		switch (str[i]) {
			case 'n':
				keys |= KEYS_ENUMERATE;
				break;
			case 'h':
				keys |= KEYS_HELP;
				break;
			case 'e':
				keys |= KEYS_SORT_FROM_END;
				break;
			case 's':
				keys |= KEYS_NO_SORT;
				break;
			case 'r':
				keys |= KEYS_REVERSE;
				break;
			default:
				printf ("parce_arguments () error: ");
				printf ("unknown argument %c\n", str[i]);
				keys |= KEYS_HELP;
				break;
		}
		i++;
	}

	return keys;
}

/**
 * Prints information about the program and keys to the screen
 *
 * \param [in]	prog_name	the name of the program being executed
 *
 * \return	Nothing
 */
void print_help (char *prog_name)
{
	printf ("Usage: %s [key [FILE]]\n", prog_name);
	printf ("This program sorts the input strings in the order you wish.\n");
	printf ("The order is specified by command line key; default is alphabetical\n");
	printf ("The input and output files are also specified by keys;\n");
	printf ("Default files are %s (to read from) and %s (to write to).\n",
			default_filename_from, default_filename_to);
	printf ("\nKeys:\n");
	printf ("	-e	print strings from their end\n");
	printf ("	-h	show this message\n");
	printf ("	-n	print strings numbers\n");
	printf ("	-r	print strings n reverse order\n");
	printf ("	-s	do not sort strings\n");
	printf ("	-i [FILE]	read strings from FILE\n");
	printf ("	-o [FILE]	write strings to FILE\n");
	printf ("\n");

	return ;
}

/**
 * Checks the input file length, allocates buffer of the same length and then copies the file content into it. Passes text length via the last argument. In case of failure, returns NULL
 *
 * \param [in]	filename_from	the name of the program being executed
 * \param [out]	text_len	pointer to a variable to store the length of the text
 *
 * \return	in case of success, returns pointer to the allocated buffer with text. If allocating the buffer, reading or opening the file fails, returns NULL
 */
char *get_text (const char *filename_from, size_t *text_len)
{
	int fd = -1;
	struct stat st;
	char *text = NULL;

	if ((fd = open (filename_from, O_RDONLY)) < 1) {
		printf ("get_text () error: can't open file \"%s\"", filename_from);
		perror ("");
		return NULL;
	}

	if (fstat (fd, &st) < 0) {
		perror ("get_text () error: fstat syscall failed");
		close (fd);
		return NULL;
	}

	*text_len = (size_t) st.st_size;

	text = (char *) calloc (*text_len + 1, sizeof (*text));
	if (!text) {
		perror ("get_text () error: can't allocate memory for text");
		close (fd);
		return NULL;
	}

	if (read (fd, text, *text_len) < (ssize_t) *text_len) {
		perror ("get_text () error: read syscall failed, can't read the whole file");
		free (text);
		close (fd);
		return NULL;
	}

	close (fd);
	return text;
}

/**
 * Divides plain text into strings. The pointers to the beginnings of strings are stored in index array. In this function it is allocated, filled in and returned in case of success
 *
 * \param [in]	text		plain text, later divided into strings with '\0' symbols
 * \param [in]	text_len	text length
 * \param [out]	index_len	pointer to the variable to put the length of index array (the number of strings)
 *
 * \return	in case of success, returns pointer to the index array. If allocating the buffer fails, returns NULL
 */
char **index_text (char *text, const size_t text_len, size_t *index_len)
{
	size_t index_cnt = 0;
	char **index = NULL;

	for (size_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
			text[i] = '\0';
			index_cnt++;
		}
	}

	*index_len = index_cnt;

	index = (char **) calloc (index_cnt, sizeof (*index));
	if (!index){
		perror ("index_text () error: read () syscall error");
		return NULL;
	}

	index_cnt = 1;
	index[0] = text;
	for (size_t i = 1; i < text_len; i++) {
		if (text[i-1] == '\0') {
			index[index_cnt] = text + i;
			index_cnt++;
		}
	}

	return index;
}

/**
 * Sorts the text by moving indices according to keys
 *
 * \param [in]	index		index array for sorting
 * \param [in]	index_len	index array length (the number of strings)
 * \param [in]	keys		keys, according to which the text is sorted
 *
 * \return	Nothing
 */
void sort_text (char **index, const size_t index_len, const int keys)
{
	if (keys & KEYS_NO_SORT)
		return ;
	if (keys & KEYS_SORT_FROM_END) {
		qsort (index, index_len, sizeof (*index), compare_reverse);
	} else {
		qsort (index, index_len, sizeof (*index), compare);
	}
	return ;
}

/**
 * Compares two strings in case insensitive manner
 *
 * \param [in]	first		void*-casted pointer to pointer to the first string
 * \param [in]	second		void*-casted pointer to pointer to the second string
 *
 * \return	Integer less than, equal to, or greater than zero if first string is found, respectively, to be less than, to match, or be greater than second
 * \note	The comparison is case insensitive and skips leading punctuation characters
 */
static int compare (const void *first, const void *second)
{
	const long int *li_ptr_first = (const long int *) first;
	const long int *li_ptr_second = (const long int *) second;
	const long int li_first = *li_ptr_first;
	const long int li_second = *li_ptr_second;
	const char *str1_unprocessed = (const char *) li_first;
	const char *str2_unprocessed = (const char *) li_second;
	const char *str1 = process_punctuation (str1_unprocessed, false);
	const char *str2 = process_punctuation (str2_unprocessed, false);

	return strcicmp (str1, str2);
}

/**
 * Compares two strings in case insensitive manner from end to beginning
 *
 * \param [in]	first		void*-casted pointer to pointer to the first string
 * \param [in]	second		void*-casted pointer to pointer to the second string
 *
 * \return	Integer less than, equal to, or greater than zero if first string is found, respectively, to be less than, to match, or be greater than second
 * \note	The comparison is made from the last string characters to first, is case insensitive and skips leading punctuation characters
 */
static int compare_reverse (const void *first, const void *second)
{
	int d = 0;
	const long int *li_ptr_first = (const long int *) first;
	const long int *li_ptr_second = (const long int *) second;
	const long int li_first = *li_ptr_first;
	const long int li_second = *li_ptr_second;
	const char *str1_unprocessed = (const char *) li_first;
	const char *str2_unprocessed = (const char *) li_second;
	const char *str1 = process_punctuation (str1_unprocessed, true);
	const char *str2 = process_punctuation (str2_unprocessed, true);

	for (;; str1--, str2--) {
		d = tolower ((unsigned char) *str1) -
		    tolower ((unsigned char) *str2);
		if (d != 0 || str1 == str1_unprocessed)
			return d;
	}
}

/**
 * Skips leading punctuation characters in a string
 *
 * \param [in]	str		pointer to the string
 * \param [in]	reverse		bool value for receiving the direction of processing
 *
 * \return	pointer to first non-punctuation character if reverse is false, otherwice pointer to the last non-punctuation character of the string str
 */
static const char *process_punctuation (const char *str, const bool reverse)
{
	const char *s = str;
	if (reverse) {
		if (strlen (str))
			s = str + strlen (str) - 1;
		while (ispunct (*s) && s != str) {
			s--;
		}
		return s;
	}

	while (ispunct (*s)) {
		s++;
	}
	return s;
}

/**
 * Compares two strings in case insensitive manner
 *
 * \param [in]	a		pointer to the first string
 * \param [in]	b		pointer to the second string
 *
 * \return	Integer less than, equal to, or greater than zero if first string is found, respectively, to be less than, to match, or be greater than second
 */
static int strcicmp (const char *a, const char *b)
{
	int d = 0;
	for (;; a++, b++) {
		d = tolower ((unsigned char) *a) -
		    tolower ((unsigned char) *b);
		if (d != 0 || !*a)
			return d;
	}
}

/**
 * Prints the sorted strings to the output file according to specified keys
 *
 * \param [in]	index		array of string indices
 * \param [in]	index_len	length of index array
 * \param [in]	filename_to	name of file to print the strings
 * \param [in]	keys		keys defining the format of printing
 *
 * \return	If opening the output file or printing to it failed, returns non-zero value; otherwice, returns 0
 * \note	Empty strings are not printed
 */
int print_text (char **index, const size_t index_len, const char *filename_to, const int keys)
{
	FILE* file = NULL;
	size_t str_len = 0, str_num = 1;

	file = fopen (filename_to, "w");
	if (!file) {
		printf ("print_text () error: can't open file \"%s\"", filename_to);
		perror ("");
		return 1;
	}

	if (keys & KEYS_REVERSE) {
		for (size_t i = index_len - 1; i != SIZE_MAX; i--) {
			str_len = strlen (index[i]);
			if (!str_len)
				continue;
			if (keys & KEYS_ENUMERATE){
				if (fprintf (file, "%6lu) %s\n", str_num++, index[i]) < (int) str_len + 9)
				{
					printf ("print_text () error: fprintf () failed on iteration %lu", i);
					perror ("");
					fclose (file);
					return 1;
				}
			} else {
				if (fprintf (file, "%s\n", index[i]) < (int) str_len + 1)
				{
					printf ("print_text () error: fprintf () failed on iteration %lu", i);
					perror ("");
					fclose (file);
					return 1;
				}
			}
		}
	}
	else {
		for (size_t i = 0; i < index_len; i++) {
			str_len = strlen (index[i]);
			if (!str_len)
				continue;
			if (keys & KEYS_ENUMERATE){
				if (fprintf (file, "%6lu) %s\n", str_num++, index[i]) < (int) str_len + 9)
				{
					printf ("print_text () error: fprintf () failed on iteration %lu", i);
					perror ("");
					fclose (file);
					return 1;
				}
			} else {
				if (fprintf (file, "%s\n", index[i]) < (int) str_len + 1)
				{
					printf ("print_text () error: fprintf () failed on iteration %lu", i);
					perror ("");
					fclose (file);
					return 1;
				}
			}
		}
	}

	fclose (file);
	return 0;
}

