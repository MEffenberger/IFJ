/**
 * @file main.c
 * 
 * IFJ23 compiler
 * 
 * @brief Main function of IFJ23 compiler
 *
 * @author Adam Valik <xvalik05>
 * @author Jmeno Prijemni <xlogin00>
 * @author Jmeno Prijemni <xlogin00>
 * @author Jmeno Prijemni <xlogin00>
 */

//#include "parser.h"
#include "parser.h"
#include "expression_parser.h"

alloc_ptr *allocated_pointers = NULL; // Top of the stack for allocated pointers
forest_node *active = NULL; // Pointer to the active node in the forest
token_t *current_token = NULL; // Pointer to the current token
token_t *token_buffer = NULL; // Buffer for tokens
queue_t *queue = NULL; // Queue for the expression parser
sym_data data = {0};
var_type letvar = -1;
bool is_defined = false;
int debug_cnt = 1;
int ifelse_cnt = 1;
int while_cnt = 1;

int main() { 

    /*token_stack stack;
    stack_init(&stack);
    token_t* tmp = token_create(TOKEN_DOLLAR);
    stack_push(&stack, tmp);
    tmp = token_create(TOKEN_SHIFT);
    stack_push(&stack, tmp);
    tmp = token_create(TOKEN_EXPRESSION);
    stack_push(&stack, tmp);
    tmp = token_create(TOKEN_EXPRESSION);
    stack_push(&stack, tmp);

    tmp = stack_top_nonterminal(&stack);
    printf("\n%d\n", tmp->type);

    int index = stack_top_terminal(&stack);
    printf("%d", index);*/


    current_token = get_me_token();
    call_expr_parser(TOKEN_BOOL);

    return 0;
}
