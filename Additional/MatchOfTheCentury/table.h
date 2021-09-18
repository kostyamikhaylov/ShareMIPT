#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

struct cell
{
	int team1;
	int team2;
};

struct table
{
	int n;
	struct cell *cell_arr;
};

struct table *allocate_table (const int n);
void free_table (struct table *t);
void put_to_table (const struct table *t, const struct cell c,
			const int team1, const int team2);
void get_from_table (const struct table *t, struct cell * c,
			const int team1, const int team2);
void print_table (const table *t);

#endif //TABLE_H
