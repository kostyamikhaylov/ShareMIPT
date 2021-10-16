#include "stack.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


#define VERIFY_AT(where)				\
do							\
{							\
	if (check_stack (stack, "Stack verification at " where)) {	\
		return STACK_CORRUPTED;			\
	}						\
} while (0);

static int alloc_more (Stack *stack);
static int free_more (Stack *stack);

enum error_type stack_ctor (Stack *stack, const int elem_size, const char *name, int (*print_elem) (const void *ptr))
{
	assert (stack);
	assert (elem_size > 0);
	assert (name);
	assert (print_elem);

	stack->data = calloc (1, 2 * sizeof (canary_t) + MIN_CAP * (size_t) elem_size);
	if (!stack->data)
	{
		fprintf (stderr, "stack_ctor (): error while allocating memory\n");
		return NO_MEMORY;
	}
	* (canary_t *) stack->data = CANARY_VALUE;
	* (canary_t *) ((char *) stack->data + MIN_CAP * (size_t) elem_size + sizeof (canary_t)) = CANARY_VALUE;
	stack->data = (void *) ((char *) stack->data + sizeof (canary_t));
	stack->canary1 = CANARY_VALUE;
	stack->canary2 = CANARY_VALUE;

	stack->size = 0;
	stack->capacity = MIN_CAP;

	stack->elem_descr.elem_size = elem_size;
	stack->elem_descr.name = name;
	stack->elem_descr.print_elem = print_elem;

	get_hash (stack);

	VERIFY_AT("stack_ctor () exit");

	return OK;
}

enum error_type stack_push (Stack *stack, const void *value)
{
	assert (value);
	VERIFY_AT("stack_push () start");

	int elem_size = stack->elem_descr.elem_size;

	if (stack->size + 1 > stack->capacity) {
		if (alloc_more (stack)) {
			fprintf (stderr, "stack_push (): failed to allocate more memory for pushing a new element\n");
			return NO_MEMORY;
		}
	}
	memcpy ((char *)stack->data + stack->size * elem_size, value, (size_t) elem_size);
	stack->size++;

	get_hash (stack);

	VERIFY_AT("stack_push () exit");

	return OK;
}

static int alloc_more (Stack *stack)
{
	assert (stack);
	int new_cap = 0;
	void *new_data = NULL;
	int elem_size = stack->elem_descr.elem_size;

	if (2 * stack->capacity > MAX_CAP)
		return 1;
	new_cap = stack->capacity * 2;
	new_data = calloc (1, 2 * sizeof (canary_t) + 2 * (size_t) (new_cap * elem_size));
	if (!new_data)
		return 1;

	* (canary_t *) new_data = CANARY_VALUE;
	* (canary_t *) ((char *) new_data + (size_t) (new_cap * elem_size) + sizeof (canary_t)) = CANARY_VALUE;
	new_data = (void *) ((char *) new_data + sizeof (canary_t));

	memcpy (new_data, stack->data, (size_t) (stack->size * elem_size));
	stack->data = (void *) ((char *) stack->data - sizeof (canary_t));
	free (stack->data);
	stack->data = new_data;
	stack->capacity = new_cap;
//	printf ("cap increased: %d->%d\n", cur_cap, stack->capacity);
	return 0;
}

enum error_type stack_pop (Stack *stack, void *value)
{
	assert (value);
	VERIFY_AT("stack_pop () start");

	int elem_size = stack->elem_descr.elem_size;

	if (stack->size <= 0) {
		fprintf (stderr, "stack_pop (): attempt to pop from empty stack\n");
		memset (value, '\0', (size_t) elem_size);
		return POP_FROM_EMPTY;
	}
	--stack->size;
	memcpy (value, (char *) stack->data + stack->size * elem_size, (size_t) elem_size);
	if (stack->size < stack->capacity / 3 && stack->capacity > MIN_CAP)
		if (free_more (stack)) {
			fprintf (stderr, "stack_pop (): failed to free extra memory");
			return RESIZE_ERROR;
		}

	get_hash (stack);

	VERIFY_AT("stack_pop () exit");

	return OK;
}

static int free_more (Stack *stack)
{
	assert (stack);
	assert (stack->capacity % 2 != 0);
	assert (stack->size < stack->capacity / 2);

	int new_cap = 0;
	void *new_data = NULL;
	int elem_size = stack->elem_descr.elem_size;

	new_cap = stack->capacity / 2;
	new_data = calloc (1, 2 * sizeof (canary_t) + 2 * (size_t) (new_cap * elem_size));
	if (!new_data)
		return 1;

	* (canary_t *) new_data = CANARY_VALUE;
	* (canary_t *) ((char *) new_data + (size_t) (new_cap * elem_size) + sizeof (canary_t)) = CANARY_VALUE;
	new_data = (void *) ((char *) new_data + sizeof (canary_t));

	memcpy (new_data, stack->data, (size_t) (stack->size * elem_size));
	stack->data = (void *) ((char *) stack->data - sizeof (canary_t));
	free (stack->data);
	stack->data = new_data;
	stack->capacity = new_cap;
//	printf ("cap decreased: %d->%d\n", 2 * new_cap, new_cap);
	return 0;
}

enum error_type stack_dtor (Stack *stack)
{
	VERIFY_AT("stack_dtor () start");

	stack->size = -1;
	stack->capacity = -1;

	stack->elem_descr.elem_size = -1;
	stack->elem_descr.name = NULL;
	stack->elem_descr.print_elem = NULL;

	stack->data = (void *) ((char *) stack->data - sizeof (canary_t));
	free (stack->data);
	return OK;
}

int get_hash (Stack *stack)
{
	assert (stack);
	assert (stack->data);
	stack->hash.data_hash = count_hash ((const char *) stack->data, (size_t) (stack->size * stack->elem_descr.elem_size));
	stack->hash.stack_hash = count_hash ((const char *) &stack->canary1, (char *) &stack->hash - (char *) &stack->canary1);
	return 0;
}

hash_t count_hash (const char *ptr, size_t len)
{
	hash_t tmp = 0xad;
	for (size_t i = 0; i < len; i++)
	{
		tmp ^= *((const hash_t *) ptr + i);
	}
	return tmp;
}
