//
// Created by marek on 20.10.2023.
//

#include "error.h"



#ifndef IFJ_TOKEN_H
#define IFJ_TOKEN_H

typedef union value_t
{
    int integer;
    double type_double;
    vector* vector;
    keyword_t keyword;
};


typedef struct token {
    char *value;
    type_t type;
    int line;

} token_t;



#endif //IFJ_TOKEN_H
