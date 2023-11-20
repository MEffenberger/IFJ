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
/* (  */ {'<','<','<','<','<','<','<','<','<','<','<','<','<','=','<',' '},
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

    case 2:
        if(token1->type == TOKEN_EXCLAM && token2->type == TOKEN_EXPRESSION){
            return RULE_EXCL;
        } else {
            error_exit(2, "expression_parser", "Wrong operator");
        }
        break;
    case 3:

        if(token1->type == TOKEN_RPAR && token3->type == TOKEN_LPAR){
            return RULE_PAR;
        }

        switch (token2->type)
        {
        case TOKEN_PLUS:
            return RULE_ADD;
        case TOKEN_MULTIPLY:
            return RULE_MUL;
        case TOKEN_MINUS:
            return RULE_SUB;
        case TOKEN_DIVIDE:
            return RULE_DIV;
        case TOKEN_LESS:
            return RULE_LESS;
        case TOKEN_LESS_EQ:
            return RULE_LEQ;
        case TOKEN_GREAT:
            return RULE_GTR;   
        case TOKEN_GREAT_EQ:
            return RULE_GEQ;       
        case TOKEN_EQEQ:
            return RULE_EQ;    
        case TOKEN_EXCLAMEQ:
            return RULE_NEQ;  
        case TOKEN_DOUBLE_QM:
            return RULE_QMS;  
        default:
            error_exit(2, "expression_parser", "Wrong operator");
            return NOT_RULE;
            break;
        }
    default:
        error_exit(2, "expression_parser", "Rule does not exist");
        return NOT_RULE;
        break;
    }
    return NOT_RULE;
}


void check_types(token_t* tmp1, token_t* tmp2, token_t* tmp3){

    if(tmp2->type == TOKEN_PLUS || tmp2->type == TOKEN_MINUS || tmp2->type == TOKEN_MULTIPLY){
        if (tmp1->exp_value == tmp3->exp_value){
            if(tmp1->exp_value == STRING && tmp3->exp_value == STRING && tmp2->type != TOKEN_PLUS){
                error_exit(2, "expression_parser", "Wrong operator in concatenation");
            }
        } else{
            if (tmp1->exp_type == CONST){
                if (tmp3->exp_type == ID){
                    if(tmp3->exp_value == DOUBLE){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = DOUBLE;
                    } else {
                        error_exit(2, "expression_parser", "ID type mismatch");
                    }
                }
                else{
                    if (tmp1->exp_value == STRING || tmp3->exp_value == STRING){
                        error_exit(2, "expression_parser", "This cannot participate in concatenation");
                    }

                    if (tmp1->exp_value == INT){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = DOUBLE;
                    }
                    else if (tmp3->exp_value == INT){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                    }
                }
            }
            else if (tmp3->exp_type == CONST){
                if (tmp1->exp_type == ID){
                    if(tmp1->exp_value == DOUBLE){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                    } else {
                        error_exit(2, "expression_parser", "ID type mismatch");
                    }
                    
                }
                else{
                    if (tmp1->exp_value == STRING || tmp3->exp_value == STRING){
                        error_exit(2, "expression_parser", "This cannot participate in concatenation");
                    }

                    if (tmp1->exp_value == INT){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = DOUBLE;
                    }
                    else if (tmp3->exp_value == INT){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                    }
                }
            }
            else{
                error_exit(2, "expression_parser", "Can not sum 2 IDs of different types");
            }
        }
    } else if(tmp2->type == TOKEN_DIVIDE){
        if (tmp1->exp_value == tmp3->exp_value){
            if(tmp1->exp_value == STRING && tmp3->exp_value == STRING){
                error_exit(2, "expression_parser", "Wrong operator in concatenation");
            }

            if(tmp1->exp_value == INT && tmp1->exp_value == INT){
                printf("IDIVS\n");
            } else {
                printf("DIVS\n");
                tmp1->exp_value = DOUBLE;
            }
        } else {
            if (tmp1->exp_type == CONST){
                if (tmp3->exp_type == ID){
                    if(tmp3->exp_value == DOUBLE){
                        printf("INT2FLOATS\n");
                        printf("DIVS\n");
                        tmp1->exp_value = DOUBLE;
                    } else {
                        error_exit(2, "expression_parser", "ID type mismatch");
                    }
                } else{
                    if (tmp1->exp_value == STRING || tmp3->exp_value == STRING){
                        error_exit(2, "expression_parser", "This cannot participate in concatenation");
                    }

                    if (tmp1->exp_value == INT){
                        printf("INT2FLOATS\n");
                        printf("DIVS\n");
                        tmp1->exp_value = DOUBLE;
                    }
                    else if (tmp3->exp_value == INT){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        printf("DIVS\n");
                    }
                }
            }
            else if (tmp3->exp_type == CONST){
                if (tmp1->exp_type == ID){
                    if(tmp1->exp_value == DOUBLE){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        printf("DIVS\n");
                    } else {
                        error_exit(2, "expression_parser", "ID type mismatch");
                    }
                }
                else{
                    if (tmp1->exp_value == STRING || tmp3->exp_value == STRING){
                        error_exit(2, "expression_parser", "This cannot participate in concatenation");
                    }

                    if (tmp1->exp_value == INT){
                        printf("INT2FLOATS\n");
                        printf("DIVS\n");
                        tmp1->exp_value = DOUBLE;
                    }
                    else if (tmp3->exp_value == INT){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        printf("DIVS\n");
                    }
                }
            }
            else{
                error_exit(2, "expression_parser", "Can not divide 2 IDs of different types");
            }
        }
    } else if(tmp2->type == TOKEN_EQEQ){
        if(tmp1->exp_value == tmp3->exp_value){
            tmp1->exp_value = BOOL;
        } else {
            if (tmp1->exp_type == CONST){
                if (tmp3->exp_type == ID){
                    if(tmp3->exp_value == DOUBLE){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = BOOL;
                    } else {
                        error_exit(7, "expression_parser", "Can not compare 2 values of different types");
                    }
                } else{

                    if (tmp1->exp_value == INT){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = BOOL;
                    }
                    else if (tmp3->exp_value == INT){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        tmp1->exp_value = BOOL;
                    }
                }
            }
            else if (tmp3->exp_type == CONST){
                if (tmp1->exp_type == ID){
                    if(tmp1->exp_value == DOUBLE){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        tmp1->exp_value = BOOL;
                    } else {
                        error_exit(7, "expression_parser", "Can not compare 2 values of different types");
                    }
                }
                else{
                    if (tmp1->exp_value == INT){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = BOOL;
                    }
                    else if (tmp3->exp_value == INT){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        tmp1->exp_value = BOOL;
                    }
                }
            }
            else{
                error_exit(7, "expression_parser", "Can not compare 2 values of different types");
            }
        }
    } else if(tmp2->type == TOKEN_EXCLAMEQ){
        if(tmp1->exp_value == tmp3->exp_value){
            tmp1->exp_value = BOOL;
        } else {
            if (tmp1->exp_type == CONST){
                if (tmp3->exp_type == ID){
                    if(tmp3->exp_value == DOUBLE){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = BOOL;
                    } else {
                        error_exit(7, "expression_parser", "Can not compare 2 values of different types");
                    }
                } else{

                    if (tmp1->exp_value == INT){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = BOOL;
                    }
                    else if (tmp3->exp_value == INT){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        tmp1->exp_value = BOOL;
                    }
                }
            } else if (tmp3->exp_type == CONST){
                if (tmp1->exp_type == ID){
                    if(tmp1->exp_value == DOUBLE){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        tmp1->exp_value = BOOL;
                    } else {
                        error_exit(7, "expression_parser", "Can not compare 2 values of different types");
                    }
                }
                else{
                    if (tmp1->exp_value == INT){
                        printf("INT2FLOATS\n");
                        tmp1->exp_value = BOOL;
                    }
                    else if (tmp3->exp_value == INT){
                        printf("DEFVAR GF@$$tmp$$\n");
                        printf("POPS GF@$$tmp$$\n");
                        printf("INT2FLOATS\n");
                        printf("PUSHS GF@$$tmp$$\n");
                        tmp1->exp_value = BOOL;
                    }
                }
            }
            else{
                error_exit(7, "expression_parser", "Can not compare 2 values of different types");
            }
        }
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
                    tmp1->exp_type = ID;
                    tmp1->exp_value = INT;
                    printf("PUSHS typ@%s\n", tmp1->value.vector->array);
                    //hledani v symtable
                } else if(tmp1->type == TOKEN_DEC){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = CONST;
                    tmp1->exp_value = DOUBLE;
                    printf("PUSHS float@%a\n", tmp1->value.type_double);
                } else if(tmp1->type == TOKEN_NUM){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = CONST;
                    tmp1->exp_value = INT;
                    printf("PUSHS int@%d\n", tmp1->value.integer);
                } else if (tmp1->type == TOKEN_EXP){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = CONST;
                    tmp1->exp_value = DOUBLE;
                    printf("PUSHS float@%a\n", tmp1->value.type_double);
                } else if (tmp1->type == TOKEN_STRING){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = CONST;
                    tmp1->exp_value = STRING;
                    printf("PUSHS string@%s\n", tmp1->value.vector->array);
                }
                stack_push(&stack, tmp1);
                break;

            case RULE_EXCL:
                stack_push(&stack, tmp2);
                break;

            
            case RULE_PAR:
                stack_push(&stack, tmp2);
                break;

            case RULE_ADD:
                check_types(tmp1,tmp2, tmp3);
                printf("ADDS\n");
                stack_push(&stack, tmp1);
                break;
            case RULE_MUL:
                check_types(tmp1, tmp2, tmp3);
                printf("MULS\n");
                stack_push(&stack, tmp1);
                break;
            case RULE_SUB:
                check_types(tmp1, tmp2, tmp3);
                printf("SUBS\n");
                stack_push(&stack, tmp1);
                break;
            case RULE_DIV:
                check_types(tmp1, tmp2, tmp3);
                stack_push(&stack, tmp1);
                break;
            case RULE_LESS:
                stack_push(&stack, tmp1);
                break;
            case RULE_LEQ:
                stack_push(&stack, tmp1);
                break;
            case RULE_GTR:
                stack_push(&stack, tmp1);
                break;
            case RULE_GEQ:
                stack_push(&stack, tmp1);
                break;
            case RULE_EQ:
                check_types(tmp1, tmp2, tmp3);
                printf("EQS\n");
                stack_push(&stack, tmp1);
                break;
            case RULE_NEQ:
                check_types(tmp1, tmp2, tmp3);
                printf("EQS\n");
                printf("NOTS\n");
                stack_push(&stack, tmp1);
                break;
            case RULE_QMS:
                stack_push(&stack, tmp1);
                break;
            default:
                break;
            }

            break;
        case '=':
            stack_push(&stack, current_token);
            current_token = get_next_token();
            break;
        
        default:
            error_exit(2, "expression_parser", "syntax error");
        }

    }

    //printf("%d", stack.size);
    //printf("%d", current_token->type);
}