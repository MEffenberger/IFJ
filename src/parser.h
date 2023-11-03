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
#include "queue.h"

#define MAKE_CHILDREN_IN_FOREST(kw, name) forest_insert(active, kw, name , &active);
#define BACK_TO_PARENT_IN_FOREST active = active->parent;



/**
 * @brief info message
 * 
 * @param token Current token
 * @param mode 1 after get_next_token, 2 after entering rule
 * @param cnt Count of get_next_token calls 
 */
void print_debug(token_t *token, int mode, int cnt);

extern token_t *current_token; // Pointer to the current token

/**
 * @brief Converts token keyword to a forest related keyword
 * 
 * @param kw Token's keyword
 * @return f_keyword_t Forest keyword
 */
f_keyword_t convert_kw(keyword_t kw);

/**
 * @brief Get the name for the forest node (scope name)
 * 
 * @param kw Token's keyword
 * @return char* Name for the scope
 */
char *get_name(keyword_t kw);


void peek();
token_t* get_next_token();

/**
 * @brief <prog> -> EOF | <func_def> <prog> | <body> <prog>
 */
void prog();

/**
 * @brief <func_def> -> func id ( <params> ) <ret_type> { <func_body> }
 */
void func_def();

/**
 * @brief <params> -> eps | <par_name> <par_id> : <type> <params_n>
 */
void params();

/**
 * @brief <par_name> -> _ | id
 */
void par_name();

/**
 * @brief <par_id> -> _ | id
 */
void par_id();

/**
 * @brief <type> -> Int | Int? | Double | Double? | String | String? | nil
 */
void type();

/**
 * @brief <params_n> -> eps | , <params>
 */
void params_n();

/**
 * @brief ret_type -> eps | -> <type>
 */
void ret_type();

/**
 * @brief <func_body> -> <body> <ret> <body>
 */
void func_body();

/**
 * @brief <body> -> eps | <var_def> | <condition> | <cycle> | <assign> | <func_call>
 */
void body();

/**
 * @brief <var_def> -> let id <opt_var_def> | var id <opt_var_def>
 */
void var_def();

/**
 * @brief <opt_var_def> -> : <type> | <assign> | : <type> <assign>
 */
void opt_var_def();

/**
 * @brief <assign> -> = <exp> | = <func_call>
 */
void assign();

/**
 * @brief <func_call> -> id ( <args> )
 */
void func_call();

/**
 * @brief <args> -> eps | <arg> <args_n>
 */
void args();

/**
 * @brief <arg> -> exp | id : exp
 */
void arg();

/**
 * @brief <args_n> -> eps | , <arg> <args_n>
 */
void args_n();

/**
 * @brief <condition> -> if <exp> { <body> } else { <body> } | if let id { <body> } else { <body> }
 */
void condition();

/**
 * @brief <ret> -> return <exp> | eps
 */
void ret();

/**
 * @brief <cycle -> while <exp> { <body> }
 */
void cycle();


int parser_parse_please ();

#endif //IFJ_PARSER_H
