#ifndef STD_OPER_H
#define STD_OPER_H

#include "../../register_machine/reg_m.h"
#include <stdio.h>

/**
 * 
 * Load variable to one of registers
 *
 * RETURN: reg_allocator with loaded variable 
*/
reg_allocator oper_load_variable(addr_t addr, FILE *file);

#endif
