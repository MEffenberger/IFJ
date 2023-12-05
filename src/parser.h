/**
 * @file parser.h
 *
 * IFJ23 compiler
 *
 * @brief Recursive descent parser for IFJ23 language including semantic analysis
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */


#ifndef IFJ_PARSER_H
#define IFJ_PARSER_H

#include "scanner.h"
#include "string_vector.h"

#define MAKE_CHILDREN_IN_FOREST(kw, name) forest_insert(active, kw, name , &active);
#define BACK_TO_PARENT_IN_FOREST active = active->parent;

// First 10 function definitions in global scope are built-in functions
#define AFTER_BUILTIN 10

extern token_t *current_token; // Pointer to the current token
extern int ifelse_cnt; // Count of if-else statements for unique labels 
extern data_type type_of_expr; // Store the type of the expression for expr_parser
extern instruction_list *inst_list; // List of instructions for codegen
extern bool return_expr; // Flag informing expr_parser that the expression is in a return statement

// Struct holding flags informing whether a built-in function was defined or not
typedef struct s_builtin_defs {
    bool readString_defined;
    bool readInt_defined;
    bool readDouble_defined;
    bool Int2Double_defined;
    bool Double2Int_defined;
    bool length_defined;
    bool substring_defined;
    bool ord_defined;
    bool chr_defined;
} builtin_defs;

//--------------------------------------------------
void print_debug(token_t *token, int mode, int cnt);
//--------------------------------------------------

/**
 * @brief Convert token's keyword to compatible data_type
 * 
 * @param token Given token
 * @return data_type Converted data_type
 */
data_type convert_dt(token_t* token);


/**
 * @brief Peek to the next token before calling get_next_token
 */
void peek();


/**
 * @brief Get the next token from the scanner
 * 
 * @return token_t* Next token
 */
token_t* get_next_token();

/**
 * @brief Insert built-in functions into the forest
 */
void insert_built_in_functions_into_forest();


/**
 * @brief <prog> -> EOF | <func_def> <prog> | <body> <prog>
 */
void prog();

/**
 * @brief <func_def> -> func id ( <params> ) <ret_type> { <local_body> }
 */
void func_def();

/**
 * @brief <local_body> -> <body> <local_body> | eps
 */
void local_body();

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
 * @brief <type> -> Int | Int? | Double | Double? | String | String? 
 */
void type();

/**
 * @brief <params_n> -> eps | , <params>
 */
void params_n();

/**
 * @brief <ret_type> -> eps | -> <type>
 */
void ret_type();

/**
 * @brief <ret> -> return <exp> | return
 */
void ret();

/**
 * @brief <body> -> eps | <var_def> | <condition> | <cycle> | <assign> | <func_call> | <ret>
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
 * @brief <arg> -> <exp> | id : <exp>
 */
void arg();

/**
 * @brief <args_n> -> eps | , <arg> <args_n>
 */
void args_n();

/**
 * @brief <condition> -> if <exp> { <local_body> } else { <local_body> } | if let id { <local_body> } else { <local_body> }
 */
void condition();

/**
 * @brief <cycle -> while <exp> { <local_body> }
 */
void cycle();


/**
 * @brief Main function of the parser
 * 
 * @return int Returns 0 if the program was parsed successfully, error_exit otherwise
 */
int parser_parse_please();


/**
 * @brief Renamer for unique names for variables to codegen
 * 
 * @param node Node to be renamed based on the nickname
 * @return char* New name of the node
 */
char *renamer(AVL_tree *node);


/**
 * @brief When a return statement is encountered, check if it is somewhere in a function
 * 
 * @param node Active node, where the return statement was encountered
 * @return forest_node* returns the function node if return statement is in a function, NULL otherwise
 */
forest_node* check_return_stmt(forest_node *node);


/**
 * @brief Validating function calls, since function definitions can be after function calls
 * 
 * @param global Global forest node
 */
void callee_validation(forest_node *global);


/**
 * @brief Validating return logic in all function definitions (if all paths return)
 * 
 * @param global Global forest node
 */
void return_logic_validation (forest_node *global);
void validate_forest(forest_node *func);
bool validate_forest_node(forest_node *node);

/**
 * @brief Generate instruction for variable definition in while cycle (needs to be moved)
 * 
 * @param type Instruction type
 * @param nickname Name for the instruction
 * @param cnt Counter for the instruction
 */
void vardef_outermost_while(inst_type type, char *nickname, int cnt);


/**
 * @brief Convert optional data type (Int?, Double?, String?) to compatible data type (Int, Double, String)
 * 
 * @param node Symbol, which data type is to be converted
 * @param mode 0 for optional to non-optional, 1 for non-optional to optional
 */
void convert_optional_data_type (AVL_tree *node, int mode);


/**
 * @brief Initialize built-in function flags
 * 
 * @param defs Struct with flags informing whether a built-in function was defined or not
 */
void builtin_defs_init(builtin_defs *defs);

/**
 * @brief Define built-in function (in codegen) if it has not been defined yet
 * 
 * @param built_in_defs Struct with flags informing whether a built-in function was defined or not
 */
void define_built_in_function(builtin_defs *built_in_defs);

#endif //IFJ_PARSER_H
