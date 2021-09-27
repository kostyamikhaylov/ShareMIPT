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
static int strcicmp (const char *a, const char *b);

/**
 * Parses command line arguments. Puts keys and source and output file names to the structure hamlet.
 *
 * \param [in]	argc		number of arguments
 * \param [in]	argv		arguments vector
 * \param [out]	hamlet		structure which holds all the information
 *
 * \return	0 in case of success, 1 otherwice
 */
int parse_arguments (const int argc, char *argv[], struct text *hamlet)
{
	int err = 0, keys = 0;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			break;
		} else if (!strcmp (argv[i], "-o")) {
			if (++i < argc) {
				hamlet->filename_to = argv[i];
				keys |= KEYS_FILENAME_TO;
			} else {
				printf ("parce_arguments () error: ");
				printf ("no filename after -o key\n");
				keys |= KEYS_HELP;
				keys |= KEYS_ERROR;
				err = 1;
			}
		} else if (!strcmp (argv[i], "-i")) {
			if (++i < argc) {
				hamlet->filename_from = argv[i];
				keys |= KEYS_FILENAME_FROM;
			} else {
				printf ("parce_arguments () error: ");
				printf ("no filename after -i key\n");
				keys |= KEYS_HELP;
				keys |= KEYS_ERROR;
				err = 1;
			}
		} else {
			keys |= get_keys (argv[i] + 1);
			if (keys & KEYS_ERROR)
				err = 1;
		}
	}

	if (!hamlet->filename_from)
		hamlet->filename_from = default_filename_from;
	if (!hamlet->filename_to)
		hamlet->filename_to = default_filename_to;

	hamlet->keys = keys;
	return err;
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
				keys |= KEYS_ERROR;
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
 * Checks the input file length, allocates buffer of the same length and then copies the file content into it. Stores the text and its length in hamlet structure
 *
 * \param [in,out]	hamlet	pointer to a structure to take input filename from and to store the length of the text in
 *
 * \return	0 in case of success, 1 otherwice (if allocating the buffer, reading or opening the file failed)
 */
int get_text (struct text *hamlet)
{
	int fd = -1;
	struct stat st;

	if ((fd = open (hamlet->filename_from, O_RDONLY)) < 1) {
		printf ("get_text () error: can't open file \"%s\"", hamlet->filename_from);
		perror ("");
		return 1;
	}

	if (fstat (fd, &st) < 0) {
		perror ("get_text () error: fstat syscall failed");
		close (fd);
		return 1;
	}

	hamlet->text_len = (size_t) st.st_size;

	hamlet->text = (char *) calloc (hamlet->text_len + 1, sizeof (*hamlet->text));
	if (!hamlet->text) {
		perror ("get_text () error: can't allocate memory for text");
		close (fd);
		return 1;
	}

	if (read (fd, hamlet->text, hamlet->text_len) < (ssize_t) hamlet->text_len) {
		perror ("get_text () error: read syscall failed, can't read the whole file");
		free (hamlet->text);
		close (fd);
		return 1;
	}

	close (fd);
	return 0;
}

/**
 * Divides plain text into strings. The pointers to the beginnings of strings are stored in index array in hamlet structure, which is allocated, filled in this function. Length of index array is also written to hamlet here
 *
 * \param [in, out]	hamlet		structure with text and indices
 *
 * \return	0 in case of success, 1 otherwice (if failed to allocate index array)
 */
int index_text (struct text *hamlet)
{
	size_t index_cnt = 0;

	for (size_t i = 0; i < hamlet->text_len; i++) {
		if (hamlet->text[i] == '\n') {
			hamlet->text[i] = '\0';
			index_cnt++;
		}
	}

	hamlet->index_len = index_cnt;

	hamlet->index = (char **) calloc (index_cnt, sizeof (*hamlet->index));
	if (!hamlet->index){
		perror ("index_text () error: read () syscall error");
		return 1;
	}

	index_cnt = 1;
	hamlet->index[0] = hamlet->text;
	for (size_t i = 1; i < hamlet->text_len; i++) {
		if (hamlet->text[i-1] == '\0') {
			hamlet->index[index_cnt] = hamlet->text + i;
			index_cnt++;
		}
	}

	return 0;
}

/**
 * Sorts the text by moving indices according to keys
 *
 * \param [in, out]	hamlet	structure that contains indices to sort and keys
 *
 * \return	Nothing
 */
void sort_text (struct text *hamlet)
{
	if (hamlet->keys & KEYS_NO_SORT)
		return ;
	if (hamlet->keys & KEYS_SORT_FROM_END) {
		qsort (hamlet->index, hamlet->index_len, sizeof (*hamlet->index), compare_reverse);
	} else {
		qsort (hamlet->index, hamlet->index_len, sizeof (*hamlet->index), compare);
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
 * \note	The comparison is case insensitive and skips all punctuation characters
 */
static int compare (const void *first, const void *second)
{
	const long int *li_ptr_first = (const long int *) first;
	const long int *li_ptr_second = (const long int *) second;
	const long int li_first = *li_ptr_first;
	const long int li_second = *li_ptr_second;
	const char *str1 = (const char *) li_first;
	const char *str2 = (const char *) li_second;

	return strcicmp (str1, str2);
}

/**
 * Compares two strings in case and punctuation insensitive manner from end to beginning
 *
 * \param [in]	first		void*-casted pointer to pointer to the first string
 * \param [in]	second		void*-casted pointer to pointer to the second string
 *
 * \return	Integer less than, equal to, or greater than zero if first string is found, respectively, to be less than, to match, or be greater than second
 * \note	The comparison is made from the last string characters to first, is case insensitive and skips all punctuation characters
 */
static int compare_reverse (const void *first, const void *second)
{
	int d = 0;
	const long int *li_ptr_first = (const long int *) first;
	const long int *li_ptr_second = (const long int *) second;
	const long int li_first = *li_ptr_first;
	const long int li_second = *li_ptr_second;
	const char *str1 = (const char *) li_first;
	const char *str2 = (const char *) li_second;
	const char *a = str1 + strlen (str1);
	const char *b = str2 + strlen (str2);

	for (;; a--, b--) {
		while (ispunct (*a)) { a--; };
		while (ispunct (*b)) { b--; };
		d = tolower ((unsigned char) *a) -
		    tolower ((unsigned char) *b);
		if (d != 0 || a == str1 || b == str2)
			return d;
	}
}

/**
 * Compares two strings in case and punctuation insensitive manner
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
		while (ispunct (*a)) { a++; };
		while (ispunct (*b)) { b++; };
		d = tolower ((unsigned char) *a) -
		    tolower ((unsigned char) *b);
		if (d != 0 || !*a)
			return d;
	}
}

/**
 * Prints the sorted strings to the output file according to specified keys
 *
 * \param [in]	hamlet		structure that contains name of output file, sorted lines and keys for printing
 *
 * \return	0 in case of success, 1 otherwice (if opening the output file or printing to it failed)
 * \note	Empty strings are not printed
 */
int print_text (struct text *hamlet)
{
	FILE* file = NULL;
	size_t str_len = 0, str_num = 1;

	file = fopen (hamlet->filename_to, "w");
	if (!file) {
		printf ("print_text () error: can't open file \"%s\"", hamlet->filename_to);
		perror ("");
		return 1;
	}

	if (hamlet->keys & KEYS_REVERSE) {
		for (size_t i = hamlet->index_len - 1; i != SIZE_MAX; i--) {
			str_len = strlen (hamlet->index[i]);
			if (!str_len)
				continue;
			if (hamlet->keys & KEYS_ENUMERATE){
				if (fprintf (file, "%6lu) %s\n", str_num++, hamlet->index[i]) < (int) str_len + 9)
				{
					printf ("print_text () error: fprintf () failed on iteration %lu", i);
					perror ("");
					fclose (file);
					return 1;
				}
			} else {
				if (fprintf (file, "%s\n", hamlet->index[i]) < (int) str_len + 1)
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
		for (size_t i = 0; i < hamlet->index_len; i++) {
			str_len = strlen (hamlet->index[i]);
			if (!str_len)
				continue;
			if (hamlet->keys & KEYS_ENUMERATE){
				if (fprintf (file, "%6lu) %s\n", str_num++, hamlet->index[i]) < (int) str_len + 9)
				{
					printf ("print_text () error: fprintf () failed on iteration %lu", i);
					perror ("");
					fclose (file);
					return 1;
				}
			} else {
				if (fprintf (file, "%s\n", hamlet->index[i]) < (int) str_len + 1)
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

