#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdint.h>

typedef enum Type {
	TYPE_CONST,
	TYPE_VAR,
	TYPE_OPER,
	TYPE_ERR
} type_t;

typedef enum Oper {
	OPER_PLUS = '+',
	OPER_MINUS = '-',
	OPER_MUL = '*',
	OPER_DIV = '/',
	OPER_POW = '^',
	OPER_LN = 'n',
	OPER_COS = 'c',
	OPER_SIN = 's',
	OPER_ERR = '?',
} oper_t;

union data_t
{
	double value = 0.0;
	char oper;
	char var;
};

typedef struct Node {
	type_t type = TYPE_ERR;
	union data_t data = {};
	struct Node *left = NULL;
	struct Node *right = NULL;
} node_t;

node_t *diff_new_node (const type_t type, const union data_t data,
			node_t *left, node_t *right);
void diff_free_tree (node_t *node);
node_t * diff_read_tree (void);

void diff_dump_dot (const node_t *node);
void diff_dump_latex (const node_t *node);

node_t *diff_differentiate_tree (const node_t *node);
bool diff_simplify_tree (node_t *node);

#endif //TREE_H
