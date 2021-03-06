#include <math.h>

#include "sq_eq.h"

/*!
 * Checks if all input floating point arguments are finite
 */
#define IS_FINITE_2(float1, float2)		\
	do					\
	{					\
		assert (isfinite(float1));	\
		assert (isfinite(float2));	\
	} while (0)

/*!
 * Checks if all input floating point arguments are finite
 */
#define IS_FINITE_3(float1, float2, float3)	\
	do					\
	{					\
		assert (isfinite(float1));	\
		assert (isfinite(float2));	\
		assert (isfinite(float3));	\
	} while (0)


/*!
 * Types of square equation
 */
enum EqType
{
	NO_ROOTS,
	LINEAR,
	SQUARE,
	INFINITE_ROOTS
};


/*!
 Provides interface to communicate with user
 */
int main ()
{
	double a = 0, b = 0, c = 0;
	double d = 0, x1 = 0, x2 = 0;

	if (run_unittests () == false)
		return 1;

	printf ("Enter coefficients a, b, c: \n");
	scanf ("%lg %lg %lg", &a, &b, &c);

	printf ("Solving...\n");
	switch(get_type (a, b, c, &d))
	{
		case NO_ROOTS:
			printf ("No roots\n");
			break;
		case LINEAR:
			x1 = solve_linear (b, c);
			printf ("One root: %lg\n", x1);
			break;
		case SQUARE:
			solve_square (a, b, d, &x1, &x2);
			if (IsEqual (d, 0, DEFAULT_PRECISION))
				printf ("One root: %lg\n", x1);
			else
				printf ("Two roots: %lg, %lg\n", x1, x2);
			break;
		case INFINITE_ROOTS:
			printf ("Any number is a root\n");
			break;
		default:
			printf ("Solving failed!\n");
			break;
	}

	return 0;
}

/*!
  Returns the type of equation a*x^2 + b*x + c = 0 by number of roots and puts to "d" parameter discriminant if possible

 \param [in]	a	a-coefficient
 \param [in]	b	b-coefficient
 \param [in]	c	c-coefficient

 \param [out]	d	pointer where discriminant (b^2 - 4*a*c) should be put

 \return	NO_ROOTS - if the equation have no roots
		LINEAR - if the equation is linear (a == 0, b != 0)
		SQUARE - if the equation is square with at least one root (a != 0 and discriminant is not negative)
		INFINITE_ROOTS - if the equation have infinite number of roots (when a = b = c = 0)

 \note	All the coefficients must be finite
 */
int get_type (const double a,
	      const double b,
	      const double c,
	      double *d)
{
	IS_FINITE_3 (a, b, c);
	assert (d != NULL);

	if (IsEqual (a, 0, DEFAULT_PRECISION))
	{
		if (IsEqual (b, 0, DEFAULT_PRECISION))
		{
			if (IsEqual (c, 0, DEFAULT_PRECISION))
				return INFINITE_ROOTS;
			else
				return NO_ROOTS;
		}
		else
			return LINEAR;
	}
	else
	{
		*d = get_discriminant (a, b, c);
		if (*d < 0)
			return NO_ROOTS;
		else
			return SQUARE;
	}
}

/*!
 Solves square equation a*x^2 + b*x + c = 0 (a is not zero)

 \param [in]	a	a-coefficient
 \param [in]	b	b-coefficient
 \param [in]	d	discriminant value (b^2 - 4*a*c)

 \param [out]	x1	Pointer to the first root
 \param [out]	x2	Pointer to the second root

 \return	None

 \note	As the equation is square, a-coef is nonzero. Also, pointers to the roots must not be NULL or be equal each other
 */
void solve_square (const double a,
		   const double b,
		   const double d,
		   double *x1,
		   double *x2)
{
	assert (x1 != NULL);
	assert (x2 != NULL);
	assert (x1 != x2);

	IS_FINITE_3 (a, b, d);
	assert (!IsEqual(a, 0, DEFAULT_PRECISION));
	assert (d > 0);

	*x1 = (-b - sqrt (d)) / 2 / a;
	*x2 = (-b + sqrt (d)) / 2 / a;

	return ;
}

/*!
 Solves linear equation k*x + b = 0 (k is not zero)

 \param [in]	k	k-coefficient
 \param [in]	b	b-coefficient

 \return	Equation solution

 \note	As the equation is linear, k-coef is nonzero
 */
double solve_linear (const double k,
		     const double b)
{
	assert (k != 0);
	IS_FINITE_2 (k, b);

	return -b / k;
}

/*!
  Returns discriminant of square equation a*x^2 + b*x + c = 0

 \param [in]	a	a-coefficient
 \param [in]	b	b-coefficient
 \param [in]	c	c-coefficient

 \return	Discriminant value

 \note	All the coefficients must be finite
 */
double get_discriminant (const double a,
			 const double b,
			 const double c)
{
	IS_FINITE_3 (a, b, c);

	return b * b - 4 * a * c;
}

/**
 * Checks that two doubles are equal accurate to precision value
 *
 * \param [in]	d1		first double
 * \param [in]	d2		second double
 * \param [in]	precision	required precision
 *
 * \return	True if |d1 - d2| < precision * max (1.0, d1, d2); False otherwise
 */
bool IsEqual(double d1, double d2, double precision)
{
	if (fabs (d1 - d2) < precision * fmax (1.0, fmax (d1, d2)))
		return true;
	return false;
}
