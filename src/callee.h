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
#include <stdlib.h>

    typedef struct callee{
    char *name;
    data_type return_type;
    int param_count;
    char **ids;
    }callee_t;

    typedef struct callee_list{
    callee_t *callee;
    struct callee_list *next;
    }callee_list_t;

callee_list_t* init_callee_list();
callee_t* init_callee(const char* name, data_type return_type, int param_count);
void insert_into_callee(callee_t* callee, char* id);




#endif //IFJ_CALLEE_H
