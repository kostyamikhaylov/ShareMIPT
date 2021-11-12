#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct List
{
	int capacity = -1;
	int *data = NULL;
	int *next = NULL;
	int *prev = NULL;
	int head = -1;
	int tail = -1;
	int free = -1;
} list;

void list_ctor (list *lst, const int capacity);
void list_dtor (list *lst);
int list_insert_back (list *lst, const int value);
int list_insert_after (list *lst, const int place, const int value);
int list_delete_head (list *lst);
int list_delete_tail (list *lst);
int list_delete_element (list *lst, const int place);

void list_dump (list *lst);
void list_draw (list *lst);

#endif // LIST_H
