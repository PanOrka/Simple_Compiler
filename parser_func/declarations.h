#ifndef DECLARATIONS_H
#define DECLARATIONS_H

/**
 * 
 * Declare new variable of size = 1 in symbol_table
 * 
 * ARGUMENTS: identificator of symbol
*/
void declare_var(char *id);

/**
 * 
 * Declare new array of size end_idx - start_idx + 1
 * 
 * ARGUMENTS: identificator of symbol, start_idx, end_idx
*/
void declare_array(char *id, int64_t start_idx, int64_t end_idx);

#endif
