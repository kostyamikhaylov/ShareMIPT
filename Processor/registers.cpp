#include "processor.h"

char register_ax[] = "ax";
char register_bx[] = "bx";
char register_cx[] = "cx";
char register_dx[] = "dx";

char *get_reg_name (const char reg_num)
{
	if (reg_num == 0) return (char *) &register_ax;
	if (reg_num == 1) return (char *) &register_bx;
	if (reg_num == 2) return (char *) &register_cx;
	if (reg_num == 3) return (char *) &register_dx;
	return NULL;
}

