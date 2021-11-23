#ifndef AKINATOR_H
#define AKINATOR_H

#include <stdio.h>

typedef struct Node {
	char *data = NULL;
	struct Node *left = NULL;
	struct Node *right = NULL;
} node;

enum mode {
	MODE_ERROR,
	MODE_LOAD,
	MODE_PLAY,
	MODE_DESC,
	MODE_DIFF,
	MODE_SAVE,
	MODE_DRAW,
	MODE_EXIT
};

enum mode akinator_get_mode (void);
void akinator_load_base (void);
void akinator_play (void);
void akinator_description (void);
void akinator_diff (void);
void akinator_save_base (void);
void akinator_draw (void);
void akinator_exit (void);

#endif // AKINATOR_H
