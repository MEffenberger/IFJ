/**
 * @file callee.h
 *
 * IFJ23 compiler
 *
 * @brief A structure to represent individual function calls and their parameters
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */


#ifndef IFJ_CALLEE_H
#define IFJ_CALLEE_H

#include "symtable.h"
#include <string.h>

// Structure to hold information about a function call
typedef struct callee {
    char *name; // name of the function
    data_type return_type; // type of assignee
    int arg_count; // number of arguments
    char **args_names; // names of arguments, '_' if unnamed
    bool *args_initialized; // whether the argument is initialized
    data_type *args_types; // types of arguments
} callee_t;

// Structure to hold a list of callees
typedef struct callee_list {
    callee_t *callee;
    struct callee_list *next;
} callee_list_t;

/**
 * @brief Allocates memory for a new callee list and initializes it
 * 
 * @return callee_list_t* Pointer to the new element of callee list
 */
callee_list_t* init_callee_list();

/**
 * @brief Allocates memory for a new callee and initializes it
 * 
 * @param name Name of the callee
 * @return callee_t* Pointer to the new callee
 */
callee_t* init_callee(const char* name);

/**
 * @brief Inserts a new callee into the callee list
 * 
 * @param list Pointer to the callee list
 * @param name Name of the callee
 */
void insert_callee_into_list(callee_list_t* list, const char* name);

/**
 * @brief Inserts a argument's name into callee 
 * 
 * @param callee Pointer to the callee
 * @param id Name of the argument
 */
void insert_name_into_callee(callee_t* callee, char* id);

/**
 * @brief Inserts a argument's type into callee
 * 
 * @param callee Pointer to the callee
 * @param type Type of the argument
 */
void insert_type_into_callee(callee_t* callee, data_type type);

/**
 * @brief Inserts a argument's initialization into callee
 * 
 * @param callee Pointer to the callee
 * @param is_initialized Whether the argument is initialized
 */
void insert_bool_into_callee(callee_t* callee, bool is_initialized);

/**
 * @brief Disposes the whole callee list
 * 
 * @param first Pointer to the first element of the callee list
 */
void callee_list_dispose(callee_list_t *first);

#endif //IFJ_CALLEE_H
