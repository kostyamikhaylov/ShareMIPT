#include <stdlib.h>

#include "sq_eq.h"

static int test_get_discriminant (void);
static int test_solve_linear (void);
static int test_solve_square (void);

/**
 * This macro calls testing function for the function name it received as an argument
 *
 * \note	It also prints if the test passed or failed
 * and raises the "res" flag in case of failure
 */
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

/**
 * @struct test_struct
 * @brief This structure contains four doubles:
 * coefficients of square equation a, b, c
 * and corresponding descriminant value d
 * @var test_struct::a
 * a-coefficient
 * @var test_struct::b
 * b-coefficient
 * @var test_struct::c
 * c-coefficient
 * @var test_struct::d
 * descriminant value (b^2 - 4*a*c)
 */
struct test_struct
{
	double a;
	double b;
	double c;
	double d;
};

/**
 * Checks that discrininant calculation via get_discriminant () is correct
 *
 * \return	0 in case of success, 1 in case of failure
 *
 * \note	Checking is based on ten predefined datasets in test_struct structures
 */
static int test_get_discriminant (void)
{
	double discr = 0;
	struct test_struct data[10] = { {100, 200, 33, 26800},
					{1, 3, 0, 9},
					{15, -49, 231, -11459},
					{-10000, 9000, 0.001, 8.1e+07},
					{0.0001, 1000, 20, 1e+06},
					{-0.01, -0.00001, -0.005, -1.99999e-05},
					{1000, 400, -400, 1.76e+06},
					{12345, 0, 0, 0},
					{0, 98, 0, 9604},
					{0, 0, 1234, 0} };

	for (int i = 0; i < 10; i++)
	{
		discr = get_discriminant (data[i].a, data[i].b, data[i].c);
		if (!IsEqual(discr, data[i].d, DEFAULT_PRECISION))
		{
			printf ("get_discrininant () failed:\n");
			printf ("Input: a = %lg, b = %lg, c = %lg\n",
					data[i].a, data[i].b, data[i].c);
			printf ("Output: d = %lg\n", discr);
			printf ("Correct output: d = %lg\n", data[i].d);
			return 1;
		}
	}
	return 0;
}

/**
 * Tests if solve_linear () function is correct
 *
 * \return	0 in case of success, 1 in case of failure
 *
 * \note	One hundred random coefficients k (!= 0) and b
 * are generated and fed to solve_linear ();
 * its output is compared with correct solution -b/k
 */
static int test_solve_linear (void)
{
	double k = 0, b = 0, x = 0;
	for (int i = 0; i < 100; i++)
	{
		k = (double) rand () + (double) rand () / RAND_MAX;
		if (IsEqual (k, 0, DEFAULT_PRECISION))
			continue;
		b = (double) rand () + (double) rand () / RAND_MAX;
		x = solve_linear (k, b);
		if (!IsEqual (k * x + b, 0, DEFAULT_PRECISION))
		{
			printf ("solve_linear () failed:\n");
			printf ("Input: k = %lg, b = %lg\n", k, b);
			printf ("Output: x = %lg\n", x);
			printf ("Correct output: x = %lg\n", -b / k);
			return 1;
		}
	}
	return 0;
}

/**
 * Tests if solve_square () function is correct
 *
 * \return	0 in case of success, 1 in case of failure
 *
 * \note	One hundred random coefficients a (!= 0), b and c
 * are generated and fed to solve_square (); then, if d >= 0,
 * root(s) is (are) found with solve_square ()
 * and are checked via substitution in the original equation
 */
static int test_solve_square (void)
{
	double a = 0, b = 0, c = 0, d = 0, x1 = 0, x2 = 0;
	for (int i = 0; i < 100; i++)
	{
		a = (double) rand () + (double) rand () / RAND_MAX;
		if (IsEqual (a, 0, DEFAULT_PRECISION))
			continue;
		b = (double) rand () + (double) rand () / RAND_MAX;
		c = (double) rand () + (double) rand () / RAND_MAX;
		d = get_discriminant (a, b, c);
		if (d < 0)
			continue;
		solve_square (a, b, d, &x1, &x2);
		if (IsEqual (x1, x2, DEFAULT_PRECISION))
		{
			if (!IsEqual (a*x1*x1 + b*x1 + c, 0, DEFAULT_PRECISION))
			{
				printf ("solve_square () failed:\n");
				printf ("Input: a = %lg, b = %lg, c = %lg\n", a, b, c);
				printf ("Output: one root x = %lg\n", x1);
				printf ("But ax^2 + bx + c = %lg\n", a*x1*x1 + b*x1 + c);
				return 1;
			}
		}
		else
		{
			if (!IsEqual (a*x1*x1 + b*x1 + c, 0, DEFAULT_PRECISION))
			{
				printf ("solve_square () failed:\n");
				printf ("Input: a = %lg, b = %lg, c = %lg\n", a, b, c);
				printf ("Output: first root x1 = %lg\n", x1);
				printf ("But ax1^2 + bx1 + c = %lg\n", a*x1*x1 + b*x1 + c);
				return 1;
			}
			if (!IsEqual (a*x2*x2 + b*x2 + c, 0, DEFAULT_PRECISION))
			{
				printf ("solve_square () failed:\n");
				printf ("Input: a = %lg, b = %lg, c = %lg\n", a, b, c);
				printf ("Output: second root x2 = %lg\n", x2);
				printf ("But ax2^2 + bx2 + c = %lg\n", a*x2*x2 + b*x2 + c);
				return 1;
			}
		}
	}
	return 0;
}

/**
 * Runs all the unit tests
 *
 * \return	True if all the tests passed successfully; False otherwise
 */
bool run_unittests (void)
{
	int res = 0;

	printf ("------------------\n"
		"Unit testing: \n");

	test(get_discriminant);
	test(solve_linear);
	test(solve_square);

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
