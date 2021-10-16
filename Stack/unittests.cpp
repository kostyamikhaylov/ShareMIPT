#include "stack.h"

#include <math.h>
#include <assert.h>

#define DEFAULT_PRECISION 0.001

static int test_int (void);
static int test_my_struct (void);
static int test_max_capacity (void);

static bool IsEqual(double d1, double d2, double precision);

int print_int (const void *ptr);
int print_my_struct (const void *ptr);
int print_double (const void *ptr);

struct my_struct
{
	int i = 1;
	double d = 0;
};

struct my_struct ms_arr[5] = {	{3, 2231.34},
				{342, -2.34},
				{0, 0.0},
				{-102012, 3212.000},
				{-1, -0.0001121}};

#define test(name)							\
	do								\
	{								\
		if (test_##name ())					\
		{							\
			printf("!!! test_" #name " test failed\n");	\
			res |= 1;					\
		}							\
		else							\
			printf("+++ test_" #name " test passed\n");	\
	}								\
	while (0)

static int test_int (void)
{
	enum error_type error = OK;
	int d = 234, p = 0;
	Stack st = {};

	error = stack_ctor (&st, sizeof (int), "int", print_int);
	if (error != OK) {
		fprintf (stderr, "Unittests: failed to create stack of type int\n");
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		d = 2 * d + 100 - i;
		error = stack_push (&st, &d);
		if (error != OK) {
			fprintf (stderr, "Unittests: failed to push int to stack\n");
			return 1;
		}
	}

	for (int i = 4; i >= 0; i--) {
		error = stack_pop (&st, &p);
		if (error != OK) {
			fprintf (stderr, "Unittests: failed to pop int from stack\n");
			return 1;
		}
		if (d != p) {
			fprintf (stderr, "Unittests: stack returned wrong value of type int\n");
			return 1;
		}
		d = (d + i - 100) / 2;
	}

	error = stack_dtor (&st);
	if (error != OK) {
		fprintf (stderr, "Unittests: failed to destroy stack of type  int\n");
		return 1;
	}
	return 0;
}

static int test_my_struct (void)
{
	enum error_type error = OK;
	struct my_struct ms = {0, 0};
	Stack st = {};

	error = stack_ctor (&st, sizeof (struct my_struct), "struct my_struct", print_my_struct);
	if (error != OK) {
		fprintf (stderr, "Unittests: failed to create stack of type my_struct\n");
		return 1;
	}

	for (int i = 0; i < 5; i++) {
		error = stack_push (&st, ms_arr + i);
		if (error != OK) {
			fprintf (stderr, "Unittests: failed to push my_struct to stack\n");
			return 1;
		}
	}

	for (int i = 4; i >= 0; i--) {
		error = stack_pop (&st, &ms);
		if (error != OK) {
			fprintf (stderr, "Unittests: failed to pop my_struct from stack\n");
			return 1;
		}
		if (ms.i != ms_arr[i].i ||
		!IsEqual(ms.d, ms_arr[i].d, DEFAULT_PRECISION)) {
			fprintf (stderr, "Unittests: stack returned wrong value of type my_struct\n");
			return 1;
		}
	}

	error = stack_dtor (&st);
	if (error != OK) {
		fprintf (stderr, "Unittests: failed to destroy stack of type  my_struct\n");
		return 1;
	}

	return 0;
}

static int test_max_capacity (void)
{
	enum error_type error = OK;
	double d = 0.22, p = 0;
	Stack st = {};

	error = stack_ctor (&st, sizeof (double), "double", print_double);
	if (error != OK) {
		fprintf (stderr, "Unittests: failed to create stack of type double\n");
		return 1;
	}

	for (int i = 0; i < MAX_CAP; i++) {
		d += 0.25;
		error = stack_push (&st, &d);
		if (error != OK) {
			fprintf (stderr, "Unittests: failed to push double to stack\n");
			return 1;
		}
	}

	for (int i = 0; i < MAX_CAP; i++) {
		error = stack_pop (&st, &p);
		if (error != OK) {
			printf ("Failed to pop %dth value from stack; exiting...\n", i + 1);
			fprintf (stderr, "Unittests: failed to pop int from stack\n");
			return 1;
		}
		if (!IsEqual(d, p, DEFAULT_PRECISION)) {
			fprintf (stderr, "Unittests: stack returned wrong value of type double\n");
			return 1;
		}
		d -= 0.25;
	}

	error = stack_dtor (&st);
	if (error != OK) {
		fprintf (stderr, "Unittests: failed to destroy stack of type  double\n");
		return 1;
	}

	return 0;
}

static bool IsEqual(double d1, double d2, double precision)
{
	if (fabs (d1 - d2) < precision * fmax (1.0, fmax (d1, d2)))
		return true;
	return false;
}

bool run_unittests (void)
{
	int res = 0;

	printf ("------------------\n"
		"Unit testing: \n");

	test(int);
	test(my_struct);
	test(max_capacity);

	if (res)
	{
		printf ("Unit testing finished: FAILED\n"
			"------------------\n");
		return false;
	}

	printf ("Unit testing finished: PASSED\n"
		"------------------\n");
	return true;
}

int print_int (const void *ptr)
{
	assert (ptr);

	printf ("%d\n", *((const int *) ptr));
	return 0;
}

int print_double (const void *ptr)
{
	assert (ptr);

	printf ("%lg\n", *((const double *) ptr));
	return 0;
}

int print_my_struct (const void *ptr)
{
	assert (ptr);

	const struct my_struct *ex = (const struct my_struct *) ptr;
	printf ("{%d; %lg}\n", ex->i, ex->d);
	return 0;
}
