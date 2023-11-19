/**
 * @file expression_parser.c
 * 
 * IFJ23 compiler
 * 
 * @brief Expression parser implemetation
 *
 * @author Dominik Horut <xhorut01>
 * @author Samuel Hejnicek <xhejni00>
 */
#include "expression_parser.h"

#define TABLE_SIZE 16

static char precedence_table[TABLE_SIZE][TABLE_SIZE] = {

/*    *//* +   -   *   /   <   >   <=  >=  !=  ==  !   ??  (   )   i   $   */
/* +  */ {'>','>','<','<','>','>','>','>','>','>','<','>','<','>','<','>'},
/* -  */ {'>','>','<','<','>','>','>','>','>','>','<','>','<','>','<','>'},
/* *  */ {'>','>','>','>','>','>','>','>','>','>','<','>','<','>','<','>'},
/* /  */ {'>','>','>','>','>','>','>','>','>','>','<','>','<','>','<','>'},
/* <  */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* >  */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* <= */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* >= */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* != */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* == */ {'<','<','<','<',' ',' ',' ',' ',' ',' ','<','>','<','>','<','>'},
/* !  */ {'>','>','>','>','>','>','>','>','>','>',' ','>',' ','>','>','>'},
/* ?? */ {'<','<','<','<','<','<','<','<','<','?','<','<','<','>','<','>'},
/* (  */ {'<','<','<','<','<','<','<','<','<','<',' ','<','<','=','<',' '},
/* )  */ {'>','>','>','>','>','>','>','>','>','>','>','>',' ','>',' ','>'},
/* i  */ {'>','>','>','>','>','>','>','>','>','>','>','>',' ','>',' ','>'},
/* $  */ {'<','<','<','<','<','<','<','<','<','<','<','<','<',' ','<',' '},

};


int get_index(token_type_t token){

    switch(token){

        case TOKEN_PLUS:
        return 0;

        case TOKEN_MINUS:
        return 1;

        case TOKEN_MULTIPLY:
        return 2;

        case TOKEN_DIVIDE:  
        return 3;

        case TOKEN_LESS:
        return 4;

        case TOKEN_GREAT:
        return 5;

        case TOKEN_LESS_EQ:
        return 6;

        case TOKEN_GREAT_EQ:
        return 7;

        case TOKEN_EXCLAMEQ:
        return 8;

        case TOKEN_EQEQ:
        return 9;

        case TOKEN_EXCLAM:
        return 10;

        case TOKEN_DOUBLE_QM:
        return 11;

        case TOKEN_LPAR:
        return 12;

        case TOKEN_RPAR:
        return 13;

        case TOKEN_ID:
        return 14;

        //case TOKEN_EXPRESSION:
        //return 14;

        case TOKEN_NUM:
        return 14;

        case TOKEN_DEC:
        return 14;

        case TOKEN_EXP:
        return 14;

        case TOKEN_STRING:
        return 14;

        case TOKEN_DOLLAR:
        return 15;

        default:
        return 15;

    }

}


token_t* token_create(token_type_t token_type){
    token_t* token = malloc(sizeof(token_t));
    token->value.vector = NULL;
    token->value.integer = 0;
    token->value.type_double = 0.0;
    token->type = token_type;
    return token;
}

void token_shift(token_stack* stack){
    token_t* shift = token_create(TOKEN_SHIFT);
    token_t* top = stack_top(stack);
    if(top->type == TOKEN_EXPRESSION){
        stack_pop(stack);
        stack_push(stack, shift);
        stack_push(stack, top);
    } else {
        stack_push(stack, shift);
    }

    stack_push(stack, current_token);
}

expression_rules_t find_reduce_rule(token_t* token1, token_t* token2, token_t* token3, int number_of_tokens){
    switch (number_of_tokens)
    {
    case 1: //E->i
        return RULE_OPERAND;
        break;

    case 3:
        switch (token2->type)
        {
        case TOKEN_PLUS:
            return RULE_ADD;
            break;
        default:
            break;
        }
    default:
        error_exit(2, "expression_parser", "Rule does not exist");
        break;
    }
}

void call_expr_parser(token_type_t return_type){
    token_stack stack;
    stack_init(&stack);

    //Dollar at the bottom of the stack
    token_t* stack_bottom = token_create(TOKEN_DOLLAR);
    stack_push(&stack, stack_bottom);

    char table_result;

    bool eval_expr = true; 

    while(eval_expr){
        
        token_t* terminal = stack_top_terminal(&stack);
        int stack_index = get_index(terminal->type);
        int next_token_index = get_index(current_token->type);
        table_result = precedence_table[stack_index][next_token_index];


        if(next_token_index == 15 && stack_top_terminal(&stack)->type == TOKEN_DOLLAR){

            if(stack.size <= 1){
                
                error_exit(2, "expression_parser", "empty expression");
            }
            eval_expr = false;
            break;
        }

        switch (table_result)
        {
        case '<':
            token_shift(&stack);
            current_token = get_next_token();
            break;
        case '>':
            int rule_params_count = 0;
            token_t* tmp1, *tmp2, *tmp3;
            if(stack_top(&stack)->type != TOKEN_SHIFT){
                tmp1 = stack_top(&stack);
                stack_pop(&stack);
                rule_params_count++;

                if(stack_top(&stack)->type != TOKEN_SHIFT){
                    tmp2 = stack_top(&stack);
                    stack_pop(&stack);
                    rule_params_count++;

                    if(stack_top(&stack)->type != TOKEN_SHIFT){
                        tmp3 = stack_top(&stack);
                        stack_pop(&stack);
                        rule_params_count++;

                        if(stack_top(&stack)->type != TOKEN_SHIFT){
                            error_exit(2, "expression_parser", "syntax error");
                        }
                    }
                }
            }


            stack_pop(&stack);
            expression_rules_t rule = find_reduce_rule(tmp1, tmp2, tmp3, rule_params_count);

            switch (rule)
            {
            case RULE_OPERAND:
                if(tmp1->type == TOKEN_ID){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->value.keyword = KW_EXP_ID_INT;
                    //hledani v symtable
                } else if(tmp1->type == TOKEN_DEC){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->value.keyword = KW_EXP_CONST_DOUBLE;
                } else if(tmp1->type == TOKEN_NUM){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->value.keyword = KW_EXP_CONST_INT;
                } else if (tmp1->type == TOKEN_EXP){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->value.keyword = KW_EXP_CONST_DOUBLE;
                } else if (tmp1->type == TOKEN_STRING){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->value.keyword = KW_EXP_CONST_STRING;  
                }
                stack_push(&stack, tmp1);
                //stack_push(&stack, current_token);
                break;
            
            case RULE_ADD:
                //check datovych typu
                //printneme 2xPUSHS ADDS
                stack_push(&stack, tmp1);
                //stack_push(&stack, current_token);
                break;
            default:
                break;
            }

            break;
        case '=':
            break;
        
        default:
            error_exit(2, "expression_parser", "syntax error");
        }

    }


    
    for(int i = 0; i < stack.size; i++){
        printf("TOKEN %d hodnota %d\n", i, stack.token_array[i]->type);
    }
    printf("%d", stack.size);
    //printf("%d", current_token->type);
}