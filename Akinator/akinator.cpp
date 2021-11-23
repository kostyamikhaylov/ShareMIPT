#include "akinator.h"
#include "../Stack/stack.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define CMD_BUF_LEN 100
#define CMD_DOT_LEN 130

enum answers {
	ANS_YES,
	ANS_NO,
	ANS_ERROR
};

struct feature {
	char *str = NULL;
	bool applicable = false;
};

node *akinator_root = NULL;
char base_root_name[] = "Nobody";
node base_root = {base_root_name, NULL, NULL};

static enum mode scanf_mode (void);
node *build_tree_from_array (const char *buf, size_t *ptr);
static enum answers ask_is_right (void);
static void add_new_entry (node *nd);
static bool find_node_and_fill_stack (const node *nd, const char *name, Stack *stk, int *depth_ptr);
static int print_feature (const void *ptr);
static void dump_tree_to_file (const node *nd, FILE *file);
static void draw_tree_to_file (const node *nd, FILE *file_dot);
static void free_tree (node *nd);

enum mode akinator_get_mode (void)
{
	enum mode md = MODE_ERROR;
	static bool first_time = true;

	if (first_time) {
		printf ("Welcome to Akinator!\n");
		akinator_root = &base_root;
		first_time = false;
	}

	printf ("What you want to do?\n");

	md = scanf_mode ();
	if (md != MODE_ERROR)
		return md;

	printf ("Incorrect game mode!\n");
	printf ("Possible game modes: "
		"\"load\" (or \"l\"), "
		"\"play\" (\"p\"), "
		"\"desc\", "
		"\"diff\", "
		"\"save\" (\"s\"), "
		"\"draw\", "
		"or \"exit\" (\"q\")\n");
	printf ("Try again:\n");
	md = scanf_mode ();
	return md;
}

static enum mode scanf_mode (void)
{
	char cmd_buf[CMD_BUF_LEN] = {};

	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	char *c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';

	if (	!strncmp (cmd_buf, "l", CMD_BUF_LEN) ||
		!strncmp (cmd_buf, "load", CMD_BUF_LEN))
		return MODE_LOAD;
	else if (	!strncmp (cmd_buf, "p", CMD_BUF_LEN) ||
			!strncmp (cmd_buf, "play", CMD_BUF_LEN))
		return MODE_PLAY;
	else if (!strncmp (cmd_buf, "desc", CMD_BUF_LEN))
		return MODE_DESC;
	else if (!strncmp (cmd_buf, "diff", CMD_BUF_LEN))
		return MODE_DIFF;
	else if (!strncmp (cmd_buf, "draw", CMD_BUF_LEN))
		return MODE_DRAW;
	else if (	!strncmp (cmd_buf, "s", CMD_BUF_LEN) ||
			!strncmp (cmd_buf, "save", CMD_BUF_LEN))
		return MODE_SAVE;
	else if (	!strncmp (cmd_buf, "q", CMD_BUF_LEN) ||
			!strncmp (cmd_buf, "exit", CMD_BUF_LEN))
		return MODE_EXIT;
	else {
		fprintf (stderr, "Can't recognize your answer: %s\n", cmd_buf);
		return MODE_ERROR;
	}
}

void akinator_load_base (void)
{
	int fd = -1;
	size_t shift = 0;
	size_t len = 0;
	node *tmp_root = NULL;
	char *text_buf = NULL;
	char cmd_buf[CMD_BUF_LEN] = {};
	struct stat st = {};

	printf ("Going to load your tree.\n"
		"Enter the name of file: ");

	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	char *c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';

	fd = open (cmd_buf, O_RDONLY);
	if (fd < 0) {
		printf ("akinator_load_base (): can't open file \"%s\"\n", cmd_buf);
		return ;
	}

	if (fstat (fd, &st) < 0) {
		fprintf (stderr, "akinator_load_base (): fstat syscall failed\n");
		close (fd);
		return ;
	}

	len = (size_t) st.st_size;

	text_buf = (char *) calloc (len + 1, sizeof (char));
	if (!text_buf) {
		fprintf (stderr, "akinator_load_base (): can't allocate memory\n");
		close (fd);
		return ;
	}

	if (read (fd, text_buf, len) < (ssize_t) len) {
		fprintf (stderr, "akinator_load_base (): read syscall failed, can't read the whole file\n");
		free (text_buf);
		close (fd);
		return ;
	}

	if ((tmp_root = build_tree_from_array (text_buf, &shift)) == NULL) {
		fprintf (stderr, "Failed to build Akinator tree from file\n");
		return ;
	}

	if (akinator_root)
		free_tree (akinator_root);
	akinator_root = tmp_root;

	printf ("Tree loaded from file \"%s\"\n", cmd_buf);
	free (text_buf);
	close (fd);
	return ;
}

node *build_tree_from_array (const char *buf, size_t *ptr)
{
	assert (buf);
	assert (ptr);
	char cmd_buf[CMD_BUF_LEN] = {};
	size_t len = 0;
	node *new_node = NULL;

	while (isspace (buf[*ptr])) { (*ptr)++; }

	if (buf[*ptr] == '{') {
		(*ptr)++;
		new_node = (node *) calloc (sizeof (node), 1);
		while (isspace (buf[*ptr])) { (*ptr)++; }
		sscanf (buf + *ptr, "%[^}{]", cmd_buf);
		len = strlen (cmd_buf);
		*ptr += len;
		while (isspace (cmd_buf[len - 1])) {
			cmd_buf[len - 1] = '\0';
			len--;
		}
		new_node->data = (char *) calloc (sizeof (char), len + 1);
		strncpy (new_node->data, cmd_buf, len);

		if (buf[*ptr] == '}') {
			(*ptr)++;
			return new_node;
		}
		// if (buf[*ptr] == '{')
		new_node->left = build_tree_from_array (buf, ptr);
		if (!new_node->left)
			return NULL;
		new_node->right = build_tree_from_array (buf, ptr);
		if (!new_node->right)
			return NULL;

		if (buf[*ptr] == '}') {
			(*ptr)++;
			return new_node;
		} else {
			fprintf (stderr, "build_tree_from_array ():"
					"unknown symbol '%c'\n", buf[*ptr]);
			return NULL;
		}
	} else {
		fprintf (stderr, "build_tree_from_array ():"
				"unknown symbol '%c'\n", buf[*ptr]);
		return NULL;
	}

	return NULL;
}

void akinator_play (void)
{
	node *cur = NULL;

	cur = akinator_root;
	while (cur->left || cur->right) {
		printf ("What you have guessed is %s?\n", cur->data);
		switch (ask_is_right ()) {
			case ANS_YES:
				cur = cur->left;
				break;
			case ANS_NO:
				cur = cur->right;
				break;
			case ANS_ERROR:
			default:
				fprintf (stderr,
					"akinator_play (): wrong answer\n");
				return ;
				break;
		}
	}

	printf ("Did you guessed %s?\n", cur->data);
	switch (ask_is_right ()) {
		case ANS_YES:
			printf ("Great, I was right!!!\n");
			break;
		case ANS_NO:
			printf ("Hmm, guess I was so close...\n");
			add_new_entry (cur);
			break;
		case ANS_ERROR:
		default:
			fprintf (stderr, "akinator_play (): "
					"answer not recognized\n");
			break;
	}

	return ;
}

static enum answers ask_is_right (void)
{
	char cmd_buf[CMD_BUF_LEN] = {};
	printf ("Print \"yes\" (\"y\") or \"no\" (\"n\"):\n");

	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	char *c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';

	if (	!strncmp (cmd_buf, "y", CMD_BUF_LEN) ||
		!strncmp (cmd_buf, "yes", CMD_BUF_LEN))
		return ANS_YES;
	if (	!strncmp (cmd_buf, "n", CMD_BUF_LEN) ||
		!strncmp (cmd_buf, "no", CMD_BUF_LEN))
		return ANS_NO;

	printf ("Just type \"yes\" (\"y\") or \"no\" (\"n\")!\n");

	memset (cmd_buf, '\0', CMD_BUF_LEN);

	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';

	if (	!strncmp (cmd_buf, "y", CMD_BUF_LEN) ||
		!strncmp (cmd_buf, "yes", CMD_BUF_LEN))
		return ANS_YES;
	if (	!strncmp (cmd_buf, "n", CMD_BUF_LEN) ||
		!strncmp (cmd_buf, "no", CMD_BUF_LEN))
		return ANS_NO;

	printf ("Where do your hands grow from?\nAborting...\n");

	return ANS_ERROR;
}

static void add_new_entry (node *nd)
{
	assert (nd);

	char cmd_buf[CMD_BUF_LEN] = {};
	printf ("What did you guessed?\nType here:\n");

	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	char *c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';

	size_t str_len = strlen (cmd_buf);
	size_t real_len = (str_len >= CMD_BUF_LEN) ? CMD_BUF_LEN - 1 : str_len;
	char *name = (char *) calloc (sizeof (char), real_len + 1);
	strncpy (name, cmd_buf, real_len);
	node *new_node_left = (node *) calloc (sizeof (node), 1);
	node *new_node_right = (node *) calloc (sizeof (node), 1);
	new_node_right->data = nd->data;
	new_node_left->data = name;
	nd->left = new_node_left;
	nd->right = new_node_right;

	memset (cmd_buf, '\0', CMD_BUF_LEN);
	printf ("What is the difference between %s and %s?\nIt is ", name, nd->data);
	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';

	str_len = strlen (cmd_buf);
	real_len = (str_len >= CMD_BUF_LEN) ? CMD_BUF_LEN - 1 : str_len;
	char *diff = (char *) calloc (sizeof (char), real_len + 1);
	strncpy (diff, cmd_buf, real_len);
	nd->data = diff;

	return ;
}

void akinator_description (void)
{
	bool found = false;
	int depth = 0;
	enum error_type error = OK;
	Stack st = {};
	char cmd_buf[CMD_BUF_LEN] = {};
	struct feature cur = {};

	error = stack_ctor (&st, sizeof (struct feature), "struct feature", print_feature);
	if (error != OK) {
		fprintf (stderr, "akinator_description (): failed to create stack\n");
		return ;
	}

	printf ("You want to get the description of: ");

	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	char *c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';

	found = find_node_and_fill_stack (akinator_root, cmd_buf, &st, &depth);
	if (!found) {
		printf ("Entry \"%s\" was not found!\n", cmd_buf);
		goto exit;
	}

	printf ("Your description:\n");
	printf ("%s is ", cmd_buf);
	if (depth == 0)
		printf ("unknown object\n");
	else {
		while (depth) {
			error = stack_pop (&st, &cur);
			if (error != OK) {
				fprintf (stderr, "akinator_description (): failed to pop from stack\n");
				return ;
			}

			if (!cur.applicable)
				printf ("not ");
			depth--;
			printf ("%s", cur.str);
			if (depth == 0)
				printf ("\n");
			else if (depth == 1)
				printf (" and ");
			else
				printf (", ");
		}
	}

exit:
	error = stack_dtor (&st);
	if (error != OK) {
		fprintf (stderr, "akinator_description (): failed to destroy stack\n");
		return ;
	}

	return ;
}

static bool find_node_and_fill_stack (const node *nd, const char *name, Stack *stk, int *depth_ptr)
{
	assert (nd);
	assert (name);
	assert (stk);

	struct feature tmp_feature = {};

	if (nd->left && nd->right) {
		tmp_feature.str = nd->data;
		if (find_node_and_fill_stack (nd->left, name, stk, depth_ptr)) {
			(*depth_ptr)++;
			tmp_feature.applicable = true;
			stack_push (stk, &tmp_feature);
			return true;
		}
		if (find_node_and_fill_stack (nd->right, name, stk, depth_ptr)) {
			(*depth_ptr)++;
			tmp_feature.applicable = false;
			stack_push (stk, &tmp_feature);
			return true;
		}
		return false;
	} else {
		if (!strcmp (nd->data, name))
			return true;
		return false;
	}
}

static int print_feature (const void *ptr)
{
	assert (ptr);
	const struct feature *f = (const struct feature *) ptr;

	printf ("%c: %s", f->applicable ? '+' : '-', f->str);
	return 0;
}

void akinator_diff (void)
{
	bool found = false, no_sim = true;
	int depth1 = 0, depth2 = 0;
	enum error_type error1 = OK, error2 = OK;
	Stack st1 = {}, st2 = {};
	char obj1[CMD_BUF_LEN] = {}, obj2[CMD_BUF_LEN] = {};
	struct feature cur1 = {}, cur2 = {};

	error1 = stack_ctor (&st1, sizeof (struct feature),
				"struct feature", print_feature);
	error2 = stack_ctor (&st2, sizeof (struct feature),
				"struct feature", print_feature);
	if (error1 != OK || error2 != OK) {
		fprintf (stderr, "akinator_description (): failed to create stack\n");
		return ;
	}

	printf ("Your two objects: \n");

	printf ("Enter the first: ");
	fgets (obj1, CMD_BUF_LEN, stdin);
	char *c = strchr (obj1, '\n');
	if (c) *c = '\0';

	printf ("Enter the second: ");
	fgets (obj2, CMD_BUF_LEN, stdin);
	c = strchr (obj2, '\n');
	if (c) *c = '\0';


	found = find_node_and_fill_stack (akinator_root, obj1, &st1, &depth1);
	if (!found) {
		printf ("Entry \"%s\" was not found!\n", obj1);
		goto exit;
	}

	found = find_node_and_fill_stack (akinator_root, obj2, &st2, &depth2);
	if (!found) {
		printf ("Entry \"%s\" was not found!\n", obj2);
		goto exit;
	}

	printf ("Similarity:\n");
	printf ("%s and %s both are:\n", obj1, obj2);
	while (depth1 && depth2) {
		error1 = stack_pop (&st1, &cur1);
		error2 = stack_pop (&st2, &cur2);
		depth1--; depth2--;
		if (error1 == OK && error2 == OK &&
		    cur1.str == cur2.str &&
		    cur1.applicable == cur2.applicable) {
			printf (" - ");
			if (!cur1.applicable)
				printf ("not ");
			printf ("%s\n", cur1.str);
			no_sim = false;
		} else if (error1 == OK && error2 == OK) {
			if (no_sim)
				printf ("Nothing in common\n");
			printf ("But %s is ", obj1);
			if (!cur1.applicable)
				printf ("not ");
			printf ("%s", cur1.str);
			printf (", while %s is ", obj2);
			if (!cur2.applicable)
				printf ("not ");
			printf ("%s\n", cur2.str);
			error1 = stack_push (&st1, &cur1);
			error2 = stack_push (&st2, &cur2);
			depth1++; depth2++;
			break;
		}
	}

exit:
	error1 = stack_dtor (&st1);
	error2 = stack_dtor (&st2);
	if (error1 != OK && error2 != OK) {
		fprintf (stderr, "akinator_description (): failed to destroy stack\n");
		return ;
	}

	return ;
}

void akinator_save_base (void)
{
	FILE *file = NULL;
	char cmd_buf[CMD_BUF_LEN] = {};

	printf ("Ok, let's save your tree.\n"
		"Enter the name of file: ");

	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	char *c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';

	file = fopen (cmd_buf, "w");
	if (!file) {
		fprintf (stderr, "akinator_save_base (): can't open file \"%s\"\n", cmd_buf);
		return ;
	}

	dump_tree_to_file (akinator_root, file);

	printf ("Tree was dumped to file \"%s\"\n", cmd_buf);
	fclose (file);
	return ;
}

static void dump_tree_to_file (const node *nd, FILE *file)
{
	assert (nd);
	assert (file);

	fprintf (file, "{ %s ", nd->data);

	if (nd->left && nd->right) {
		dump_tree_to_file (nd->left, file);
		dump_tree_to_file (nd->right, file);
	}

	fprintf (file, "}");
	return ;
}

void akinator_draw (void)
{
	FILE *file_dot = NULL, *file_png = NULL;
	char cmd_buf[CMD_BUF_LEN] = {};
	char cmd_dot[CMD_DOT_LEN] = {};

	printf ("Graphical dump called\n"
		"Enter the name of file (E.g., \"graph.png\"): ");

	fgets (cmd_buf, CMD_BUF_LEN, stdin);
	char *c = strchr (cmd_buf, '\n');
	if (c) *c = '\0';


	file_dot = fopen ("tmp.dot", "w");
	if (!file_dot) {
		printf ("akinator_save_base (): failed to open file \"tmp.dot\"\n");
		return ;
	}
	file_png = fopen (cmd_buf, "w");
	if (!file_png) {
		printf ("akinator_save_base (): failed to open file \"%s\"\n", cmd_buf);
		fclose (file_dot);
		return ;
	}

	fprintf (file_dot, "digraph G {\n");
	draw_tree_to_file (akinator_root, file_dot);
	fprintf (file_dot, "}");

	fclose (file_dot);
	snprintf (cmd_dot, CMD_DOT_LEN, "dot tmp.dot -Tpng -o %s", cmd_buf);
	system (cmd_dot);
	system ("rm -f tmp.dot");
	printf ("Tree was dumped to file \"%s\"\n", cmd_buf);
	fclose (file_png);
	return ;
}

static void draw_tree_to_file (const node *nd, FILE *file_dot)
{
	assert (nd);
	assert (file_dot);


	if (nd->left && nd->right) {
		fprintf (file_dot, "\"%s\"->", nd->data);
		draw_tree_to_file (nd->left, file_dot);
		fprintf (file_dot, "\"%s\"->", nd->data);
		draw_tree_to_file (nd->right, file_dot);
	} else
		fprintf (file_dot, "\"%s\"\n", nd->data);

	return ;

}

void akinator_exit (void)
{
	if (akinator_root)
		free_tree (akinator_root);
	return ;
}

static void free_tree (node *nd)
{
	assert (nd);
	if (nd->left && nd->right) {
		free_tree (nd->left);
		free_tree (nd->right);
		nd->left = NULL;
		nd->right = NULL;
	}

	if (nd->data != base_root_name) {
		free (nd->data);
		nd->data = NULL;
	}
	if (nd != &base_root)
		free (nd);

	return ;
}

