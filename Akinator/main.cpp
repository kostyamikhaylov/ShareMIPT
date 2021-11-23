#include "akinator.h"


int main ()
{
	while (1) {
		switch (akinator_get_mode ()) {
			case MODE_LOAD:
				akinator_load_base ();
				break;
			case MODE_PLAY:
				akinator_play ();
				break;
			case MODE_DESC:
				akinator_description ();
				break;
			case MODE_DIFF:
				akinator_diff ();
				break;
			case MODE_SAVE:
				akinator_save_base ();
				break;
			case MODE_DRAW:
				akinator_draw ();
				break;
			case MODE_EXIT:
				akinator_exit ();
				printf ("Thank you for playing!\n");
				return 0;
				break;
			case MODE_ERROR:
			default:
				fprintf (stderr, "Can't recognize game mode.\nExiting...\n");
				return 1;
				break;
		}
	}
}

