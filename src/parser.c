/**
 * @file parser.c
 *
 * IFJ23 compiler
 *
 * @brief Recursive descent parser for IFJ23 language including semantic analysis
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */

#include "callee.h"
#include "cnt_stack.h"
#include "codegen.h"
#include "error.h"
#include "expression_parser.h"
#include "forest.h"
#include "parser.h"
#include "queue.h"
#include "scanner.h"
#include "string_vector.h"
#include "symtable.h"
#include "token_stack.h"
#include <string.h>

forest_node *active = NULL; // Pointer to the active node in the forest
token_t *current_token = NULL; // Pointer to the current token
token_t *token_buffer = NULL; // Buffer for tokens
queue_t *queue = NULL; // Queue for the expression parser
instruction_list *inst_list = NULL; // List of instructions for codegen
cnt_stack_t *cnt_stack = NULL; // Stack for appropriate counting of if-else statements
callee_list_t *callee_list = NULL; // List of function calls and their parameters, types, returns
callee_list_t *callee_list_first = NULL; // Pointer to the first node of callee_list, needed for main validation
var_type letvar = 0; // It is changed in var_def() to either LET or VAR
data_type type_of_expr = UNKNOWN; // For expression parser to return the data type of expression
data_type type_of_assignee = UNKNOWN; // The type of variable being assigned to
bool is_initialized = false; // For var_def() to know whether the variable is initialized or not
int debug_cnt = 1; // For debugging purposes
int ifelse_cnt = 0; // For counting if-else statements
int while_cnt = 0; // For counting while statements
char node_name[20] = {0}; // For naming nodes in forest
char *var_name = NULL; // To find the data type of variable for expression parser
int param_order = 0; // For counting parameters of function
bool vardef_assign = false; // For assign() to know where to get info about the variable (from queue or from symtable)
bool function_write = false; // For parser to know that the function being handled is write() and needs special treatment
bool return_expr = false; // For expression parser to know that the expression is in return statement
builtin_defs *built_in_defs = NULL; // Flags for defining built-in functions in codegen, so they are not defined multiple times
extern FILE *file;



//Conversion between enums of lexer and parser
data_type convert_dt(token_t* token) {
    if (token->type == TOKEN_KEYWORD) {
        switch (token->value.keyword) {
            case KW_INT:
                return INT;
            case KW_DOUBLE:
                return DOUBLE;
            case KW_STRING:
                return STRING;
            case KW_NIL:
                return NIL;
            default:
                return -1;
        }
    }
    else if (token->type == TOKEN_KEYWORD_QM) {
        switch (token->value.keyword) {
            case KW_INT:
                return INT_QM;
            case KW_DOUBLE:
                return DOUBLE_QM;
            case KW_STRING:
                return STRING_QM;
            default:
                return -1;
        }
    }
    else if (token->type == TOKEN_LEFT_BRACKET) {
        return VOID; // void function (absence of return type)
    }
    else {
        return -1; 
    }
}

// Needed in decision procedure to rightfully determine the next path in recursive descent parser
void peek() {
    token_t *token = get_me_token();
    
    // Mechanism for detecting EOLs
    bool eol = false;
    while (token->type == TOKEN_EOL) {
        token = get_me_token();
        eol = true;
    }
    if (eol) {
        token->prev_was_eol = true;
    }

    token_buffer = token;
}

// Function which loads in the next token from scanner
token_t* get_next_token() {
    if (token_buffer == NULL) {

        token_t *token = get_me_token();

        // Mechanism for detecting EOLs
        bool eol = false;
        while (token->type == TOKEN_EOL) {
            token = get_me_token();
            eol = true;
        }
        if (eol) {
            token->prev_was_eol = true;
        }

        return token;
    } 
    else {
        token_t* tmp = token_buffer;
        token_buffer = NULL;
        return tmp;
    }
}

// Function Called in the beginning, at all times the built-ins are recognized by the inner representation
void insert_built_in_functions_into_forest() {
    // func readString() -> String?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readString");
    sym_data *readString = set_data_func(STRING_QM);
    symtable_insert(&active->symtable, "readString", readString);
    BACK_TO_PARENT_IN_FOREST;

    // func readInt() -> Int?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readInt");
    sym_data *readInt = set_data_func(INT_QM);
    symtable_insert(&active->symtable, "readInt", readInt);
    BACK_TO_PARENT_IN_FOREST;

    // func readDouble() -> Double?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readDouble");
    sym_data *readDouble = set_data_func(DOUBLE_QM);
    symtable_insert(&active->symtable, "readDouble", readDouble);
    BACK_TO_PARENT_IN_FOREST;

    // func write(term_1, term_2, ..., term_n)
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "write");
    sym_data *write = set_data_func(VOID);
    symtable_insert(&active->symtable, "write", write);
    BACK_TO_PARENT_IN_FOREST;

    // func Int2Double(_ term : Int) -> Double
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "Int2Double");
    sym_data *Int2Double = set_data_func(DOUBLE);
    symtable_insert(&active->symtable, "Int2Double", Int2Double);
    sym_data *Int2Double_param_data = set_data_param(INT, "_", 1);
    symtable_insert(&active->symtable, "term", Int2Double_param_data);
    active->param_cnt = 1;
    BACK_TO_PARENT_IN_FOREST;

    // func Double2Int(_ term : Double) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "Double2Int");
    sym_data *Double2Int = set_data_func(INT);
    symtable_insert(&active->symtable, "Double2Int", Double2Int);
    sym_data *Double2Int_param_data = set_data_param(DOUBLE, "_", 1);
    symtable_insert(&active->symtable, "term", Double2Int_param_data);
    active->param_cnt = 1;
    BACK_TO_PARENT_IN_FOREST;

    // func length(_ s : String) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "length");
    sym_data *length = set_data_func(INT);
    symtable_insert(&active->symtable, "length", length);
    sym_data *length_param_data = set_data_param(STRING, "_", 1);
    symtable_insert(&active->symtable, "s", length_param_data);
    active->param_cnt = 1;
    BACK_TO_PARENT_IN_FOREST;

    // func substring(of s : String, startingAt i : Int, endingBefore j : Int) -> String?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "substring");
    sym_data *substring = set_data_func(STRING_QM);
    symtable_insert(&active->symtable, "substring", substring);
    sym_data *substring_param_data1 = set_data_param(STRING, "of", 1);
    symtable_insert(&active->symtable, "s", substring_param_data1);
    sym_data *substring_param_data2 = set_data_param(INT, "startingAt", 2);
    symtable_insert(&active->symtable, "i", substring_param_data2);
    sym_data *substring_param_data3 = set_data_param(INT, "endingBefore", 3);
    symtable_insert(&active->symtable, "j", substring_param_data3);
    active->param_cnt = 3;
    BACK_TO_PARENT_IN_FOREST;

    // func ord(_ c : String) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "ord");
    sym_data *ord = set_data_func(INT);
    symtable_insert(&active->symtable, "ord", ord);
    sym_data *ord_param_data = set_data_param(STRING, "_", 1);
    symtable_insert(&active->symtable, "c", ord_param_data);
    active->param_cnt = 1;
    BACK_TO_PARENT_IN_FOREST;

    // func chr(_ i : Int) -> String
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "chr");
    sym_data *chr = set_data_func(STRING);
    symtable_insert(&active->symtable, "chr", chr);
    sym_data *chr_param_data = set_data_param(INT, "_", 1);
    symtable_insert(&active->symtable, "i", chr_param_data);
    active->param_cnt = 1;
    BACK_TO_PARENT_IN_FOREST;
}


// First function in the recursive descent parser, always called
void prog() {
    // <prog> -> EOF | <func_def> <prog> | <body> <prog>

    if (current_token->type == TOKEN_EOF) {
        return;
    }
    else if (current_token->value.keyword == KW_FUNC) {
        func_def();
        prog();
    }
    else {
        body();
        prog();
    }
}

// Defining a function
void func_def() {
    // <func_def> -> func id ( <params> ) <ret_type> { <body> }

    current_token = get_next_token();

    if (current_token->type == TOKEN_ID) {

        // Check if the function is already defined
        forest_node *func_check = forest_search_function(active, current_token->value.vector->array);
        if (func_check != NULL) {
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Function is already defined and cannot be redefined");
        }

        AVL_tree *sym_check = symtable_search(active->symtable, current_token->value.vector->array);
        if (sym_check != NULL) {
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Cannot use the same name for function as was used for variable");
        }

        // The function is now recognized by the IR of the compiler, it is now set to be active
        MAKE_CHILDREN_IN_FOREST(W_FUNCTION, current_token->value.vector->array);
        
        current_token = get_next_token();

        if (current_token->type == TOKEN_LPAR) {
            current_token = get_next_token();

            params();

            // Set the number of parameters of the function
            active->param_cnt = param_order;
            param_order = 0;

            // CODEGEN
            instruction *inst = inst_init(FUNC_DEF, 'G', active->name, active->param_cnt, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);

            if (current_token->type == TOKEN_RPAR) {
                current_token = get_next_token();

                ret_type();

                // Insert function with its return type to symtable
                sym_data *func_data = set_data_func(convert_dt(queue->first->token));
                symtable_insert(&active->symtable, active->name, func_data);
                queue_dispose(queue);

                if (current_token->type == TOKEN_LEFT_BRACKET) {
                    // Get the next token, body expects first token of body
                    current_token = get_next_token();

                    // The IR needs to separate the function header from the function body because of possible overlapping
                    MAKE_CHILDREN_IN_FOREST(W_FUNCTION_BODY, "body");

                    local_body();

                    BACK_TO_PARENT_IN_FOREST;

                    if (current_token->type == TOKEN_RIGHT_BRACKET) {
                        // Func_def ends, go back to parent in forest
                        current_token = get_next_token();

                        // CODEGEN
                        instruction *inst = inst_init(FUNC_DEF_END, 'G', active->name, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        BACK_TO_PARENT_IN_FOREST;

                        return;
                    }
                    else {
                        error_exit(ERROR_SYN, "PARSER", "Missing right bracket in function definition");
                    }
                }
                else {
                    error_exit(ERROR_SYN, "PARSER", "Missing left bracket in function definition");
                }
            }
            else {
                error_exit(ERROR_SYN, "PARSER", "Missing right paranthesis in function definition");
            }
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Missing left paranthesis in function definition");
        }
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing identifier of funciton");
    }
}

// Need to separate local_body from body because of different grammar mechanisms
void local_body() {
    // <local_body> -> <body> <local_body> | eps

    while (current_token->type != TOKEN_RIGHT_BRACKET) {
        body();
    }
}

// Function for parameters handling
void params() {
    // <params> -> eps | <par_name> <par_id> : <type> <params_n>

    // Void function - encountering a ')' right after '('
    if (current_token->type == TOKEN_RPAR) {
        return;
    }

    // First load the name of the parameter
    par_name();

    current_token = get_next_token();

    // Then load the id of the parameter
    par_id();

    current_token = get_next_token();

    if (current_token->type == TOKEN_COLON) {
        current_token = get_next_token();

        // Then load the type of the parameter if the colon is present
        type();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing colon in function's parameter");
    }

    // Name of the parameter has to differ from the identifier of the parameter (except for case when the name and id is _)
    if (strcmp(queue->first->token->value.vector->array, queue->first->next->token->value.vector->array) == 0 && 
        strcmp(queue->first->token->value.vector->array, "_") != 0) {
        error_exit(ERROR_SEM_OTHER, "PARSER", "Parameter's name has to differ from its identifier");
    }
    
    // Insert parameter to function's symtable
    sym_data *param_data = set_data_param(convert_dt(current_token), queue->first->token->value.vector->array, ++param_order);
    symtable_insert(&active->symtable, queue->first->next->token->value.vector->array, param_data);
    queue_dispose(queue);

    current_token = get_next_token();

    // If there is a comma, there are more parameters to be loaded, otherwise the function definition ends or there is a syntax error
    if (current_token->type == TOKEN_RPAR) {
        return;
    }
    else if (current_token->type == TOKEN_COMMA) {
        params_n();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function's parameter");
    }
}

// Function for handling the name of the parameter
void par_name() {
    // <par_name> -> _ | id

    // It has to be ID or _ otherwise it is a syntax error
    if (current_token->type == TOKEN_UNDERSCORE || current_token->type == TOKEN_ID) {
        // Store parameter's name
        queue_push(queue, current_token);

        return;
    }
    else { // Not _ or id
        error_exit(ERROR_SEM_TYPE, "PARSER", "Parameter's name is not _ or id as expected");
    }
}

// Function for handling the id of the parameter
void par_id() {
    // <par_id> -> _ | id

    // It has to be ID or _ otherwise it is a syntax error
    if (current_token->type == TOKEN_UNDERSCORE || current_token->type == TOKEN_ID) {
        // Store parameter's id
        // Token_push(token_stack, current_token);
        queue_push(queue, current_token);

        return;
    }
    else { // Not _ or id
        error_exit(ERROR_SEM_TYPE, "PARSER", "Parameter's id is not _ or id as expected");
    }
}

// Type of variable/function/parameter
void type() {
    // <type> -> Int | Int? | Double | Double? | String | String?

    if (current_token->type == TOKEN_KEYWORD || current_token->type == TOKEN_KEYWORD_QM) {
        if (current_token->value.keyword == KW_INT || current_token->value.keyword == KW_DOUBLE || current_token->value.keyword == KW_STRING) {
            // store type
            type_of_assignee = convert_dt(current_token);
            queue_push(queue, current_token);
            return;
        }
        else {
            error_exit(ERROR_SEM_TYPE, "PARSER", "Error type not Int(?), Double(?), String(?) or nil");
        }
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing type");
    }

}

// Loading next parameter
void params_n() {
    // <params_n> -> eps | , <params>
    
    peek();
    if (token_buffer->type != TOKEN_ID && token_buffer->type != TOKEN_UNDERSCORE) {
        error_exit(ERROR_SYN, "PARSER", "Missing name of function's parameter");
    }
    else {
        current_token = get_next_token();
        params();
    }
}

// Function to properly handle the returns in functions
void ret_type() {
    // ret_type -> eps | -> <type>

    if (current_token->type == TOKEN_RET_TYPE) {
        current_token = get_next_token();

        type();

        current_token = get_next_token();

        return;
    }
    else if (current_token->type == TOKEN_LEFT_BRACKET) { // void function
        queue_push(queue, current_token); 
        return;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function's return type");
    }
}

// Function to handle return statements
void ret() {
    // <ret> -> return <exp> | return

    // Checks whether the return is in the function as it should be, error otherwise
    forest_node *tmp = check_return_stmt(active);
    
    // Set has_return flag to true, so the return logic knows this scope has return in it
    if (!active->has_return) {
        active->has_return = true;
    }
    else {
        error_exit(ERROR_SEM_EXPR_RET, "PARSER", "Function has multiple returns in one scope");
    }
    
    // If the function is void, there should be no expression after return
    sym_data *tmp_data = symtable_search(tmp->symtable, tmp->name)->data;


    if (tmp_data->return_type == VOID) { // Void function
        current_token = get_next_token();

        // CODEGEN
        instruction *inst = inst_init(FUNC_DEF_RETURN_VOID, 'G', tmp->name, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);

        return;
    }
    else { // Non-void function
        current_token = get_next_token();

        return_expr = true;
        call_expr_parser(tmp_data->return_type);
        return_expr = false;

        // CODEGEN
        instruction *inst = inst_init(FUNC_DEF_RETURN, 'G', tmp->name, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }
}

// Body of the function/ global scope
void body() {
    // <body> -> eps | <var_def> | <condition> | <cycle> | <assign> | <func_call> | <ret>

    var_name = NULL;
    type_of_assignee = UNKNOWN;

    // Possible assignment to variable
    if (current_token->type == TOKEN_ID) {
        peek();
        if (token_buffer->type == TOKEN_EQ) {
            var_name = current_token->value.vector->array; // for case: id = <exp>
            AVL_tree *tmp = forest_search_symbol(active, var_name);

            // check if the id is in symtable, so the variable is declared
            if (tmp == NULL) {
                error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable is not declared");
            }
            if (tmp->data->var_type == LET && tmp->data->defined) {
                error_exit(ERROR_SEM_OTHER, "PARSER", "Unmodifiable variable cannot be redefined");
            }
            if (!(tmp->data->defined)) {
                tmp->data->defined = true;
            }
            vardef_assign = false;
            assign();

        }
        else if (token_buffer->type == TOKEN_LPAR) {
            // Function call without assigning, expecting void function
            func_call();
            callee_list = callee_list->next;
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
        }
    }
    // Handling of all the possible keywords
    else if (current_token->type == TOKEN_KEYWORD) {
        switch (current_token->value.keyword) {
            case KW_RETURN:
                ret();
                break;

            case KW_LET:
                letvar = LET;
                var_def();
                break;

            case KW_VAR:
                letvar = VAR;
                var_def();
                break;

            case KW_IF:
                condition();
                break;

            case KW_WHILE:
                cycle();                       
                break;

            // Write(term_1, term_2, ..., term_n)
            case KW_WRT:
                peek();
                if (token_buffer->type == TOKEN_LPAR) {
                    function_write = true;
                    func_call();
                    function_write = false;

                    // CODEGEN
                    instruction *inst = inst_init(WRITE, 'G', NULL, callee_list->callee->arg_count, 0, 0.0, NULL);
                    inst_list_insert_last(inst_list, inst);
                    callee_list = callee_list->next;
                    break;
                }
                else {
                    error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
                }

            case KW_RD_STR:
            case KW_RD_INT:
            case KW_RD_DBL:
            case KW_INT_2_DBL:
            case KW_DBL_2_INT:
            case KW_LENGHT:
            case KW_SUBSTR:
            case KW_ORD:
            case KW_CHR:
                define_built_in_function(built_in_defs);
                func_call();
                callee_list = callee_list->next;
                break;

            case KW_FUNC:
                error_exit(ERROR_SEM_OTHER, "PARSER", "Cannot define function inside another function");
                break;

            default:
                error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
                break;
        }
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
    }
}

// Defining a variable
void var_def() {
    // <var_def> -> let id <opt_var_def> | var id <opt_var_def>
   
    current_token = get_next_token();

    if (current_token->type == TOKEN_ID) {
        var_name = current_token->value.vector->array; // For a case: let/var id = <exp>
        
        // A case, where variable is declared with the same name as function already existing
        forest_node *func_check = forest_search_function(active, var_name);
        if (func_check != NULL) {
            error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable cannot have the same name as function");
        }

        // The node is in the current symtable, error is thrown as multiple declarations of the same name are not allowed
        AVL_tree *check = symtable_search(active->symtable, var_name);
        if (check != NULL && !(check->data->is_param)) { 
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Multiple declarations of the same name are not allowed");
        }

        queue_push(queue, current_token);
        opt_var_def();

        sym_data *var_data = NULL;

        // Insert variable to symtable
        if (queue->first->next == NULL) { // The data type is not specified, expression parser determined it
            if (type_of_expr == NIL) {
                error_exit(ERROR_SEM_DERIV, "PARSER", "Variable cannot derive its type from nil");
            }
            else if (type_of_expr == BOOL) {
                error_exit(ERROR_SEM_OTHER, "PARSER", "Variable cannot be of type bool");
            } 
            else {
                if (type_of_assignee != UNKNOWN) {
                    var_data = set_data_var(is_initialized, type_of_assignee, letvar);
                }
                else {
                    var_data = set_data_var(is_initialized, type_of_expr, letvar);
                }
            }
        }
        else { // The data type was specified, expression parser will handle it as there is expected data type
            // '= nil' does not go through the expression parses, has to be handled here
            if (type_of_expr == NIL) {
                if (convert_dt(queue->first->next->token) != INT_QM && 
                    convert_dt(queue->first->next->token) != DOUBLE_QM && 
                    convert_dt(queue->first->next->token) != STRING_QM)
                {
                    error_exit(ERROR_SEM_EXPR_TYPE, "PARSER", "Variable cannot be of type nil");
                }
            }
            var_data = set_data_var(is_initialized, convert_dt(queue->first->next->token), letvar);
        }        

        symtable_insert(&active->symtable, queue->first->token->value.vector->array, var_data);
        queue_dispose(queue);

        AVL_tree *symbol = symtable_search(active->symtable, var_name);
        symbol->nickname = active->node_cnt;
        char *nickname = renamer(symbol);

        // If the variable is defined in while loop, it has to be defined before the outermost while loop (in codegen)
        vardef_outermost_while(VAR_DEF, nickname, 0);

        if (symbol->data->defined) {
            if (type_of_expr == NIL) {
                // CODEGEN
                instruction *inst = inst_init(VAR_ASSIGN_NIL, active->frame, nickname, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
            }
            else {
                // CODEGEN
                instruction *inst = inst_init(VAR_ASSIGN, active->frame, nickname, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
            }
        }
        if (!symbol->data->defined && (symbol->data->data_type == INT_QM || symbol->data->data_type == DOUBLE_QM || symbol->data->data_type == STRING_QM)) {
            // CODEGEN
            instruction *inst = inst_init(IMPLICIT_NIL, active->frame, nickname, 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
        }

        return;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing identifier in variable definition");
    }
}

// Continues the variable definition if there is a colon or an assignment
void opt_var_def() {
    // <opt_var_def> -> : <type> | <assign> | : <type> <assign>
    
    peek();    
    if (token_buffer->type == TOKEN_COLON) {
        
        // Get TOKEN_COLON from buffer
        current_token = get_next_token();

        current_token = get_next_token();

        type();

        // Variable is declared
        is_initialized = false;

        peek();
        if (token_buffer->type == TOKEN_EQ) {
            vardef_assign = true;
            assign();
            // Variable is defined
            is_initialized = true;
        }
        else {
            // Let a : Int
            current_token = get_next_token();
        }
    }
    else if (token_buffer->type == TOKEN_EQ) {
        vardef_assign = true;
        assign();
        // Variable is defined
        is_initialized = true;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in variable definition");
    }
}

// Assigning to a variable
void assign() {
    // <assign> -> = <exp> | = <func_call>

    // Id is in var_name
    // Assigning to variable while defining it
    if (vardef_assign) {
        // Get TOKEN_EQ from buffer
        current_token = get_next_token();

        // Here: var id = | let id =

        current_token = get_next_token();

        // Looking for a function call
        if (current_token->type == TOKEN_ID) {
            peek();
            if (token_buffer->type == TOKEN_LPAR) {
                // Expecting user-defined function
                func_call();
                callee_list = callee_list->next;

            }
            else {
                // In queue->first->next should be the data type of the variable, if it's NULL, the data type is unknown and should be determined by expression
                if (queue->first->next == NULL) {
                    call_expr_parser(UNKNOWN); // In type_of_expr should be the data type of the expression
                }
                else {
                    call_expr_parser(convert_dt(queue->first->next->token));
                }
            }
        }
        else if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword != KW_NIL) {
            switch (current_token->value.keyword) {
                case KW_RD_STR:
                case KW_RD_INT:
                case KW_RD_DBL:
                case KW_INT_2_DBL:
                case KW_DBL_2_INT:
                case KW_LENGHT:
                case KW_SUBSTR:
                case KW_ORD:
                case KW_CHR:
                    define_built_in_function(built_in_defs);
                    break;
                
                default:
                    error_exit(ERROR_SYN, "PARSER", "Unexpected token in assignment to variable while defining it");
                    break;
            }
            func_call();
            callee_list = callee_list->next;
        }
        else {
            // In queue->first->next should be the data type of the variable, if it's NULL, the data type is unknown and should be determined by expression
            if (queue->first->next == NULL) {
                call_expr_parser(UNKNOWN); // In type_of_expr should be the data type of the expression
            }
            else {
                call_expr_parser(convert_dt(queue->first->next->token));
            }
        }    
    }
    else { // Assigning to already defined variable
        forest_node *scope = forest_search_scope(active, var_name);
        AVL_tree *symbol = symtable_search(scope->symtable, var_name);
        type_of_assignee = symbol->data->data_type;

        // Cannot assign to a parameter in function definition
        if (symbol->data->is_param) {
            error_exit(ERROR_SEM_OTHER, "PARSER", "Cannot assign to parameter");
        }

        
        // Get TOKEN_EQ from buffer
        current_token = get_next_token();

        // Here: id =
   
        current_token = get_next_token();

        // Looking for function call
        if (current_token->type == TOKEN_ID) {
            peek();
            if (token_buffer->type == TOKEN_LPAR) {
                // Expecting user-defined function
                func_call();
                
                instruction *inst = inst_init(VAR_ASSIGN, scope->frame, renamer(symbol), 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                callee_list = callee_list->next;
            }
            else { // Variable is already defined, so it's data_type is known
                call_expr_parser(symbol->data->data_type);

                // CODEGEN
                instruction *inst = inst_init(VAR_ASSIGN, scope->frame, renamer(symbol), 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
            }
        }
        else if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword != KW_NIL) {
            switch (current_token->value.keyword) {
                case KW_RD_STR:
                case KW_RD_INT:
                case KW_RD_DBL:
                case KW_INT_2_DBL:
                case KW_DBL_2_INT:
                case KW_LENGHT:
                case KW_SUBSTR:
                case KW_ORD:
                case KW_CHR:
                    define_built_in_function(built_in_defs);
                    break;

                default:
                    error_exit(ERROR_SYN, "PARSER", "Unexpected token in assignment to already defined variable");
                    break;
            }
            func_call();
        
            instruction *inst = inst_init(VAR_ASSIGN, scope->frame, renamer(symbol), 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);

            callee_list = callee_list->next;

        }
        else {
            call_expr_parser(symbol->data->data_type);

            // CODEGEN
            instruction *inst = inst_init(VAR_ASSIGN, active->frame, renamer(symbol), 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
        }
    }    
}

// Handling function calls
void func_call() {
    // <func_call> -> id ( <args> )

    // Store the function's name for later usage (for codegen)
    char *func_name = allocate_memory(sizeof(char) * strlen(current_token->value.vector->array) + 1);
    func_name = strcpy(func_name, current_token->value.vector->array);

    insert_callee_into_list(callee_list, func_name);

    if (var_name == NULL) {
        callee_list->callee->return_type = VOID;
    }
    else if (type_of_assignee == UNKNOWN) {
        // Find global node
        forest_node *global = active;
        while (global->parent != NULL) {
            global = global->parent;
        }
        forest_node *func = forest_search_function(global, func_name);
        if (func == NULL) {
            error_exit(ERROR_SEM_DERIV, "PARSER", "Function is not defined when assigning to variable while defining it, cannot derive its type");
        }
        AVL_tree *func_symbol = symtable_search(func->symtable, func_name);
        if (func_symbol->data->return_type == VOID) {
            error_exit(ERROR_SEM_EXPR_TYPE, "PARSER", "Void function call cannot be assigned to a variable");
        }
        type_of_assignee = func_symbol->data->return_type;
        type_of_expr = type_of_assignee; // Variable's type for symtable
        callee_list->callee->return_type = type_of_assignee; // For callee validation to work smoothly
    }
    else {
        callee_list->callee->return_type = type_of_assignee; 
    }

    if (!function_write) {
        // CODEGEN
        instruction *inst = inst_init(FUNC_CALL_START, 'G', NULL, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }

    // Get TOKEN_LPAR from buffer
    current_token = get_next_token();

    args();

    if (current_token->type == TOKEN_RPAR) {
        if (!function_write) {
            // CODEGEN
            instruction *func_call = inst_init(FUNC_CALL, 'G', func_name, 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, func_call);
            // CODEGEN
            instruction *retval = inst_init(FUNC_CALL_RETVAL, 'G', NULL, 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, retval);
        }
        current_token = get_next_token();

        return;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing right paranthesis in function call");
    }
}

// Arguments of function call
void args() {
    // <args> -> eps | <arg> <args_n>

    peek();
    if (token_buffer->type == TOKEN_RPAR) {

        // Get TOKEN_RPAR from buffer
        current_token = get_next_token();

        return;
    }
    else {
        // <args> -> <arg> <args_n>
        arg();

        if (current_token->type == TOKEN_RPAR) {
            return;
        }
        else if (current_token->type == TOKEN_COMMA) {
            args_n();
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Unexpected token in function call");
        }
    }
}

// Argument of function call being expression or id
void arg() {
    // <arg> -> exp | id : exp

    current_token = get_next_token();

    if (current_token->type == TOKEN_ID) {

        peek();
        if (token_buffer->type == TOKEN_COLON) {
            // <arg> -> id : exp
            insert_name_into_callee(callee_list->callee, current_token->value.vector->array);

            // Get TOKEN_COLON from buffer
            current_token = get_next_token();

            current_token = get_next_token();
        }
        else { // Calling without name
            insert_name_into_callee(callee_list->callee, "_");

        }
    }
    else if (current_token->type == TOKEN_UNDERSCORE) {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function call, underscore cannot be used as argument's name");
    }
    else { // Calling without name and the first token of expression is not id
        insert_name_into_callee(callee_list->callee, "_");
    }

    if (current_token->type == TOKEN_ID) {
        AVL_tree *symbol = forest_search_symbol(active, current_token->value.vector->array);
        if (symbol == NULL) {
            error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable in function call passed as argument is not declared");
        }
        else {
            if (symbol->data->is_param) {
                insert_bool_into_callee(callee_list->callee, true);
            }
            else if (function_write && (symbol->data->data_type == INT_QM || symbol->data->data_type == DOUBLE_QM || symbol->data->data_type == STRING_QM)) {
                // In built-in function write, when passing argument of optional type and uninitialized, it is implicitly set to nil and printing ""
                insert_bool_into_callee(callee_list->callee, true);
            }
            else {
                insert_bool_into_callee(callee_list->callee, symbol->data->defined);
            }
        }
    }
    else {
        insert_bool_into_callee(callee_list->callee, true);
    }

    call_expr_parser(UNKNOWN);
    insert_type_into_callee(callee_list->callee, type_of_expr);

    if (!function_write) {
        // CODEGEN
        instruction *inst = inst_init(ADD_ARG, 'G', NULL, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }
}

// There can be more than 0/1 argument to load in function call
void args_n() {
    // <args_n> -> eps | , <arg> <args_n>

    arg();

    if (current_token->type == TOKEN_RPAR) {
        return;
    }
    else if (current_token->type == TOKEN_COMMA) {
        args_n();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function call, missing right paranthesis or comma between arguments");
    }
}

// Function for handling if and if let statements
void condition() {
    // <condition> -> if <exp> { <local_body> } else { <local_body> } | if let id { <local_body> } else { <local_body> }

    int cnt = 0;

    cnt_push(cnt_stack); // Push ifelse_cnt to stack and increment
    sprintf(node_name, "if_%d", ifelse_cnt);
    char *node_name2 = (char*)allocate_memory(sizeof(char) * 20);
    strcpy(node_name2, node_name);

    MAKE_CHILDREN_IN_FOREST(W_IF, node_name2);
    active->cond_cnt = ifelse_cnt;

    bool if_let = false;
    AVL_tree *symbol_q = NULL;

    current_token = get_next_token();

    if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword == KW_LET) {
        // if let id { <body> } <else> { <body> }
        current_token = get_next_token();

        if (current_token->type == TOKEN_ID) {
            // Check if the id is in symtable, so the variable is declared
            AVL_tree *symbol = forest_search_symbol(active, current_token->value.vector->array);
            symbol_q = symbol; // For later usage (converting optional type to non-optional and back)
            if (symbol == NULL) {
                error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable is not declared");
            }
            else if (symbol->data->var_type == VAR) {
                error_exit(ERROR_SEM_OTHER, "PARSER", "Modifiable variable \"var\" cannot be used as follows: \"if let id\"");
            }
            else if (symbol->data->data_type != INT_QM && symbol->data->data_type != DOUBLE_QM && symbol->data->data_type != STRING_QM) {
                error_exit(ERROR_SEM_TYPE, "PARSER", "Initializer for conditional binding must have optional type");
            }
            else {
                if_let = true;

                // Get TOKEN_LEFT_BRACKET
                current_token = get_next_token();

                // CODEGEN
                instruction *inst1 = inst_init(IF_LABEL, active->frame, NULL, active->cond_cnt, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst1);
                vardef_outermost_while(IF_DEFVAR, renamer(symbol), active->cond_cnt);
                instruction *inst = inst_init(IF_LET, active->frame, renamer(symbol), active->cond_cnt, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
            }
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Missing identifier in condition using \"if let id\"");
        }
    }
    else {
        call_expr_parser(BOOL);

        // CODEGEN
        instruction *inst1 = inst_init(IF_LABEL, active->frame, NULL, active->cond_cnt, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst1);
        vardef_outermost_while(IF_DEFVAR, NULL, active->cond_cnt);
        instruction *inst = inst_init(IF, active->frame, NULL, active->cond_cnt, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }


    if (current_token->type == TOKEN_LEFT_BRACKET) {
        
        ifelse_cnt++;

        current_token = get_next_token();

        convert_optional_data_type(symbol_q, 1, if_let); // convert optional type to non-optional in case of if let
        // IF BODY
        local_body();

        convert_optional_data_type(symbol_q, 2, if_let); // convert non-optional type back to optional in case of if let

        if (current_token->type == TOKEN_RIGHT_BRACKET) {
            // Closing bracket of if statement, go back to parent in forest
            BACK_TO_PARENT_IN_FOREST;
            current_token = get_next_token();

            if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword == KW_ELSE) {

                cnt = cnt_top(cnt_stack); // Get ifelse_cnt from stack
                sprintf(node_name, "else_%d", cnt);
                char *node_name3 = (char*)allocate_memory(sizeof(char) * 20);
                strcpy(node_name3, node_name);
                MAKE_CHILDREN_IN_FOREST(W_ELSE, node_name3);
                active->cond_cnt = cnt;

                // CODEGEN
                instruction *inst = inst_init(ELSE, active->frame, NULL, active->cond_cnt, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                current_token = get_next_token();

                if (current_token->type == TOKEN_LEFT_BRACKET) {
                    current_token = get_next_token();

                    // ELSE BODY
                    local_body();

                    if (current_token->type == TOKEN_RIGHT_BRACKET) {
                        // Closing bracket of else statement, go back to parent in forest
                        current_token = get_next_token();

                        active->cond_cnt = cnt_top(cnt_stack); // get ifelse_cnt from stack
                        
                        // CODEGEN
                        instruction *inst = inst_init(IFELSE_END, active->frame, NULL, active->cond_cnt, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        cnt_pop(cnt_stack); // Pop ifelse_cnt from stack

                        BACK_TO_PARENT_IN_FOREST;

                        return;
                    }
                    else {
                        error_exit(ERROR_SYN, "PARSER", "Missing right bracket in else statement");
                    }
                }
                else {
                    error_exit(ERROR_SYN, "PARSER", "Missing left bracket in else statement");
                }
            }
            else {
                error_exit(ERROR_SYN, "PARSER", "Missing else statement");
            }
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Missing right bracket in if statement");
        }
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing left bracket in if statement");
    }
}


// Handling while statements
void cycle() {
    // <cycle -> while <exp> { <local_body> }

    sprintf(node_name, "while_%d", while_cnt);
    char *node_name1 = (char*)allocate_memory(sizeof(char) * 20);
    strcpy(node_name1, node_name);
  
    while_cnt++;
    // Variables declared in while loop have to be pushed outside the while loop in codegen
    forest_node *outermost_while = forest_search_while(active);
    MAKE_CHILDREN_IN_FOREST(W_WHILE, node_name1);

    // CODEGEN
    if (outermost_while == NULL) {
        // CODEGEN
        instruction *inst = inst_init(WHILE_COND_DEF, active->frame, node_name1, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }
    else {
        inst_list_search_while(inst_list, outermost_while->name); // Int_list->active is now set on the outermost while -> insert before it
        // CODEGEN
        instruction *inst = inst_init(WHILE_COND_DEF, active->frame, node_name1, 0, 0, 0.0, NULL);
        inst_list_insert_before(inst_list, inst);
    }

    // CODEGEN
    instruction *inst1 = inst_init(WHILE_START, 'G', node_name1, 0, 0, 0.0, NULL);
    inst_list_insert_last(inst_list, inst1);

    current_token = get_next_token();

    call_expr_parser(BOOL);

    // The while had to be parsed to be well documented by the codegen
    if (current_token->type == TOKEN_LEFT_BRACKET) {
       
        // CODEGEN
        instruction *inst = inst_init(WHILE_DO, active->frame, node_name1, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);

        current_token = get_next_token();

        local_body();

        if (current_token->type == TOKEN_RIGHT_BRACKET) {

            // CODEGEN
            instruction *inst = inst_init(WHILE_END, active->frame, node_name1, 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
            
            // Closing bracket of while statement, go back to parent in forest
            BACK_TO_PARENT_IN_FOREST;
            current_token = get_next_token();

            return;
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Missing right bracket in while statement");
        }
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing left bracket in while statement");
    }
}

// Main function of the parser, calling prog and initializing all the structures
int parser_parse_please () {

    // Inst_list - list of instructions for codegen
    inst_list = (instruction_list*)allocate_memory(sizeof(instruction_list));
    inst_list_init(inst_list);

    // Boolean structure, to not declare built-ins multiple times in codegen
    built_in_defs = (builtin_defs*)allocate_memory(sizeof(builtin_defs));
    builtin_defs_init(built_in_defs);

    // Handler of the function calls, making sure that the function call is valid with the function header
    callee_list = init_callee_list();
    callee_list_first = callee_list;

    // Stack for ifelse_cnt
    cnt_stack = (cnt_stack_t*)allocate_memory(sizeof(cnt_stack_t));
    cnt_init(cnt_stack);

    // Queue for variable definition
    queue = (queue_t*)allocate_memory(sizeof(queue_t));
    init_queue(queue);

    // Forest for the whole program, needed for IR of compiler
    forest_node *global = forest_insert_global();
    active = global; // Setting the Active pointer to root of the forest
    
    // The built-ins are by default children of the forest's root (GS)
    insert_built_in_functions_into_forest();

    // Loading the first token
    current_token = get_next_token();

    // Entrance to the recursive descent parser
    prog();

    // Validation of the function calls
    callee_validation(global);
    // Validation of the return statements
    return_logic_validation(global);

    // After all the instructions are loaded, we can generate the code
    codegen_generate_code_please(inst_list);

    // Cleaning up
    callee_list_dispose(callee_list_first);
    forest_dispose(global);
    cnt_dispose_stack(cnt_stack);
    inst_list_dispose(inst_list);
    free(built_in_defs);

    // If the program gets to this point, it means that it was successfully parsed
    return 0;
}



// Function for renaming the node, needed for codegen
char *renamer(AVL_tree *node) {
    if (node != NULL) {
        vector *v = vector_init();
        for (int i = 0; i < node->nickname; i++) {
            vector_append(v, '*');
        }
        vector_str_append(v, node->key);
        return v->array;
    }
    else {
        return NULL;
    }
}



// When a return statement is encountered, check if it is somewhere in a function
forest_node* check_return_stmt(forest_node *node) {

    // Function gets to the global scope, so it is not part of the function
    if (node->parent == NULL) {
        error_exit(ERROR_SYN, "PARSER", "Return statement is not in a function");
        return NULL;
    }
    else {
        if (node->keyword == W_FUNCTION) {
            return node;
        }
        else {
            return check_return_stmt(node->parent);
        }
    }
}


// Validating function calls, since function definitions can be after function calls
void callee_validation(forest_node *global){
    while (callee_list_first->next != NULL) {
        forest_node *func_def = forest_search_function(global, callee_list_first->callee->name);
        if (func_def == NULL) {
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Function is not defined");
        }
        // If the function is void, there has to be removed the expected return value from the ifjcode
        if (callee_list_first->callee->return_type == VOID && strcmp(func_def->name, "write") != 0) {
            inst_list_search_void_func_call(inst_list, callee_list_first->callee->name);
            inst_list_delete_after(inst_list);
        }
        if (callee_list_first->callee->arg_count != func_def->param_cnt && strcmp(func_def->name, "write") != 0) { // In case of built-in write function, the number of arguments is not checked
            error_exit(ERROR_SEM_TYPE, "PARSER", "Number of arguments in function call does not match the number of parameters in function definition");
        } else {
            // Check if the return type of the function call matches the return type in function definition (ignore when the callee's return type is void -> not assigning retval)
            if (callee_list_first->callee->return_type != (symtable_search(func_def->symtable, func_def->name))->data->return_type && callee_list_first->callee->return_type != VOID) {
                error_exit(ERROR_SEM_TYPE, "PARSER", "Function's return type does not match the return type in function definition");
            } else {
                for (int i = 1; i <= func_def->param_cnt; i++) {
                    // Check if the variables given as args was initialized
                    if (callee_list_first->callee->args_initialized[i] == false) {
                        error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Argument in function call is not initialized");
                    }

                    AVL_tree *param = symtable_find_param(func_def->symtable, i);
                    if (strcmp(callee_list_first->callee->args_names[i], param->data->param_name) != 0) {
                        error_exit(ERROR_SEM_OTHER, "PARSER", "Argument's name does not match the parameter's name in function definition");
                    }
                    // Check if the argument's type matches the parameter's type, if the parameter's type include '?', the argument's type can be nil
                    switch (param->data->param_type) {
                        case INT_QM:
                            if (callee_list_first->callee->args_types[i] != INT_QM && 
                                callee_list_first->callee->args_types[i] != INT &&
                                callee_list_first->callee->args_types[i] != NIL) {
                                error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's type does not match the parameter's type in function definition");
                            }
                            break;
                        case DOUBLE_QM:
                            if (callee_list_first->callee->args_types[i] != DOUBLE_QM &&
                                callee_list_first->callee->args_types[i] != DOUBLE &&
                                callee_list_first->callee->args_types[i] != NIL) {
                                error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's type does not match the parameter's type in function definition");
                            }
                            break;
                        case STRING_QM:
                            if (callee_list_first->callee->args_types[i] != STRING_QM &&
                                callee_list_first->callee->args_types[i] != STRING &&
                                callee_list_first->callee->args_types[i] != NIL) {
                                error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's type does not match the parameter's type in function definition");
                            }
                            break;
                        case INT:
                        case DOUBLE:
                        case STRING:
                            if (callee_list_first->callee->args_types[i] != param->data->param_type) {
                                error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's type does not match the parameter's type in function definition");
                            }
                            break;
                        default:
                            break;
                    }
                }
                // Write() has to be treated separately, since it have various number of arguments
                if (strcmp(func_def->name, "write") == 0) {
                    for (int i = 1; i <= callee_list_first->callee->arg_count; i++) {
                        if (callee_list_first->callee->args_initialized[1] == false) {
                            error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Argument in function call is not initialized");
                        }
                    }
                }
            }
        }
        callee_list_first = callee_list_first->next; // Move to the next function call
    }
}

// Validating return statements
void return_logic_validation (forest_node *global) {
    // Go through all functions in global scope (starting after all built-in functions)
    for (int i = AFTER_BUILTIN; i < global->children_count; i++) {
        if (global->children[i]->keyword == W_FUNCTION) { // Work only with non-void functions
            // Look at the children of the first children of the global scope - at the function's body
            if (symtable_search(global->children[i]->symtable, global->children[i]->name)->data->return_type != VOID) {
                validate_forest(global->children[i]->children[0]);
            }
        }
    }
}

// Function which recursively validates the return statements - for the body or internal if-else statements
void validate_forest(forest_node *func) {
    // If the function has return, it's already valid
    if (!func->has_return) {
        bool if_valid = false;
        bool else_valid = false;

        for (int i = 0; i < func->children_count; i++) {
            if (func->children[i]->keyword == W_IF) { // Work only with if-else statements
                if_valid = validate_forest_node(func->children[i]);
                else_valid = validate_forest_node(func->children[i+1]);

                if (if_valid && else_valid) {
                    return;
                }
            }
        }
        // If this function goes through all of its children and does not return
        error_exit(ERROR_SEM_EXPR_RET, "PARSER", "Return logic in function is not valid");
    }
}

// Validating individual node of the forest - the return has to be in the if and its else or in the both children if-else
bool validate_forest_node(forest_node *node) {
    // If the node has return, it's valid
    if (node->has_return) {
        return true;
    }

    bool if_valid = false;
    bool else_valid = false;

    for (int i = 0; i < node->children_count; i++) {
        if (node->children[i]->keyword == W_IF) { // Work only with if-else statements
            if_valid = validate_forest_node(node->children[i]);
            else_valid = validate_forest_node(node->children[i+1]);

            // Both children of if-else have return
            if (if_valid && else_valid) {
                    return true;
            }
        }
    }
    return false;
}

// Function for inserting built-in functions into the forest
void vardef_outermost_while(inst_type type, char *nickname, int cnt) {
    forest_node *outermost_while = forest_search_while(active); // NULL if not anywhere in while, outermost while otherwise
    if (outermost_while == NULL) { 
        // CODEGEN
        instruction *inst = inst_init(type, active->frame, nickname, cnt, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }
    else {
        inst_list_search_while(inst_list, outermost_while->name); // Int_list->active is now set on the outermost while -> insert before it
        // CODEGEN
        instruction *inst = inst_init(type, active->frame, nickname, cnt, 0, 0.0, NULL);
        inst_list_insert_before(inst_list, inst);
    }
}



// mode 1: convert node's data_type from optional to non-optional
// mode 2: convert node's data_type from non-optional to optional
void convert_optional_data_type (AVL_tree *node, int mode, bool if_let) {
    if (node != NULL && if_let) {
        if (mode == 1) {
            switch (node->data->data_type) {
                case INT_QM:
                    node->data->data_type = INT;
                    break;
                case DOUBLE_QM:
                    node->data->data_type = DOUBLE;
                    break;
                case STRING_QM:
                    node->data->data_type = STRING;
                    break;
                default:
                    break;
            }
        }
        else if (mode == 2) {
            switch (node->data->data_type) {
                case INT:
                    node->data->data_type = INT_QM;
                    break;
                case DOUBLE:
                    node->data->data_type = DOUBLE_QM;
                    break;
                case STRING:
                    node->data->data_type = STRING_QM;
                    break;
                default:
                    break;
            }
        }
        else {
            return;
        }
    }
}

// Setting all the flags to false
void builtin_defs_init(builtin_defs *defs) {
    defs->readInt_defined = false;
    defs->readDouble_defined = false;
    defs->readString_defined = false;
    defs->Int2Double_defined = false;
    defs->Double2Int_defined = false;
    defs->length_defined = false;
    defs->substring_defined = false;
    defs->ord_defined = false;
    defs->chr_defined = false;
}

// Inserting built-in functions into the codegen doubly linked list
void define_built_in_function(builtin_defs *built_in_defs) {
    switch (current_token->value.keyword) {
        case KW_RD_STR:
            if (!built_in_defs->readString_defined) {
                // CODEGEN
                instruction *inst = inst_init(READ_STRING, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->readString_defined = true;
            }
            break;
        
        case KW_RD_INT:
            if (!built_in_defs->readInt_defined) {
                // CODEGEN
                instruction *inst = inst_init(READ_INT, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->readInt_defined = true;
            }
            break;
        
        case KW_RD_DBL:
            if (!built_in_defs->readDouble_defined) {
                // CODEGEN
                instruction *inst = inst_init(READ_DOUBLE, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->readDouble_defined = true;
            }
            break;

        case KW_INT_2_DBL:
            if (!built_in_defs->Int2Double_defined) {
                // CODEGEN
                instruction *inst = inst_init(INT2DOUBLE, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->Int2Double_defined = true;
            }
            break;
        
        case KW_DBL_2_INT:
            if (!built_in_defs->Double2Int_defined) {
                // CODEGEN
                instruction *inst = inst_init(DOUBLE2INT, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->Double2Int_defined = true;
            }
            break;

        case KW_LENGHT:
            if (!built_in_defs->length_defined) {
                // CODEGEN
                instruction *inst = inst_init(LENGTH, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->length_defined = true;
            }
            break;

        case KW_SUBSTR:
            if (!built_in_defs->substring_defined) {
                // CODEGEN
                instruction *inst = inst_init(SUBSTRING, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->substring_defined = true;
            }
            break;
        
        case KW_ORD:
            if (!built_in_defs->ord_defined) {
                // CODEGEN
                instruction *inst = inst_init(ORD, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->ord_defined = true;
            }
            break;

        case KW_CHR:
            if (!built_in_defs->chr_defined) {
                // CODEGEN
                instruction *inst = inst_init(CHR, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                built_in_defs->chr_defined = true;
            }
            break;
        default:
            break;
    }
}
