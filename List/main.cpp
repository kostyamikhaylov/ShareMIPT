#include "list.h"

#include <stdio.h>


int main ()
{
	list lst = {};
	list_ctor (&lst, 12);
	/*int place10 = */list_insert_back (&lst, 10);
	int place20 = list_insert_back (&lst, 20);
	/*int place30 = */list_insert_back (&lst, 30);
	int place40 = list_insert_back (&lst, 40);
	/*int new_elem_place25 = */list_insert_after (&lst, place20, 25);
	list_dump (&lst);
	int new_elem_place45 = list_insert_after (&lst, place40, 45);
	list_delete_head (&lst);
	/*int new_elem_place23 = */list_insert_after (&lst, place20, 23);
	list_dump (&lst);
	list_draw (&lst);
	list_delete_element (&lst, place40);
	list_delete_element (&lst, place20);
	list_delete_element (&lst, new_elem_place45);
	list_dump (&lst);
	list_insert_back (&lst, 1);
	list_insert_back (&lst, 2);
	list_insert_back (&lst, 3);
	list_insert_back (&lst, 4);
	list_insert_back (&lst, 5);
	list_dump (&lst);
	list_draw (&lst);
	list_dtor (&lst);

	return 0;
}

