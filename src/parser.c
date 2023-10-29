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

alloc_ptr *allocated_pointers = NULL; // Top of the stack for allocated pointers
forest_node *active = NULL; // Pointer to the active node in the forest
token_t *current_token = NULL; // Pointer to the current token
token_t *token_buffer[MAX_TOKENS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL}; // Buffer for tokens
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
    switch (kw) {
        case KW_GLOBAL:
            return "global";
        case KW_FUNC:
            token_t* peeek = peek();
            return peeek->value.vector->array;
        case KW_IF:
            return "if";
        case KW_ELSE:
            return "else";
        default:
            return "none";
    }
}

/**
 * @brief If the token is a keyword, the forest is updated and the active node is changed
 */
void make_child_in_forest() {
    if (current_token->value.keyword == KW_FUNC ||
        current_token->value.keyword == KW_IF ||
        current_token->value.keyword == KW_WHILE ||
        current_token->value.keyword == KW_ELSE) {
        
        forest_insert(active, convert_kw(current_token->value.keyword), get_name(current_token->value.keyword), &active);
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
 * @brief Looks to another token in the input using get_me_token()
 */
token_t* peek() {
    token_t *token = get_me_token();
    token_buffer[0] = token;
    token_buffer_cnt++;
    return token;
}

/**
 * @brief Function to call when want to get a token to current
 */
token_t* get_next_token() {
    if (token_buffer_cnt == 0) {
        return get_me_token();
    } 
    else {
        token_buffer_cnt--;
        return token_buffer[0];
    }

}

void parser_parse_please (){

    sym_data data = {0};
    forest_node *global = forest_insert_global();
    active = global;
    symtable_init(&global->symtable);

    token_t token = get_me_token();

    if (token.type == T_EOF) {
        symtable_dispose(&global->symtable);
        forest_dispose(global);
        return 0;
    } else if (token->value.keyword == KW_FUNC){
        void function_def();
    } else if (token->value.keyword ==
            token->value.keyword ==
            token->value.keyword ==
            token->value.keyword ==
            token->value.keyword ==
            token->value.keyword ==
            token->value.keyword ==
            );






}
