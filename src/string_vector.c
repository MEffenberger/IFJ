/**
 * @file string_vector.c
 * 
 * IFJ compiler 2023
 * 
 * @brief String vector file 
 * @author Samuel Hejnicek
 * @date 2023-10-21
 */

#include "string_vector.h"

vector* vector_init(){

    vector* v = malloc(sizeof(vector));
    if(!(v)){
        return NULL;
    }

    v->size = 0;
    v->size_of_alloc = VECTOR_LENGTH;
    v->array = (char* ) malloc(VECTOR_LENGTH* (sizeof(char)));

    if(!(v->array)){
        free(v);
        return NULL;
    }

    v->array[0] = '\0';

    return v;
}

bool vector_append(vector* v, char c){
    if(v->size + 1 == v->size_of_alloc){
        char *new_vec = realloc(v->array, v->size_of_alloc * 2 * sizeof(char));

        if(!(new_vec)){
            return false;
        }

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

void vector_clear(vector* v){
    v->size = 0;
    v->array[0] = '\0';
}

void vector_dispose(vector* v){
    v->size = 0;
    v->size_of_alloc = 0;
    free(v->array);
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
