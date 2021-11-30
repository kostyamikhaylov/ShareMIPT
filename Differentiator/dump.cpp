#include "diff.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define BUF_LEN 100
#define FILENAME "graph"
#define TEX_FILE "document"

static void dump_dot_node (const node_t *node, FILE *file);
static void dump_latex_node (const node_t *node, FILE *file);

void diff_dump_dot (const node_t *node)
{
	static int print_counter = 1;
	char buf[BUF_LEN] = {};

	if (!node) {
		fprintf (stderr, "%s (): null-pointer tree\n", __func__);
		return ;
	}

	snprintf (buf, BUF_LEN - 1, FILENAME "_%d.dot", print_counter);
	FILE *file = fopen (buf, "w");
	if (!file) {
		fprintf (stderr, "%s () error: can't open file " FILENAME ".dot\n", __func__);
		return ;
	}

	fprintf (file, "digraph G {\n");

	dump_dot_node (node, file);

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

static void dump_dot_node (const node_t *node, FILE *file)
{
	assert (node);
	assert (file);

	switch (node->type) {
		case TYPE_CONST:
			fprintf (file, "node%p [shape=oval, label=\"%lg\"]\n", node, node->data.value);
			break;
		case TYPE_VAR:
			fprintf (file, "node%p [shape=hexagon, label=\"%c\"]\n", node, node->data.var);
			break;
		case TYPE_OPER:
			fprintf (file, "node%p [shape=diamond, label=\"%c\"]\n", node, node->data.oper);
			break;
		case TYPE_ERR:
		default:
			fprintf (file, "node%p [shape=box, label=\"?%lg?%c?%c?\"]\n", node, node->data.value, node->data.var, node->data.oper);
			break;
	}
	if (node->left) {
		fprintf (file, "node%p -> node%p\n", node, node->left);
		dump_dot_node (node->left, file);
	}
	if (node->right) {
		fprintf (file, "node%p -> node%p\n", node, node->right);
		dump_dot_node (node->right, file);
	}
}

void diff_dump_latex (const node_t *node)
{
	FILE* file = NULL;

	if (!node) {
		fprintf (stderr, "%s (): null-pointer tree\n", __func__);
		return ;
	}
	file = fopen (TEX_FILE ".tex", "r+");
	if (!file)
	{
		fprintf (stderr, "%s (): failed to open file " TEX_FILE "\n", __func__);
		return ;
	}
	fseek (file, -15, SEEK_END);
	fprintf (file, "\n\\[");

	dump_latex_node (node, file);

	fprintf (file, "\\]\n\\end{document}");
	fclose (file);
	system ("pdflatex " TEX_FILE ".tex > /dev/null");
	system ("rm -f " TEX_FILE ".log " TEX_FILE ".aux " TEX_FILE ".out");
}

static void dump_latex_node (const node_t *node, FILE *file)
{
	assert (node);
	assert (file);

	switch (node->type)
	{
		case TYPE_CONST:
			fprintf (file, "%lg", node->data.value);
			break;
		case TYPE_VAR:
			fprintf (file, "%c", node->data.var);
			break;
		case TYPE_OPER:
			switch (node->data.oper) {
				case OPER_PLUS:
					fprintf (file, "(");
					dump_latex_node (node->left, file);
					fprintf (file, "+");
					dump_latex_node (node->right, file);
					fprintf (file, ")");
					break;
				case OPER_MINUS:
					fprintf (file, "(");
					dump_latex_node (node->left, file);
					fprintf (file, "-");
					dump_latex_node (node->right, file);
					fprintf (file, ")");
					break;
				case OPER_MUL:
					dump_latex_node (node->left, file);
					fprintf (file, " \\cdot ");
					dump_latex_node (node->right, file);
					break;
				case OPER_DIV:
					fprintf (file, " \\frac{");
					dump_latex_node (node->left, file);
					fprintf (file, "}{");
					dump_latex_node (node->right, file);
					fprintf (file, "}");
					break;
				case OPER_POW:
					fprintf (file, "(");
					dump_latex_node (node->left, file);
					fprintf (file, ")^{");
					dump_latex_node (node->right, file);
					fprintf (file, "}");
					break;
				case OPER_LN:
					fprintf (file, "\\ln{");
					dump_latex_node (node->right, file);
					fprintf (file, "}");
					break;
				case OPER_COS:
					fprintf (file, "\\cos{");
					dump_latex_node (node->right, file);
					fprintf (file, "}");
					break;
				case OPER_SIN:
					fprintf (file, "\\sin{");
					dump_latex_node (node->right, file);
					fprintf (file, "}");
					break;
				case OPER_ERR:
				default:
					fprintf (stderr, "%s (): unknown operator %c\n", __func__, (char) node->data.oper);
					break;
			}
			break;
		case TYPE_ERR:
		default:
			fprintf (stderr, "%s (): unknown type %d\n", __func__, node->type);
			break;
	}
}

