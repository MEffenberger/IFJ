#include "scanner.h"


token_t* get_me_token(){
    token_t* token = malloc(sizeof(token_t));
    //push_alloc_ptr(token, TOKEN);
    char readchar;
    automat_state_t a_state = S_START;
    vector* buffer = vector_init();

    while ((readchar = (char) getc(stdin))){
        switch(a_state)
        {
            case (S_START):
                    switch(readchar)
                    {
                        case(':'):
                            token->question_mark = false;
                            token->type = TOKEN_COLON;
                            vector_dispose(buffer);
                            return token;

                        case(','):
                            token->question_mark = false;
                            token->type = TOKEN_COMMA;
                            vector_dispose(buffer);
                            return token;

                        case('?'):
                            a_state = S_QM;
                            token->question_mark = false;
                            vector_dispose(buffer);
                            break;

                        case('('):
                            //a_state = S_LPAR;
                            token->question_mark = false;
                            token->type = TOKEN_LPAR;
                            vector_dispose(buffer);
                            return token;

                        case(')'):
                            //a_state = S_LPAR;
                            token->question_mark = false;
                            token->type = TOKEN_RPAR;
                            vector_dispose(buffer);
                            return token;
                    }
            /*case(S_QM):
                if(readchar == '?'){
                    token->question_mark = false;
                    token->type = TOKEN_DOUBLE_QM;
                    return token;
                } else {
                    exit(ERROR_LEX);
                    free(token);
                    token = NULL;
                    return NULL;
                }*/
            default:
                if((int) readchar == EOF){
                token->type = TOKEN_EOF;
                vector_dispose(buffer);
                return token;
            }

        }
    }
    return NULL;
}