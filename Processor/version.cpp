#include "processor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIGNATURE "KM"
#define VERSION "v4"


int write_sign_and_ver (FILE * file)
{
	if (!file) {
		fprintf (stderr, "write_sign_ans_ver () error: file was not opened\n");
		return 1;
	}

	if (ftell (file) != 0) {
		fprintf (stderr, "write_sign_ans_ver () warning: file position was rewinded\n");
		rewind (file);
	}

	if (fprintf (file, "%s%s", SIGNATURE, VERSION) <
		(int) (sizeof (SIGNATURE) - 1 + sizeof (VERSION) - 1)) {
		fprintf (stderr, "write_sign_ans_ver () error: failed to write\n");
		return 1;
	}

	return 0;
}

int check_sign_and_ver (FILE * file)
{
	size_t len = sizeof (SIGNATURE) - 1 + sizeof (VERSION) - 1;
	char *sign_and_ver = NULL;

	sign_and_ver = (char *) calloc (len + 1, sizeof (char));
	if (!sign_and_ver) {
		fprintf (stderr, "write_sign_ans_ver () error: failed to allocate memory\n");
		return -1;
	}

	if (!file) {
		fprintf (stderr, "write_sign_ans_ver () error: file was not opened\n");
		free (sign_and_ver);
		return -1;
	}

	rewind (file);

	if (fread (sign_and_ver, sizeof (char), len, file) < len) {
		fprintf (stderr, "check_sign_ans_ver () error: failed to read\n");
		free (sign_and_ver);
		return -1;
	}

	if (strncmp (sign_and_ver, SIGNATURE, sizeof (SIGNATURE) - 1)) {
		fprintf (stderr, "check_sign_ans_ver () error: signature is not correct\n");
		free (sign_and_ver);
		return -1;
	}

	if (strncmp (sign_and_ver + sizeof (SIGNATURE) - 1, VERSION, sizeof (VERSION) - 1)) {
		fprintf (stderr, "check_sign_ans_ver () error: version is not correct\n");
		free (sign_and_ver);
		return -1;
	}

	free (sign_and_ver);
	return (int) len;
}

