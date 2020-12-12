#ifndef GETTERS_H
#define GETTERS_H

#include "../register_machine/reg_m.h"
#include "../symbol_table/symbol_table.h"

/**
 * 
 * Get current symbol_table
 * 
 * RETURN: pointer to symbol_table used in compiling time
*/
symbol_table * get_symbol_table();

/**
 * 
 * Get current reg_set
 * 
 * RETURN: pointer to reg_set used in compiling time
*/
reg_set * get_reg_set();

#endif
