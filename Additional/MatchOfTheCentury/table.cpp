#include "table.h"


static int get_cells_number (int n);
static int get_shift (const int n, const int x, const int y);

struct table *allocate_table (const int n)
{
	assert (n > 1);

	struct table *t = (struct table *) calloc (1, sizeof (struct table));
	if (!t)
	{
		printf ("allocate_table () failed: can't allocate memory for struct table\n");
		return NULL;
	}

	t->n = n;

	t->cell_arr = (struct cell *) calloc ((size_t) get_cells_number (n), sizeof (struct cell));
	if (!t->cell_arr)
	{
		printf ("allocate_table () failed: can't allocate memory for %d structs cell\n", n);
		free (t);
		return NULL;
	}

	return t;
}

void free_table (struct table *t)
{
	assert (t != NULL);
	assert (t->n > 1);
	assert (t->cell_arr != NULL);

	free (t->cell_arr);
	free (t);

	return ;
}

void put_to_table (const struct table *t, const struct cell c,
			const int team1, const int team2)
{
	assert (t != NULL);
	assert (t->n > 1);
	assert (t->cell_arr != NULL);

	const int n = t->n;

	assert (team1 > 0);
	assert (team1 <= n);
	assert (team2 > 0);
	assert (team2 <= n);
	assert (team1 != team2);

	if (team1 < team2)
	{
		t->cell_arr[get_shift (n, team1 - 1, team2 - 1)].team1 = c.team1;
		t->cell_arr[get_shift (n, team1 - 1, team2 - 1)].team2 = c.team2;
	}
	else
	{
		t->cell_arr[get_shift (n, team2 - 1, team1 - 1)].team2 = c.team1;
		t->cell_arr[get_shift (n, team2 - 1, team1 - 1)].team1 = c.team2;
	}

	return ;
}

void get_from_table (const struct table *t, struct cell * c,
			const int team1, const int team2)
{
	assert (t != NULL);
	assert (t->n > 1);
	assert (t->cell_arr != NULL);

	const int n = t->n;

	assert (team1 > 0);
	assert (team1 <= n);
	assert (team2 > 0);
	assert (team2 <= n);
	assert (team1 != team2);

	if (team1 < team2)
	{
		c->team1 = t->cell_arr[get_shift (n, team1 - 1, team2 - 1)].team1;
		c->team2 = t->cell_arr[get_shift (n, team1 - 1, team2 - 1)].team2;
	}
	else
	{
		c->team1 = t->cell_arr[get_shift (n, team2 - 1, team1 - 1)].team2;
		c->team2 = t->cell_arr[get_shift (n, team2 - 1, team1 - 1)].team1;
	}

	return ;
}

void print_table (const struct table *t)
{
	assert (t != NULL);
	assert (t->n > 1);
	assert (t->cell_arr != NULL);

	const int n = t->n;

	for (int y = 0; y < n; y++)
		for (int x = 0; x < n; x++)
		{
			if (x >= y)
				printf ("|xxxxxx:xxxxxx|%c", (x == n - 1) ? '\n' : '\0');
			else
				printf ("|%6d:%-6d|%c",
						t->cell_arr[get_shift (n, x, y)].team1,
						t->cell_arr[get_shift (n, x, y)].team2,
						(x == n - 1) ? '\n' : '\0');
		}
	return ;
}

static int get_shift (const int n, const int x, const int y)
{
	assert (n > 0);
	assert (x >= 0);
	assert (x < n);
	assert (y >= 0);
	assert (y < n);
	assert (x < y);

	//TODO
	int c = n;
	c++;
	return y * (y - 1) / 2 + x;
}

static int get_cells_number (const int n)
{
	assert (n > 1);

	return n * (n - 1) / 2;
}
