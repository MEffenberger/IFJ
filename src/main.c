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
    token_array = malloc(20*sizeof(token_t*));

    for(int i = 0; i < 20; i++){
        token_t* token = get_me_token();
        token_array[i] = token;
    }

    for (int i = 0; i < 20; i++)
    {
        printf("\n, %d, \n", token_array[i]->type);
    }
    
    printf("%s\n", token_array[17]->value.vector->array);
    printf("%s\n", token_array[18]->value.vector->array);
    printf("%s\n", token_array[19]->value.vector->array);


    /*if(token_array[0]->type == TOKEN_LPAR){
        printf("YEP\n");
    }
    if(token_array[1]->type == TOKEN_EOF){
        printf("Spravne");
    }*/
    //free(token_array[0]);
    //free(token_array[1]);
    //free(token_array);
    //free_alloc_memory(); // Free all allocated memory
    return 0;
}