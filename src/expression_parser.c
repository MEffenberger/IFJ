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

#define TABLE_SIZE 16

int variable_counter = 1;
bool concat = false;
bool stop_expression = false;

extern FILE* file;

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

// funkce se podiva na current token a vyhodi odpovidajici index v precedencni tabulce
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

// pomocna fce pro vytvoreni zarazky v zasobniku
token_t* token_create(token_type_t token_type){
    token_t* token = malloc(sizeof(token_t));
    token->value.vector = NULL;
    token->value.integer = 0;
    token->value.type_double = 0.0;
    token->type = token_type;
    return token;
}

// v precedencni tabulce je na miste [i][j] < coz znamena neredukuj ale pridej token na zasobnik a 
// pred nejblizsi terminal pridej shift token <
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


// v podstate vraci pravidlo redukce podle poctu operandu, vetsinou jsou binarni takze 3(E operator E), 1 (E -> i) a 2 (E -> !E)
expression_rules_t find_reduce_rule(token_t* token1, token_t* token2, token_t* token3, int number_of_tokens){
    switch (number_of_tokens)
    {
    case 1: //E->i
        return RULE_OPERAND;

    case 2:
        if(token1->type == TOKEN_EXCLAM && token2->type == TOKEN_EXPRESSION){
            return RULE_EXCL;
        } else {
            error_exit(ERROR_SYN, "EXPRESSION PARSER", "Wrong operator");
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


void convert_qm(token_t* tmp1){
    if(tmp1->exp_value == INT_QM){
        tmp1->exp_value = INT;
    } else if(tmp1->exp_value == DOUBLE_QM){
        tmp1->exp_value = DOUBLE;
    } else if(tmp1->exp_value == STRING_QM){
        tmp1->exp_value = STRING;
    }
}



// kontrola typu => tri prdele case splitu, pokud je to token id konstanta tak ji lze pretypovat ale pouze z int na double
void check_types(token_t* tmp1, token_t* tmp2, token_t* tmp3){


    // klasicke operace kde se resi jen typy
    if(tmp2->type == TOKEN_PLUS || tmp2->type == TOKEN_MINUS || tmp2->type == TOKEN_MULTIPLY){

        if(tmp1->exp_value == NIL || tmp3->exp_value == NIL){
        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do arithmetic opeation with nils");
        }

        convert_qm(tmp1);
        convert_qm(tmp3);
            
        if(tmp1->exp_value == BOOL || tmp3->exp_value == BOOL){
            error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do arithmetic opeation with booleans");
        }

        // typy jsou stejne
        if (tmp1->exp_value == tmp3->exp_value){
            if((tmp1->exp_type == CONST && tmp3->exp_type == ID) || (tmp1->exp_type == INT && tmp3->exp_type == CONST)){
                tmp1->exp_type = ID;
            }
            // jedine co se stringy lze je +, * - je invalid
            if(tmp1->exp_value == STRING && tmp3->exp_value == STRING && tmp2->type != TOKEN_PLUS){
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Wrong operator in concatenation");

            } else if(tmp1->exp_value == STRING && tmp3->exp_value == STRING && tmp2->type == TOKEN_PLUS){
                concat = true;

                // CODEGEN
                vardef_outermost_while(CONCAT_DEFVAR, NULL);
                instruction *inst = inst_init(CONCAT, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                variable_counter++;
                variable_counter++;
                return;

            }
        // typy nejsou stejne
        } else{

            // pokud je jeden id a druhy konstanta nebo naopak a lze pretypovat (z int na double) az po line 313
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
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
                        instruction *inst = inst_init(INT2FLOATS_2, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                        inst_list_insert_last(inst_list, inst);

                        variable_counter++;
                    }
                }
            } else if (tmp3->exp_type == CONST){
                if (tmp1->exp_type == ID){
                    if(tmp1->exp_value == DOUBLE || tmp1->exp_value == DOUBLE_QM){
                        
                        // CODEGEN
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
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
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
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

    // tady se resi celociselne a desetinne deleni (div x idiv)
    } else if(tmp2->type == TOKEN_DIVIDE){

        if(tmp1->exp_value == NIL || tmp3->exp_value == NIL){
            error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do arithmetic opeation with nils");
        }
        convert_qm(tmp1);
        convert_qm(tmp3);

        if(tmp1->exp_value == BOOL || tmp3->exp_value == BOOL){
            error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not divide bool");
        }

        // typy se rovnaji, neni problem
        if (tmp1->exp_value == tmp3->exp_value){

            if((tmp1->exp_type == CONST && tmp3->exp_type == ID) || (tmp1->exp_type == INT && tmp3->exp_type == CONST)){
                tmp1->exp_type = ID;
            }

            if((tmp1->value.integer == 0 && tmp1->exp_value == INT && tmp1->exp_type == CONST) || (tmp1->value.type_double == 0 && tmp1->exp_value == DOUBLE && tmp1->exp_type == CONST)){
                    error_exit(ERROR_SEM_OTHER, "EXPRESSION PARSER", "Division by zero");
            }

            if(tmp1->exp_value == STRING && tmp3->exp_value == STRING){
                error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Wrong operator in concatenation");
            }

            if(tmp1->exp_value == INT){
                
                // CODEGEN
                instruction *inst = inst_init(IDIVS, 'G', NULL, variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

            } else {
                
                // CODEGEN
                instruction *inst = inst_init(DIVS, 'G', NULL, variable_counter, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst);

                tmp1->exp_value = DOUBLE;
            }

        // typy se nerovnaji to same jako u + - * az po line 412
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
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
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
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
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
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
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

    // relacni operatory
    } else if(tmp2->type == TOKEN_EQEQ || tmp2->type == TOKEN_EXCLAMEQ || tmp2->type == TOKEN_LESS || tmp2->type == TOKEN_LESS_EQ || tmp2->type == TOKEN_GREAT || tmp2->type == TOKEN_GREAT_EQ){
        

        if(tmp3->value.keyword == KW_NIL){
            if(tmp1->exp_value != INT_QM){
                if(tmp1->exp_value != DOUBLE_QM){
                    if(tmp1->exp_value != STRING_QM){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not compare non nill types");
                    }
                }
            }
            tmp1->exp_value = BOOL;
            return;
        } else if(tmp1->value.keyword == KW_NIL){
            if(tmp3->exp_value != INT_QM){
                if(tmp3->exp_value != DOUBLE_QM){
                    if(tmp3->exp_value != STRING_QM){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not compare non nill types");
                    }
                }
            }
            tmp1->exp_value = BOOL;
            return;
        }


        convert_qm(tmp1);
        convert_qm(tmp3);

        // typy se rovnaji a navratova hodnota bude bool
        if(tmp1->exp_value == tmp3->exp_value){
            tmp1->exp_value = BOOL;
        
        // nerovnaji se => opet pretypovani pokud lze az po konec funkce
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
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
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
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
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
                        vardef_outermost_while(INT2FLOATS_2_DEFVAR, NULL);
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

// u <= a >= je problem ze se musi provest zaroven operace (less nebo greater) + equal a vysledek tech dvou se ORuje
// takze se do zasobniku hodnot v codegenu musi pushovat 2x
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
            char *string = malloc(strlen(tmp3->value.vector->array)+1); // new memory has to be allocated for string
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
            char *string = malloc(strlen(tmp1->value.vector->array)+1); // new memory has to be allocated for string
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


// hlavni fce
void call_expr_parser(data_type return_type) {

    token_stack stack;
    stack_init(&stack);

    //Dollar at the bottom of the stack
    token_t* stack_bottom = token_create(TOKEN_DOLLAR);
    stack_push(&stack, stack_bottom);

    int rule_params_count = 0;
    
    char table_result;

    bool eval_expr = true; 

    int stack_index;
    int next_token_index;
    
    while(eval_expr){
        
        // bereme nejvrchnejsi terminal na zasobniku
        token_t* terminal = stack_top_terminal(&stack);

        //int stack_index = get_index(terminal->type);
        //int next_token_index = get_index(current_token->type);
        //table_result = precedence_table[stack_index][next_token_index];
    
        // pokud jsme uz narazili na token, ktery nelze vyhodnotit pomoci prec. tabulky, pokusime se 
        // zredukovat dosavdani expression
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

        // podminka kdy ukoncit celkove vyhodnoceni vyrazu = na zasobniku je jen $ a E
        if(next_token_index == 15 && stack_top_terminal(&stack)->type == TOKEN_DOLLAR){

            if(stack.size <= 1){
                
                error_exit(ERROR_SYN, "EXPRESSION PARSER", "empty expression");
            }
            eval_expr = false;
            type_of_expr = stack_top(&stack)->exp_value;
            
            // Check ze vracime spravny typ
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
                } else {
                    if (return_expr) {
                        printf("RETTYPE:%d\n", return_type);
                        printf("TOP:%d", stack_top(&stack)->exp_value);
                        error_exit(ERROR_SEM_TYPE, "EXPRESSION PARSER", "Wrong data type of the return value");
                    }
                    else {
                        printf("RETTYPE:%d\n", return_type);
                        printf("TOP:%d", stack_top(&stack)->exp_value);
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Wrong data type result of expression");
                    }
                }
            }

            dispose_stack(&stack);
            stop_expression = false;
            break;
        }

        // podle vysledku z precedecni tab. bud shiftujeme nebo aplikujeme redukcni pravidlo
        switch (table_result)
        {
        case '<':
            token_shift(&stack);
            current_token = get_next_token();
            printf("(EXP)G.N.T.: Current token: %d\n", current_token->type);

            break;

        // aplikujeme redukcni pravidlo
        case '>':

            rule_params_count = 0;
            token_t* tmp1, *tmp2, *tmp3 = NULL;

            // zjistime pocet operandu 
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

            stack_pop(&stack);
            expression_rules_t rule = find_reduce_rule(tmp1, tmp2, tmp3, rule_params_count);
            switch (rule)
            {

            // E -> i, ziskani vsech informaci o tokenu - typ,...
            case RULE_OPERAND:

                if(tmp1->type == TOKEN_ID){
                    forest_node* forest = forest_search_scope(active, tmp1->value.vector->array);
                    AVL_tree* node = forest_search_symbol(active, tmp1->value.vector->array);


                    if(node == NULL){
                        error_exit(ERROR_SEM_UNDEF_VAR, "EXPRESSION PARSER", "Variable does not exist");
                    }

                    char *nickname = renamer(node);
                    
                    data_type variable_type;
                    if(node->data.is_param == false){
                        variable_type = node->data.data_type;
                    } else {
                        variable_type = node->data.param_type;
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
                    } else if(variable_type == NIL){
                        //
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
                    char *string = malloc(strlen(tmp1->value.vector->array)+1); // new memory has to be allocated for string
                    instruction *inst = inst_init(PUSHS_STRING_CONST, 'G', NULL, 0, 0, 0.0, strcpy(string, tmp1->value.vector->array));
                    inst_list_insert_last(inst_list, inst);

                } else if(tmp1->type == TOKEN_KEYWORD){
                    if(tmp1->value.keyword != KW_NIL){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not do operations with keywords");
                    } else {
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
                    vardef_outermost_while(EXCLAMATION_RULE_DEFVAR, NULL);
                    instruction *inst = inst_init(EXCLAMATION_RULE, active->frame, NULL, variable_counter, 0, 0.0, NULL);
                    inst_list_insert_last(inst_list, inst);

                    variable_counter++;
                    stack_push(&stack, tmp2);
                } else {
                    error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "Can not apply ! to non qm operands");
                }           
            break;

            // E -> (E)
            case RULE_PAR:
                stack_push(&stack, tmp2);
                break;

            // vsehny easy binarni operace, neni moc co resit, check typu atd. viz check_types
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
                    tmp1->was_exp = true;
                }

                stack_push(&stack, tmp1);
                break;
            case RULE_DIV:
                check_types(tmp1, tmp2, tmp3);

                if(tmp1->exp_type == ID || tmp3->exp_type == ID){
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

            // v podstate to same jen vice printu, skrz duvod u push_for_leq_geq()
            case RULE_LEQ:
                push_for_leq_geq(tmp1, tmp3);
                check_types(tmp1, tmp2, tmp3);

                // CODEGEN
                instruction *inst31 = inst_init(LTS, 'G', NULL, 0, 0, 0.0, NULL);
                inst_list_insert_last(inst_list, inst31);
                vardef_outermost_while(LEQ_RULE_DEFVAR, NULL);
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
                vardef_outermost_while(GEQ_RULE_DEFVAR, NULL);
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

            // totok uz delal Sam solo ale pravdepodobne radky navic kvuli praci se zasobnikem
            case RULE_QMS:
                if(tmp3->exp_value == INT_QM || tmp3->exp_value == DOUBLE_QM || tmp3->exp_value == STRING_QM){

                    if((tmp3->exp_value == INT_QM && tmp1->exp_value != INT) || (tmp3->exp_value == DOUBLE_QM && tmp1->exp_value != DOUBLE) || (tmp3->exp_value == STRING_QM && tmp1->exp_value != STRING)){
                        error_exit(ERROR_SEM_EXPR_TYPE, "EXPRESSION PARSER", "wrong ID type for right side of ??");
                    }
              

                    // CODEGEN
                    vardef_outermost_while(QMS_RULE_DEFVAR, NULL);
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
        // pripad () proste pokracujeme
        case '=':
            stack_push(&stack, current_token);
            current_token = get_next_token();
            printf("(EXP)G.N.T.: Current token: %d\n", current_token->type);
            break;
        
        default:

            // redukujeme dokud to jde
            if(current_token->type == TOKEN_ID || current_token->type == TOKEN_RPAR){
            table_result = '>';
            next_token_index = 15;
            stop_expression = true;
            break;
            }

            /*for(int i =0; i < stack.size; i++){
                fprintf(file, "STACK:%d\n", stack.token_array[i]->type);
            }*/

            error_exit(ERROR_SYN, "EXPRESSION PARSER", "syntax error");
        }

    }
}