#include "onegin_toolkit.h"


int main (int argc, char *argv[])
{
	int err = 0;
	struct text hamlet = {};

	err = parse_arguments (argc, argv, &hamlet);

	if (hamlet.keys & KEYS_HELP) {
		print_help (argv[0]);
		return (hamlet.keys & KEYS_ERROR);
	}

	err = get_text (&hamlet);
	if (err) {
		return 1;
	}

	err = index_text (&hamlet);
	if (err) {
		free (hamlet.text);
		return 1;
	}

	sort_text (&hamlet);

	err = print_text (&hamlet);
	{
		free (hamlet.index);
		free (hamlet.text);
		return 1;
	}

	free (hamlet.text);
	free (hamlet.index);
	return 0;
}

