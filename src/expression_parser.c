/**
 * @file expression_parser.c
 * 
 * IFJ23 compiler
 * 
 * @brief Expression parser implementation
 *
 * @author Dominik Horut <xhorut01>
 * @author Samuel Hejnicek <xhejni00>
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

#define TABLE_SIZE 16 //Number of lines and columns in precedence table

int variable_counter = 1; //Counter for unique variables needed in codegen
bool concat = false; //True if concat will apply
bool stop_expression = false; //True if expression will be reduced and no other new tokens will be accepted


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

        case TOKEN_KEYWORD:
        return 14;

        case TOKEN_NUM:
        return 14;

        case TOKEN_DEC:
        return 14;

        case TOKEN_EXP:
        return 14;

        case TOKEN_STRING:
        return 14;

        case TOKEN_ML_STRING:
        return 14;

        case TOKEN_DOLLAR:
        return 15;

        default:
        return 15;

    }

}

token_t* token_create(token_type_t token_type){
    token_t* token = (token_t*)allocate_memory(sizeof(token_t));
    token->value.vector = NULL;
    token->value.integer = 0;
    token->value.type_double = 0.0;
    token->type = token_type;
    return token;
}


void token_shift(token_stack* stack){
    token_t* shift = token_create(TOKEN_SHIFT);
    token_t* top = stack_top(stack);

    //expression is on top, shift is pushed before it
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

    case 2: //E->E!
        if(token1->type == TOKEN_EXCLAM && token2->type == TOKEN_EXPRESSION){
            return RULE_EXCL;
        } else {
            error_exit(ERROR_SYN, "EXPRESSION PARSER", "Wrong operator");
        }
        break;
    case 3:

        if(token1->type == TOKEN_RPAR && token3->type == TOKEN_LPAR){
            return RULE_PAR; //E->(E)
        }

        switch (token2->type) //Rule found via operator in token2
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
            error_exit(ERROR_SYN, "EXPRESSION PARSER", "Wrong operator");
            return NOT_RULE;
            break;
        }
    default:
        error_exit(ERROR_SYN, "EXPRESSION PARSER", "Rule does not exist");
        return NOT_RULE;
        break;
    }
    return NOT_RULE;
}

//Converts nillable types to nonnillable types
void convert_qm(token_t* tmp1){
    if(tmp1->exp_value == INT_QM){
        tmp1->exp_value = INT;
    } else if(tmp1->exp_value == DOUBLE_QM){
        tmp1->exp_value = DOUBLE;
    } else if(tmp1->exp_value == STRING_QM){
        tmp1->exp_value = STRING;
    }
}



void check_types(token_t* tmp1, token_t* tmp2, token_t* tmp3){


    //First case, same for +,-,*
    if(tmp2->type == TOKEN_PLUS || tmp2->type == TOKEN_MINUS || tmp2->type == TOKEN_MULTIPLY){

        if(tmp1->exp_value == NIL || tmp3->exp_value == NIL){
        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do arithmetic opeation with nils");
        }
            
        if(tmp1->exp_value == BOOL || tmp3->exp_value == BOOL){
            error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do arithmetic opeation with booleans");
        }

        //Same token types
        if (tmp1->exp_value == tmp3->exp_value){

            if((tmp1->exp_value == INT_QM && tmp3->exp_value == INT_QM) || (tmp1->exp_value == DOUBLE_QM && tmp3->exp_value == DOUBLE_QM) || (tmp1->exp_value == STRING_QM && tmp3->exp_value == STRING_QM)){
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do operation with 2 QM types without ! them");
            }

            if((tmp1->exp_type == CONST && tmp3->exp_type == ID) || (tmp1->exp_type == ID && tmp3->exp_type == CONST)){
                //tmp1 is always token to be pushed onto stack, so it is changed to id
                tmp1->exp_type = ID;
            }
            //Strings are only allowed in concat
            if(tmp1->exp_value == STRING && tmp3->exp_value == STRING && tmp2->type != TOKEN_PLUS){
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Wrong operator in concatenation");

            } else if(tmp1->exp_value == STRING && tmp3->exp_value == STRING && tmp2->type == TOKEN_PLUS){
                concat = true;

                // CODEGEN
                vardef_outermost_while(CONCAT_DEFVAR, NULL, variable_counter);
                instruction *inst = inst_init(CONCAT, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                variable_counter++;
                variable_counter++;
                return;

            }
        //types are not the same
        } else{

            //if one is id and other constant, it could be converted into double if possible
            if (tmp1->exp_type == CONST){
                if (tmp3->exp_type == ID){
                    if(tmp3->exp_value == DOUBLE || tmp3->exp_value == DOUBLE_QM){
                        
                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        tmp1->exp_value = DOUBLE;
                    } else {
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "ID type mismatch");
                    }
                }
                else{
                    if (tmp1->exp_value == STRING || tmp3->exp_value == STRING){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "This cannot participate in concatenation");
                    }

                    if (tmp1->exp_value == INT || tmp1->exp_value == INT_QM){
                        
                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        tmp1->exp_value = DOUBLE;
                    }
                    else if (tmp3->exp_value == INT || tmp3->exp_value == INT_QM){
                        
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        variable_counter++;
                    }
                }
            } else if (tmp3->exp_type == CONST){
                if (tmp1->exp_type == ID){
                    if(tmp1->exp_value == DOUBLE || tmp1->exp_value == DOUBLE_QM){
                        
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                        
                        variable_counter++;
                    } else {
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "ID type mismatch");
                    }
                    
                }
                else{
                    if (tmp1->exp_value == STRING || tmp3->exp_value == STRING){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "This cannot participate in concatenation");
                    }

                    if (tmp1->exp_value == INT || tmp1->exp_value == INT_QM){
                        
                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        tmp1->exp_value = DOUBLE;
                    }
                    else if (tmp3->exp_value == INT || tmp3->exp_value == INT_QM){
                        
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                        
                        variable_counter++;
                    }
                }
            }
            else{  
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not sum 2 IDs of different types");
            }
        }

    //Case for division
    } else if(tmp2->type == TOKEN_DIVIDE){

        if(tmp1->exp_value == NIL || tmp3->exp_value == NIL){
            error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do arithmetic opeation with nils");
        }

        if(tmp1->exp_value == BOOL || tmp3->exp_value == BOOL){
            error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not divide bool");
        }

        //types are same
        if (tmp1->exp_value == tmp3->exp_value){

            if((tmp1->exp_type == CONST && tmp3->exp_type == ID) || (tmp1->exp_type == ID && tmp3->exp_type == CONST)){
                tmp1->exp_type = ID;
            }

            if((tmp1->value.integer == 0 && tmp1->exp_value == INT && tmp1->exp_type == CONST) || (tmp1->value.type_double == 0 && tmp1->exp_value == DOUBLE && tmp1->exp_type == CONST)){
                    error_exit(ERROR_SEM_OTHER, "EXPRESSION PARSER", "Division by zero");
            }
            //String can not be in division
            if((tmp1->exp_value == STRING || tmp1->exp_value == STRING_QM) && (tmp3->exp_value == STRING || tmp3->exp_value == STRING_QM)){
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Wrong operator in concatenation");
            }

            if((tmp1->exp_value == INT_QM && tmp3->exp_value == INT_QM) || (tmp1->exp_value == DOUBLE_QM && tmp3->exp_value == DOUBLE_QM) || (tmp1->exp_value == STRING_QM && tmp3->exp_value == STRING_QM)){
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do operation with 2 QM types without ! them");
            }

            //Tokens are integers, it is gonna be IDIV
            if(tmp1->exp_value == INT){

                // CODEGEN
                vardef_outermost_while(IDIV_ZERO_DEFVAR, NULL, variable_counter);
                instruction *inst_zero = inst_init(IDIV_BY_ZERO, active->frame, "idiv_zero_", variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst_zero);
                instruction *inst = inst_init(IDIVS, 'G', NULL, variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);
                
                variable_counter++;
            //Tokens are floats, it is gonna be div
            } else {
                
                // CODEGEN
                vardef_outermost_while(DIV_ZERO_DEFVAR, NULL, variable_counter);
                instruction *inst_zero = inst_init(DIV_BY_ZERO, active->frame, "div_zero_", variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst_zero);
                instruction *inst = inst_init(DIVS, 'G', NULL, variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                variable_counter++;

                tmp1->exp_value = DOUBLE;
            }

        //types are not same in division
        } else {
            if (tmp1->exp_type == CONST){

                if((tmp1->value.integer == 0 && tmp1->exp_value == INT) || (tmp1->value.type_double == 0 && tmp1->exp_value == DOUBLE)){
                    error_exit(ERROR_SEM_OTHER, "EXPRESSION PARSER", "Division by zero");
                }

                if (tmp3->exp_type == ID){
                    if(tmp3->exp_value == DOUBLE){

                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        // CODEGEN
                        instruction *inst1 = inst_init(DIVS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst1);

                        tmp1->exp_value = DOUBLE;
                    } else {
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "ID type mismatch");
                    }
                } else{
                    if (tmp1->exp_value == STRING || tmp3->exp_value == STRING){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "This cannot participate in concatenation");
                    }

                    if (tmp1->exp_value == INT){

                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        // CODEGEN
                        instruction *inst1 = inst_init(DIVS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst1);

                        tmp1->exp_value = DOUBLE;
                    }
                    else if (tmp3->exp_value == INT){
                      
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        // CODEGEN
                        instruction *inst1 = inst_init(DIVS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst1);

                        variable_counter++;
                    }
                }
            }
            else if (tmp3->exp_type == CONST){
                if (tmp1->exp_type == ID){
                    if(tmp1->exp_value == DOUBLE){
                        
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        // CODEGEN
                        instruction *inst1 = inst_init(DIVS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst1);

                        variable_counter++;
                    } else {
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "ID type mismatch");
                    }
                }
                else{
                    if (tmp1->exp_value == STRING || tmp3->exp_value == STRING){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "This cannot participate in concatenation");
                    }

                    if (tmp1->exp_value == INT){
                        
                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        // CODEGEN
                        instruction *inst1 = inst_init(DIVS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst1);

                        tmp1->exp_value = DOUBLE;
                    }
                    else if (tmp3->exp_value == INT){

                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        // CODEGEN
                        instruction *inst1 = inst_init(DIVS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst1);

                        variable_counter++;
                    }
                }
            }
            else{
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not divide 2 IDs of different types");
            }
        }

    //Relational operators
    } else if(tmp2->type == TOKEN_EQEQ || tmp2->type == TOKEN_EXCLAMEQ || tmp2->type == TOKEN_LESS || tmp2->type == TOKEN_LESS_EQ || tmp2->type == TOKEN_GREAT || tmp2->type == TOKEN_GREAT_EQ){

        if(tmp2->type == TOKEN_LESS || tmp2->type == TOKEN_LESS_EQ || tmp2->type == TOKEN_GREAT || tmp2->type == TOKEN_GREAT_EQ){
            //Nillable types cannot be in <, >, <=, >=
            if((tmp1->exp_value == INT_QM || tmp3->exp_value == INT_QM || tmp1->exp_value == DOUBLE_QM || tmp3->exp_value == DOUBLE_QM || tmp1->exp_value == STRING_QM || tmp3->exp_value == STRING_QM)){
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do operation with 2 QM types without ! them");
        
            }
            //Nil constant cannot be in <, >, <=, >=
            if(tmp1->exp_value == NIL || tmp3->exp_value == NIL){
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not compare with nill constant");
            }
        }
        if(tmp2->type == TOKEN_EQEQ || tmp2->type == TOKEN_EXCLAMEQ){
            //If one of the operands is nil constant the other one has to be nillable or nil
            if(tmp1->exp_value == NIL){
                if(tmp3->exp_value == INT_QM){
                    tmp1->exp_value = BOOL;
                    return;
                } else if(tmp3->exp_value == DOUBLE_QM){
                    tmp1->exp_value = BOOL;
                    return;
                } else if(tmp3->exp_value == STRING_QM){
                    tmp1->exp_value = BOOL;
                    return;
                } else if(tmp3->exp_value == NIL){
                    tmp1->exp_value = BOOL;
                    return;
                } else {
                    error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do eq with non QM types");
                }
            } else if(tmp3->exp_value == NIL){
                if(tmp1->exp_value == INT_QM){
                    tmp1->exp_value = BOOL;
                    return;
                } else if(tmp1->exp_value == DOUBLE_QM){
                    tmp1->exp_value = BOOL;
                    return;
                } else if(tmp1->exp_value == STRING_QM){
                    tmp1->exp_value = BOOL;
                    return;
                } else if(tmp1->exp_value == NIL){
                    tmp1->exp_value = BOOL;
                    return;
                } else {
                    error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do eq with non QM types");
                }
            }
            //Converts nillable types to nonnillable because they can be in expression with == or !=
            convert_qm(tmp1);
            convert_qm(tmp3);
        }

        //Types are equal, value of expression is bool
        if(tmp1->exp_value == tmp3->exp_value){
            tmp1->exp_value = BOOL;
        
        //Not a match, conversion if possible
        } else {
            if (tmp1->exp_type == CONST){
                if (tmp3->exp_type == ID){
                    if(tmp3->exp_value == DOUBLE && tmp1->exp_value == INT){
                        
                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        tmp1->exp_value = BOOL;
                    } else {
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not compare 2 values of different types");
                    }
                } else{

                    if (tmp1->exp_value == INT && tmp3->exp_value == DOUBLE){

                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        tmp1->exp_value = BOOL;
                    }
                    else if (tmp3->exp_value == INT && tmp1->exp_value == DOUBLE){
                        
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                        
                        tmp1->exp_value = BOOL;
                        variable_counter++;
                    }
                }
            }
            else if (tmp3->exp_type == CONST){
                if (tmp1->exp_type == ID){
                    if(tmp1->exp_value == DOUBLE && tmp3->exp_value == INT){
                        
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                        
                        tmp1->exp_value = BOOL;
                        variable_counter++;
                    } else {
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not compare 2 values of different types");
                    }
                }
                else{
                    if (tmp1->exp_value == INT && tmp3->exp_value == DOUBLE){
                        
                        // CODEGEN
                        instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                        
                        tmp1->exp_value = BOOL;
                    }
                    else if (tmp3->exp_value == INT && tmp1->exp_value == DOUBLE){
                        
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL, variable_counter);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                        
                        tmp1->exp_value = BOOL;
                        variable_counter++;
                    }
                }
            }
            else{
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not compare 2 values of different types");
            }
        }
    }
    
}

//Additional codegen for <= and >= because first less or greater operation needs to be done, then equal and then OR
void push_for_leq_geq(token_t* tmp1, token_t* tmp3){
    if (tmp3->exp_type == CONST){
        if (tmp3->exp_value == INT){                                    
            // CODEGEN
            instruction *inst = inst_init(PUSHS_INT_CONST, 'G', NULL, 0, tmp3->value.integer, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
        } else if (tmp3->exp_value == DOUBLE){
            // CODEGEN
            instruction *inst = inst_init(PUSHS_FLOAT_CONST, 'G', NULL, 0, 0, tmp3->value.type_double, NULL);
            inst_list_insert_last(inst_list, inst);
        } else if (tmp3->exp_value == STRING){
            // CODEGEN
            char *string = allocate_memory(strlen(tmp3->value.vector->array)+1); // new memory has to be allocated for string
            instruction *inst = inst_init(PUSHS_STRING_CONST, 'G', NULL, 0, 0, 0.0, strcpy(string, tmp3->value.vector->array));
            inst_list_insert_last(inst_list, inst);

        }
    } else {
        forest_node* scope = forest_search_scope(active, tmp3->value.vector->array);
        AVL_tree* node = forest_search_symbol(active, tmp3->value.vector->array);
        char *nickname = renamer(node);
        if (tmp3->exp_value == INT || tmp3->exp_value == DOUBLE || tmp3->exp_value == STRING){
            // CODEGEN
            instruction *inst = inst_init(PUSHS, scope->frame, nickname, 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
        }
    }

    if (tmp1->exp_type == CONST){
        if (tmp1->exp_value == INT){
            // CODEGEN
            instruction *inst = inst_init(PUSHS_INT_CONST, 'G', NULL, 0, tmp1->value.integer, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
        } else if (tmp1->exp_value == DOUBLE){
            // CODEGEN
            instruction *inst = inst_init(PUSHS_FLOAT_CONST, 'G', NULL, 0, 0, tmp1->value.type_double, NULL);
            inst_list_insert_last(inst_list, inst);
        } else if (tmp1->exp_value == STRING){
            // CODEGEN
            char *string = allocate_memory(strlen(tmp1->value.vector->array)+1); // new memory has to be allocated for string
            instruction *inst = inst_init(PUSHS_STRING_CONST, 'G', NULL, 0, 0, 0.0, strcpy(string, tmp1->value.vector->array));
            inst_list_insert_last(inst_list, inst);

        }
    } else {
        forest_node* scope = forest_search_scope(active, tmp1->value.vector->array);
        AVL_tree* node = forest_search_symbol(active, tmp1->value.vector->array);
        char *nickname = renamer(node);
        if (tmp1->exp_value == INT || tmp1->exp_value == DOUBLE || tmp1->exp_value == STRING){
            // CODEGEN
            instruction *inst = inst_init(PUSHS, scope->frame, nickname, 0, 0, 0.0, NULL);
            inst_list_insert_last(inst_list, inst);
        }
    }
}


//main function
void call_expr_parser(data_type return_type) {

    token_stack stack;
    stack_init(&stack);

    //Dollar at the bottom of the stack
    token_t* stack_bottom = token_create(TOKEN_DOLLAR);
    stack_push(&stack, stack_bottom);

    int rule_params_count = 0;
    
    char table_result;

    bool eval_expr = true; 
    current_token->was_exp = false;
    int stack_index;
    int next_token_index;
    
    while(eval_expr){
        
        
        token_t* terminal = stack_top_terminal(&stack);
    
        //If there is a token that we can not process we will start the reduction of current expression (stop_expression will be true) and wont look for another result from precedence table
        if(!stop_expression){
            stack_index = get_index(terminal->type);
            next_token_index = get_index(current_token->type);
            if(current_token->type == TOKEN_KEYWORD && current_token->value.keyword != KW_NIL){
                table_result = '>';
                next_token_index = 15;
            } else {
                table_result = precedence_table[stack_index][next_token_index];
            }
        }

        //Evaluation ends when next token index is 15 which is default for all tokens not supported by precedence table and also when only dollar as a terminal is on the stack
        if(next_token_index == 15 && stack_top_terminal(&stack)->type == TOKEN_DOLLAR){

            if(stack.size <= 1){
                
                error_exit(ERROR_SYN, "EXPRESSION PARSER", "empty expression");
            }
            eval_expr = false;
            type_of_expr = stack_top(&stack)->exp_value;
            
            //Check return type that parser wants with our return type in expression, conversion between QM and non QM types if needed
            if((return_type != UNKNOWN) && (return_type != stack_top(&stack)->exp_value)){

                if((return_type == DOUBLE || return_type == DOUBLE_QM) && stack_top(&stack)->exp_value == INT && stack_top(&stack)->was_exp == false){
                    instruction *inst = inst_init(INT2FLOATS, 'G', NULL, 0, 0, 0.0, NULL);
                    inst_list_insert_last(inst_list, inst);
                } else if(return_type == INT_QM && stack_top(&stack)->exp_value == INT){
                    type_of_expr = INT;
                } else if(return_type == DOUBLE_QM && stack_top(&stack)->exp_value == DOUBLE){
                    type_of_expr = DOUBLE;
                } else if(return_type == STRING_QM && stack_top(&stack)->exp_value == STRING){
                    type_of_expr = STRING;
                } else if(return_type == STRING && stack_top(&stack)->exp_value == STRING_QM){
                    type_of_expr = STRING;
                } else if(return_type == DOUBLE && stack_top(&stack)->exp_value == DOUBLE_QM){
                    type_of_expr = DOUBLE;
                } else if(return_type == INT && stack_top(&stack)->exp_value == INT_QM){
                    type_of_expr = INT;
                } else if((return_type == INT_QM || return_type == DOUBLE_QM || return_type == STRING_QM) && stack_top(&stack)->exp_value == NIL){
                    type_of_expr = NIL;
                } else {
                    //If expression was in return statement it has different error code
                    if (return_expr) {
                        error_exit(ERROR_SEM_TYPE, "EXPRESSION PARSER", "Wrong data type of the return value");
                    }
                    else {
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Wrong data type result of expression");
                    }
                }
            }
            //return type was unknown, parser will know type of expression from global variable
            dispose_stack(&stack);
            stop_expression = false;
            break;
        }

        //Accoreding to table result we will shift or reduce 
        switch (table_result)
        {
        //shift rule
        case '<':
            token_shift(&stack);
            current_token = get_next_token();
            break;

        //reduction rule
        case '>':

            rule_params_count = 0;
            token_t* tmp1, *tmp2, *tmp3 = NULL;

            //find out number of operands
            while(stack_top(&stack)->type != TOKEN_SHIFT){
                if(rule_params_count == 0){
                    tmp1 = stack_top(&stack);
                    stack_pop(&stack);
                }

                if(rule_params_count == 1){
                    tmp2 = stack_top(&stack);
                    stack_pop(&stack);
                }

                if(rule_params_count == 2){
                    tmp3 = stack_top(&stack);
                    stack_pop(&stack);
                }

                if(rule_params_count > 2){
                    error_exit(ERROR_SYN, "EXPRESSION PARSER", "syntax error");
                }
                rule_params_count++;
            }

            //Pops token_shift
            stack_pop(&stack);

            //find the rule to reduce the expression
            expression_rules_t rule = find_reduce_rule(tmp1, tmp2, tmp3, rule_params_count);
            switch (rule)
            {

            // E -> i
            case RULE_OPERAND:

                if(tmp1->type == TOKEN_ID){
                    forest_node* forest = forest_search_scope(active, tmp1->value.vector->array);
                    //Search in AVL tree to find node with specific ID
                    AVL_tree* node = forest_search_symbol(active, tmp1->value.vector->array);
                    if(node == NULL){
                        error_exit(ERROR_SEM_UNDEF_VAR, "EXPRESSION PARSER", "Variable does not exist");
                    }
                    else if (!node->data->defined && !(node->data->data_type == INT_QM || node->data->data_type == DOUBLE_QM || node->data->data_type == STRING_QM)) {
                        error_exit(ERROR_SEM_UNDEF_VAR, "EXPRESSION PARSER", "Variable is not initialized");
                    }
                    //creating unique nickname for the id, usefull later in codegen
                    char *nickname = renamer(node);
                    
                    data_type variable_type;
                    if(node->data->is_param == false){
                        variable_type = node->data->data_type;
                    } else {
                        variable_type = node->data->param_type;
                    }
                    

                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = ID;
                    tmp1->exp_value = variable_type;

                    if(variable_type == INT || variable_type == INT_QM){
                        // CODEGEN
                        instruction *inst = inst_init(PUSHS, forest->frame, nickname, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                    } else if (variable_type == DOUBLE || variable_type == DOUBLE_QM){
                        // CODEGEN
                        instruction *inst = inst_init(PUSHS, forest->frame, nickname, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                    } else if(variable_type == STRING || variable_type == STRING_QM){
                        // CODEGEN
                        instruction *inst = inst_init(PUSHS, forest->frame, nickname, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);
                    }

                } else if(tmp1->type == TOKEN_DEC){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = CONST;
                    tmp1->exp_value = DOUBLE;
                    
                    // CODEGEN
                    instruction *inst = inst_init(PUSHS_FLOAT_CONST, 'G', NULL, 0, 0, tmp1->value.type_double, NULL);
                    inst_list_insert_last(inst_list, inst);

                } else if(tmp1->type == TOKEN_NUM){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = CONST;
                    tmp1->exp_value = INT;

                    // CODEGEN
                    instruction *inst = inst_init(PUSHS_INT_CONST, 'G', NULL, 0, tmp1->value.integer, 0.0, NULL);
                    inst_list_insert_last(inst_list, inst);

                } else if (tmp1->type == TOKEN_EXP){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = CONST;
                    tmp1->exp_value = DOUBLE;
                    
                    // CODEGEN
                    instruction *inst = inst_init(PUSHS_FLOAT_CONST, 'G', NULL, 0, 0, tmp1->value.type_double, NULL);
                    inst_list_insert_last(inst_list, inst);

                } else if (tmp1->type == TOKEN_STRING || tmp1->type == TOKEN_ML_STRING){
                    tmp1->type = TOKEN_EXPRESSION;
                    tmp1->exp_type = CONST;
                    tmp1->exp_value = STRING;
                                        
                    // CODEGEN
                    char *string = allocate_memory(strlen(tmp1->value.vector->array)+1); // new memory has to be allocated for string
                    instruction *inst = inst_init(PUSHS_STRING_CONST, 'G', NULL, 0, 0, 0.0, strcpy(string, tmp1->value.vector->array));
                    inst_list_insert_last(inst_list, inst);

                } else if(tmp1->type == TOKEN_KEYWORD){
                    if(tmp1->value.keyword != KW_NIL){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do operations with keywords");
                    } else {
                        //Exp. parser support only nil keyword as constant
                        tmp1->type = TOKEN_EXPRESSION;
                        tmp1->exp_type = CONST;
                        tmp1->exp_value = NIL;
                    
                        // CODEGEN
                        instruction *inst = inst_init(PUSHS_NIL, 'G', NULL, 0, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                    }
                } else {
                    error_exit(ERROR_SYN, "EXPRESSION PARSER", "It its not a valid operand");
                }
                stack_push(&stack, tmp1);
                break;

            case RULE_EXCL:
                if(tmp2->exp_value == INT_QM || tmp2->exp_value == DOUBLE_QM || tmp2->exp_value == STRING_QM){

                    // CODEGEN
                    vardef_outermost_while(EXCLAMATION_RULE_DEFVAR, NULL, variable_counter);
                    instruction *inst = inst_init(EXCLAMATION_RULE, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                    inst_list_insert_last(inst_list, inst);

                    variable_counter++;
                    //Nillable value is now converted to nonnillable
                    if(tmp2->exp_value == INT_QM){
                        tmp2->exp_value = INT;
                    } else if(tmp2->exp_value == DOUBLE_QM){
                        tmp2->exp_value = DOUBLE;
                    } else {
                        tmp2->exp_value = STRING;
                    }
                    stack_push(&stack, tmp2);
                } else {
                    error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not apply ! to non qm operands");
                }           
            break;

            // E -> (E)
            case RULE_PAR:
                stack_push(&stack, tmp2);
                break;

            case RULE_ADD:
                check_types(tmp1,tmp2, tmp3);
                if(concat){
                    concat = false;
                    
                } else { 
                    // CODEGEN
                    instruction *inst = inst_init(ADDS, 'G', NULL, 0, 0, 0.0, NULL);
                    inst_list_insert_last(inst_list, inst);
                }

                if(tmp1->exp_type == ID || tmp3->exp_type == ID){
                    //ID was used in addition, cant be converted later
                    tmp1->was_exp = true;
                }

                stack_push(&stack, tmp1);
                break;
            case RULE_MUL:
                check_types(tmp1, tmp2, tmp3);
                // CODEGEN
                instruction *inst = inst_init(MULS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                if(tmp1->exp_type == ID || tmp3->exp_type == ID){
                    //ID was used in addition, cant be converted later
                    tmp1->was_exp = true;
                }

                stack_push(&stack, tmp1);
                break;
            case RULE_SUB:
                check_types(tmp1, tmp2, tmp3);
                // CODEGEN
                instruction *inst1 = inst_init(SUBS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst1);

                if(tmp1->exp_type == ID || tmp3->exp_type == ID){
                    //ID was used in addition, cant be converted later
                    tmp1->was_exp = true;
                }

                stack_push(&stack, tmp1);
                break;
            case RULE_DIV:
                check_types(tmp1, tmp2, tmp3);

                if(tmp1->exp_type == ID || tmp3->exp_type == ID){
                    //ID was used in addition, cant be converted later
                    tmp1->was_exp = true;
                }

                stack_push(&stack, tmp1);
                break;
            case RULE_LESS:
                check_types(tmp1, tmp2, tmp3);
                // CODEGEN
                instruction *inst2 = inst_init(LTS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst2);

                stack_push(&stack, tmp1);
                break;

            case RULE_LEQ:
                //Another codegen push because of more operations (Lower, equal and then OR)
                push_for_leq_geq(tmp1, tmp3);
                check_types(tmp1, tmp2, tmp3);

                // CODEGEN
                instruction *inst31 = inst_init(LTS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst31);
                vardef_outermost_while(LEQ_RULE_DEFVAR, NULL, variable_counter);
                instruction *inst3 = inst_init(LEQ_RULE, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst3);

                stack_push(&stack, tmp1);
                variable_counter++;
                variable_counter++;
                break;
            case RULE_GTR:
                check_types(tmp1, tmp2, tmp3);
                // CODEGEN
                instruction *inst4 = inst_init(GTS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst4);

                stack_push(&stack, tmp1);
                break;
            case RULE_GEQ:
                push_for_leq_geq(tmp1, tmp3);
                check_types(tmp1, tmp2, tmp3);

                // CODEGEN
                instruction *inst51 = inst_init(GTS,'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst51);
                vardef_outermost_while(GEQ_RULE_DEFVAR, NULL, variable_counter);
                instruction *inst5 = inst_init(GEQ_RULE, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst5);

                stack_push(&stack, tmp1);
                variable_counter++;
                variable_counter++;
                break;
            case RULE_EQ:
                check_types(tmp1, tmp2, tmp3);
                // CODEGEN
                instruction *inst6 = inst_init(EQS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst6);

                stack_push(&stack, tmp1);
                break;
            case RULE_NEQ:
                check_types(tmp1, tmp2, tmp3);
                // CODEGEN
                instruction *inst7 = inst_init(EQS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst7);

                // CODEGEN
                instruction *inst8 = inst_init(NOTS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst8);

                stack_push(&stack, tmp1);
                break;

            case RULE_QMS:
                //Rule accepts only nillable value on left side or nil itself
                if(tmp3->exp_value == INT_QM || tmp3->exp_value == DOUBLE_QM || tmp3->exp_value == STRING_QM || tmp3->exp_value == NIL){
                    //Right side has to be the same as left except it is non-nillable
                    if((tmp3->exp_value == INT_QM && tmp1->exp_value != INT) || (tmp3->exp_value == DOUBLE_QM && tmp1->exp_value != DOUBLE) || (tmp3->exp_value == STRING_QM && tmp1->exp_value != STRING)){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "wrong ID type for right side of ??");
                    }
              

                    // CODEGEN
                    vardef_outermost_while(QMS_RULE_DEFVAR, NULL, variable_counter);
                    instruction *inst = inst_init(QMS_RULE, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                    inst_list_insert_last(inst_list, inst);


                    variable_counter++;
                    variable_counter++;
                    stack_push(&stack, tmp1);

                } else {
                    error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "wrong ID type for left side of ??");
                }
                break;
            default:
                break;
            }

            break;
        //same as <, used for parenthesis
        case '=':
            stack_push(&stack, current_token);
            current_token = get_next_token();
            break;
        
        default:

            //Result from precedence table is empty, however if exp. parser got id or rpar expression reduction will start and current token will be handed to parser
            if(current_token->type == TOKEN_ID || current_token->type == TOKEN_RPAR){
            table_result = '>';
            next_token_index = 15;
            stop_expression = true;
            break;
            }

            error_exit(ERROR_SYN, "EXPRESSION PARSER", "syntax error");
        }

    }
}
