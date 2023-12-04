/**
 * @file string_vector.h
 * 
 * IFJ compiler 2023
 * 
 * @brief String vector header file 
 * @author Samuel Hejnicek <xhejni00>
 * @date 2023-10-21
 */
#ifndef STRING_VECTOR_H
#define STRING_VECTOR_H

#include <stdbool.h>

#define VECTOR_LENGTH 8 //default size of dynamic array

typedef struct string_vector {
    char* array; //array of strings
    int size; //size of the vector
    int size_of_alloc; //size of allocated chars
} vector;

/**
 * @brief inicialization of the vector
 * 
 * @return pointer to allocated vector
 */
vector* vector_init();

/**
 * @brief Adds char to the  array
 * 
 * @param v pointer to vector
 * @param c char to be added
 * @return true if done correctly, false otherwise 
 */
bool vector_append(vector* v, char c);

/**
 * @brief Adds string to the end of the actual vector
 * 
 * @param v pointer to vector
 * @param s pointer to string to be added
 * @return true if done correctly, false otherwise 
 */
bool vector_str_append(vector *v, char *s);

/**
 * @brief Free vector
 * 
 * @param v pointer to vector
 */
void vector_dispose(vector *v);

/**
 * @brief Compares vector and string
 * 
 * @param v1 pointer to 1st vector
 * @param v2 pointer to string to be compared
 * @return true if vectors match, false otherwise
 */
bool vector_str_cmp(vector* v1, char* s);

/**
 * @brief Return size of vector
 * 
 * @param v pointer to vector
 * @return size of vector
 */
int vector_size(vector *v);

/**
 * @brief Return size of allocated space in vector
 * 
 * @param v pointer to vector
 * @return size of allocated space for chars in vector
 */
int vector_alloc_size(vector *v);
#endif