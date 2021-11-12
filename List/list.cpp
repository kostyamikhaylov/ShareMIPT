#include "list.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 100
#define FILENAME "graph"

void list_ctor (list *lst, const int capacity)
{
	assert (lst);
	assert (capacity > 0);

	lst->data = (int *) calloc (sizeof (int), (size_t) capacity);
	lst->next = (int *) calloc (sizeof (int), (size_t) capacity);
	lst->prev = (int *) calloc (sizeof (int), (size_t) capacity);
	if (!lst->data || !lst->next || !lst->prev)
	{
		fprintf (stderr, "list_ctor (): failed to allocate memory\n");
		return ;
	}

	lst->next[0] = 0;
	lst->prev[0] = 0;
	for (int i = 1; i < capacity; i++) {
		lst->next[i] = (i + 1) % capacity;
		lst->prev[i] = -1;
	}
	lst->capacity = capacity;
	lst->head = 0;
	lst->tail = 0;
	lst->free = 1;
	return ;
}

void list_dtor (list *lst)
{
	assert (lst);

	free (lst->data);
	free (lst->next);
	free (lst->prev);
	lst->capacity = -1;
	lst->head = -1;
	lst->tail = -1;
	lst->free = -1;
}

int list_insert_back (list *lst, const int value)
{
	assert (lst);

	int place = lst->free;
	if (place == 0) {
		fprintf (stderr, "list_insert_back (): no free space\n");
		return -1;
	}

	if (lst->prev[place] != -1) {
		fprintf (stderr, "list_insert_back (): free elements list is corrupted\n");
		return -1;
	}
	lst->data[place] = value;
	lst->free = lst->next[place];
	lst->next[place] = 0;
	lst->prev[place] = lst->tail;
	if (lst->tail != 0)
		lst->next[lst->tail] = place;
	lst->tail = place;
	if (lst->head == 0)
		lst->head = place;
	return place;
}

int list_insert_after (list *lst, const int place, const int value)
{
	assert (lst);
	assert (place > 0);
	assert (place < lst->capacity);

	int free = lst->free;
	if (free == 0) {
		fprintf (stderr, "list_insert_after (): no free space\n");
		return -1;
	}
	if (lst->prev[place] == -1) {
		fprintf (stderr, "list_insert_after (): element on place %d doesn't exist\n", place);
		return -1;
	}

	if (lst->tail == place) {
		return list_insert_back (lst, value);
	}

	if (lst->prev[free] != -1) {
		fprintf (stderr, "list_insert_after (): free elements list is corrupted\n");
		return -1;
	}

	lst->free = lst->next[free];
	lst->data[free] = value;
	lst->prev[free] = place;
	if (lst->next[place] != 0)
		lst->prev[lst->next[place]] = free;
	lst->next[free] = lst->next[place];
	lst->next[place] = free;
	return free;
}

int list_delete_head (list *lst)
{
	assert (lst);

	int head = lst->head;
	int new_head = lst->next[head];

	if (head == 0) {
		fprintf (stderr, "list_delete_head (): can't delete list head, list is empty\n");
		return -1;
	}

	lst->head = new_head;
	if (new_head == 0)
		lst->tail = 0;
	else
		lst->prev[new_head] = 0;
	lst->prev[head] = -1;
	lst->data[head] = 0;
	lst->next[head] = lst->free;
	lst->free = head;
	return 0;
}

int list_delete_tail (list *lst)
{
	assert (lst);

	int tail = lst->tail;
	int new_tail = lst->prev[tail];

	if (tail == 0) {
		fprintf (stderr, "list_delete_tail (): can't delete list tail, list is empty\n");
		return -1;
	}

	lst->tail = new_tail;
	if (new_tail == 0)
		lst->head = 0;
	else
		lst->next[new_tail] = 0;
	lst->next[tail] = -1;
	lst->prev[tail] = -1;
	lst->data[tail] = 0;
	lst->next[tail] = lst->free;
	lst->free = tail;
	return new_tail;
}

int list_delete_element (list *lst, const int place)
{
	assert (lst);
	assert (place > 0);
	assert (place < lst->capacity);

	int next = lst->next[place];
	int prev = lst->prev[place];
	if ((lst->next[place] == -1) || (lst->prev[place] == -1)) {
		fprintf (stderr, "list_delete_element (): wrong place %d\n", place);
		return -1;
	}
	if (place == lst->head)
		return list_delete_head (lst);
	if (place == lst->tail)
		return list_delete_tail (lst);

	lst->data[place] = 0;
	lst->next[place] = -1;
	lst->prev[place] = -1;
	lst->prev[next] = prev;
	lst->next[prev] = next;

	lst->next[place] = lst->free;
	lst->free = place;

	return prev;
}

void list_dump (list *lst)
{
	assert (lst);

	printf ("LIST DUMP:\n"
			"\tdata [%p]\n"
			"\tnext [%p]\n"
			"\tprev [%p]\n"
			"\tcapacity: %d\n"
			"\thead: %d\n"
			"\ttail: %d\n"
			"\tfree: %d\n",
			lst->data, lst->next, lst->prev,
			lst->capacity, lst->head,
			lst->tail, lst->free);

	printf ("List elements (format [prev:value:next]):\n{\n");
	for (int i = 0; i < lst->capacity; i++) {
		if (i == 0)
			continue;
		if ((lst->prev[i] != -1) && (lst->next[i] != -1))
			printf ("\033[0;32m"); // red
		if (i == lst->head)
			printf ("\033[0;36m"); // cyan
		if (i == lst->tail)
			printf ("\033[0;35m"); // magenta
		printf ("[%d:%d:%d] ", lst->prev[i], lst->data[i], lst->next[i]);
		if ((lst->prev[i] != -1) && (lst->next[i] != -1))
			printf ("\033[0m"); // reset color
		if (i == lst->head)
			printf ("\033[0m"); // reset color
		if (i == lst->tail)
			printf ("\033[0m"); // reset color
	}
	printf ("\n}\n");
}

void list_draw (list *lst)
{
	static int print_counter = 1;
	char buf[BUF_LEN] = {};
	assert (lst);

	snprintf (buf, BUF_LEN - 1, FILENAME "_%d.dot", print_counter);
	FILE *file = fopen (buf, "w");
	if (!file) {
		printf ("list_draw () error: can't open file " FILENAME ".dot\n");
		return ;
	}

	fprintf (file, "digraph G {\n");
	fprintf (file, "rankdir=TB;\n");

	for (int i = 0; i < lst->capacity; i++) {
		fprintf (file, "node%d [shape=record,label=\" <prev%d> %d | <data%d> %d | <next%d> %d\"]\n", i, i, lst->prev[i], i, lst->data[i], i, lst->next[i]);
	}
	for (int i = 1; i < lst->capacity; i++) {
		fprintf (file, "node%d -> node%d [color=\"white\"];\n", i - 1, i);
	}

	fprintf (file, "node_head [shape=record,label=\"head\"]\n");
	fprintf (file, "node_head -> node%d;\n", lst->head);
	fprintf (file, "node_tail [shape=record,label=\"tail\"]\n");
	fprintf (file, "node_tail -> node%d;\n", lst->tail);
	fprintf (file, "node_free [shape=record,label=\"free\"]\n");
	fprintf (file, "node_free -> node%d;\n", lst->free);

	fprintf (file, "{rank=same; \"node0\"; \"node_head\"; \"node_tail\"; \"node_free\"; }\n");

	int head = lst->head;
	int tail = lst->tail;
	int free = lst->free;
	while (head != tail) {
		fprintf (file, "node%d:<next%d> -> node%d:<next%d> [color=\"blue\"];\n", head, head, lst->next[head], lst->next[head]);
		head = lst->next[head];
	}
	head = lst->head;
	while (tail != head) {
		fprintf (file, "node%d:<prev%d> -> node%d:<prev%d> [color=\"cyan\"];\n", tail, tail, lst->prev[tail], lst->prev[tail]);
		tail = lst->prev[tail];
	}
	while (free != 0) {
		fprintf (file, "node%d:<next%d> -> node%d:<data%d> [color=\"green\"];\n", free, free, lst->next[free], lst->next[free]);
		free = lst->next[free];
	}

	fprintf (file, "}\n");

	fclose (file);

	memset (buf, '\0', BUF_LEN);
	snprintf (buf, BUF_LEN, "dot " FILENAME "_%d.dot -Tpng -o " FILENAME "_%d.png", print_counter, print_counter);
	system (buf);

	memset (buf, '\0', BUF_LEN);
	snprintf (buf, BUF_LEN, "rm -f " FILENAME "_%d.dot", print_counter);
	system (buf);
	print_counter++;
}
