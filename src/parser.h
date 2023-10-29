/**
 * @file error.h
 *
 * IFJ23 compiler
 *
 * @brief Error handling header file
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */

#ifndef IFJ_PARSER_H
#define IFJ_PARSER_H

#include "error.h"
#include "scanner.h"
#include "symtable.h"
#include "forest.h"


#define MAKE_CHILDREN_IN_FOREST() forest_insert(active, convert_kw(current_token->value.keyword), get_name(current_token->value.keyword), &active);


extern token_t *current_token; // Pointer to the current token
extern token_t *token_buffer[2]; // Buffer for tokens
extern int token_buffer_cnt; // Index of the last token in the buffer

f_keyword_t convert_kw(keyword_t kw);
char *get_name(keyword_t kw);
void back_to_parent_in_forest();
void peek();
token_t* get_next_token();

void prog();
void func_def();
void body();
void type();
void params();
void params_n();
void par_name();
void par_id();
void ret_type();
void func_body();
void ret();
void func_call();
void args();
void args_n();
void arg();
void var_def();
void opt_var_def();
void assign();
void assignment();
void condition();
void cycle();

int parser_parse_please ();

#endif //IFJ_PARSER_H
