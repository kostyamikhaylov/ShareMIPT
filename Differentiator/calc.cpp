#include "diff.h"

#include <assert.h>
#include <math.h>

#define DEFAULT_PRECISION 0.001

#define NODE(type, val, ptr1, ptr2) diff_new_node (type, val, ptr1, ptr2)
#define TERM(type, val) NODE (type, val, NULL, NULL)
#define TERM_CONST(val) TERM (TYPE_CONST, { .value = val })
#define TERM_VAR(val) TERM (TYPE_VAR, { .var = val })

#define IS_CONST(node) is_const_subtree (node)
#define D(node) diff_differentiate_tree (node)
#define LN node->left
#define RN node->right
#define CL copy_tree (LN)
#define CR copy_tree (RN)
#define CN copy_tree (node)
#define DL D(LN)
#define DR D(RN)
#define OPER_NODE(oper_val, node1, node2) diff_new_node (TYPE_OPER, { .oper = oper_val }, node1, node2)
#define ADD(node1, node2) OPER_NODE (OPER_PLUS, node1, node2)
#define SUB(node1, node2) OPER_NODE (OPER_MINUS, node1, node2)
#define MUL(node1, node2) OPER_NODE (OPER_MUL, node1, node2)
#define DIV(node1, node2) OPER_NODE (OPER_DIV, node1, node2)
#define POW(node1, node2) OPER_NODE (OPER_POW, node1, node2)
#define LOGN(node) OPER_NODE (OPER_LN, NULL, node)
#define COS(node) OPER_NODE (OPER_COS, NULL, node)
#define SIN(node) OPER_NODE (OPER_SIN, NULL, node)

#define IS_NODE_EQUAL(node, val) (node && node->type == TYPE_CONST && is_equal (node->data.value, val, DEFAULT_PRECISION))

#define MAKE_CONST(node, num)			\
do {						\
	node->type = TYPE_CONST;		\
	node->data.value = num;			\
	node->left = NULL;			\
	node->right = NULL;			\
} while (0)

#define MAKE_CONST_ZERO(node) MAKE_CONST (node, 0)
#define MAKE_CONST_ONE(node) MAKE_CONST (node, 1)

static node_t *copy_tree (const node_t *node);
static bool calculate_constants (node_t *node);
static double calculate_operation (const char oper,
				   const double arg1,
				   const double arg2);
static bool simplify_operations (node_t *node);
static bool is_const_subtree (const node_t *node);
static void copy_node (node_t *dest, const node_t *src);
static void free_node (node_t *node);

static bool simplify_plus (node_t *node);
static bool simplify_minus (node_t *node);
static bool simplify_mul (node_t *node);
static bool simplify_div (node_t *node);
static bool simplify_pow (node_t *node);
static bool is_equal(double d1, double d2, double precision);

node_t *diff_differentiate_tree (const node_t *node)
{
	if (!node) {
		fprintf (stderr, "%s (): null-pointer tree\n", __func__);
		return NULL;
	}

	if (IS_CONST (node))
		return TERM_CONST (0);

	switch (node->type) {
		case TYPE_CONST:
			return TERM_CONST (0);
			break;
		case TYPE_VAR:
			if (node->data.var == 'x') {
				return TERM_CONST (1);
			} else {
				fprintf (stderr, "%s (): unknown variable %c\n", __func__, node->data.var);
				return NULL;
			}
			break;
		case TYPE_OPER:
			switch (node->data.oper) {
				case OPER_PLUS:
					return ADD (DL, DR);
					break;
				case OPER_MINUS:
					return SUB (DL, DR);
					break;
				case OPER_MUL:
					return ADD (MUL (DL, CR), MUL (CL, DR));
					break;
				case OPER_DIV:
					return DIV (SUB (MUL (DL, CR), MUL (CL, DR)), POW (CR, TERM_CONST (2)));
					break;
				case OPER_POW:
					if (IS_CONST (LN) && IS_CONST (RN))
						return TERM_CONST (0);
					else if (IS_CONST (LN))
						return MUL (MUL (CN, LOGN (CL)), DR);
					else if (IS_CONST (RN))
						return MUL (MUL (CR, POW (CL, SUB (CR, TERM_CONST (1)))), DL);
					else {
						fprintf (stderr, "%s (): can't deal with functions of the form f(x)^g(x)\n", __func__);
						return NULL;
					}
					break;
				case OPER_LN:
					return MUL (DIV (TERM_CONST (1), CR), DR);
					break;
				case OPER_COS:
					return MUL (MUL (TERM_CONST (-1), SIN (CR)), DR);
					break;
				case OPER_SIN:
					return MUL (COS (CR), DR);
					break;
				case OPER_ERR:
				default:
					fprintf (stderr, "%s (): error operator %c\n", __func__, node->data.oper);
					return NULL;
					break;
			}
			break;
		case TYPE_ERR:
		default:
			fprintf (stderr, "%s (): unknown type %d\n", __func__, node->type);
			return NULL;
			break;
	}
	return NULL;
}

bool diff_simplify_tree (node_t *node)
{
	bool simplified = false;
	while (calculate_constants (node) || simplify_operations (node)) {
		simplified = true;
	}
	return simplified;
}

static bool calculate_constants (node_t *node)
{
	if (!node) {
		fprintf (stderr, "%s (): null-pointer tree\n", __func__);
		return false;
	}

	switch (node->type) {
		case TYPE_CONST:
			return false;
			break;
		case TYPE_VAR:
			return false;
			break;
		case TYPE_OPER:
			if (node->left && node->right) {
				bool changed = false;
				changed = calculate_constants (node->left) || changed;
				changed = calculate_constants (node->right) || changed;
				if (node->left->type == TYPE_CONST &&
				    node->right->type == TYPE_CONST) {
					node->data.value = calculate_operation (node->data.oper, node->left->data.value, node->right->data.value);
					node->type = TYPE_CONST;
					free_node (node->left);
					node->left = NULL;
					free_node (node->right);
					node->right = NULL;
					return true;
				}
				return changed;
			} else if (node->right) {
				bool changed = calculate_constants (node->right);
				if (node->right->type == TYPE_CONST) {
					node->data.value = calculate_operation (node->data.oper, 0, node->right->data.value);
					node->type = TYPE_CONST;
					free_node (node->right);
					node->right = NULL;
					return true;
				}
				return changed;
			}
			return false;
			break;
		case TYPE_ERR:
		default:
			fprintf (stderr, "%s (): error node\n", __func__);
			return false;
			break;
	}
}

static double calculate_operation (const char oper,
				   const double arg1,
				   const double arg2)
{
	switch (oper) {
		case OPER_PLUS:
			return arg1 + arg2;
			break;
		case OPER_MINUS:
			return arg1 - arg2;
			break;
		case OPER_MUL:
			return arg1 * arg2;
			break;
		case OPER_DIV:
			if (is_equal (arg2, 0, DEFAULT_PRECISION))
				return arg1 / arg2;
			else {
				fprintf (stderr, "%s (): zero division\n", __func__);
					return arg1;
			}
			break;
		case OPER_POW:
			return pow (arg1, arg2);
			break;
		case OPER_LN:
			return log (arg2);
			break;
		case OPER_COS:
			return cos (arg2);
			break;
		case OPER_SIN:
			return sin (arg2);
			break;
		case OPER_ERR:
		default:
			fprintf (stderr, "%s (): wrong operation\n", __func__);
			return 0;
			break;
	}
}

static bool simplify_operations (node_t *node)
{
	assert (node);
	bool simplify = false;

	switch (node->type) {
		case TYPE_CONST:
			return false;
			break;
		case TYPE_VAR:
			return false;
			break;
		case TYPE_OPER:
			if (LN)
				simplify_operations (LN);
			if (RN)
				simplify_operations (RN);
			switch (node->data.oper) {
				case OPER_PLUS:
					simplify = simplify || simplify_plus (node);
					break;
				case OPER_MINUS:
					simplify = simplify || simplify_minus (node);
					break;
				case OPER_MUL:
					simplify = simplify || simplify_mul (node);
					break;
				case OPER_DIV:
					simplify = simplify || simplify_div (node);
					break;
				case OPER_POW:
					simplify = simplify || simplify_pow (node);
					break;
				case OPER_LN:
				case OPER_COS:
				case OPER_SIN:
					//Nothing to simplify
					break;
				case OPER_ERR:
				default:
					fprintf (stderr, "%s (): wrong operation\n", __func__);
					return false;
					break;
			}
			return simplify;
		case TYPE_ERR:
		default:
			fprintf (stderr, "%s (): error node\n", __func__);
			return false;
			break;
	}
}

static bool simplify_plus (node_t *node)
{
	assert (node);
	assert (node->type == TYPE_OPER);
	assert (node->data.oper == OPER_PLUS);
	node_t *save_left = LN;
	node_t *save_right = RN;

	if (IS_NODE_EQUAL (LN, 0)) {
		copy_node (node, RN);
		free_node (save_left);
		free_node (save_right);
		return true;
	}
	if (IS_NODE_EQUAL (RN, 0)) {
		copy_node (node, LN);
		free_node (save_left);
		free_node (save_right);
		return true;
	}

	return false;
}

static bool simplify_minus (node_t *node)
{
	assert (node);
	assert (node->type == TYPE_OPER);
	assert (node->data.oper == OPER_MINUS);
	node_t *save_left = LN;
	node_t *save_right = RN;

	if (IS_NODE_EQUAL (RN, 0)) {
		copy_node (node, LN);
		free_node (save_left);
		free_node (save_right);
		return true;
	}

	return false;
}

static bool simplify_mul (node_t *node)
{
	assert (node);
	assert (node->type == TYPE_OPER);
	assert (node->data.oper == OPER_MUL);
	node_t *save_left = LN;
	node_t *save_right = RN;

	if (IS_NODE_EQUAL (LN, 1)) {
		copy_node (node, RN);
		free_node (save_left);
		free_node (save_right);
		return true;
	}
	if (IS_NODE_EQUAL (LN, 0)) {
		MAKE_CONST_ZERO (node);
		free_node (save_left);
		diff_free_tree (save_right);
		return true;
	}
	if (IS_NODE_EQUAL (RN, 1)) {
		copy_node (node, LN);
		free_node (save_left);
		free_node (save_right);
		return true;
	}
	if (IS_NODE_EQUAL (RN, 0)) {
		MAKE_CONST_ZERO (node);
		diff_free_tree (save_left);
		free_node (save_right);
		return true;
	}
	return false;
}

static bool simplify_div (node_t *node)
{
	assert (node);
	assert (node->type == TYPE_OPER);
	assert (node->data.oper == OPER_DIV);
	node_t *save_left = LN;
	node_t *save_right = RN;

	if (IS_NODE_EQUAL (LN, 0)) {
		MAKE_CONST_ZERO (node);
		free_node (save_left);
		diff_free_tree (save_right);
		return true;
	}
	if (IS_NODE_EQUAL (RN, 1)) {
		copy_node (node, LN);
		free_node (save_left);
		free_node (save_right);
		return true;
	}

	return false;
}

static bool simplify_pow (node_t *node)
{
	assert (node);
	assert (node->type == TYPE_OPER);
	assert (node->data.oper == OPER_POW);
	node_t *save_left = LN;
	node_t *save_right = RN;

	if (IS_NODE_EQUAL (LN, 1)) {
		MAKE_CONST_ONE (node);
		free_node (save_left);
		diff_free_tree (save_right);
		return true;
	}
	if (IS_NODE_EQUAL (LN, 0)) {
		MAKE_CONST_ZERO (node);
		free_node (save_left);
		diff_free_tree (save_right);
		return true;
	}
	if (IS_NODE_EQUAL (RN, 1)) {
		copy_node (node, LN);
		free_node (save_left);
		free_node (save_right);
		return true;
	}
	if (IS_NODE_EQUAL (RN, 0)) {
		MAKE_CONST_ONE (node);
		diff_free_tree (save_left);
		free_node (save_right);
		return true;
	}

	return false;
}

static void copy_node (node_t *dest, const node_t *src)
{
	assert (dest);
	assert (src);

	dest->type = src->type;
	dest->data = src->data;
	dest->left = src->left;
	dest->right = src->right;
}

static void free_node (node_t *node)
{
	assert (node);

	node->type = TYPE_ERR;
	node->data.value = 0;
	node->left = NULL;
	node->right = NULL;

	free (node);
}

static node_t *copy_tree (const node_t *node)
{
	if (!node) {
		fprintf (stderr, "%s (): null-pointer tree\n", __func__);
		return NULL;
	}
	if (LN && RN)
		return NODE (node->type, node->data, CL, CR);
	if (LN)
		return NODE (node->type, node->data, CL, NULL);
	if (RN)
		return NODE (node->type, node->data, NULL, CR);
	return TERM (node->type, node->data);
}

static bool is_const_subtree (const node_t *node)
{
	bool const_tree = true;
	if (!node) {
		fprintf (stderr, "%s (): null-pointer tree\n", __func__);
		return false;
	}
	if (LN) const_tree = (const_tree && IS_CONST (LN));
	if (RN) const_tree = (const_tree && IS_CONST (RN));
	const_tree = (const_tree && (node->type != TYPE_VAR));
	return const_tree;
}

static bool is_equal(double d1, double d2, double precision)
{
	if (fabs (d1 - d2) < precision * fmax (1.0, fmax (d1, d2)))
		return true;
	return false;
}
