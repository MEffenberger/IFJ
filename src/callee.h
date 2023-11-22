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
#include "error.h"

typedef struct callee {
    char *name;
    data_type return_type;
    int arg_count;
    char **args_names; // '_' for unnamed
    data_type *args_types;
} callee_t;

typedef struct callee_list {
    callee_t *callee;
    struct callee_list *next;
} callee_list_t;

callee_list_t* init_callee_list();
callee_t* init_callee(const char* name, data_type return_type);
void insert_callee_into_list(callee_list_t* list, const char* name, data_type return_type);
void insert_name_into_callee(callee_t* callee, char* id);
void insert_type_into_callee(callee_t* callee, data_type type);

void callee_list_dispose(callee_list_t* list);
void callee_dispose(callee_t* callee);





#endif //IFJ_CALLEE_H