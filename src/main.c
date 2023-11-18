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
#include "scanner.h"
#include "token_stack.h"


alloc_ptr *allocated_pointers = NULL; // Top of the stack for allocated pointers
forest_node *active = NULL; // Pointer to the active node in the forest


int main() { 
    //parser();
    //token_t** token_array;
    token_stack stack;
    stack_init(&stack);
   // token_array = malloc(100*sizeof(token_t*));

    for(int i = 0; i < 20; i++){
        token_t* token = get_me_token();
        stack_push(&stack, token);
        printf("%d\n", stack.token_array[i]->type);
    }
    printf("%d\n", stack.size);
    printf("%d\n", stack.capacity);

    dispose_stack(&stack);

    
    /*
    for (int i = 0; i < 100; i++)
    {
        //printf("\n, %d, \n", token_array[i]->type);
        if(token_array[i]->type == 0){
            printf("\n WHITESPACE \n");
        } else if(token_array[i]->type == 1){
            printf("\n TAB \n");
        }else if(token_array[i]->type == 2){
            printf("\n EOL \n");
        }else if(token_array[i]->type == 3){
            printf("\n EOF \n");
        }else if(token_array[i]->type == 4){
            printf("\n MINUS \n");
        }else if(token_array[i]->type == 5){
            printf("\n PLUS \n");
        }else if(token_array[i]->type == 6){
            printf("\n MULTIPLY \n");
        }else if(token_array[i]->type == 7){
            printf("\n LESS \n");
        }else if(token_array[i]->type == 8){
            printf("\n LESS_EQ \n");
        }else if(token_array[i]->type == 9){
            printf("\n GREAT \n");
        }else if(token_array[i]->type == 10){
            printf("\n GREAT_EQ \n");
        }else if(token_array[i]->type == 11){
            printf("\n LPAR \n");
        }else if(token_array[i]->type == 12){
            printf("\n RPAR \n");
        }else if(token_array[i]->type == 13){
            printf("\n EQ \n");
        }else if(token_array[i]->type == 14){
            printf("\n EQEQ \n");
        }else if(token_array[i]->type == 15){
            printf("\n EXCLAM \n");
        }else if(token_array[i]->type == 16){
            printf("\n EXCLAMEQ \n");
        }else if(token_array[i]->type == 17){
            printf("\n COMMA \n");
        }else if(token_array[i]->type == 18){
            printf("\n COLON \n");
        }else if(token_array[i]->type == 19){
            printf("\n DOUBLE_QM \n");
        }else if(token_array[i]->type == 20){
            printf("\n RET_TYPE \n");
        }else if(token_array[i]->type == 21){
            printf("\n ID \n");
        }else if(token_array[i]->type == 22){
            printf("\n ID_QM \n");
        }else if(token_array[i]->type == 23){
            printf("\n NUM \t%d \n", token_array[i]->value.integer);
        }else if(token_array[i]->type == 24){
            printf("\n EXP \n");
        }else if(token_array[i]->type == 25){
            printf("\n DEC \t%lf \n", token_array[i]->value.type_double);
        }else if(token_array[i]->type == 26){
            printf("\n STRING \n %s \n", token_array[i]->value.vector->array);
        }else if(token_array[i]->type == 27){
            printf("\n ML STRING \n %s \n", token_array[i]->value.vector->array);
        }else if(token_array[i]->type == 28){ //keyword
            if(strcmp(token_array[i]->value.vector->array,"Double") == 0){
                printf("\n KEYWORD Double\n");
            } else if(strcmp(token_array[i]->value.vector->array,"Int") == 0){
                printf("\n KEYWORD Int \n");
            } else if(strcmp(token_array[i]->value.vector->array,"String") == 0){
                printf("\n KEYWORD String \n");
            } else if(strcmp(token_array[i]->value.vector->array,"else") == 0){
                printf("\n KEYWORD else \n");
            } else if(strcmp(token_array[i]->value.vector->array,"func") == 0){
                printf("\n KEYWORD func \n");
            } else if(strcmp(token_array[i]->value.vector->array,"if") == 0){
                printf("\n KEYWORD if \n");
            } else if(strcmp(token_array[i]->value.vector->array,"let") == 0){
                printf("\n KEYWORD let \n");
            } else if(strcmp(token_array[i]->value.vector->array,"nil") == 0){
                printf("\n KEYWORD nil \n");
            } else if(strcmp(token_array[i]->value.vector->array,"return") == 0){
                printf("\n KEYWORD return \n");
            } else if(strcmp(token_array[i]->value.vector->array,"var") == 0){
                printf("\n KEYWORD var \n");
            } else if(strcmp(token_array[i]->value.vector->array,"while") == 0){
                printf("\n KEYWORD while \n");
            } else if(strcmp(token_array[i]->value.vector->array,"readString") == 0){
                printf("\n KEYWORD readString \n");
            } else if(strcmp(token_array[i]->value.vector->array,"readInt") == 0){
                printf("\n KEYWORD readInt \n");
            } else if(strcmp(token_array[i]->value.vector->array,"readDouble") == 0){
                printf("\n KEYWORD readDouble \n");
            } else if(strcmp(token_array[i]->value.vector->array,"write") == 0){
                printf("\n KEYWORD write \n");
            } else if(strcmp(token_array[i]->value.vector->array,"Int2Double") == 0){
                printf("\n KEYWORD Int2Double \n");
            } else if(strcmp(token_array[i]->value.vector->array,"Double2Int") == 0){
                printf("\n KEYWORD Double2Int \n");
            } else if(strcmp(token_array[i]->value.vector->array,"length") == 0){
                printf("\n KEYWORD lenght \n");
            } else if(strcmp(token_array[i]->value.vector->array,"substring") == 0){
                printf("\n KEYWORD substring \n");
            } else if(strcmp(token_array[i]->value.vector->array,"ord") == 0){
                printf("\n KEYWORD ord \n");
            } else if(strcmp(token_array[i]->value.vector->array,"chr") == 0){
                printf("\n KEYWORD chr \n");
            }
        }else if(token_array[i]->type == 29){
            printf("\n UNDERSCORE \n");
        }else if(token_array[i]->type == 30){
            printf("\n LEFT_BRACKET \n");
        }else if(token_array[i]->type == 31){
            printf("\n RIGHT_BRACKET \n");
        } else if(token_array[i]->type == 32){
            printf("\n DIVIDE \n");
        }
    }

    //printf("%d\n", token_array[0]->value.integer);
    //printf("%lf\n", token_array[1]->value.type_double);
    //printf("%lf\n", token_array[2]->value.type_double);

    //printf("%s\n", token_array[17]->value.vector->array);
    //printf("%d\n", token_array[17]->value.keyword);
    //printf("%d\n", token_array[17]->type);
    //printf("%s\n", token_array[18]->value.vector->array);
    //printf("%d\n", token_array[18]->value.keyword);
    //printf("%d\n", token_array[18]->type);
    //printf("%s\n", token_array[19]->value.vector->array);
    //printf("%d\n", token_array[19]->value.keyword);
    //printf("%d\n", token_array[19]->type);


    /*if(token_array[0]->type == TOKEN_LPAR){
        printf("YEP\n");
    }
    if(token_array[1]->type == TOKEN_EOF){
        printf("Spravne");
    }*/
    //vector_dispose(token_array[17]->value.vector);
    //vector_dispose(token_array[18]->value.vector);
    //vector_dispose(token_array[19]->value.vector);

    //printf("%d", token_array[6]->value.integer);
    /*if(token_array[6]->value.vector){
        vector_dispose(token_array[6]->value.vector);
    }
    if(token_array[7]->value.vector){
        vector_dispose(token_array[7]->value.vector);
    }*/

    



    /*
    for (int i = 0; i < 100; i++)
    {   
        if(token_array[i]->value.vector){
        vector_dispose(token_array[i]->value.vector);
        }
        free(token_array[i]);
    }
    free(token_array);

    */
    
    //free(token_array[1]);
    //free(token_array);
    //free_alloc_memory(); // Free all allocated memory
    return 0;
}
