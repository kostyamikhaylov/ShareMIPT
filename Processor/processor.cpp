#include "processor.h"
#include "../Stack/stack.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define RAM_SIZE 1024
#define REGS_NUM 4

char ram[RAM_SIZE];
int regs[REGS_NUM];

int print_int (const void *ptr);

int main (int argc, char *argv[])
{
	FILE *input = NULL;
	enum error_type stack_error = OK;
	Stack stack = {};
	char cmd = 0;
	int arg = 0, op1 = 0, op2 = 0;
	char *byte_code = NULL;
	size_t byte_code_len = 0, pc = 0;
	struct stat st = {};
	int sign_and_ver_len = 0;
	bool imm = false, reg = false, mem = false;


	if (argc != 2) {
		fprintf (stderr, "Usage: %s filename\n", argv[0]);
		return 1;
	}

	input = fopen (argv[1], "r");
	if (!input) {
		fprintf (stderr, "Can't open file %s\n", argv[1]);
		return 1;
	}

	if (stat (argv[1], &st) < 0) {
		fprintf (stderr, "Stat syscall failed\n");
		fclose (input);
		return 1;
	}

	byte_code_len = (size_t) st.st_size;

	if ((sign_and_ver_len = check_sign_and_ver (input)) < 0) {
		fprintf (stderr, "Input file type verification failed\n");
		fclose (input);
		return 1;
	}

	byte_code_len -= (size_t) sign_and_ver_len;

	byte_code = (char *) calloc (byte_code_len, sizeof (char));
	if (!byte_code) {
		fprintf (stderr, "Can't allocate memory\n");
		fclose (input);
		return 1;
	}

	if (fread (byte_code, sizeof (char), byte_code_len, input) < byte_code_len) {
		fprintf (stderr, "Failed to read byte_code from %s\n", argv[1]);
		free (byte_code);
		fclose (input);
		return 1;
	}

	stack_error = stack_ctor (&stack, sizeof (int), "int", print_int);
	if (stack_error != OK) {
		fprintf (stderr, "Stack creator returned code %d\n", stack_error);
		free (byte_code);
		fclose (input);
		return 1;
	}

	while (pc < byte_code_len) {
		cmd = byte_code[pc++];
		imm = (cmd & IMM);
		reg = (cmd & REG);
		mem = (cmd & MEM);

		switch (cmd & CMD) {
			case CMD_PUSH:
				arg = 0;
				if (!imm && !reg) {
					fprintf (stderr, "Processor: push format error %d\n", cmd);
					break;
				}
				if (imm) {
					arg = *(int *)(byte_code + pc);
					pc += sizeof (int);
				}
				if (reg)
					arg += regs[(size_t) byte_code[pc++]];
				if (mem)
					arg = ram[arg];
				stack_push (&stack, (void *) &arg);
				break;
			case CMD_POP:
				arg = 0;
				if ((imm && !mem) ||
				    (mem && !imm && !reg)) {
					fprintf (stderr, "Processor: pop format error %d\n", cmd);
					break;
				} else if (mem) {
					if (imm) {
						arg = *(int *)(byte_code + pc);
						pc += sizeof (int);
					}
					if (reg)
						arg += regs[(size_t) byte_code[pc++] - 1];
					if (arg < 0 || arg >= RAM_SIZE)
					{
						fprintf (stderr, "Processor: segmantation fault, can't pop to address %d\n", arg);
						break;
					}
					stack_pop (&stack, (void *) &ram[arg]);
				} else if (reg && !imm && !mem) {

					stack_pop (&stack, (void *) (regs + byte_code[pc++]));
				} else {
					stack_pop (&stack, (void *) &arg);
					printf ("Stack returned %d\n", arg);
				}
				break;
			case CMD_ADD:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				arg = op1 + op2;
				stack_push (&stack, (void *) &arg);
				break;
			case CMD_SUB:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				arg = op1 - op2;
				stack_push (&stack, (void *) &arg);
				break;
			case CMD_MUL:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				arg = op1 * op2;
				stack_push (&stack, (void *) &arg);
				break;
			case CMD_DIV:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				if (op2 != 0) {
					arg = op1 / op2;
					stack_push (&stack, (void *) &arg);
				} else {
					fprintf (stderr, "Processor: zero division\n");
					return 4;
				}
				break;
			case CMD_IN:
				if (scanf ("%d", &arg) > 0) {
					stack_push (&stack, (void *) &arg);
				} else {
					fprintf (stderr, "Processor: \"in\" operation error\n");
				}
				break;
			case CMD_OUT:
				stack_pop (&stack, (void *) &arg);
				printf ("out: %d\n", arg);
				break;
			case CMD_HLT:
				goto out;
				break;
			case CMD_JMP:
				arg = *(int *) (byte_code + pc);
				pc = (size_t) arg;
				break;
			case CMD_JA:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				if (op1 > op2) {
					arg = *(int *) (byte_code + pc);
					pc = (size_t) arg;
				} else {
					pc += sizeof (int);
				}
				break;
			case CMD_JAE:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				if (op1 >= op2) {
					arg = *(int *) (byte_code + pc);
					pc = (size_t) arg;
				} else {
					pc += sizeof (int);
				}
				break;
			case CMD_JB:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				if (op1 < op2) {
					arg = *(int *) (byte_code + pc);
					pc = (size_t) arg;
				} else {
					pc += sizeof (int);
				}
				break;
			case CMD_JBE:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				if (op1 <= op2) {
					arg = *(int *) (byte_code + pc);
					pc = (size_t) arg;
				} else {
					pc += sizeof (int);
				}
				break;
			case CMD_JE:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				if (op1 == op2) {
					arg = *(int *) (byte_code + pc);
					pc = (size_t) arg;
				} else {
					pc += sizeof (int);
				}
				break;
			case CMD_JNE:
				stack_pop (&stack, (void *) &op2);
				stack_pop (&stack, (void *) &op1);
				if (op1 != op2) {
					arg = *(int *) (byte_code + pc);
					pc = (size_t) arg;
				} else {
					pc += sizeof (int);
				}
				break;
			case CMD_CALL:
				arg = (int) (pc + 4);
				stack_push (&stack, (void *) &arg);
				arg = *(int *) (byte_code + pc);
				pc = (size_t) arg;
				break;
			case CMD_RET:
				stack_pop (&stack, (void *) &arg);
				pc = (size_t) arg;
				break;
			default:
				fprintf (stderr, "Processor: unknown command: %d\n", cmd);
				goto out;
				break;
		}
	}
out:

	stack_error = stack_dtor (&stack);
	if (stack_error != OK) {
		fprintf (stderr, "Stack destructor returned code %d\n", stack_error);
		fclose (input);
		free (byte_code);
		return 1;
	}

	fclose (input);
	free (byte_code);
	return 0;
}

int print_int (const void *ptr)
{
	assert (ptr);

	printf ("%d\n", *((const int *) ptr));
	return 0;
}

