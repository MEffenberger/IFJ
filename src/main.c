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

struct AllocatedPointer *allocated_pointers = NULL; // Top of the stack for allocated pointers


int main() { 
    //parser();
    token_t** token_array;
    token_array = malloc(10*sizeof(token_t*));

    for(int i = 0; i < 10; i++){
        token_t* token = get_me_token();
        token_array[i] = token;
    }

    for (int i = 0; i < 10; i++)
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
            printf("\n STRING %s \n", token_array[i]->value.vector->array);
        }else if(token_array[i]->type == 27){
            printf("\n ML_STRING \n");
        }else if(token_array[i]->type == 28){
            printf("\n KEYWORD \n");
        }else if(token_array[i]->type == 29){
            printf("\n UNDERSCORE \n");
        }else if(token_array[i]->type == 30){
            printf("\n LEFT_BRACKET \n");
        }else if(token_array[i]->type == 31){
            printf("\n RIGHT_BRACKET \n");
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

    



    
    for (int i = 0; i < 10; i++)
    {   
        if(token_array[i]->value.vector){
        vector_dispose(token_array[i]->value.vector);
        }
        free(token_array[i]);
    }
    free(token_array);

    
    
    //free(token_array[1]);
    //free(token_array);
    //free_alloc_memory(); // Free all allocated memory
    return 0;
}