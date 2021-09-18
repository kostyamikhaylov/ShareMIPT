#include "table.h"


int main ()
{
	int n = 4;
	struct table *t = allocate_table (n);

	put_to_table (t, {10, 11}, 2, 3);
	put_to_table (t, {23, 21}, 1, 4);
	put_to_table (t, {9, 1}, 3, 4);
	struct cell c = {0, 0};
	get_from_table (t, &c, 2, 3);
	printf ("%d VS %d -->  %d:%d\n", 2, 3, c.team1, c.team2);
	get_from_table (t, &c, 3, 2);
	printf ("%d VS %d -->  %d:%d\n", 3, 2, c.team1, c.team2);

	print_table (t);

	free_table (t);
	return 0;
}

