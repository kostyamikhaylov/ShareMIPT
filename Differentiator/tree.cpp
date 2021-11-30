#include "diff.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define MAX_EXPR_LEN 1000
#define SUB_EXPR_LEN 20

static node_t *build_tree_from_array (const char *buf, size_t *ptr);

node_t *diff_new_node (const type_t type, const union data_t data,
			node_t *left, node_t *right)
{
	node_t *new_node = (node_t *) calloc (sizeof (node_t), 1);
	if (!new_node) {
		fprintf (stderr, "%s (): failed to allocate memory\n", __func__);
		return NULL;
	}

	new_node->type = type;
	switch (type) {
		case TYPE_CONST:
			new_node->data.value = data.value;
			break;
		case TYPE_VAR:
			new_node->data.var = data.var;
			break;
		case TYPE_OPER:
			new_node->data.oper = data.oper;
			break;
		case TYPE_ERR:
		default:
			fprintf (stderr, "%s (): incorrect node type\n", __func__);
			return NULL;
			break;
	}
	new_node->left = left;
	new_node->right = right;

	return new_node;
}

void diff_free_tree (node_t *node)
{
	if (!node) {
		fprintf (stderr, "%s (): null-pointer tree\n", __func__);
		return ;
	}
	if (node->left) {
		diff_free_tree (node->left);
		node->left = NULL;
	}
	if (node->right) {
		diff_free_tree (node->right);
		node->right = NULL;
	}
	node->data.value = 0;
	node->type = TYPE_ERR;
	free (node);
	return ;
}

node_t * diff_read_tree (void)
{
	size_t shift = 0;
	node_t *new_tree = NULL;
	char expr_buf[MAX_EXPR_LEN] = {};

	fgets (expr_buf, MAX_EXPR_LEN, stdin);
	char *c = strchr (expr_buf, '\n');
	if (c) *c = '\0';

	new_tree = build_tree_from_array (expr_buf, &shift);
	if (new_tree == NULL) {
		fprintf (stderr, "%s (): failed to build expression tree from your line: \"%s\"\n", __func__, expr_buf);
		return NULL;
	}

	return new_tree;
}

static node_t *build_tree_from_array (const char *buf, size_t *ptr)
{
	assert (buf);
	assert (ptr);

	char sub_expr_buf[SUB_EXPR_LEN] = {};
	size_t len = 0;
	node_t *new_node = NULL;

	while (isspace (buf[*ptr])) { (*ptr)++; }

	if (buf[*ptr] == '(') {
		(*ptr)++;
		new_node = (node_t *) calloc (sizeof (node_t), 1);
		if (!new_node) return NULL;
		new_node->left = build_tree_from_array (buf, ptr);
		while (isspace (buf[*ptr])) { (*ptr)++; }

		sscanf (buf + *ptr, "%[^)(]", sub_expr_buf);
		len = strlen (sub_expr_buf);
		*ptr += len;

		if (!strncmp (sub_expr_buf, "x", 1)) {
			new_node->type = TYPE_VAR;
			new_node->data.var = 'x';
		} else if (!strncmp (sub_expr_buf, "+", 1)) {
			new_node->type = TYPE_OPER;
			new_node->data.oper = OPER_PLUS;
			new_node->right = build_tree_from_array (buf, ptr);
		} else if (!strncmp (sub_expr_buf, "-", 1)) {
			new_node->type = TYPE_OPER;
			new_node->data.oper = OPER_MINUS;
			new_node->right = build_tree_from_array (buf, ptr);
		} else if (!strncmp (sub_expr_buf, "*", 1)) {
			new_node->type = TYPE_OPER;
			new_node->data.oper = OPER_MUL;
			new_node->right = build_tree_from_array (buf, ptr);
		} else if (!strncmp (sub_expr_buf, "/", 1)) {
			new_node->type = TYPE_OPER;
			new_node->data.oper = OPER_DIV;
			new_node->right = build_tree_from_array (buf, ptr);
		} else if (!strncmp (sub_expr_buf, "^", 1)) {
			new_node->type = TYPE_OPER;
			new_node->data.oper = OPER_POW;
			new_node->right = build_tree_from_array (buf, ptr);
		} else if (!strncmp (sub_expr_buf, "ln", 2)) {
			new_node->type = TYPE_OPER;
			new_node->data.oper = OPER_LN;
			new_node->left = NULL;
			new_node->right = build_tree_from_array (buf, ptr);
		} else if (!strncmp (sub_expr_buf, "cos", 3)) {
			new_node->type = TYPE_OPER;
			new_node->data.oper = OPER_COS;
			new_node->left = NULL;
			new_node->right = build_tree_from_array (buf, ptr);
		} else if (!strncmp (sub_expr_buf, "sin", 3)) {
			new_node->type = TYPE_OPER;
			new_node->data.oper = OPER_SIN;
			new_node->left = NULL;
			new_node->right = build_tree_from_array (buf, ptr);
		} else {
			new_node->type = TYPE_CONST;
			new_node->data.value = strtod (sub_expr_buf, NULL);
		}

		while (isspace (buf[*ptr])) { (*ptr)++; }

		if (buf[*ptr] == ')') {
			(*ptr)++;
			return new_node;
		} else {
			fprintf (stderr, "%s (): unknown symbol "
					"'%s'\n", __func__, buf + *ptr);
			return NULL;
		}
	} else {
		return NULL;
	}
}
