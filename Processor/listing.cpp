#include "processor.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>


static void print_listing (FILE *file,
			   const size_t pc,
			   const char cmd,
			   const bool mem,
			   const char *reg_num,
			   const int *arg_ptr,
			   const char *cmd_str);

int main (int argc, char *argv[])
{
	FILE *input = NULL, *output = NULL;
	int arg = 0;
	char cmd = 0, reg_num = 0;
	char *byte_code = NULL, *reg_name = NULL;
	size_t byte_code_len = 0, pc = 0, prev_pc = 0;
	struct stat st = {};
	int sign_and_ver_len = 0;
	bool imm = false, reg = false, mem = false;

	if (argc != 3) {
		fprintf (stderr, "Usage: %s input output\n", argv[0]);
		return 1;
	}

	input = fopen (argv[1], "r");
	if (!input) {
		fprintf (stderr, "Can't open file %s\n", argv[1]);
		return 2;
	}

	output = fopen (argv[2], "w");
	if (!output) {
		fprintf (stderr, "Can't open file %s\n", argv[2]);
		fclose (input);
		return 3;
	}

	if (stat (argv[1], &st) < 0) {
		fprintf (stderr, "Stat syscall failed\n");
		fclose (input);
		fclose (output);
		return 1;
	}

	byte_code_len = (size_t) st.st_size;

	if ((sign_and_ver_len = check_sign_and_ver (input)) < 0) {
		fprintf (stderr, "Input file type verification failed\n");
		fclose (input);
		fclose (output);
		return 1;
	}

	byte_code_len -= (size_t) sign_and_ver_len;

	byte_code = (char *) calloc (byte_code_len, sizeof (char));
	if (!byte_code) {
		fprintf (stderr, "Can't allocate memory\n");
		fclose (input);
		fclose (output);
		return 1;
	}

	if (fread (byte_code, sizeof (char), byte_code_len, input) < byte_code_len) {
		fprintf (stderr, "Failed to read byte_code from %s\n", argv[1]);
		free (byte_code);
		fclose (input);
		fclose (output);
		return 1;
	}

	while (pc < byte_code_len) {
		prev_pc = pc;
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
						fclose (output);
						return 1;
					}
				}
				print_listing (output, prev_pc, cmd, mem, (reg) ? &reg_num : NULL, (imm) ? &arg : NULL, "PUSH");
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
						fclose (output);
						return 1;
					}
				}
				print_listing (output, prev_pc, cmd, mem, (reg) ? &reg_num : NULL, (imm) ? &arg : NULL, "POP");
				break;
			case CMD_ADD:
				print_listing (output, prev_pc, cmd, false, NULL, NULL, "ADD");
				break;
			case CMD_SUB:
				print_listing (output, prev_pc, cmd, false, NULL, NULL, "SUB");
				break;
			case CMD_MUL:
				print_listing (output, prev_pc, cmd, false, NULL, NULL, "MUL");
				break;
			case CMD_DIV:
				print_listing (output, prev_pc, cmd, false, NULL, NULL, "DIV");
				break;
			case CMD_IN:
				print_listing (output, prev_pc, cmd, false, NULL, NULL, "IN");
				break;
			case CMD_OUT:
				print_listing (output, prev_pc, cmd, false, NULL, NULL, "OUT");
				break;
			case CMD_HLT:
				print_listing (output, prev_pc, cmd, false, NULL, NULL, "HLT");
				break;
			case CMD_JMP:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				print_listing (output, prev_pc, cmd, false, NULL, &arg, "JMP");
				break;
			case CMD_JA:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				print_listing (output, prev_pc, cmd, false, NULL, &arg, "JA");
				break;
			case CMD_JAE:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				print_listing (output, prev_pc, cmd, false, NULL, &arg, "JAE");
				break;
			case CMD_JB:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				print_listing (output, prev_pc, cmd, false, NULL, &arg, "JB");
				break;
			case CMD_JBE:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				print_listing (output, prev_pc, cmd, false, NULL, &arg, "JBE");
				break;
			case CMD_JE:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				print_listing (output, prev_pc, cmd, false, NULL, &arg, "JE");
				break;
			case CMD_JNE:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				print_listing (output, prev_pc, cmd, false, NULL, &arg, "JNE");
				break;
			case CMD_CALL:
				arg = *(int *)(byte_code + pc);
				pc += sizeof (int);
				print_listing (output, prev_pc, cmd, false, NULL, &arg, "CALL");
				break;
			case CMD_RET:
				print_listing (output, prev_pc, cmd, false, NULL, NULL, "RET");
				break;
			default:
				fprintf (stderr, "Listing: unknown command: %d\n", cmd);
				free (byte_code);
				fclose (input);
				fclose (output);
				return 1;
				break;
		}
	}

	free (byte_code);
	fclose (input);
	fclose (output);
	return 0;
}

static void print_listing (FILE *file,
			   const size_t pc,
			   const char cmd,
			   const bool mem,
			   const char *reg_num,
			   const int *arg_ptr,
			   const char *cmd_str)
{
	char *reg_name = NULL;
	char arg_print_arr[12];
	memset (arg_print_arr, ' ', 12);
	arg_print_arr[2] = '\0';
	arg_print_arr[5] = '\0';
	arg_print_arr[8] = '\0';
	arg_print_arr[11] = '\0';
	char reg_print_arr[3];
	memset (reg_print_arr, ' ', 3);
	reg_print_arr[2] = '\0';
	char arg_print_arr_decimal[20];
	memset (arg_print_arr_decimal, '\0', 20);
	char empty_string[1] = "";

	if (arg_ptr) {
		sprintf (arg_print_arr + 0, "%02x",
				(((unsigned int) (*arg_ptr)) >> 0) & 0xff);
		sprintf (arg_print_arr + 3, "%02x",
				(((unsigned int) (*arg_ptr)) >> 8) & 0xff);
		sprintf (arg_print_arr + 6, "%02x",
				(((unsigned int) (*arg_ptr)) >> 16) & 0xff);
		sprintf (arg_print_arr + 9, "%02x",
				(((unsigned int) (*arg_ptr)) >> 24) & 0xff);
		sprintf (arg_print_arr_decimal, "%d", *arg_ptr);
	}
	if (reg_num) {
		sprintf (reg_print_arr, "%02x", (unsigned int) (*reg_num));
	}

	if (!cmd_str) {
		fprintf (stderr, "print_listing () error: cmd_str parameter is NULL\n");
		return ;
	}

	if (!file) {
		fprintf (stderr, "print_listing () error: file was not opened\n");
		return ;
	}

	if (reg_num) {
		reg_name = get_reg_name (*reg_num);
		if (!reg_name) {
			fprintf (stderr, "print_listing (): wrong register #%d\n", *reg_num);
			return ;
		}
	} else
		reg_name = empty_string;

	fprintf (file, "%04lx  %02x  %c %s %s %s %s   %s %c   %s %s%s%s%s%s\n",
		pc,
		(unsigned char) cmd,
		(mem) ? '[' : ' ',
		arg_print_arr + 0,
		arg_print_arr + 3,
		arg_print_arr + 6,
		arg_print_arr + 9,
		reg_print_arr,
		(mem) ? ']' : ' ',
		cmd_str,
		(mem) ? "[" : "",
		reg_name,
		(reg_num && arg_ptr) ? "+" : empty_string,
		(arg_ptr) ? arg_print_arr_decimal : empty_string,
		(mem) ? "]" : "");

	return ;
}
