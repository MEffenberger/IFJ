/**
 * @file string_vector.c
 * 
 * IFJ compiler 2023
 * 
 * @brief String vector file 
 * @author Samuel Hejnicek <hejni00>
 * @date 2023-10-21
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
#include <string.h>

vector* vector_init(){

    vector* v = (vector*)allocate_memory(sizeof(vector));

    v->size = 0;
    v->size_of_alloc = VECTOR_LENGTH;
    v->array = (char*)allocate_memory(VECTOR_LENGTH* (sizeof(char)));
    v->array[0] = '\0';
    return v;
}

bool vector_append(vector* v, char c){
    if(v->size + 1 == v->size_of_alloc){
        char *new_vec = reallocate_memory(v->array, v->size_of_alloc * 2 * sizeof(char));

        v->array = new_vec;
        v->size_of_alloc *= 2;
    }

    v->array[v->size] = c;
    v->size++;
    v->array[v->size] = '\0';
    return true;
}

bool vector_str_append(vector* v, char* c){
    for(unsigned int i = 0; i < strlen(c); i++){
        if(!(vector_append(v, c[i]))){
            return false;
        }
    }
    return true;
}

void vector_dispose(vector* v){
    v->size = 0;
    v->size_of_alloc = 0;
    if(v->array){
        free(v->array);
    }
    v->array = NULL;
    free(v);
    v = NULL;
}

bool vector_str_cmp(vector* v, char* s){
    return !(strcmp(v->array, s));
}

int vector_size(vector* v){
    return v->size;
}

int vector_alloc_size(vector* v){
    return v->size_of_alloc;
}
