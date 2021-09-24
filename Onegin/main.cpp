#include "onegin_toolkit.h"


int main (int argc, char *argv[])
{
	int keys = 0;
	char *filename_to = NULL, *filename_from = NULL;
	char *text = NULL;
	size_t text_len = 0, index_len = 0;
	char **index = NULL;

	keys = parse_arguments (argc, argv, &filename_from, &filename_to);

	if (keys & KEYS_HELP) {
		print_help (argv[0]);
		return 0;
	}

	text = get_text (filename_from, &text_len);
	if (!text) {
		return 1;
	}

	index = index_text (text, text_len, &index_len);
	if (!index) {
		free (text);
		return 1;
	}

	sort_text (index, index_len, keys);

	if (print_text (index, index_len, filename_to, keys))
	{
		free (index);
		free (text);
		return 1;
	}

	free (text);
	free (index);
	return 0;
}

