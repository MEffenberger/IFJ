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

#include "parser.h"

forest_node *active = NULL; // Pointer to the active node in the forest
token_t *current_token = NULL; // Pointer to the current token
token_t *token_buffer = NULL; // Buffer for tokens
queue_t *queue = NULL; // Queue for the expression parser
queue_t *fn_call_queue = NULL; // Queue for function calls
sym_data data = {0};
var_type letvar = -1;
bool is_defined = false;
int debug_cnt = 1;
int ifelse_cnt = 0;
int while_cnt = 0;
char node_name[20] = {0};
cnt_stack_t *cnt_stack = NULL;
token_stack_t *token_stack = NULL;
int cnt = 0;
int renamer3000 = 0;




data_type convert_dt(token_t* token) {
    if (token->type == TOKEN_KEYWORD) {
        switch (token->value.keyword) {
            case KW_INT:
                return T_INT;
            case KW_DOUBLE:
                return T_DOUBLE;
            case KW_STRING:
                return T_STRING;
            case KW_NIL:
                return T_NIL;
            default:
                return -1;
        }
    }
    else if (token->type == TOKEN_KEYWORD_QM) {
        switch (token->value.keyword) {
            case KW_INT:
                return T_INT_Q;
            case KW_DOUBLE:
                return T_DOUBLE_Q;
            case KW_STRING:
                return T_STRING_Q;
            default:
                return -1;
        }
    }
    else if (token->type == TOKEN_LEFT_BRACKET) {
        return T_VOID; // void function (absence of return type)
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


void built_in_functions() {
    // func readString() -> String?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readString");
    data = set_data_func(&data, T_STRING_Q);
    symtable_insert(&active->symtable, "readString", data);
    BACK_TO_PARENT_IN_FOREST;

    // func readInt() -> Int?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readInt");
    data = set_data_func(&data, T_INT_Q);
    symtable_insert(&active->symtable, "readInt", data);
    BACK_TO_PARENT_IN_FOREST;

    // func readDouble() -> Double?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "readDouble");
    data = set_data_func(&data, T_DOUBLE_Q);
    symtable_insert(&active->symtable, "readDouble", data);
    BACK_TO_PARENT_IN_FOREST;

    // tODO picovINA PRBLM
    // func write(term_1, term_2, ..., term_n)
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "write");
    data = set_data_func(&data, T_NIL);
    symtable_insert(&active->symtable, "write", data);
    BACK_TO_PARENT_IN_FOREST;

    // func Int2Double(_ term : Int) -> Double
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "Int2Double");
    data = set_data_func(&data, T_DOUBLE);
    symtable_insert(&active->symtable, "Int2Double", data);
    data = set_data_param(&data, T_INT, "_");
    symtable_insert(&active->symtable, "term", data);
    BACK_TO_PARENT_IN_FOREST;

    // func Double2Int(_ term : Double) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "Double2Int");
    data = set_data_func(&data, T_INT);
    symtable_insert(&active->symtable, "Double2Int", data);
    data = set_data_param(&data, T_DOUBLE, "_");
    symtable_insert(&active->symtable, "term", data);
    BACK_TO_PARENT_IN_FOREST;

    // func length(_ s : String) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "length");
    data = set_data_func(&data, T_INT);
    symtable_insert(&active->symtable, "length", data);
    data = set_data_param(&data, T_STRING, "_");
    symtable_insert(&active->symtable, "s", data);
    BACK_TO_PARENT_IN_FOREST;

    // func substring(of s : String, startingAt i : Int, endingBefore j : Int) -> String?
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "substring");
    data = set_data_func(&data, T_STRING_Q);
    symtable_insert(&active->symtable, "substring", data);
    data = set_data_param(&data, T_STRING, "of");
    symtable_insert(&active->symtable, "s", data);
    data = set_data_param(&data, T_INT, "startingAt");
    symtable_insert(&active->symtable, "i", data);
    data = set_data_param(&data, T_INT, "endingBefore");
    symtable_insert(&active->symtable, "j", data);
    BACK_TO_PARENT_IN_FOREST;

    // func ord(_ c : String) -> Int
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "ord");
    data = set_data_func(&data, T_INT);
    symtable_insert(&active->symtable, "ord", data);
    data = set_data_param(&data, T_STRING, "_");
    symtable_insert(&active->symtable, "c", data);
    BACK_TO_PARENT_IN_FOREST;

    // func chr(_ i : Int) -> String
    MAKE_CHILDREN_IN_FOREST(W_FUNCTION, "chr");
    data = set_data_func(&data, T_STRING);
    symtable_insert(&active->symtable, "chr", data);
    data = set_data_param(&data, T_INT, "_");
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
        MAKE_CHILDREN_IN_FOREST(W_FUNCTION, current_token->value.vector->array);
        
        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        if (current_token->type == TOKEN_LPAR) {
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);

            params();

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

    // insert parameter to function's symtable
    data = set_data_param(&data, convert_dt(current_token), queue->first->token->value.vector->array);
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
        token_push(token_stack, current_token);
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
    // <type> -> Int | Int? | Double | Double? | String | String? | nil
    printf("-- entering TYPE --\n");
    print_debug(current_token, 2, debug_cnt);

    if (current_token->type == TOKEN_KEYWORD || current_token->type == TOKEN_KEYWORD_QM) {
        if (current_token->value.keyword == KW_INT || current_token->value.keyword == KW_DOUBLE || current_token->value.keyword == KW_STRING || current_token->value.keyword == KW_NIL) {
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
        queue_print(queue);
        printf("-- returning...\n\n");
        return;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token in function's return type");
    }
}


// void func_body() {
//     // <func_body> -> <body> <body> <body> ?!?!? <ret> <body>
//     printf("-- entering FUNC_BODY --\n");
//     print_debug(current_token, 2, debug_cnt);

//     current_token = get_next_token();
//     print_debug(current_token, 1, debug_cnt++);

//     // dokud func_body nechce skončit (via '}'), tak dělej body
//     // TODO: what about return??
//     while (current_token->type != TOKEN_RIGHT_BRACKET) {
//         body();
//     }

//     // TODO: problem kdyz se body koncici '}' vraci (do progu vs do func_body)

//     printf("-- returning...\n\n");
//     return;
// }

// void ret() {
//     // <ret> -> return <exp> | eps
//     printf("-- entering RET --\n");
//     print_debug(current_token, 2, debug_cnt);

// }


void body() {
    // <body> -> eps | <var_def> | <condition> | <cycle> | <assign> | <func_call>
    printf("-- entering BODY --\n");
    print_debug(current_token, 2, debug_cnt);

    // note: když je <body> -> eps, tak body() se vůbec nezavolá

    if (current_token->type == TOKEN_ID) {
        peek();
        if (token_buffer->type == TOKEN_EQ) {
            // check if the id is in symtable, so the variable is declared
            if (forest_search_symbol(active, current_token->value.vector->array) == NULL) {
                error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable is not declared");
            }
            assign();
        }
        else if (token_buffer->type == TOKEN_LPAR) {
            // TODO: check if the id is forest, so the function is defined? Problem with recursive calling fo two functions
            queue_push(fn_call_queue, current_token); // second queue, stores the IDs of function calls

            // if (false) {  
            //     error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Function is not defined");
            // }
            func_call();
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
        }
    }
    else if (current_token->type == TOKEN_KEYWORD) {
        switch (current_token->value.keyword) {
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
                //cycle();                       
                break;

            case KW_RD_STR: // func readString() -> String?

                break;

            case KW_RD_INT:

                break;

            case KW_RD_DBL:

                break;

            case KW_WRT:

                break;

            case KW_INT_2_DBL:

                break;

            case KW_DBL_2_INT:

                break;

            case KW_LENGHT:

                break;

            case KW_SUBSTR:

                break;

            case KW_ORD:

                break;

            case KW_CHR:

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
//        if (symtable_search(active->symtable, current_token->value.vector->array) != NULL) {
//            error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Variable is already declared");
//        }
        //QUITE POSSIBLY NOT NEEDED

        // store variable's name
        rename_keep_exit();
        queue_push(queue, current_token);
        queue_print(queue);
        opt_var_def();

        // insert variable to symtable
        if (queue->first->next == NULL) {
            // TODO: set_data_var: data_type musi byt odvozen z hodnoty (není expicitně zadán)
        }
        else {
            data = set_data_var(data, is_defined, convert_dt(queue->first->next->token), letvar);
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

    // get TOKEN_EQ from buffer 
    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    // here: id = | var id = | let id = 

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    if (current_token->type == TOKEN_ID) {
        peek();
        if (token_buffer->type == TOKEN_LPAR) {
            // TODO: check if the id is forest, so the function is defined? Problem with recursive calling fo two functions //////////////////////////OSETRENO?
            // if (false) {  
            //     error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Function is not defined");
            // }
            queue_push(fn_call_queue, current_token); // second queue, stores the IDs of function calls //TOTO BY TO MOHLO RESIT

            func_call();
        }
        else {
            //expression_parser(); calling with the first token of expression in current_token
            //when expr-parser returns, current_token is first token after the expression
        }
    }
    else {
        //expression_parser(); calling with the first token of expression in current_token
        //when expr-parser returns, current_token is first token after the expression
    }    
}


void func_call() {
    // <func_call> -> id ( <args> )
    printf("-- entering FUNC_CALL --\n");
    print_debug(current_token, 2, debug_cnt);


    // store the function's name for later usage
    char *func_name = malloc(sizeof(char) * 20);
    func_name = strcpy(func_name, current_token->value.vector->array);
    // queue_push(queue, current_token); bacha ve var_def aby se to nebylo, možná druhou queue?

    // get TOKEN_LPAR from buffer
    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    // TODO: hodně validation ze symtable dané funkce, jeslti všechno sedí dle definice
    // jestli matchuje počet argumentů, jména a datové typy atd.
    args();

    if (current_token->type == TOKEN_RPAR) {
        codegen_func_call(func_name);
        free(func_name); func_name = NULL;

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
        // check if the name of the argument in function call matches the name of the parameter in function definition
        //

        peek();
        if (token_buffer->type == TOKEN_COLON) {
            // <arg> -> id : exp
            // get TOKEN_COLON from buffer
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);
            
            current_token = get_next_token();
            print_debug(current_token, 1, debug_cnt++);
        }
    }
    //expression_parser(); calling with the first token of expression in current_token
    //when expr-parser returns, current_token is first token after the expression
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
            if (forest_search_symbol(active, current_token->value.vector->array) == NULL) {
                error_exit(ERROR_SEM_UNDEF_VAR, "PARSER", "Variable is not declared");
            }
            else {
                // TODO: Místo pravdivostního výrazu výraz lze alternativně použít syntaxi: ’let id’, 
                // kde id zastupuje dříve definovanou (nemodifikovatelnou) proměnnou. Je-li pak proměnná id
                // hodnoty nil, vykoná se sekvence_příkazů2, jinak se vykoná sekvence_příkazů1, kde navíc bude typ 
                // id upraven tak, že nebude (pouze v tomto bloku) zahrnovat hodnotu nil 
                // (tj. např. proměnná původního typu String? bude v tomto bloku typu String).
                // get TOKEN_LEFT_BRACKET
                current_token = get_next_token();
                print_debug(current_token, 1, debug_cnt++);


                // TODO: vyřešit let id a PUSHS bool pro codegen
            }
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Missing identifier in condition using if \"let id\"");
        }
    }
    else {
        //expression_parser(); calling with the first token of expression in current_token
        //when expr-parser returns, current_token is first token after the expression
    }


    if (current_token->type == TOKEN_LEFT_BRACKET) {
        
        codegen_if();
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

    //expression_parser(); calling with the first token of expression in current_token
    //when expr-parser returns, current_token is first token after the expression

    if (current_token->type == TOKEN_LEFT_BRACKET) {
       
        codegen_while_start();

        current_token = get_next_token();
        print_debug(current_token, 1, debug_cnt++);

        local_body();

        if (current_token->type == TOKEN_RIGHT_BRACKET) {

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

    cnt_stack = (cnt_stack_t*)malloc(sizeof(cnt_stack_t));
    cnt_init(cnt_stack);
    token_stack = (token_stack_t*)malloc(sizeof(token_stack_t));
    token_init(token_stack);
    queue = (queue_t*)malloc(sizeof(queue_t));
    init_queue(queue);
    fn_call_queue = (queue_t*)malloc(sizeof(queue_t));
    init_queue(fn_call_queue);
    forest_node *global = forest_insert_global();
    active = global;
    //built_in_functions(); // insert built-in functions to the global symtable
    
    //volam prog

    current_token = get_next_token();
    print_debug(current_token, 1, debug_cnt++);

    prog();

    validate_fn_calls();

    traverse_forest(global);

    printf("\n---------------------------\n");
    printf("PARSER: Parsed successfully\n");
    printf("---------------------------\n\n");
    return 0;
}


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


void rename_keep_exit(){

        // The node is in the current symtable, error is thrown as multiple declarations of the same name are not allowed

        if (symtable_lookup(active->symtable, current_token->value.vector->array) == NULL) {
            printf("active->symtable: %p\n", active->symtable);
            printf("Printing active->symtable in order:\n");
            inorder(&(active->symtable));
            printf("\n");

            // The node is not in any symtable above the current node, the name can be kept
            if (forest_search_symbol(active->parent, current_token->value.vector->array) == NULL)
            {
                printf("should be here");
                return;
            } else  // The node is in any symtable above, no matter what, the name must be changed to unique identifier
            {
                printf("ich here");
                int value = renamer3000;
                char bytes[5];

                sprintf(bytes, "%d", value);

                for (int i = 0; bytes[i] != '\0'; i++) {
                    vector_append(current_token->value.vector, bytes[i]);
                }

                renamer3000++;
                return;
            }
        }
        error_exit(ERROR_SEM_UNDEF_FUN, "REDECLARATION", "Multiple declarations of the same name are not allowed");

}

// Used for fn_calls validation as function can be called before its own declaration (recursive function calls)
void validate_fn_calls(){
    while (!queue_is_empty(fn_call_queue)) {
        token_t *token = queue_pop(fn_call_queue);
        if (token == NULL) {
            return;
        }
        if (forest_search_symbol(active, token->value.vector->array) == NULL) {
            error_exit(ERROR_SYN, "PARSER", "Function is not defined");
        }
    }
}