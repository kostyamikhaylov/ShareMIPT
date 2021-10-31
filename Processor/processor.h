#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>

#define IMM 0x20
#define REG 0x40
#define MEM 0x80
#define CMD 0x1F

enum commands
{
	CMD_HLT,
	CMD_PUSH,
	CMD_POP,
	CMD_ADD,
	CMD_SUB,
	CMD_MUL,
	CMD_DIV,
	CMD_IN,
	CMD_OUT,
	CMD_JMP,
	CMD_JA,
	CMD_JAE,
	CMD_JB,
	CMD_JBE,
	CMD_JE,
	CMD_JNE,
	CMD_CALL,
	CMD_RET
};

char *get_reg_name (const char reg_num);

int write_sign_and_ver (FILE * file);
int check_sign_and_ver (FILE * file);

#endif // PROCESSOR_H
