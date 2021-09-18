#include "print_array.h"

#define X_SIZE 4
#define Y_SIZE 3

#define DOUBLE_SIZE 5


int main ()
{
	int data[Y_SIZE][X_SIZE] = {{21, 34, 56, 34},
				     {76, 423, 4223, 234},
				     {1, 2, 76, 8}};
	print_array_2D_int ((int *) data, X_SIZE, Y_SIZE);
	double doubles[DOUBLE_SIZE] = {1.0, 3.0202, 0.33221, 31221.12, 312312};
	print_array_1D_double (doubles, DOUBLE_SIZE);
	return 0;
}

