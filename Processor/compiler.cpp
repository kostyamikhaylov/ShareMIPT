#include "processor.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define MAX_LABELS_NUM 20
#define MAX_LABEL_LEN 32

#define MAX_LINE_LEN 20
#define MAX_CMD_LEN MAX_LABEL_LEN
#define MAX_REG_LEN 5

enum parse_str_to_cmd_return
{
	RET_ERR,
	RET_CMD,
	RET_LABEL
};

enum label_entry
{
	LABEL_EMPTY,
	LABEL_NAME_ONLY,
	LABEL_SHIFT_ONLY,
	LABEL_VALID
};

struct label
{
	enum label_entry status = LABEL_EMPTY;
	int shift = -1;
	char name[MAX_LABEL_LEN] = {};
};

struct label labels[MAX_LABELS_NUM] = {};

static enum parse_str_to_cmd_return parse_str_to_cmd (const char *str, char *cmd_ptr, char *reg_num_ptr, int *arg_ptr);
static int register_label_by_name (const char *label_name);
char choose_cmd (const char *cmd_str);
static int parse_reg_and_arg (const char *str, char *reg_num_ptr, int *arg_ptr);
int get_reg_num (const char *reg);
static bool is_jump (const char cmd);
static void dump_labels (void);
static int verify_labels (void);

int main (int argc, char *argv[])
{
	FILE *input = NULL, *output = NULL;
	char str[MAX_LINE_LEN];
	char cmd = 0, reg_num = 0;
	int arg = 0;
	char *byte_code = NULL;
	struct stat st = {};
	size_t pc = 0;
	bool first_pass = true;

	if (argc != 3) {
		fprintf (stderr, "Usage: %s input output\n", argv[0]);
		return 1;
	}

	input = fopen (argv[1], "r");
	if (!input) {
		fprintf (stderr, "Can't open file %s\n", argv[1]);
		return 1;
	}

	output = fopen (argv[2], "w");
	if (!output) {
		fprintf (stderr, "Can't open file %s\n", argv[2]);
		fclose (input);
		return 1;
	}

	if (stat (argv[1], &st) < 0) {
		fprintf (stderr, "Stat syscall failed\n");
		fclose (input);
		fclose (output);
		return 1;
	}

	byte_code = (char *) calloc ((size_t) st.st_size, 1 + sizeof (int));
	if (!byte_code) {
		fprintf (stderr, "Can't allocate memory\n");
		fclose (input);
		fclose (output);
		return 1;
	}

next_pass:
	while (fgets (str, MAX_LINE_LEN, input)) {
		char *c = strchr (str, '\n');
		if (c)
			*c = '\0';

		switch (parse_str_to_cmd (str, &cmd, &reg_num, &arg))
		{
			case RET_CMD:
				break;
			case RET_LABEL:
				labels[arg].shift = (int) pc;
				labels[arg].status = LABEL_VALID;
				continue;
				break;
			case RET_ERR:
			default:
				fprintf (stderr, "Compiler: parsing command \"%s\" failed\n", str);
				goto out_err;
				break;
		}

		if (is_jump (cmd)) {
			byte_code[pc++] = cmd;
			if (labels[arg].status == LABEL_VALID) {
				*(int *) (byte_code + pc) =
					labels[arg].shift;
			}
			pc += sizeof (int);
			continue;
		}

		bool imm = false, reg = false;
		imm = (cmd & IMM);
		reg = (cmd & REG);

		/*
		bool mem = false;
		mem = (cmd & MEM);

		printf ("imm = %d, reg = %d, mem = %d, cmd: %c%c%c%c%c%c%c%c\n", imm, reg, mem,
			(cmd & 0x80) ? '1' : '0',
			(cmd & 0x40) ? '1' : '0',
			(cmd & 0x20) ? '1' : '0',
			(cmd & 0x10) ? '1' : '0',
			(cmd & 0x08) ? '1' : '0',
			(cmd & 0x04) ? '1' : '0',
			(cmd & 0x02) ? '1' : '0',
			(cmd & 0x01) ? '1' : '0'
			);
		*/

		byte_code[pc++] = cmd;
		if (imm) {
			*(int *)(byte_code + pc) = arg;
			pc += sizeof (int);
		}
		if (reg) {
			*(byte_code + pc) = reg_num;
			pc++;
		}

		memset (str, '\0', MAX_LINE_LEN);
	}
	if (first_pass) {
		pc = 0;
		fseek (input, 0, SEEK_SET);
		if (verify_labels () < 0) {
			fprintf (stderr, "Labels table is incorrect after first pass\n");
			dump_labels ();
			goto out_err;
		}
		first_pass = false;
		goto next_pass;
	}

	if (write_sign_and_ver (output)) {
		fprintf (stderr, "Failed to write signature and version to %s\n", argv[2]);
		goto out_err;
	}

	if (fwrite (byte_code, sizeof (char), pc, output) < pc) {
		fprintf (stderr, "Failed to write byte_code to %s\n", argv[2]);
		goto out_err;
	}

	free (byte_code);
	fclose (input);
	fclose (output);
	return 0;

out_err:
	free (byte_code);
	fclose (input);
	fclose (output);
	return 1;
}

static enum parse_str_to_cmd_return parse_str_to_cmd (const char *str, char *cmd_ptr, char *reg_num_ptr, int *arg_ptr)
{
	assert (str);
	assert (cmd_ptr);
	assert (reg_num_ptr);
	assert (arg_ptr);
	int cmd = 0, ptr = 0, ptr1 = 0;
	int scanf_ok = 0;
	int cmd_reg_and_arg = 0;
	char cmd_str[MAX_CMD_LEN + 1];
	char reg_and_arg_str[MAX_LINE_LEN + 1];
	memset (cmd_str, '\0', MAX_CMD_LEN + 1);
	memset (reg_and_arg_str, '\0', MAX_LINE_LEN + 1);

	while (isspace (str[ptr]))
		ptr++;

	while ( islower (str[ptr]) &&
		str[ptr] != '\0' &&
		ptr != MAX_CMD_LEN) {

		cmd_str[ptr1] = str[ptr];
		ptr++;
		ptr1++;
	}
	if (str[ptr] == ':') {
		*arg_ptr = register_label_by_name (cmd_str);
		return RET_LABEL;
	} else {
		cmd = choose_cmd (cmd_str);
		if (cmd < 0) {
			fprintf (stderr, "parse_str_to_cmd failed: can't recognize command %s\n", cmd_str);
			return RET_ERR;
		}
	}

	if (is_jump ((char) cmd)) {
		while (isspace (str[ptr]))
			ptr++;
		memset (cmd_str, '\0', MAX_CMD_LEN);
		ptr1 = 0;
		while ( islower (str[ptr]) &&
			str[ptr] != '\0' &&
			ptr != MAX_CMD_LEN) {

			cmd_str[ptr1] = str[ptr];
			ptr++;
			ptr1++;
		}
		*arg_ptr = register_label_by_name (cmd_str);
		*cmd_ptr = (char) cmd;
		return RET_CMD;
	}

	while (isspace (str[ptr]))
		ptr++;
	if (str[ptr] == '\0') {
		*cmd_ptr = (char) cmd;
		return RET_CMD;
	}

	if (sscanf (str + ptr, "[%[^]]]%n", reg_and_arg_str, &scanf_ok) && scanf_ok) {
		if ((cmd_reg_and_arg = parse_reg_and_arg (reg_and_arg_str, reg_num_ptr, arg_ptr)) < 0) {
			fprintf (stderr, "parse_str_to_cmd failed: register and argment parsing error\n");
			return RET_ERR;
		}
		cmd = cmd | MEM;
		if (cmd_reg_and_arg & IMM) cmd = cmd | IMM;
		if (cmd_reg_and_arg & REG) cmd = cmd | REG;
		*cmd_ptr = (char) cmd;
		return RET_CMD;
	} else if (sscanf (str + ptr, "%s", reg_and_arg_str)) {
		if ((cmd_reg_and_arg = parse_reg_and_arg (reg_and_arg_str, reg_num_ptr, arg_ptr)) < 0) {
			fprintf (stderr, "parse_str_to_cmd failed: register and argment parsing error\n");
			return RET_ERR;
		}
		if (cmd_reg_and_arg & IMM) cmd = cmd | IMM;
		if (cmd_reg_and_arg & REG) cmd = cmd | REG;
		*cmd_ptr = (char) cmd;
		return RET_CMD;
	} else {
		fprintf (stderr, "parse_str_to_cmd failed: can't parse \"%s\"\n", str + ptr);
		return RET_ERR;
	}

	return RET_CMD;
}

static int register_label_by_name (const char *label_name)
{
	int i = 0;
	enum label_entry st = LABEL_EMPTY;
	for (i = 0; i < MAX_LABELS_NUM; i++) {
		st = labels[i].status;
		if (st == LABEL_VALID) {
			if (!strcmp (labels[i].name, label_name))
				return i;
			continue;
		}
		else if (st == LABEL_NAME_ONLY) {
			if (!strcmp (labels[i].name, label_name)) {
				return i;
			} else {
				continue;
			}
		} else if (st == LABEL_SHIFT_ONLY) {
			fprintf (stderr, "register_label_by_name error: "
					"status field is LABEL_SHIFT_ONLY, "
					"that's incorrect\n");
			return -1;
		} else if (st == LABEL_EMPTY) {
			strncpy (labels[i].name, label_name, MAX_LABEL_LEN);
			labels[i].status = LABEL_NAME_ONLY;
			return i;
		} else {
			fprintf (stderr, "register_label_by_name error: "
					"status doesn't match any of "
					"possible values\n");
			return -1;
		}
	}
	fprintf (stderr, "register_label_by_name error: maximum number of labels exceeded\n");
	return -1;
}

char choose_cmd (const char *cmd_str)
{
	if (!strcmp (cmd_str, "push")) {
		return CMD_PUSH;
	} else if (!strcmp (cmd_str, "pop")) {
		return CMD_POP;
	} else if (!strcmp (cmd_str, "add")) {
		return CMD_ADD;
	} else if (!strcmp (cmd_str, "sub")) {
		return CMD_SUB;
	} else if (!strcmp (cmd_str, "mul")) {
		return CMD_MUL;
	} else if (!strcmp (cmd_str, "div")) {
		return CMD_DIV;
	} else if (!strcmp (cmd_str, "in")) {
		return CMD_IN;
	} else if (!strcmp (cmd_str, "out")) {
		return CMD_OUT;
	} else if (!strcmp (cmd_str, "hlt")) {
		return CMD_HLT;
	} else if (!strcmp (cmd_str, "jmp")) {
		return CMD_JMP;
	} else if (!strcmp (cmd_str, "ja")) {
		return CMD_JA;
	} else if (!strcmp (cmd_str, "jae")) {
		return CMD_JAE;
	} else if (!strcmp (cmd_str, "jb")) {
		return CMD_JB;
	} else if (!strcmp (cmd_str, "jbe")) {
		return CMD_JBE;
	} else if (!strcmp (cmd_str, "je")) {
		return CMD_JE;
	} else if (!strcmp (cmd_str, "jne")) {
		return CMD_JNE;
	} else if (!strcmp (cmd_str, "call")) {
		return CMD_CALL;
	} else if (!strcmp (cmd_str, "ret")) {
		return CMD_RET;
	} else {
		fprintf (stderr, "choose_cmd () error: unknown command: %s\n", cmd_str);
		return -1;
	}
}


static int parse_reg_and_arg (const char *str, char *reg_num_ptr, int *arg_ptr)
{
	assert (str);
	assert (reg_num_ptr);
	assert (arg_ptr);
	int reg_num = 0, arg = 0;
	int ptr = 0;
	int cmd = 0;
	int scanf_ok = 0;
	char reg_str[MAX_REG_LEN + 1];

	while (isspace (str[ptr])) ptr++;
	if (*(str + ptr) == '\0') return cmd;

	if (isdigit (str[ptr])) {
		if (sscanf (str + ptr, "%d%n", &arg, &scanf_ok) != 1) {
			fprintf (stderr, "parse_reg_and_arg () failed: inccorrect syntax\n");
			return -1;
		}
		ptr += scanf_ok;
		*arg_ptr = arg;
		cmd = cmd | IMM;

		while (isspace (str[ptr])) ptr++;
		if (*(str + ptr) == '\0') {
			return cmd;
		}

		if (*(str + ptr) == '+') ptr++;

		while (isspace (str[ptr])) ptr++;

		if (!isalpha (str[ptr])) {
			fprintf (stderr, "parse_reg_and_arg () failed: inccorrect syntax\n");
			return -1;
		}
		if (sscanf (str + ptr, "%s", reg_str) != 1) {
			fprintf (stderr, "parse_reg_and_arg () failed: inccorrect syntax\n");
			return -1;
		}
		reg_num = get_reg_num (reg_str);
		if (reg_num < 0) {
			fprintf (stderr, "parse_reg_and_arg () failed: register not recognized\n");
			return -1;
		}
		*reg_num_ptr = (char) reg_num;
		cmd = cmd | REG;
		return cmd;
	} else if (isalpha (str[ptr])) {
		int i = 0;
		while ( isalpha (str[ptr]) &&
			str[ptr] != '\0') {
			reg_str[i] = str[ptr];
			i++;
			ptr++;
		}
		reg_num = get_reg_num (reg_str);
		if (reg_num < 0) {
			fprintf (stderr, "parse_reg_and_arg () failed: register not recognized\n");
			return -1;
		}
		*reg_num_ptr = (char) reg_num;
		cmd = cmd | REG;

		while (isspace (str[ptr])) ptr++;
		if (*(str + ptr) == '\0') return cmd;

		if (*(str + ptr) == '+') ptr++;

		while (isspace (str[ptr])) ptr++;

		if (isdigit (*(str + ptr))) {
			if (sscanf (str + ptr, "%d", &arg) != 1) {
				fprintf (stderr, "parse_reg_and_arg () failed: inccorrect syntax\n");
				return -1;
			}
			*arg_ptr = arg;
			cmd = cmd | IMM;
			return cmd;
		} else {
			fprintf (stderr, "parse_reg_and_arg () failed: inccorrect syntax\n");
			return -1;
		}
	} else {
		fprintf (stderr, "parse_reg_and_arg () failed: can't parse string \"%s\"\n", str);
		return -1;
	}
}

int get_reg_num (const char *reg)
{
	if (!strncmp (reg, "ax", 2)) return 0;
	if (!strncmp (reg, "bx", 2)) return 1;
	if (!strncmp (reg, "cx", 2)) return 2;
	if (!strncmp (reg, "dx", 2)) return 3;
	return -1;
}

static bool is_jump (const char cmd)
{
	if (	(cmd == CMD_JMP) ||
		(cmd == CMD_JA)  ||
		(cmd == CMD_JAE) ||
		(cmd == CMD_JB)  ||
		(cmd == CMD_JAE) ||
		(cmd == CMD_JE)  ||
		(cmd == CMD_JNE) ||
		(cmd == CMD_CALL))
		return true;
	return false;
}

static void dump_labels (void)
{
	printf ("Labels:\n{\n");
	for (int i = 0; i < MAX_LABELS_NUM; i++) {
		printf ("%d:\t{status: %d; shift: %d; name: %s}\n", i,
				labels[i].status,
				labels[i].shift,
				(strlen (labels[i].name) != 0) ?
					labels[i].name : "-");
	}
	printf ("}\n");
}

static int verify_labels (void)
{
	int i = 0;
	for (i = 0; i < MAX_LABELS_NUM; i++) {
		if (	labels[i].status != LABEL_VALID &&
			labels[i].status != LABEL_EMPTY ) {
			printf ("%d:\t{status: %d; shift: %d; name: %s}\n", i,
					labels[i].status,
					labels[i].shift,
					(strlen (labels[i].name) != 0) ?
						labels[i].name : "-");
			return -1;
		}
	}
	return 0;
}
