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
cnt_stack_t *cnt_stack = NULL;
callee_list_t *callee_list = NULL;
callee_list_t *callee_list_first = NULL;
var_type letvar = 99;
data_type type_of_expr = UNKNOWN; // for expression parser to return the data type of expression
data_type type_of_assignee = UNKNOWN; // callee
bool is_initialized = false;
int debug_cnt = 1;
int ifelse_cnt = 0;
int while_cnt = 0;
char node_name[20] = {0};
char *var_name = NULL; // to find the data type of variable for expression parser
int param_order = 0;
bool vardef_assign = false; // for assign() to know where to get info about the variable (from queue or from symtable)
bool function_write = false; // for parser to know that the function being handled is write() and needs special treatment
// flags for defining built-in functions in codegen, so they are not defined multiple times
bool return_expr = false; // for expression parser to know that the expression is in return statement
builtin_defs *built_in_defs = NULL;
extern FILE *file;




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


void peek() {
    token_t *token = get_me_token();
    
    // mechanism for detecting EOLs
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


token_t* get_next_token() {
    if (token_buffer == NULL) {

        token_t *token = get_me_token();

        // mechanism for detecting EOLs
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



void prog() {
    // <prog> -> EOF | <func_def> <prog> | <body> <prog>
    printf("-- entering PROG --\n");
    print_debug(current_token, 2, debug_cnt);

    if (current_token->type == TOKEN_EOF) {
        printf("-- returning...\n\n");
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

void func_def() {
    // <func_def> -> func id ( <params> ) <ret_type> { <body> }
    printf("-- entering FUNC_DEF --\n");
    print_debug(current_token, 2, debug_cnt);

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    if (current_token->type == TOKEN_ID) {

        // check if the function is already defined
        forest_node *func_check = forest_search_function(active, current_token->value.vector->array);
        if (func_check != NULL) {
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Function is already defined and cannot be redefined");
        }

        AVL_tree *sym_check = symtable_search(active->symtable, current_token->value.vector->array);
        if (sym_check != NULL) {
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Cannot use the same name for function as was used for variable");
        }
     
        MAKE_CHILDREN_IN_FOREST(W_FUNCTION, current_token->value.vector->array);
        
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        if (current_token->type == TOKEN_LPAR) {
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);

            params();

            // set the number of parameters of the function
            active->param_cnt = param_order;
            param_order = 0;

            // CODEGEN
            instruction *inst = inst_init(FUNC_DEF, 'G', active->name, active->param_cnt, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);

            if (current_token->type == TOKEN_RPAR) {
                current_token = get_next_token();
                print_debug(current_token, 1, debug_cnt++);

                ret_type();

                // insert function with its return type to symtable
                sym_data *func_data = set_data_func(convert_dt(queue->first->token));
                symtable_insert(&active->symtable, active->name, func_data);
                queue_dispose(queue);

                if (current_token->type == TOKEN_LEFT_BRACKET) {
                    // get next token, body expects first token of body
                    current_token = get_next_token();
                    print_debug(current_token, 1, debug_cnt++);

                    MAKE_CHILDREN_IN_FOREST(W_FUNCTION_BODY, "body");

                    local_body();

                    BACK_TO_PARENT_IN_FOREST;

                    if (current_token->type == TOKEN_RIGHT_BRACKET) {
                        // func_def ends, go back to parent in forest
                        current_token = get_next_token();
                        print_debug(current_token, 1, debug_cnt++);

                        // CODEGEN
                        instruction *inst = inst_init(FUNC_DEF_END, 'G', active->name, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        BACK_TO_PARENT_IN_FOREST;
                        
                        printf("-- returning...\n\n");
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

void local_body() {
    // <local_body> -> <body> <local_body> | eps

    while (current_token->type != TOKEN_RIGHT_BRACKET) {
        body();
    }
}

void params() {
    // <params> -> eps | <par_name> <par_id> : <type> <params_n>
    printf("-- entering PARAMS --\n");
    print_debug(current_token, 2, debug_cnt);

    // void function
    if (current_token->type == TOKEN_RPAR) {
        printf("-- returning...\n\n");
        return;
    }
    
    par_name();

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);
    
    par_id();

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);


    if (current_token->type == TOKEN_COLON) {
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        type();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing colon in function's parameter");
    }

    // name of the parameter has to differ from the identifier of the parameter (except for case when the name and id is _)
    if (strcmp(queue->first->token->value.vector->array, queue->first->next->token->value.vector->array) == 0 && 
        strcmp(queue->first->token->value.vector->array, "_") != 0) {
        error_exit(ERROR_SEM_OTHER, "PARSER", "Parameter's name has to differ from its identifier");
    }
    
    // insert parameter to function's symtable
    sym_data *param_data = set_data_param(convert_dt(current_token), queue->first->token->value.vector->array, ++param_order);
    symtable_insert(&active->symtable, queue->first->next->token->value.vector->array, param_data);
    queue_dispose(queue);

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    if (current_token->type == TOKEN_RPAR) {
        printf("-- returning...\n\n");
        return;
    }
    else if (current_token->type == TOKEN_COMMA) {
        params_n();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function's parameter");
    }
}

void par_name() {
    // <par_name> -> _ | id
    printf("-- entering PAR_NAME --\n");
    print_debug(current_token, 2, debug_cnt);


    if (current_token->type == TOKEN_UNDERSCORE || current_token->type == TOKEN_ID) {
        // store parameter's name
        queue_push(queue, current_token);
        printf("-- returning...\n\n");
        return;
    }
    else { // not _ or id
        error_exit(ERROR_SEM_TYPE, "PARSER", "Parameter's name is not _ or id as expected");
    }
}

void par_id() {
    // <par_id> -> _ | id
    printf("-- entering PAR_ID --\n");
    print_debug(current_token, 2, debug_cnt);  


    if (current_token->type == TOKEN_UNDERSCORE || current_token->type == TOKEN_ID) {
        // store parameter's id
        //token_push(token_stack, current_token);
        queue_push(queue, current_token);
        printf("-- returning...\n\n");
        return;
    }
    else { // not _ or id
        error_exit(ERROR_SEM_TYPE, "PARSER", "Parameter's id is not _ or id as expected");
    }
}

void type() {
    // <type> -> Int | Int? | Double | Double? | String | String?
    printf("-- entering TYPE --\n");
    print_debug(current_token, 2, debug_cnt);

    if (current_token->type == TOKEN_KEYWORD || current_token->type == TOKEN_KEYWORD_QM) {
        if (current_token->value.keyword == KW_INT || current_token->value.keyword == KW_DOUBLE || current_token->value.keyword == KW_STRING) {
            // store type
            type_of_assignee = convert_dt(current_token);
            queue_push(queue, current_token);
            printf("-- returning...\n\n");
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

void params_n() {
    // <params_n> -> eps | , <params>
    printf("-- entering PARAMS_N --\n");
    print_debug(current_token, 2, debug_cnt);

    
    peek();
    if (token_buffer->type != TOKEN_ID && token_buffer->type != TOKEN_UNDERSCORE) {
        error_exit(ERROR_SYN, "PARSER", "Missing name of function's parameter");
    }
    else {
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);
        params();
    }
    
}

void ret_type() {
    // ret_type -> eps | -> <type>
    printf("-- entering RET_TYPE --\n");
    print_debug(current_token, 2, debug_cnt);

    if (current_token->type == TOKEN_RET_TYPE) {
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        type();

        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        printf("-- returning...\n\n");
        return;
    }
    else if (current_token->type == TOKEN_LEFT_BRACKET) { // void function
        queue_push(queue, current_token); 
        printf("-- returning...\n\n");
        return;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function's return type");
    }
}


void ret() {
    // <ret> -> return <exp> | return
    printf("-- entering RET --\n");
    print_debug(current_token, 2, debug_cnt);


    // checks whether the return is in the function as it should be, error otherwise
    forest_node *tmp = check_return_stmt(active);
    
    // set has_return flag to true, so the return logic knows this scope has return in it
    if (!active->has_return) {
        active->has_return = true;
    }
    else {
        error_exit(ERROR_SEM_EXPR_RET, "PARSER", "Function has multiple returns in one scope");
    }
    
    
    sym_data *tmp_data = symtable_search(tmp->symtable, tmp->name)->data;


    if (tmp_data->return_type == VOID) { // void function 
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        // CODEGEN
        instruction *inst = inst_init(FUNC_DEF_RETURN_VOID, 'G', tmp->name, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);

        printf("-- returning...\n\n");
        return;
    }
    else { // non-void function
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        return_expr = true;
        printf("ENTERING WORLD OF EXPRESSION PARSER with %d\n", tmp_data->return_type);
        call_expr_parser(tmp_data->return_type); 
        printf("COMING BACK FROM EXPR_PARSER\n");
        return_expr = false;

        // CODEGEN
        instruction *inst = inst_init(FUNC_DEF_RETURN, 'G', tmp->name, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }
}

void body() {
    // <body> -> eps | <var_def> | <condition> | <cycle> | <assign> | <func_call> | <ret>
    printf("-- entering BODY --\n");
    print_debug(current_token, 2, debug_cnt);

    var_name = NULL;
    type_of_assignee = UNKNOWN;

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
            // function call without assigning, expecting void function
            func_call();
            callee_list = callee_list->next;
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
        }
    }
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

            // write(term_1, term_2, ..., term_n)
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
    
    printf("-- returning...\n\n");
}

void var_def() {
    // <var_def> -> let id <opt_var_def> | var id <opt_var_def>
    printf("-- entering VAR_DEF --\n");
    print_debug(current_token, 2, debug_cnt);
   
    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    if (current_token->type == TOKEN_ID) {
        var_name = current_token->value.vector->array; // for case: let/var id = <exp>
        
        // case, where variable is declared with the same name as function already existing
        forest_node *func_check = forest_search_function(active, var_name);
        if (func_check != NULL) {
            error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable cannot have the same name as function");
        }

        // the node is in the current symtable, error is thrown as multiple declarations of the same name are not allowed
        /// TODO: param-var shadowing & func-var shadowing & func-par shadowing
        AVL_tree *check = symtable_search(active->symtable, var_name);
        if (check != NULL && !(check->data->is_param)) { 
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Multiple declarations of the same name are not allowed");
        }

        queue_push(queue, current_token);
        opt_var_def();

        sym_data *var_data = NULL;

        // insert variable to symtable
        if (queue->first->next == NULL) { // the data type is not specified, expression parser determined it
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
        else { // the data type was specified, expression parser will handle it as there is expected data type
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

        // if the variable is defined in while loop, it has to be defined before the outermost while loop (in codegen)
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


        printf("-- returning...\n\n");
        return;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing identifier in variable definition");
    }
}

void opt_var_def() {
    // <opt_var_def> -> : <type> | <assign> | : <type> <assign>
    printf("-- entering OPT_VAR_DEF --\n");
    print_debug(current_token, 2, debug_cnt);
    
    peek();    
    if (token_buffer->type == TOKEN_COLON) {
        
        // get TOKEN_COLON from buffer
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        type();

        // variable is declared
        is_initialized = false;

        peek();
        if (token_buffer->type == TOKEN_EQ) {
            vardef_assign = true;
            assign();
            // variable is defined
            is_initialized = true;
        }
        else {
            // let a : Int
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);
        }
        printf("-- returning...\n\n");
    }
    else if (token_buffer->type == TOKEN_EQ) {
        vardef_assign = true;
        assign();
        // variable is defined
        is_initialized = true;
        printf("-- returning...\n\n");
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in variable definition");
    }
}



void assign() {
    // <assign> -> = <exp> | = <func_call>
    printf("-- entering ASSIGN --\n");
    print_debug(current_token, 2, debug_cnt);
    
    // id is in var_name

    // assigning to variable while defining it
    if (vardef_assign) {
        // get TOKEN_EQ from buffer 
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        // here: var id = | let id = 

        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        // looking for function call
        if (current_token->type == TOKEN_ID) {
            peek();
            if (token_buffer->type == TOKEN_LPAR) {
                // expecting user-defined function
                func_call();
                callee_list = callee_list->next;

            }
            else {
                // in queue->first->next should be the data type of the variable, if it's NULL, the data type is unknown and should be determined by expression
                if (queue->first->next == NULL) {
                    printf("ENTERING WORLD OF EXPRESSION PARSER with unknown\n");
                    call_expr_parser(UNKNOWN); // in type_of_expr should be the data type of the expression
                    printf("COMING BACK FROM EXPR_PARSER\n");
                }
                else {
                    printf("ENTERING WORLD OF EXPRESSION PARSER with %d\n", convert_dt(queue->first->next->token));
                    call_expr_parser(convert_dt(queue->first->next->token));
                    printf("COMING BACK FROM EXPR_PARSER\n");
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
            // in queue->first->next should be the data type of the variable, if it's NULL, the data type is unknown and should be determined by expression
            if (queue->first->next == NULL) {
                printf("ENTERING WORLD OF EXPRESSION PARSER with unknown\n");
                call_expr_parser(UNKNOWN); // in type_of_expr should be the data type of the expression
                printf("COMING BACK FROM EXPR_PARSER\n");
            }
            else {
                printf("ENTERING WORLD OF EXPRESSION PARSER with %d\n", convert_dt(queue->first->next->token));
                call_expr_parser(convert_dt(queue->first->next->token));
                printf("COMING BACK FROM EXPR_PARSER\n");
                printf("TYPE OF EXPR: %d\n", type_of_expr);
            }
        }    
    }
    else { // assigning to already defined variable
        forest_node *scope = forest_search_scope(active, var_name);
        AVL_tree *symbol = symtable_search(scope->symtable, var_name);
        type_of_assignee = symbol->data->data_type;

        // cannot assign to a parameter in function definition
        if (symbol->data->is_param) {
            error_exit(ERROR_SEM_OTHER, "PARSER", "Cannot assign to parameter");
        }

        
        // get TOKEN_EQ from buffer 
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        // here: id =
   
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        // looking for function call
        if (current_token->type == TOKEN_ID) {
            peek();
            if (token_buffer->type == TOKEN_LPAR) {
                // expecting user-defined function
                func_call();
                
                instruction *inst = inst_init(VAR_ASSIGN, scope->frame, renamer(symbol), 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                callee_list = callee_list->next;
            }
            else { // variable is already defined, so it's data_type is known
                printf("ENTERING WORLD OF EXPRESSION PARSER with %d\n", symbol->data->data_type);
                call_expr_parser(symbol->data->data_type);

                // CODEGEN
                instruction *inst = inst_init(VAR_ASSIGN, scope->frame, renamer(symbol), 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                printf("COMING BACK FROM EXPR_PARSER\n");
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
            printf("ENTERING WORLD OF EXPRESSION PARSER with %d\n", symbol->data->data_type);
            call_expr_parser(symbol->data->data_type);

            // CODEGEN
            instruction *inst = inst_init(VAR_ASSIGN, active->frame, renamer(symbol), 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
            printf("COMING BACK FROM EXPR_PARSER\n");
        }
    }    
}


void func_call() {
    // <func_call> -> id ( <args> )
    printf("-- entering FUNC_CALL --\n");
    print_debug(current_token, 2, debug_cnt);


    // store the function's name for later usage (for codegen)
    char *func_name = allocate_memory(sizeof(char) * strlen(current_token->value.vector->array) + 1);
    func_name = strcpy(func_name, current_token->value.vector->array);

    insert_callee_into_list(callee_list, func_name);

    if (var_name == NULL) {
        callee_list->callee->return_type = VOID;
    }
    else if (type_of_assignee == UNKNOWN) {
        // find global node
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
        type_of_expr = type_of_assignee; // variable's type for symtable
        callee_list->callee->return_type = type_of_assignee; // for callee validation to work smoothly
    }
    else {
        callee_list->callee->return_type = type_of_assignee; 
    }

    if (!function_write) {
        // CODEGEN
        instruction *inst = inst_init(FUNC_CALL_START, 'G', NULL, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }

    // get TOKEN_LPAR from buffer
    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

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
        print_debug(current_token, 1, debug_cnt++);
        printf("-- returning...\n\n");
        return;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing right paranthesis in function call");
    }
}



void args() {
    // <args> -> eps | <arg> <args_n>
    printf("-- entering ARGS --\n");
    print_debug(current_token, 2, debug_cnt);

    peek();
    if (token_buffer->type == TOKEN_RPAR) {

        // get TOKEN_RPAR from buffer
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        printf("-- returning...\n\n");
        return;
    }
    else {
        // <args> -> <arg> <args_n>
        arg();

        if (current_token->type == TOKEN_RPAR) {
            printf("-- returning...\n\n");
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

void arg() {
    // <arg> -> exp | id : exp
    printf("-- entering ARG --\n");
    print_debug(current_token, 2, debug_cnt);

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    if (current_token->type == TOKEN_ID) {

        peek();
        if (token_buffer->type == TOKEN_COLON) {
            // <arg> -> id : exp
            insert_name_into_callee(callee_list->callee, current_token->value.vector->array);

            // get TOKEN_COLON from buffer
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);
            
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);
        }
        else { // calling without name
            insert_name_into_callee(callee_list->callee, "_");

        }
    }
    else if (current_token->type == TOKEN_UNDERSCORE) {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function call, underscore cannot be used as argument's name");
    }
    else { // calling without name and the first token of expression is not id
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
                // in built-in function write, when passing argument of optional type and uninitialized, it is implicitly set to nil and printing ""
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

    printf("ENTERING WORLD OF EXPRESSION PARSER with unknown\n");
    call_expr_parser(UNKNOWN);
    printf("COMING BACK FROM EXPR_PARSER\n");
    insert_type_into_callee(callee_list->callee, type_of_expr);

    if (!function_write) {
        // CODEGEN
        instruction *inst = inst_init(ADD_ARG, 'G', NULL, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }
}

void args_n() {
    // <args_n> -> eps | , <arg> <args_n>
    printf("-- entering ARGS_N --\n");
    print_debug(current_token, 2, debug_cnt);

    arg();

    if (current_token->type == TOKEN_RPAR) {
        printf("-- returning...\n\n");
        return;
    }
    else if (current_token->type == TOKEN_COMMA) {
        args_n();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function call, missing right paranthesis or comma between arguments");
    }
}


void condition() {
    // <condition> -> if <exp> { <local_body> } else { <local_body> } | if let id { <local_body> } else { <local_body> }
    printf("-- entering CONDITION --\n");
    print_debug(current_token, 2, debug_cnt);

    int cnt = 0;

    cnt_push(cnt_stack); // push ifelse_cnt to stack and increment
    sprintf(node_name, "if_%d", ifelse_cnt);
    char *node_name2 = (char*)allocate_memory(sizeof(char) * 20);
    strcpy(node_name2, node_name);

    MAKE_CHILDREN_IN_FOREST(W_IF, node_name2);
    active->cond_cnt = ifelse_cnt;

    bool if_let = false;
    AVL_tree *symbol_q = NULL;

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword == KW_LET) {
        // if let id { <body> } <else> { <body> }
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        if (current_token->type == TOKEN_ID) {
            // check if the id is in symtable, so the variable is declared
            AVL_tree *symbol = forest_search_symbol(active, current_token->value.vector->array);
            symbol_q = symbol; // for later usage (converting optional type to non-optional and back)
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

                // get TOKEN_LEFT_BRACKET
                current_token = get_next_token();
                print_debug(current_token, 1, debug_cnt++);
                
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
        printf("ENTERING WORLD OF EXPRESSION PARSER with bool\n");
        call_expr_parser(BOOL); 
        printf("COMING BACK FROM EXPR_PARSER\n");

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
        print_debug(current_token, 1, debug_cnt++);
        
        convert_optional_data_type(symbol_q, 1); // convert optional type to non-optional
        // IF BODY
        local_body();

        convert_optional_data_type(symbol_q, 2); // convert non-optional type back to optional

        if (current_token->type == TOKEN_RIGHT_BRACKET) {
            // closing bracket of if statement, go back to parent in forest
            BACK_TO_PARENT_IN_FOREST;
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);

            if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword == KW_ELSE) {

                cnt = cnt_top(cnt_stack); // get ifelse_cnt from stack
                sprintf(node_name, "else_%d", cnt);
                char *node_name3 = (char*)allocate_memory(sizeof(char) * 20);
                strcpy(node_name3, node_name);
                MAKE_CHILDREN_IN_FOREST(W_ELSE, node_name3);
                active->cond_cnt = cnt;

                // CODEGEN
                instruction *inst = inst_init(ELSE, active->frame, NULL, active->cond_cnt, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                current_token = get_next_token();
                print_debug(current_token, 1, debug_cnt++);

                if (current_token->type == TOKEN_LEFT_BRACKET) {
                    current_token = get_next_token();
                    print_debug(current_token, 1, debug_cnt++);

                    // ELSE BODY
                    local_body();

                    if (current_token->type == TOKEN_RIGHT_BRACKET) {
                        // closing bracket of else statement, go back to parent in forest
                        current_token = get_next_token();
                        print_debug(current_token, 1, debug_cnt++);
                    
                        active->cond_cnt = cnt_top(cnt_stack); // get ifelse_cnt from stack
                        
                        // CODEGEN
                        instruction *inst = inst_init(IFELSE_END, active->frame, NULL, active->cond_cnt, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        cnt_pop(cnt_stack); // pop ifelse_cnt from stack

                        BACK_TO_PARENT_IN_FOREST;

                        printf("-- returning...\n\n");
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



void cycle() {
    // <cycle -> while <exp> { <local_body> }
    printf("-- entering CYCLE --\n");
    print_debug(current_token, 2, debug_cnt);

    sprintf(node_name, "while_%d", while_cnt);
    char *node_name1 = (char*)allocate_memory(sizeof(char) * 20);
    strcpy(node_name1, node_name);
  
    while_cnt++;
    forest_node *outermost_while = forest_search_while(active);
    MAKE_CHILDREN_IN_FOREST(W_WHILE, node_name1);

    // CODEGEN
    if (outermost_while == NULL) {
        // CODEGEN
        instruction *inst = inst_init(WHILE_COND_DEF, active->frame, node_name1, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }
    else {
        inst_list_search_while(inst_list, outermost_while->name); // int_list->active is now set on the outermost while -> insert before it
        // CODEGEN
        instruction *inst = inst_init(WHILE_COND_DEF, active->frame, node_name1, 0, 0, 0.0, NULL);
        inst_list_insert_before(inst_list, inst);
    }


    // CODEGEN
    instruction *inst1 = inst_init(WHILE_START, 'G', node_name1, 0, 0, 0.0, NULL);
    inst_list_insert_last(inst_list, inst1);


    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    printf("ENTERING WORLD OF EXPRESSION PARSER with bool\n");
    call_expr_parser(BOOL);
    printf("COMING BACK FROM EXPR_PARSER\n");


    if (current_token->type == TOKEN_LEFT_BRACKET) {
       
        // CODEGEN
        instruction *inst = inst_init(WHILE_DO, active->frame, node_name1, 0, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);

        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        local_body();

        if (current_token->type == TOKEN_RIGHT_BRACKET) {

            // CODEGEN
            instruction *inst = inst_init(WHILE_END, active->frame, node_name1, 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
            
            // closing bracket of while statement, go back to parent in forest
            BACK_TO_PARENT_IN_FOREST;
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);

            printf("-- returning...\n\n");

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

int parser_parse_please () {
    printf("\n---------------------\n");
    printf("Parser parse please\n");
    printf("---------------------\n\n");

    inst_list = (instruction_list*)allocate_memory(sizeof(instruction_list));
    inst_list_init(inst_list);

    built_in_defs = (builtin_defs*)allocate_memory(sizeof(builtin_defs));
    builtin_defs_init(built_in_defs);


    callee_list = init_callee_list();
    callee_list_first = callee_list;

    cnt_stack = (cnt_stack_t*)allocate_memory(sizeof(cnt_stack_t));
    cnt_init(cnt_stack);

    queue = (queue_t*)allocate_memory(sizeof(queue_t));
    init_queue(queue);

    forest_node *global = forest_insert_global();
    active = global;
    
    
    insert_built_in_functions_into_forest();

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    prog();



    callee_validation(global);
    return_logic_validation(global);

    codegen_generate_code_please(inst_list);
    ////////////
    fclose(file);
    ////////////

    callee_list_dispose(callee_list_first);
    forest_dispose(global);
    cnt_dispose_stack(cnt_stack);
    inst_list_dispose(inst_list);
    free(built_in_defs);

    printf("\n-------------------------------------------\n");
    printf("PARSER: Parsed successfully, you're welcome\n");
    printf("-------------------------------------------\n\n");
    return 0;
}



// function for renaming the node  
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



// when a return statement is encountered, check if it is somewhere in a function
forest_node* check_return_stmt(forest_node *node) {

    // function gets to the global scope, so it is not part of the function
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


// validating function calls, since function definitions can be after function calls
void callee_validation(forest_node *global){
    while (callee_list_first->next != NULL) {
        forest_node *func_def = forest_search_function(global, callee_list_first->callee->name);
        if (func_def == NULL) {
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Function is not defined");
        }
        // if the function is void, there has to be removed the expected return value ftom the ifjcode
        if (callee_list_first->callee->return_type == VOID && strcmp(func_def->name, "write") != 0) {
            inst_list_search_void_func_call(inst_list, callee_list_first->callee->name);
            inst_list_delete_after(inst_list);
        }
        if (callee_list_first->callee->arg_count != func_def->param_cnt && strcmp(func_def->name, "write") != 0) { // in case of built-in write function, the number of arguments is not checked
            error_exit(ERROR_SEM_TYPE, "PARSER", "Number of arguments in function call does not match the number of parameters in function definition");
        } else {
            // check if the return type of the function call matches the return type in function definition (ignore when the callee's return type is void -> not assigning retval)
            if (callee_list_first->callee->return_type != (symtable_search(func_def->symtable, func_def->name))->data->return_type && callee_list_first->callee->return_type != VOID) {
                error_exit(ERROR_SEM_TYPE, "PARSER", "Function's return type does not match the return type in function definition");
            } else {
                for (int i = 1; i <= func_def->param_cnt; i++) {
                    // check if the variables given as args was initialized
                    if (callee_list_first->callee->args_initialized[i] == false) {
                        error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Argument in function call is not initialized");
                    }

                    AVL_tree *param = symtable_find_param(func_def->symtable, i);
                    if (strcmp(callee_list_first->callee->args_names[i], param->data->param_name) != 0) {
                        error_exit(ERROR_SEM_OTHER, "PARSER", "Argument's name does not match the parameter's name in function definition");
                    }
                    // check if the argument's type matches the parameter's type, if the parameter's type include '?', the argument's type can be nil
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
                // write() has to be treated separately, since it have various number of arguments
                if (strcmp(func_def->name, "write") == 0) {
                    for (int i = 1; i <= callee_list_first->callee->arg_count; i++) {
                        if (callee_list_first->callee->args_initialized[1] == false) {
                            error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Argument in function call is not initialized");
                        }
                    }
                }
            }
        }
        callee_list_first = callee_list_first->next;
    }
}



void return_logic_validation (forest_node *global) {
    // go through all functions in global scope (starting after all built-in functions)
    for (int i = AFTER_BUILTIN; i < global->children_count; i++) {
        if (global->children[i]->keyword == W_FUNCTION) { // work only with non-void functions
            if (symtable_search(global->children[i]->symtable, global->children[i]->name)->data->return_type != VOID) {
                validate_forest(global->children[i]->children[0]);
            }
        }
    }
}

void validate_forest(forest_node *func) {
    // if the function has return, it's already valid
    if (!func->has_return) {
        bool if_valid = false;
        bool else_valid = false;

        for (int i = 0; i < func->children_count; i++) {
            if (func->children[i]->keyword == W_IF) { // work only with if-else statements
                if_valid = validate_forest_node(func->children[i]);
                else_valid = validate_forest_node(func->children[i+1]);

                if (if_valid && else_valid) {
                    return;
                }
            }
        }
        // if this function goes through all of its children and does not return
        error_exit(ERROR_SEM_EXPR_RET, "PARSER", "Return logic in function is not valid");
    }
}

bool validate_forest_node(forest_node *node) {
    // if the node has return, it's valid
    if (node->has_return) {
        return true;
    }

    bool if_valid = false;
    bool else_valid = false;

    for (int i = 0; i < node->children_count; i++) {
        if (node->children[i]->keyword == W_IF) { // work only with if-else statements
            if_valid = validate_forest_node(node->children[i]);
            else_valid = validate_forest_node(node->children[i+1]);

            if (if_valid && else_valid) {
                    return true;
            }
        }
    }
    return false;
}



void vardef_outermost_while(inst_type type, char *nickname, int cnt) {
    forest_node *outermost_while = forest_search_while(active); // NULL if not anywhere in while, outermost while otherwise
    if (outermost_while == NULL) { 
        // CODEGEN
        instruction *inst = inst_init(type, active->frame, nickname, cnt, 0, 0.0, NULL);
        inst_list_insert_last(inst_list, inst);
    }
    else {
        inst_list_search_while(inst_list, outermost_while->name); // int_list->active is now set on the outermost while -> insert before it
        // CODEGEN
        instruction *inst = inst_init(type, active->frame, nickname, cnt, 0, 0.0, NULL);
        inst_list_insert_before(inst_list, inst);
    }
}



// mode 1: convert node's data_type from optional to non-optional
// mode 2: convert node's data_type from non-optional to optional
void convert_optional_data_type (AVL_tree *node, int mode) {
    if (node != NULL) {
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



// –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
// –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
// –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––

void print_debug(token_t *token, int mode, int cnt) {
    if (token != NULL) {
        char* type = NULL;
        char* keyword = NULL;
        switch (token->type) {
            case TOKEN_WHITESPACE:
                type = "TOKEN_WHITESPACE";
                break;
            case TOKEN_TAB:
                type = "TOKEN_TAB";
                break;
            case TOKEN_EOL:
                type = "TOKEN_EOL";
                break;
            case TOKEN_EOF:
                type = "TOKEN_EOF";
                break;
            case TOKEN_MINUS:
                type = "TOKEN_MINUS";
                break;
            case TOKEN_PLUS:
                type = "TOKEN_PLUS";
                break;
            case TOKEN_MULTIPLY:
                type = "TOKEN_MULTIPLY";
                break;
            case TOKEN_LESS:
                type = "TOKEN_LESS";
                break;
            case TOKEN_LESS_EQ:
                type = "TOKEN_LESS_EQ";
                break;
            case TOKEN_GREAT:
                type = "TOKEN_GREAT";
                break;
            case TOKEN_GREAT_EQ:
                type = "TOKEN_GREAT_EQ";
                break;
            case TOKEN_LPAR:
                type = "TOKEN_LPAR";
                break;
            case TOKEN_RPAR:
                type = "TOKEN_RPAR";
                break;
            case TOKEN_EQ:
                type = "TOKEN_EQ";
                break;
            case TOKEN_EQEQ:
                type = "TOKEN_EQEQ";
                break;
            case TOKEN_EXCLAM:
                type = "TOKEN_EXCLAM";
                break;
            case TOKEN_EXCLAMEQ:
                type = "TOKEN_EXCLAMEQ";
                break;
            case TOKEN_COMMA:
                type = "TOKEN_COMMA";
                break;  
            case TOKEN_COLON:
                type = "TOKEN_COLON";
                break;
            case TOKEN_DOUBLE_QM:
                type = "TOKEN_DOUBLE_QM";
                break;
            case TOKEN_RET_TYPE:
                type = "TOKEN_RET_TYPE";
                break;
            case TOKEN_ID:
                type = "TOKEN_ID";
                break;
            case TOKEN_KEYWORD_QM:
            case TOKEN_KEYWORD:
                if (token->type == TOKEN_KEYWORD_QM) {
                    type = "TOKEN_KEYWORD_QM";
                }
                else {
                    type = "TOKEN_KEYWORD";
                }
                switch (token->value.keyword) {
                    case KW_DOUBLE:
                        keyword = "KW_DOUBLE";
                        break;
                    case KW_INT:
                        keyword = "KW_INT";
                        break;
                    case KW_STRING:
                        keyword = "KW_STRING";
                        break;
                    case KW_ELSE:
                        keyword = "KW_ELSE";
                        break;
                    case KW_FUNC:
                        keyword = "KW_FUNC";
                        break;
                    case KW_IF:
                        keyword = "KW_IF";
                        break;
                    case KW_LET:
                        keyword = "KW_LET";
                        break;
                    case KW_NIL:
                        keyword = "KW_NIL";
                        break;
                    case KW_RETURN:
                        keyword = "KW_RETURN";
                        break;
                    case KW_VAR:
                        keyword = "KW_VAR";
                        break;
                    case KW_WHILE:
                        keyword = "KW_WHILE";
                        break;
                    case KW_RD_STR:
                        keyword = "KW_RD_STR";
                        break;
                    case KW_RD_INT:
                        keyword = "KW_RD_INT";
                        break;
                    case KW_RD_DBL:
                        keyword = "KW_RD_DBL";
                        break;
                    case KW_WRT:
                        keyword = "KW_WRT";
                        break;
                    case KW_INT_2_DBL:
                        keyword = "KW_INT_2_DBL";
                        break;
                    case KW_DBL_2_INT:
                        keyword = "KW_DBL_2_INT";
                        break;
                    case KW_LENGHT:
                        keyword = "KW_LENGHT";
                        break;
                    case KW_SUBSTR:
                        keyword = "KW_SUBSTR";
                        break;
                    case KW_ORD:
                        keyword = "KW_ORD";
                        break;
                    case KW_CHR:
                        keyword = "KW_CHR";
                        break;
                    case KW_GLOBAL:
                        keyword = "KW_GLOBAL";
                        break;
                    default:
                        break;
                    }
                break;
            case TOKEN_NUM:
                type = "TOKEN_NUM";
                break;
            case TOKEN_EXP:
                type = "TOKEN_EXP";
                break;
            case TOKEN_DEC:
                type = "TOKEN_DEC";
                break;
            case TOKEN_STRING:
                type = "TOKEN_STRING";
                break;
            case TOKEN_ML_STRING:
                type = "TOKEN_ML_STRING";
                break;
            case TOKEN_UNDERSCORE:
                type = "TOKEN_UNDERSCORE";
                break;
            case TOKEN_LEFT_BRACKET:
                type = "TOKEN_LEFT_BRACKET";
                break;
            case TOKEN_RIGHT_BRACKET:
                type = "TOKEN_RIGHT_BRACKET";
                break;
            case TOKEN_DIVIDE:
                type = "TOKEN_DIVIDE";
                break;
            default:
                break;
        }

        if (mode == 1) {
            if (token->type == TOKEN_KEYWORD || token->type == TOKEN_KEYWORD_QM) {
                printf("(%d)G.N.T.: Current token: %s: %s\n\n", cnt, type, keyword);
            }
            else if (token->type == TOKEN_ID) {
                printf("(%d)G.N.T.: Current token: %s: %s\n\n", cnt, type, token->value.vector->array);
            }
            else if (token->type == TOKEN_NUM) {
                printf("(%d)G.N.T.: Current token: %s: %d\n\n", cnt, type, token->value.integer);
            }
            else if (token->type == TOKEN_DEC) {
                printf("(%d)G.N.T.: Current token: %s: %f\n\n", cnt, type, token->value.type_double);
            }
            else {
                printf("(%d)G.N.T.: Current token: %s\n\n", cnt, type);
            }
        }
        else if (mode == 2) {
            if (token->type == TOKEN_KEYWORD || token->type == TOKEN_KEYWORD_QM) {
                printf("-- with current token: %s: %s\n\n", type, keyword);
            }
            else if (token->type == TOKEN_ID) {
                printf("-- with current token: %s: %s\n\n", type, token->value.vector->array);
            }
            else if (token->type == TOKEN_NUM) {
                printf("-- with current token: %s: %d\n\n", type, token->value.integer);
            }
            else if (token->type == TOKEN_DEC) {
                printf("-- with current token: %s: %f\n\n", type, token->value.type_double);
            }
            else {
                printf("-- with current token: %s\n\n", type);
            }
        }
        else { 
            printf("PRINT_DEBUG: Unknown mode\n");
        }
    }
}
