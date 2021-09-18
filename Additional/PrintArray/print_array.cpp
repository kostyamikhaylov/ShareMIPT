#include "print_array.h"


void print_array_2D_int (int *data, int size_x, int size_y)
{
	assert (size_x >= 0);
	assert (size_y >= 0);

	printf ("----------\nArray of integers %dx%d:\n", size_x, size_y);
	for (int y = 0; y < size_y; y++)
		for (int x = 0; x < size_x; x++)
		{
			printf ("%d%c", data[y*size_x + x],
					(x == size_x - 1) ? '\n' : ' ');
		}
	printf ("----------\n");
}

void print_array_1D_double (double *data, int size_x)
{
	assert (size_x >= 0);

	printf ("----------\nArray of doubles of %d length\n", size_x);
	for (int x = 0; x < size_x; x++)
	{
		printf ("%lg%c", data[x],
				(x == size_x - 1) ? '\n' : ' ');
	}
	printf ("----------\n");
}
