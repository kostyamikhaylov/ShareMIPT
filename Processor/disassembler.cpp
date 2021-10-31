#include "processor.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>


int main (int argc, char *argv[])
{
	FILE *input = NULL, *output = NULL;
	int arg = 0;
	char cmd = 0, reg_num;
	char *byte_code = NULL, *reg_name = NULL;
	size_t byte_code_len = 0, pc = 0;
	struct stat st = {};
	int sign_and_ver_len = 0;
	bool imm = false, reg = false, mem = false;

	if (argc < 2 || argc > 3) {
		fprintf (stderr, "Usage: %s input [output]\n", argv[0]);
		return 1;
	}

	input = fopen (argv[1], "r");
	if (!input) {
		fprintf (stderr, "Can't open file %s\n", argv[1]);
		return 2;
	}

	if (argc == 3) {
		output = fopen (argv[2], "w");
		if (!output) {
			fprintf (stderr, "Can't open file %s\n", argv[2]);
			fclose (input);
			return 3;
		}
	} else {
		output = stdout;
	}

	if (stat (argv[1], &st) < 0) {
		fprintf (stderr, "Stat syscall failed\n");
		fclose (input);
		if (output != stdout) fclose (output);
		return 1;
	}

	byte_code_len = (size_t) st.st_size;

	if ((sign_and_ver_len = check_sign_and_ver (input)) < 0) {
		fprintf (stderr, "Input file type verification failed\n");
		fclose (input);
		if (output != stdout) fclose (output);
		return 1;
	}

	byte_code_len -= (size_t) sign_and_ver_len;

	byte_code = (char *) calloc (byte_code_len, sizeof (char));
	if (!byte_code) {
		fprintf (stderr, "Can't allocate memory\n");
		fclose (input);
		if (output != stdout) fclose (output);
		return 1;
	}

	if (fread (byte_code, sizeof (char), byte_code_len, input) < byte_code_len) {
		fprintf (stderr, "Failed to read byte_code from %s\n", argv[1]);
		free (byte_code);
		fclose (input);
		if (output != stdout) fclose (output);
		return 1;
	}

	while (pc < byte_code_len) {
		cmd = byte_code[pc++];
		imm = (cmd & IMM);
		reg = (cmd & REG);
		mem = (cmd & MEM);

		switch (cmd & CMD) {
			case CMD_PUSH:
				if (imm) {
					arg = *(int *)(byte_code + pc);
					pc += sizeof (int);
				}
				if (reg) {
					reg_num = byte_code[pc++];
					reg_name = get_reg_name (reg_num);
					if (!reg_name) {
						fprintf (stderr, "Disassebler: wrong register #%d\n", reg_num);
						free (byte_code);
						fclose (input);
						if (output != stdout) fclose (output);
						return 1;
					}
				}
				fprintf (output, "push ");
				if (mem)
					fprintf (output, "[");
				if (reg)
					fprintf (output, "%s", reg_name);
				if (reg && imm)
					fprintf (output, "+");
				if (imm)
					fprintf (output, "%d", arg);
				if (mem)
					fprintf (output, "]");
				fprintf (output, "\n");
				break;
			case CMD_POP:
				if (imm) {
					arg = *(int *)(byte_code + pc);
					pc += sizeof (int);
				}
				if (reg) {
					reg_num = byte_code[pc++];
					reg_name = get_reg_name (reg_num);
					if (!reg_name) {
						fprintf (stderr, "Disassebler: wrong register #%d\n", reg_num);
						free (byte_code);
						fclose (input);
						if (output != stdout) fclose (output);
						return 1;
					}
				}
				fprintf (output, "pop ");
				if (mem)
					fprintf (output, "[");
				if (reg)
					fprintf (output, "%s", reg_name);
				if (reg && imm)
					fprintf (output, "+");
				if (imm)
					fprintf (output, "%d", arg);
				if (mem)
					fprintf (output, "]");
				fprintf (output, "\n");
				break;
			case CMD_ADD:
				fprintf (output, "%s\n", "add");
				break;
			case CMD_SUB:
				fprintf (output, "%s\n", "sub");
				break;
			case CMD_MUL:
				fprintf (output, "%s\n", "mul");
				break;
			case CMD_DIV:
				fprintf (output, "%s\n", "div");
				break;
			case CMD_IN:
				fprintf (output, "%s\n", "in");
				break;
			case CMD_OUT:
				fprintf (output, "%s\n", "out");
				break;
			case CMD_HLT:
				fprintf (output, "%s\n", "hlt");
				break;
			case CMD_JMP:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				fprintf (output, "%s %d\n", "jmp", arg);
				break;
			case CMD_JA:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				fprintf (output, "%s %d\n", "ja", arg);
				break;
			case CMD_JAE:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				fprintf (output, "%s %d\n", "jae", arg);
				break;
			case CMD_JB:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				fprintf (output, "%s %d\n", "jb", arg);
				break;
			case CMD_JBE:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				fprintf (output, "%s %d\n", "jbe", arg);
				break;
			case CMD_JE:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				fprintf (output, "%s %d\n", "je", arg);
				break;
			case CMD_JNE:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				fprintf (output, "%s %d\n", "jne", arg);
				break;
			case CMD_CALL:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				fprintf (output, "%s %d\n", "call", arg);
				break;
			case CMD_RET:
				fprintf (output, "%s\n", "ret");
				break;
			default:
				fprintf (stderr, "Disassebler: unknown command: %d\n", cmd);
				free (byte_code);
				fclose (input);
				if (output != stdout) fclose (output);
				return 1;
				break;
		}
	}

	free (byte_code);
	fclose (input);
	if (output != stdout) fclose (output);
	return 0;
}

