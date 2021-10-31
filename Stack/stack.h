#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdint.h>

#define MIN_CAP 512
#define MAX_CAP 65536

#define CANARY_VALUE 0xDEDAADED

typedef uint64_t canary_t;
typedef uint8_t hash_t;

typedef struct element_description
{
	int elem_size = 0;
	const char *name = NULL;
	int (*print_elem) (const void *ptr) = NULL;
} Elem;

struct Hash
{
	hash_t stack_hash;
	hash_t data_hash;
};

typedef struct my_stack
{
	canary_t canary1;
	int capacity = 0;
	int size = 0;
	void *data = NULL;
	Elem elem_descr;
	struct Hash hash;
	canary_t canary2;
} Stack;

enum error_type
{
	OK,
	NO_MEMORY,
	RESIZE_ERROR,
	POP_FROM_EMPTY,
	STACK_CORRUPTED,
	UNKNOWN_ERROR
};

enum error_type stack_ctor (Stack *stack, const int elem_size, const char *name, int (*print_elem) (const void *ptr));
enum error_type stack_push (Stack *stack, const void *value);
enum error_type stack_pop (Stack *stack, void *value);
enum error_type stack_dtor (Stack *stack);

int check_stack (const Stack *stack, const char *reason);
void dump_stack (const Stack *stack, const char *reason, const char *detected_corruption);

int get_hash (Stack *stack);
hash_t count_hash (const char *ptr, size_t len);

#endif // STACK_H
