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
token_t *token_buffer[2] = {NULL, NULL}; // Buffer for tokens
int token_buffer_cnt = 0; // Index of the last token in the buffer


f_keyword_t convert_kw(keyword_t kw) {
    switch (kw) {
        case KW_GLOBAL:
            return W_GLOBAL;
        case KW_FUNC:
            return W_FUNCTION;
        case KW_IF:
            return W_IF;
        case KW_ELSE:
            return W_ELSE;
        default:
            return W_NONE;
    }
}


char *get_name(keyword_t kw) {
    token_t * peeek;
    switch (kw) {
        case KW_GLOBAL:
            return "global";
        case KW_FUNC:
            peek();
            return token_buffer[0]->value.vector->array;
        case KW_IF:
            return "if";
        case KW_ELSE:
            return "else";
        default:
            return "none";
    }
}



/**
 * @brief If the token is a right bracket, the forest is updated and the active node is changed
 */
void back_to_parent_in_forest(){
    if (current_token->type == TOKEN_RIGHT_BRACKET) {
        active = active->parent;
    }
}


/**
 * @brief Looks to another token in the input using get_m e_token()
 */
void peek() {
    token_t *token = get_me_token();
    token_buffer[token_buffer_cnt] = token;
    token_buffer_cnt++;
}

/**
 * @brief Returns the index of the first non-NULL token in the buffer
 * 
 * @return int Index of the first non-NULL token in the buffer
 */
int first_index_in_token_buffer() {

    for (int i = 0; i < token_buffer_cnt; i++) {
        if (token_buffer[i] != NULL) {
            return i;
        }
    }
    
}



/**
 * @brief Function to call when want to get a token to current
 */
token_t* get_next_token() {
    if (token_buffer_cnt == 0) {
        if (current_token != NULL) {
            free(current_token); // token_delete();
        }
        return get_me_token();
    } 
    else {


        token_t* tmp = token_buffer[first_index_in_token_buffer()];
        token_buffer[first_index_in_token_buffer()] = NULL;
        return tmp;








        token_buffer_cnt--;
        token_t* tmp = token_buffer[0];
        token_buffer[0] = NULL;
        return tmp;
    }

}

void prog () {
    // <prog> -> EOF | <func_def> <prog> | <body> <prog>
    printf("PROG\n");
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);

    if (current_token->type == TOKEN_EOF) {
        return;
    }
    else if (current_token->value.keyword == KW_FUNC) {
        //MAKE_CHILDREN_IN_FOREST();
        func_def();
        printf("\nEND OF FUNCDEF\n");
        prog();
    }
    else {
        body();
        printf("\nEND OF BODY\n");
        prog();
    }
}

void func_def () {
    // <func_def> -> func id ( <params> ) <ret_type> { <func_body> }
    printf("FUNC_DEF\n");
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);

    if (current_token->type == TOKEN_ID) {
        current_token = get_next_token();
            printf("curr_token: %d\n", current_token->type);

        if (current_token->type == TOKEN_LPAR) {
            current_token = get_next_token();
            printf("curr_token: %d\n", current_token->type);

            params();
            if (current_token->type == TOKEN_RPAR) {
                current_token = get_next_token();
                    printf("curr_token: %d\n", current_token->type);

                ret_type();

                if (current_token->type == TOKEN_LEFT_BRACKET) {
                    printf("LEFT BRACKET\n");

                    func_body();

                    //mam uz tu zavorku?
                    
                    if (current_token->type == TOKEN_RIGHT_BRACKET) {
                        printf("RIGHT BRACKET\n");
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
                error_exit(ERROR_SYN, "PARSER", "Missing right paranthesis");
            }
    
        }
        else {
            error_exit(ERROR_SYN, "PARSER", "Missing left paranthesis");
        }
    }
    else {
        error_exit(ERROR_SEM_UNDEF_FUN, "PARSER", "Missing identifier of funciton");
    }

}

void params() {
    // <params> -> eps | <par_name> <par_id> : <type> <params_n>
    printf("PARAMS\n");


    
    par_name();
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);


    par_id();
    current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);


    if (current_token->type == TOKEN_COLON) {
        current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);

        type();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing colon in function's parameter");
    }

    current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);

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

void par_name() {
    // <par_name> -> _ | id
    printf("PAR_NAME\n");
    printf("par_name: %s\n", current_token->value.vector->array);
    if (current_token->type == TOKEN_UNDERSCORE || current_token->type == TOKEN_ID) {
        return;
    }
    else { // not _ or id
        error_exit(ERROR_SEM_TYPE, "PARSER", "Error name not _ or id");
    }
}

void par_id() {
    // <par_id> -> _ | id
    printf("PAR_ID\n"); 
    printf("par_id: %s\n", current_token->value.vector->array);
    if (current_token->type == TOKEN_UNDERSCORE || current_token->type == TOKEN_ID) {
        return;
    }
    else { // not _ or id
        error_exit(ERROR_SEM_TYPE, "PARSER", "Error id not _ or id");
    }
}

void type() {
    // <type> -> Int | Int? | Double | Double? | String | String? | nil
    printf("TYPE\n");   
    if (current_token->type == TOKEN_KEYWORD || current_token->type == TOKEN_KEYWORD_QM) {
        if (current_token->value.keyword == KW_INT || current_token->value.keyword == KW_DOUBLE || current_token->value.keyword == KW_STRING || current_token->value.keyword == KW_NIL) {
            //symbol info
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
    printf("PARAMS_N\n");   
    
        peek();
        if (token_buffer[0]->type != TOKEN_ID) {
            error_exit(ERROR_SYN, "PARSER", "Missing identifier of function's parameter");
        }
        else {
            current_token = get_next_token();
            printf("curr_token: %d\n", current_token->type);
            params();
        }
    
}

void ret_type() {
    // ret_type -> eps | -> <type>
    printf("RET_TYPE\n");
    if (current_token->type == TOKEN_RET_TYPE) {
        current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);
        type();
        current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);

        return;
    }
    else { // void function
        if (token_buffer[0]->type != TOKEN_LEFT_BRACKET) {
            error_exit(ERROR_SYN, "PARSER", "Unexpected token in function's return type");
        }
        else {
            return;
        }
    }
}


void func_body() {
    // <func_body> -> <body> <ret> <body>
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);


}


void body() {
    // <body> -> eps | <var_def> | <condition> | <cycle> | <assign> | <func_call>
    printf("BODY\n");

    if (current_token->type == TOKEN_EOL) { // if EOL: <body> = eps
        return;
    }


    switch (current_token->type){
        case TOKEN_ID:
            
            peek();
            if (token_buffer[0]->type == TOKEN_EQ) {
                assign();
            }
            else if (token_buffer[0]->type == TOKEN_LPAR) {
                // current token je ID pri vstupu do func_call
                func_call();
            }
            else { // problem s EOLem auaua zahazovat nevim vubec
                error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
            }


            


        case TOKEN_KEYWORD:
            switch (current_token->value.keyword) {
                case KW_LET:
                case KW_VAR:
                    var_def();



                    break;
                case KW_IF:
                    condition();
                    break;
                case KW_WHILE:

                    cycle();                       
                    break;
                default:
                    break;
            }



        default:
            error_exit(ERROR_SYN, "PARSER", "Unexpected token in body");
            break;

    }
    




}

void var_def() {
    // <var_def> -> let id <opt_var_def> | var id <opt_var_def>
    printf("VAR_DEF\n");
    // zpracovat VAR nebo LET informaci do symbolu
   
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);

    if (current_token->type == TOKEN_ID) {
        opt_var_def();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing identifier in variable definition");
    }


}

void opt_var_def() {
    // <opt_var_def> -> : <type> | <assign> | : <type> <assign>
    printf("OPT_VAR_DEF\n");
    
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);



    if (token_buffer[0]->type == TOKEN_COLON) {
        // kdyz bude ":" tak ho nacti a nacti ocekavany type na nim
        current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);

        current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);

        type();
        // current_token je ten type (Int)


        if (token_buffer[0]->type == TOKEN_EQ) {

            assign();
        }
        else {
            return;
        }
    }
    else { // neni tam ":" (je tam "=" / ((((nebo neco jineho)))))
        assign();
    }

}

void assign() {
    // <assign> -> = <exp> | = <func_call>
    printf("ASSIGN\n");

    // jsi v assign, nacti "=" a pokracuj
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);

    // id = / letvar id =   momentalne

    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);
    // musi byt ID (za =)
    if (current_token->type == TOKEN_ID) {

        // --------------------------------------------------------------------
        //  help: double peek?! how to recognize if it is func_call or exp?
        // --------------------------------------------------------------------

        if (token_buffer[0]->type == TOKEN_LPAR) {
            func_call();
        }
        else {
            // expression parser call (first id is in current_token, second is in peek buffer)
        }

    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Unexpected token during assigning");
    }
}


void func_call() {
    // <func_call> -> id ( <args> )
    printf("FUNC_CALL\n");
    
    // volam z body: current token je ID pri vstupu do func_call (v bufferu je LPAR)

    // volam z assign:

    // načtu leovu zavorku
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);

    args();

    // načtu pravou zavorku
    // current_token = get_next_token();
    // printf("curr_token: %d\n", current_token->type);
    
    // args se vrati a current token je prava zavorka
    
    if (current_token->type == TOKEN_RPAR) {
        return;
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Missing right paranthesis in function call");
    }
}



void args() {
    // <args> -> eps | <arg> <args_n>
    printf("ARGS\n");
    peek();
    if (token_buffer[0]->type == TOKEN_RPAR) {
        // <args> -> eps
        return;
    }
    else {
        // <args> -> <arg> <args_n>
        arg();
        args_n();
        return;
    }
}

void arg() {
    // <arg> -> exp | id : exp
    printf("ARG\n");

    // nactem to neco co neni RPAR nebo carka
    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);

    if (current_token->type == TOKEN_ID) {
        peek();
        if (token_buffer[0]->type == TOKEN_COLON) {
            // <arg> -> id : exp
            // načtu ":"
            current_token = get_next_token();
            printf("curr_token: %d\n", current_token->type);
            
            // načtu první věc exp
            current_token = get_next_token();
            printf("curr_token: %d\n", current_token->type);

            // expression parser call (first id is in current_token)
        }
        else {
            // <arg> -> exp
            // expression parser call (first id is in current_token)
        }
    }
    else {
        // <arg> -> exp
        // expression parser call (first id is in current_token)
    }


}

void args_n() {
    // <args_n> -> eps | , <arg> <args_n>
    printf("ARGS_N\n");
    peek();
    if (token_buffer[0]->type == TOKEN_RPAR) {
        // <args_n> -> eps
        return;
    }
    else if (token_buffer[0]->type == TOKEN_COMMA) {
        // <args_n> -> , <args>
        current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);

        args();
    }
    else {
        error_exit(ERROR_SYN, "PARSER", "Neni zavorka ani carka chyba pomoc");
    }
}

void condition() {
    // <condition> ->   if <exp> { <body> } <else> { <body> } | if let id { <body> } <else> { <body> }
    printf("CONDITION\n");

    current_token = get_next_token();
    printf("curr_token: %d\n", current_token->type);

    if (current_token->type == TOKEN_KEYWORD && current_token->value.keyword == KW_LET) {
        current_token = get_next_token();
        printf("curr_token: %d\n", current_token->type);

        if (current_token->type == TOKEN_ID) {

            //zalezi co vraci expression parser
            // current_token = get_next_token();
            // printf("curr_token: %d\n", current_token->type);

        }
        else {
            //error_exit chyba
        }

    }
    else {
        // expression parser call (first id is in current_token)
    }

    if (current_token->type == TOKEN_LEFT_BRACKET) {
        // body ocekava ze prvni token pro neho relevantni je v current
        body();
        
    }



    
}




void ret();
void cycle();

int parser_parse_please () {

    // sym_data data = {0};
    // forest_node *global = forest_insert_global();
    // active = global;

    
    //volam prog

    prog();





    return 0;
}
