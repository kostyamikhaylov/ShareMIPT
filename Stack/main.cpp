#include "stack.h"

#include <assert.h>

int print_double (const void *ptr);
int print_struct_example (const void *ptr);

struct example
{
	int i = 1;
	double d = 0;
};

int main ()
{
	//double d = 0.1, x = 0.0;
	struct example e = {3, 0.2};
	Stack st = {};
	//check_stack (&st, "Just to test dump_stack ()");
	enum error_type error = OK;
	error = stack_ctor (&st, sizeof (struct example), "struct example", print_struct_example);
	if (error != OK) {
		printf ("Failed to initialize stack; exiting...\n");
		return 1;
	}

	error = stack_push (&st, &e);
	if (error != OK) {
		printf ("Failed to push first value to stack; exiting...\n");
		return 1;
	}
	error = stack_push (&st, &e);

	/*
	for (int i = 0; i < 65531; i++) {
		e.d += 0.5;
		e.i += 1;
		error = stack_push (&st, &e);
		if (error != OK) {
			printf ("Failed to push %dth value to stack; exiting...\n", i + 1);
			return 1;
		}
		printf ("Pushed value {%d;%lg}\n", e.i, e.d);
	}
	*/

	dump_stack (&st, "Just to test dump_stack ()", "");
	/*
	for (int i = 0; i < 65531; i++) {
		error = stack_pop (&st, &e);
		if (error != OK) {
			printf ("Failed to pop %dth value from stack; exiting...\n", i + 1);
			return 1;
		}
		printf ("Stack returned value {%d;%lg}\n", e.i, e.d);
	}
	*/

	error = stack_pop (&st, &e);
	if (error != OK) {
		printf ("Failed to pop the first value from stack; exiting...\n");
		return 1;
	}
	printf ("Stack returned value {%d;%lg}\n", e.i, e.d);

	error = stack_dtor (&st);
	if (error != OK) {
		printf ("Failed to destroy stack; exiting...\n");
		return 1;
	}

	return 0;
}

int print_double (const void *ptr)
{
	assert (ptr);

	printf ("%lg\n", *((const double *) ptr));
	return 0;
}

int print_struct_example (const void *ptr)
{
	assert (ptr);

	const struct example *ex = (const struct example *) ptr;
	printf ("{%d; %lg}\n", ex->i, ex->d);
	return 0;
}
