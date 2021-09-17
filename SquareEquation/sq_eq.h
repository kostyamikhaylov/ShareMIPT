#ifndef SQ_EQ_H
#define SQ_EQ_H

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define DEFAULT_PRECISION 0.001


int get_type (const double a,
	      const double b,
	      const double c,
	      double *d);

double get_discriminant (const double a,
			 const double b,
			 const double c);

double solve_linear (const double k,
		     const double b);

void solve_square (const double a,
		   const double b,
		   const double d,
		   double *x1,
		   double *x2);

bool IsEqual(double d1, double d2, double precision);

#ifdef UNIT_TESTING
bool run_unittests (void);
#else // !UNIT_TESTING
bool run_unittests (void);
bool run_unittests (void) { return true; }
#endif // INUT_TESTING

#endif //SQ_EQ_H
