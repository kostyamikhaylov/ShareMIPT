#include "diff.h"


int main ()
{
	data_t data = {};
	data.value = 11;
	node_t *node_11 = diff_new_node (TYPE_CONST, { .value = 11 }, NULL, NULL);
	data.value = 34;
	node_t *node_34 = diff_new_node (TYPE_CONST, data, NULL, NULL);
	data.oper = OPER_MUL;
	node_t *node_mul = diff_new_node (TYPE_OPER, data, node_11, node_34);
	data.value = 1;
	node_t *node_1 = diff_new_node (TYPE_CONST, data, NULL, NULL);
	data.var = 'x';
	node_t *node_x = diff_new_node (TYPE_VAR, data, NULL, NULL);
	data.oper = OPER_PLUS;
	node_t *node_plus = diff_new_node (TYPE_OPER, data, node_x, node_1);
	data.oper = OPER_MINUS;
	node_t *node_minus = diff_new_node (TYPE_OPER, data, node_plus, node_mul);

	diff_dump_dot (node_minus);
	diff_dump_latex (node_minus);

	node_t *node_read = diff_read_tree ();

	diff_dump_dot (node_read);
	diff_dump_latex (node_read);

	node_t *node_diff1 = diff_differentiate_tree (node_minus);
	node_t *node_diff2 = diff_differentiate_tree (node_read);

	diff_dump_dot (node_diff1);
	diff_dump_latex (node_diff1);

	diff_simplify_tree (node_diff1);

	diff_dump_dot (node_diff1);
	diff_dump_latex (node_diff1);

	diff_free_tree (node_diff1);

	diff_dump_dot (node_diff2);
	diff_dump_latex (node_diff2);

	diff_simplify_tree (node_diff2);

	diff_dump_dot (node_diff2);
	diff_dump_latex (node_diff2);

	diff_free_tree (node_diff2);

	diff_free_tree (node_minus);
	diff_free_tree (node_read);

	node_read = diff_read_tree ();

	diff_dump_dot (node_read);
	diff_dump_latex (node_read);

	node_t *node_diff3 = diff_differentiate_tree (node_read);

	diff_dump_dot (node_diff3);
	diff_dump_latex (node_diff3);

	diff_simplify_tree (node_diff3);

	diff_dump_dot (node_diff3);
	diff_dump_latex (node_diff3);

	diff_free_tree (node_diff3);
	diff_free_tree (node_read);

	return 0;
}
