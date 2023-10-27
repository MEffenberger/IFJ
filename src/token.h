//
// Created by marek on 20.10.2023.
//

#include "error.h"

#ifndef IFJ_TOKEN_H
#define IFJ_TOKEN_H

typedef union value_type_t
{
    int integer;
    double type_double;
    vector* vector;
    keyword_t keyword;
};


typedef struct token {
    value_type_t value;
    token_type_t type;
    int line;
    bool question_mark;
} token_t;



#endif //IFJ_TOKEN_H
