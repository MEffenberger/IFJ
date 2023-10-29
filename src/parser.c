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

/**
 * @brief If the token is a keyword, the forest is updated and the active node is changed
 */
void active_forest_down(){
    if (current_token->value->keyword == KW_FUNC ||
    current_token->value->keyword == KW_IF ||
    current_token->value->keyword == KW_WHILE ||
    current_token->value->keyword == KW_ELSE){
        forest_insert(active);
    }
}
/**
 * @brief If the token is a right bracket, the forest is updated and the active node is changed
 */
void active_forest_up(){
    if (current_token->type == TOKEN_RIGHT_BRACKET) {
        active = active->parent;
    }
}



void peek() {


void parser (){

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
            )






}
