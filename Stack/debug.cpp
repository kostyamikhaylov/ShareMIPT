#include "stack.h"

#include <string.h>

int check_stack (const Stack *stack, const char *reason)
{
	int err = 0;
	if (!stack) {
		printf ("check_stack () error: NULL-pointer to stack\n");
		err = 1;
		return err;
	}

	if (!stack->data) {
		dump_stack (stack, reason, "Warning: data field is NULL");
		err = 1;
	}
	if (stack->size < 0 || stack->size > stack->capacity) {
		if (stack->size < 0)
			dump_stack (stack, reason, "Error: size field is less than zero");
		else
			dump_stack (stack, reason, "Error: size field is greater than capacity");
		err = 1;
	}
	if (stack->capacity < MIN_CAP || stack->capacity > MAX_CAP) {
		if (stack->capacity < MIN_CAP)
			dump_stack (stack, reason, "Error: capacity is less than MIN_CAP");
		else
			dump_stack (stack, reason, "Error: capacity is greater than MAX_CAP");
		err = 1;
	}
	if (stack->elem_descr.elem_size <= 0) {
		dump_stack (stack, reason, "Error: element size is not positive");
		err = 1;
	}
	if (!stack->elem_descr.name) {
		dump_stack (stack, reason, "Error: no name provided for elements");
		err = 1;
	}
	if (!stack->elem_descr.print_elem) {
		dump_stack (stack, reason, "Error: no printing function provided");
		err = 1;
	}
	if (stack->canary1 != CANARY_VALUE ||
	    stack->canary2 != CANARY_VALUE) {
		dump_stack (stack, reason, "Error: struct Stack canary died");
		err = 1;
	}

	canary_t data_canary1 =
		* (canary_t *) ((char *) stack->data - sizeof (canary_t));
	canary_t data_canary2 =
		* (canary_t *) ((char *) stack->data +
		(size_t) (stack->capacity * stack->elem_descr.elem_size));

	if (data_canary1 != CANARY_VALUE ||
	    data_canary2 != CANARY_VALUE) {
		dump_stack (stack, reason, "Error: stack data canary died");
		err = 1;
	}

	hash_t dt_hash = count_hash ((const char *) stack->data, (size_t) (stack->size * stack->elem_descr.elem_size));
	if (stack->hash.data_hash != dt_hash) {
		dump_stack (stack, reason, "Error: stack hash is not correct");
		err = 1;
	}

	hash_t stk_hash = count_hash ((const char *) &stack->canary1, (const char *) &stack->hash - (const char *) &stack->canary1);
	if (stack->hash.stack_hash != stk_hash) {
		dump_stack (stack, reason, "Error: data hash is not correct");
		err = 1;
	}

	return err;
}

void dump_stack (const Stack *stack, const char *reason, const char *detected_corruption)
{
	if (!stack) {
		printf ("dump_stack () error: NULL-pointer to stack\n");
		return ;
	}
	int elem_size = stack->elem_descr.elem_size;
	canary_t *data_canary1 = (canary_t *) ((char *) stack->data - sizeof (canary_t));
	canary_t *data_canary2 = (canary_t *) ((char *) stack->data + (size_t) (stack->capacity * elem_size));
	printf ("----------------------------\n");
	printf ("dump_stack () called because: %s\n", reason);
	printf ("Stack<%s>[%p] at %s () at %s(%d)\n", stack->elem_descr.name, stack, __FUNCTION__, __FILE__, __LINE__);
	if (strcmp (detected_corruption, ""))
		printf ("\033[0;31mNOT OK\033[0m: %s\n", detected_corruption);
	else
		printf ("\033[0;32mOK\033[0m\n");

	printf ("\t|CANARY#1|		= %lx\n", stack->canary1);
	printf ("\tsize			= %d items\n", stack->size);
	printf ("\tcapacity		= %d items\n", stack->capacity);
	printf ("\tdata			[%p]\n", stack->data);
	printf ("\telement info:\n");
	printf ("\t    size		= %d bytes\n", stack->elem_descr.elem_size);
	printf ("\t    name		= \"%s\"\n", stack->elem_descr.name);
	printf ("\t    print function	[%p]\n", stack->elem_descr.print_elem);
	printf ("\thash info:\n");
	printf ("\t    stack_hash		= %02x\n", stack->hash.stack_hash);
	printf ("\t    data_hash		= %02x\n", stack->hash.data_hash);
	printf ("\t|CANARY#2|		= %lx\n", stack->canary2);



	printf ("|CANARY#1|		= %lx\n", *data_canary1);
	if (stack->size < 10) {
		printf ("{\n");
		for (int i = 0; i < stack->size; i++) {
			printf ("\t[%d] = ", i);
			stack->elem_descr.print_elem ((char *) stack->data + i * elem_size);
			printf ("\n");
		}
		printf ("}\n");
	} else {
		printf ("{\n");
		for (int i = 0; i < 5; i++) {
			printf ("\t[%d] = ", i);
			stack->elem_descr.print_elem ((char *) stack->data + i * elem_size);
			printf ("\n");
		}
		printf ("\n\t...\n\n");
		for (int i = stack->size - 5; i < stack->size; i++) {
			printf ("\t[%d] = ", i);
			stack->elem_descr.print_elem ((char *) stack->data + i * elem_size);
			printf ("\n");
		}
		printf ("}\n");
	}
	printf ("|CANARY#2|		= %lx\n", *data_canary2);

	printf ("----------------------------\n");
	return ;
}

