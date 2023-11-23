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
cnt_stack_t *cnt_stack = NULL;
callee_list_t *callee_list = NULL;
sym_data data = {0};
var_type letvar = -1;
data_type type_of_expr = UNKNOWN; // for expression parser to return the data type of expression
bool is_defined = false;
int debug_cnt = 1;
int ifelse_cnt = 0;
int while_cnt = 0;
char node_name[20] = {0};
char *func_name_validate = NULL;
int cnt = 0; // used for generating unique labels for if-else
char *var_name = NULL; // to find the data type of variable for expression parser
int param_order = 0;
bool vardef_assign = false; // for assign() to know where to get info about the variable (from queue or from symtable)
bool function_write = true; // for parser to know that the function being handled is write() and needs special treatment



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



/**
 * @brief Looks to another token in the input using get_me_token()
 */
void peek() {
    if (token_buffer != NULL) {
        printf("\nWARNING: peek() voláš podruhé a token_buffer se přepíše!!! exit(1)...\n");
        exit(1);
    }
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


/**
 * @brief Function to call when want to get a token to current
 */
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


void define_built_in_functions() {
    // func readString() -> String?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readString");
    data = set_data_func(&data, STRING_QM);
    symtable_insert(&active->symtable, "readString", data);
    BACK_TO_PARENT_IN_FOREST;

    // func readInt() -> Int?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readInt");
    data = set_data_func(&data, INT_QM);
    symtable_insert(&active->symtable, "readInt", data);
    BACK_TO_PARENT_IN_FOREST;

    // func readDouble() -> Double?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readDouble");
    data = set_data_func(&data, DOUBLE_QM);
    symtable_insert(&active->symtable, "readDouble", data);
    BACK_TO_PARENT_IN_FOREST;

    // func write(term_1, term_2, ..., term_n)
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "write");
    data = set_data_func(&data, VOID);
    symtable_insert(&active->symtable, "write", data);
    BACK_TO_PARENT_IN_FOREST;

    // func Int2Double(_ term : Int) -> Double
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "Int2Double");
    data = set_data_func(&data, DOUBLE);
    symtable_insert(&active->symtable, "Int2Double", data);
    data = set_data_param(&data, INT, "_", 1);
    symtable_insert(&active->symtable, "term", data);
    BACK_TO_PARENT_IN_FOREST;

    // func Double2Int(_ term : Double) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "Double2Int");
    data = set_data_func(&data, INT);
    symtable_insert(&active->symtable, "Double2Int", data);
    data = set_data_param(&data, DOUBLE, "_", 1);
    symtable_insert(&active->symtable, "term", data);
    BACK_TO_PARENT_IN_FOREST;

    // func length(_ s : String) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "length");
    data = set_data_func(&data, INT);
    symtable_insert(&active->symtable, "length", data);
    data = set_data_param(&data, STRING, "_", 1);
    symtable_insert(&active->symtable, "s", data);
    BACK_TO_PARENT_IN_FOREST;

    // func substring(of s : String, startingAt i : Int, endingBefore j : Int) -> String?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "substring");
    data = set_data_func(&data, STRING_QM);
    symtable_insert(&active->symtable, "substring", data);
    data = set_data_param(&data, STRING, "of", 1);
    symtable_insert(&active->symtable, "s", data);
    data = set_data_param(&data, INT, "startingAt", 2);
    symtable_insert(&active->symtable, "i", data);
    data = set_data_param(&data, INT, "endingBefore", 3);
    symtable_insert(&active->symtable, "j", data);
    BACK_TO_PARENT_IN_FOREST;

    // func ord(_ c : String) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "ord");
    data = set_data_func(&data, INT);
    symtable_insert(&active->symtable, "ord", data);
    data = set_data_param(&data, STRING, "_", 1);
    symtable_insert(&active->symtable, "c", data);
    BACK_TO_PARENT_IN_FOREST;

    // func chr(_ i : Int) -> String
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "chr");
    data = set_data_func(&data, STRING);
    symtable_insert(&active->symtable, "chr", data);
    data = set_data_param(&data, INT, "_", 1);
    symtable_insert(&active->symtable, "i", data);
    BACK_TO_PARENT_IN_FOREST;
}



void prog() {
    // <prog> -> EOF | <func_def> <prog> | <body> <prog>
    printf("-- entering PROG --\n");
    print_debug(current_token, 2, debug_cnt);

    if (current_token->type == TOKEN_EOF) {
        printf("Printing global symtable in order:\n");
        inorder(&(active->symtable));
        printf("\n");
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
        forest_node *tmp = forest_search_function(active, current_token->value.vector->array);
        if (tmp != NULL) {
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Function is already defined and cannot be redefined");
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
            codegen_func_def();

            if (current_token->type == TOKEN_RPAR) {
                current_token = get_next_token();
                print_debug(current_token, 1, debug_cnt++);

                ret_type();

                // insert function with its return type to symtable
                data = set_data_func(&data, convert_dt(queue->first->token));
                symtable_insert(&active->symtable, active->name, data);
                queue_dispose(queue);

                if (current_token->type == TOKEN_LEFT_BRACKET) {
                    // get next token, body expects first token of body
                    current_token = get_next_token();
                    print_debug(current_token, 1, debug_cnt++);

                    local_body();

                    if (current_token->type == TOKEN_RIGHT_BRACKET) {
                        // func_def ends, go back to parent in forest
                        printf("Printing active->symtable in order:\n");
                        inorder(&(active->symtable));
                        printf("\n");
                        BACK_TO_PARENT_IN_FOREST;
                        current_token = get_next_token();
                        print_debug(current_token, 1, debug_cnt++);

                        // CODEGEN
                        codegen_func_def_end();

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
        error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Missing identifier of funciton");
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
    data = set_data_param(&data, convert_dt(current_token), queue->first->token->value.vector->array, ++param_order);

    symtable_insert(&active->symtable, queue->first->next->token->value.vector->array, data);

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
        queue_print(queue);
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
        queue_print(queue);
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
            queue_push(queue, current_token);
            queue_print(queue);
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
    // <ret> -> return <exp> | eps
    printf("-- entering RET --\n");
    print_debug(current_token, 2, debug_cnt);

    // checks whether the return is in the function as it should be, error otherwise
    forest_node *tmp = check_return_stmt(active);
    sym_data *tmp_data = symtable_lookup(tmp->symtable, tmp->name);

    if (tmp_data->return_type == VOID) { // void function 
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        // CODEGEN
        codegen_func_def_return_void(tmp->name);

        printf("-- returning...\n\n");
        return;
    }
    else { // non-void function
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        printf("ENTERING WORLD OF EXPRESSION PARSER\n");
        call_expr_parser(tmp->symtable->data.return_type); 
        printf("COMING BACK FROM EXPR_PARSER");

        // CODEGEN
        codegen_func_def_return(tmp->name);
    }
}

void body() {
    // <body> -> eps | <var_def> | <condition> | <cycle> | <assign> | <func_call> | <ret>
    printf("-- entering BODY --\n");
    print_debug(current_token, 2, debug_cnt);

    var_name = NULL;

    // note: když je <body> -> eps, tak body() se vůbec nezavolá

    if (current_token->type == TOKEN_ID) {
        peek();
        if (token_buffer->type == TOKEN_EQ) {
            var_name = current_token->value.vector->array; // for case: id = <exp>
            AVL_tree *tmp = forest_search_symbol(active, var_name);

            // check if the id is in symtable, so the variable is declared
            if (tmp == NULL) {
                error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable is not declared");
            }
            if (tmp->data.var_type == LET && tmp->data.defined) {
                error_exit(ERROR_SEM_OTHER, "PARSER", "Unmodifiable variable cannot be redefined");
            }
            if (!(tmp->data.defined)) {
                tmp->data.defined = true;
            }
            vardef_assign = false;
            assign();

        }
        else if (token_buffer->type == TOKEN_LPAR) {
            // function call without assigning, expecting void function
            func_call();
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
                    printf("\n\n\n\n\naaaaaaPICO cum na write input: %d\n\n\n\n\n", callee_list->callee->arg_count);
                    codegen_write(callee_list->callee->arg_count);

                    break;
                }
                else {
                    error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
                }

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

    //    rename_keep_exit();
        queue_push(queue, current_token);
        queue_print(queue);
        opt_var_def();

        // insert variable to symtable
        if (queue->first->next == NULL) { // the data type is not specified, expression parser determined it

            if (type_of_expr == NIL) {
                error_exit(ERROR_SEM_DERIV, "PARSER", "Variable cannot derive its type from nil");
            }
            else if (type_of_expr == BOOL) {
                error_exit(ERROR_SEM_OTHER, "PARSER", "Variable cannot be of type bool");
            } 
            else {
                data = set_data_var(data, is_defined, type_of_expr, letvar);
            }
        }
        else { // the data type was specified, expression parser will handle it as there is expected data type
            // '= nil' does not go through the expression parses, has to be handled here
            if (type_of_expr == NIL) {
                if (convert_dt(queue->first->next->token) != INT_QM && 
                    convert_dt(queue->first->next->token) != DOUBLE_QM && 
                    convert_dt(queue->first->next->token) != STRING_QM)
                {
                    error_exit(ERROR_SEM_TYPE, "PARSER", "Variable cannot be of type nil");
                }
            }
            else {
                data = set_data_var(data, is_defined, convert_dt(queue->first->next->token), letvar);
            }
        }        

        symtable_insert(&active->symtable, queue->first->token->value.vector->array, data);
        queue_dispose(queue);
        
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
        is_defined = false;

        peek();
        if (token_buffer->type == TOKEN_EQ) {
            vardef_assign = true;
            assign();
            // variable is defined
            is_defined = true;
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
        is_defined = true;
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
            }
            else {
                // in queue->first->next should be the data type of the variable, if it's NULL, the data type is unknown and should be determined by expression
                if (queue->first->next == NULL) {
                    printf("ENTERING WORLD OF EXPRESSION PARSER\n");
                    call_expr_parser(UNKNOWN); // in type_of_expr should be the data type of the expression
                    printf("COMING BACK FROM EXPR_PARSER");
                }
                else {
                    printf("ENTERING WORLD OF EXPRESSION PARSER\n");
                    call_expr_parser(convert_dt(queue->first->next->token));
                    printf("COMING BACK FROM EXPR_PARSER");
                }
            }
        }
        else if (current_token->type == TOKEN_KEYWORD) {
            switch (current_token->value.keyword) {
                case KW_RD_STR:
                    // CODEGEN
                    codegen_readString();
                    break;
                
                case KW_RD_INT:
                    // CODEGEN
                    codegen_readInt();
                    break;
                
                case KW_RD_DBL:
                    // CODEGEN
                    codegen_readDouble();
                    break;

                case KW_INT_2_DBL:
                    // CODEGEN
                    codegen_Int2Double();
                    break;
                
                case KW_DBL_2_INT:
                    // CODEGEN
                    codegen_Double2Int();
                    break;

                case KW_LENGHT:
                    // CODEGEN
                    codegen_length();
                    break;

                case KW_SUBSTR:
                    // CODEGEN
                    codegen_substring();
                    break;
                
                case KW_ORD:
                    // CODEGEN
                    codegen_ord();
                    break;

                case KW_CHR:
                    // CODEGEN
                    codegen_chr();
                    break;

                case KW_NIL:
                    // assigning nil
                    type_of_expr = NIL;

                    current_token = get_next_token();
                    print_debug(current_token, 1, debug_cnt++);
                    return;
                
                default:
                    error_exit(ERROR_SYN, "PARSER", "Unexpected token in assignment to variable while defining it");
                    break;
            }
            func_call();
        }
        else {
            // in queue->first->next should be the data type of the variable, if it's NULL, the data type is unknown and should be determined by expression
            if (queue->first->next == NULL) {
                printf("ENTERING WORLD OF EXPRESSION PARSER\n");
                call_expr_parser(UNKNOWN); // in type_of_expr should be the data type of the expression
                printf("COMING BACK FROM EXPR_PARSER");
            }
            else {
                printf("ENTERING WORLD OF EXPRESSION PARSER\n");
                call_expr_parser(convert_dt(queue->first->next->token));
                printf("COMING BACK FROM EXPR_PARSER");
            }
        }    
    }
    else { // assigning to already defined variable
        AVL_tree *tmp = forest_search_symbol(active, var_name);
        
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
            }
            else { // variable is already defined, so it's data_type is known
                printf("ENTERING WORLD OF EXPRESSION PARSER\n");
                call_expr_parser(tmp->data.data_type);
                printf("COMING BACK FROM EXPR_PARSER");
            }
        }
        else if (current_token->type == TOKEN_KEYWORD) {
            switch (current_token->value.keyword) {
                case KW_RD_STR:
                    // CODEGEN
                    codegen_readString();
                    break;
                
                case KW_RD_INT:
                    // CODEGEN
                    codegen_readInt();
                    break;
                
                case KW_RD_DBL:
                    // CODEGEN
                    codegen_readDouble();
                    break;

                case KW_INT_2_DBL:
                    // CODEGEN
                    codegen_Int2Double();
                    break;
                
                case KW_DBL_2_INT:
                    // CODEGEN
                    codegen_Double2Int();
                    break;

                case KW_LENGHT:
                    // CODEGEN
                    codegen_length();
                    break;

                case KW_SUBSTR:
                    // CODEGEN
                    codegen_substring();
                    break;
                
                case KW_ORD:
                    // CODEGEN
                    codegen_ord();
                    break;

                case KW_CHR:
                    // CODEGEN
                    codegen_chr();
                    break;
               
                case KW_NIL:
                    // assigning nil
                    type_of_expr = NIL;
                    
                    current_token = get_next_token();
                    print_debug(current_token, 1, debug_cnt++);
                    return;

                default:
                    error_exit(ERROR_SYN, "PARSER", "Unexpected token in assignment to already defined variable");
                    break;
            }
            func_call();
        }
        else {
            printf("ENTERING WORLD OF EXPRESSION PARSER\n");
            call_expr_parser(tmp->data.data_type);
            printf("COMING BACK FROM EXPR_PARSER");
        }
    }    
}


void func_call() {
    // <func_call> -> id ( <args> )
    printf("-- entering FUNC_CALL --\n");
    print_debug(current_token, 2, debug_cnt);


    // store the function's name for later usage (for codegen)
    char *func_name = malloc(sizeof(char) * strlen(current_token->value.vector->array) + 1);
    func_name = strcpy(func_name, current_token->value.vector->array);



    // func_call is not assigned 
    if (var_name == NULL) {
        insert_callee_into_list(callee_list, func_name, VOID);
    }
    else {
        AVL_tree* tmp = forest_search_symbol(active, var_name);
        insert_callee_into_list(callee_list, func_name, tmp->data.data_type);
    }

    if (!function_write) {
        // CODEGEN
        codegen_func_call_start();
    }

    // get TOKEN_LPAR from buffer
    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    args();

    if (current_token->type == TOKEN_RPAR) {
        if (!function_write) {
            // CODEGEN
            codegen_func_call_end(func_name);
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
    else { // calling without name and the first token of expression is not id
        insert_name_into_callee(callee_list->callee, "_");
    }

    printf("ENTERING WORLD OF EXPRESSION PARSER\n");
    call_expr_parser(UNKNOWN);
    printf("COMING BACK FROM EXPR_PARSER");

    insert_type_into_callee(callee_list->callee, type_of_expr);    

    if (!function_write) {
        // CODEGEN
        codegen_add_arg();
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

    cnt_push(cnt_stack); // push ifelse_cnt to stack and increment
    sprintf(node_name, "if_%d", ifelse_cnt);
    char *node_name2 = malloc(sizeof(char) * 20);
    node_name2 = strcpy(node_name2, node_name);
    printf("node_name: %s\n", node_name);
    MAKE_CHILDREN_IN_FOREST(W_IF, node_name2);
    active->cond_cnt = ifelse_cnt;

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword == KW_LET) {
        // if let id { <body> } <else> { <body> }
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        if (current_token->type == TOKEN_ID) {
            // check if the id is in symtable, so the variable is declared
            AVL_tree *tmp = forest_search_symbol(active, current_token->value.vector->array);
            if (tmp == NULL) {
                error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable is not declared");
            }
            else if (tmp->data.var_type == VAR) {
                error_exit(ERROR_SEM_OTHER, "PARSER", "Modifiable variable \"var\" cannot used as follows: \"if let id\"");

            }
            else if (tmp->data.data_type != INT_QM && tmp->data.data_type != DOUBLE_QM && tmp->data.data_type != STRING_QM) {
                error_exit(ERROR_SEM_TYPE, "PARSER", "Initializer for conditional binding must have optional type");
            }
            else {
                /// TODO: ??? vykoná sekvence_příkazů1, kde navíc bude typ 
                // id upraven tak, že nebude (pouze v tomto bloku) zahrnovat hodnotu nil 
                // (tj. např. proměnná původního typu String? bude v tomto bloku typu String).

                // get TOKEN_LEFT_BRACKET
                current_token = get_next_token();
                print_debug(current_token, 1, debug_cnt++);

                codegen_if_let(renamer(tmp));
            }
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Missing identifier in condition using \"if let id\"");
        }
    }
    else {
        printf("ENTERING WORLD OF EXPRESSION PARSER\n");
        call_expr_parser(BOOL); 
        printf("COMING BACK FROM EXPR_PARSER");

        codegen_if();
    }


    if (current_token->type == TOKEN_LEFT_BRACKET) {
        
        // CODEGEN
        ifelse_cnt++;

        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        local_body();

        if (current_token->type == TOKEN_RIGHT_BRACKET) {
            // closing bracket of if statement, go back to parent in forest
            BACK_TO_PARENT_IN_FOREST;
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);

            if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword == KW_ELSE) {

                cnt = cnt_top(cnt_stack); // get ifelse_cnt from stack
                sprintf(node_name, "else_%d", cnt);
                char *node_name3 = malloc(sizeof(char) * 20);
                node_name3 = strcpy(node_name3, node_name);
                printf("node_name: %s\n", node_name);
                MAKE_CHILDREN_IN_FOREST(W_ELSE, node_name3);
                active->cond_cnt = cnt;

                // CODEGEN
                codegen_else();

                current_token = get_next_token();
                print_debug(current_token, 1, debug_cnt++);

                if (current_token->type == TOKEN_LEFT_BRACKET) {
                    current_token = get_next_token();
                    print_debug(current_token, 1, debug_cnt++);

                    local_body();

                    if (current_token->type == TOKEN_RIGHT_BRACKET) {
                        // closing bracket of else statement, go back to parent in forest
                        BACK_TO_PARENT_IN_FOREST;
                        current_token = get_next_token();
                        print_debug(current_token, 1, debug_cnt++);

                    
                        active->cond_cnt = cnt_top(cnt_stack); // get ifelse_cnt from stack
                        
                        // CODEGEN
                        codegen_ifelse_end();  

                        cnt_pop(cnt_stack); // pop ifelse_cnt from stack


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
    while_cnt++;
    MAKE_CHILDREN_IN_FOREST(W_WHILE, node_name);

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    printf("ENTERING WORLD OF EXPRESSION PARSER\n");
    call_expr_parser(BOOL);
    printf("COMING BACK FROM EXPR_PARSER");


    if (current_token->type == TOKEN_LEFT_BRACKET) {
       
        // CODEGEN
        codegen_while_start();

        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        local_body();

        if (current_token->type == TOKEN_RIGHT_BRACKET) {

            // CODEGEN
            codegen_while_end();
            
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

    callee_list = init_callee_list();

    cnt_stack = (cnt_stack_t*)malloc(sizeof(cnt_stack_t));
    cnt_init(cnt_stack);

    queue = (queue_t*)malloc(sizeof(queue_t));
    init_queue(queue);

    forest_node *global = forest_insert_global();
    active = global;
    
    
    define_built_in_functions();

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    prog();




    callee_validation(global);

    //traverse_forest(global); // printing the forest // IS SEGFAULTING (not on mac though)

    printf("\n-------------------------------------------\n");
    printf("PARSER: Parsed successfully, you're welcome\n");
    printf("-------------------------------------------\n\n");
    return 0;
}


void rename_keep_exit() {
        AVL_tree *tmp = symtable_search(active->symtable, current_token->value.vector->array);
        if (tmp == NULL) {
            // the node is not in any symtable above the current node, the name can be kept
            if (forest_search_symbol(active->parent, current_token->value.vector->array) == NULL) {
                return;
            } 
            else { // the node is in any symtable above, no matter what, the name must be changed to unique identifier
                // nickname is set based on node_cnt from forest node (active), which is incremented after each insertions
                tmp->nickname = active->node_cnt;
                return;
            }
        }
        else { // the node is in the current symtable, error is thrown as multiple declarations of the same name are not allowed
            error_exit(ERROR_SEM_UNDEF_FUN, "REDECLARATION", "Multiple declarations of the same name are not allowed");
        }
}


// function for renaming the node  
char *renamer(AVL_tree *node) {
    vector *v = vector_init();
    for (int i = 0; i < node->nickname; i++) {
        vector_append(v, '*');
    }
    vector_str_append(v, node->key);
    return v->array;
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
void callee_validation(forest_node *global) {
    while (callee_list->next != NULL) {


        forest_node *tmp = forest_search_function(global, callee_list->callee->name);
        if (tmp == NULL) {
            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Function is not defined");
        }
        else {
            if (callee_list->callee->arg_count != tmp->param_cnt && strcmp(tmp->name, "write") != 0) { // in case of built-in write function, the number of arguments is not checked
                error_exit(ERROR_SEM_TYPE, "PARSER", "Number of arguments in function call does not match the number of parameters in function definition");
            }
            else {
                if (callee_list->callee->return_type != (symtable_search(tmp->symtable, tmp->name))->data.return_type) {
                    error_exit(ERROR_SEM_TYPE, "PARSER", "Function's return type does not match the return type in function definition");
                }
                else {
                    for (int i = 1; i <= tmp->param_cnt; i++) {
                        AVL_tree* param = symtable_find_param(tmp->symtable, i);
                        if (strcmp(callee_list->callee->args_names[i], param->data.param_name) != 0) {
                            error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's name does not match the parameter's name in function definition");
                        }
                        // check if the argument's type matches the parameter's type, if the parameter's type include '?', the argument's type can be nil
                        switch (param->data.param_type) {
                            case INT_QM:
                                if (callee_list->callee->args_types[i] != INT && callee_list->callee->args_types[i] != NIL) {
                                    printf("intq\n");
                                    error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's type does not match the parameter's type in function definition");
                                }
                                break;
                            case DOUBLE_QM:
                                if (callee_list->callee->args_types[i] != DOUBLE && callee_list->callee->args_types[i] != NIL) {
                                    printf("dblq\n");
                                    error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's type does not match the parameter's type in function definition");
                                }
                                break;
                            case STRING_QM:
                                if (callee_list->callee->args_types[i] != STRING && callee_list->callee->args_types[i] != NIL) {
                                    printf("srtnignq\n");
                                    error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's type does not match the parameter's type in function definition");
                                }
                                break;
                            case INT:
                            case DOUBLE:
                            case STRING:
                                if (callee_list->callee->args_types[i] != param->data.param_type) {
                                    printf("intdblstrng\n");
                                    error_exit(ERROR_SEM_TYPE, "PARSER", "Argument's type does not match the parameter's type in function definition");
                                }
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
        callee_list = callee_list->next;
    }
}


// void convert_to_nonq_data_type(char *key){
//     AVL_tree *node = forest_search_symbol(active, key);
//     if (node != NULL) {
//             sym_data *data = symtable_lookup(node, key);
//             if (data != NULL) {
//                 if (data->data_type == STRING_QM) {
//                     data->data_type = STRING;
//                 } else if (data->data_type == INT_QM) {
//                     data->data_type = INT;
//                 } else if (data->data_type == DOUBLE_QM) {
//                     data->data_type = DOUBLE;
//                 }
//         }
//     }
// }






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