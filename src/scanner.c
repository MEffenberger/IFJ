#include "scanner.h"


token_t* get_me_token(){
    token_t* token = malloc(sizeof(token_t));
    //push_alloc_ptr(token, TOKEN);
    char readchar, next_char;
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
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_LPAR;
                            vector_dispose(buffer);
                            return token;

                        case(')'):
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_RPAR;
                            vector_dispose(buffer);
                            return token;

                        case('>'):
                            token->question_mark = false;
                            token->type = TOKEN_GREAT;

                            if((next_char = (char) getc(stdin)) == '='){
                                a_state = S_START;
                                token->question_mark = false;
                                token->type = TOKEN_GREAT_EQ;
                                return token;
                            } else {
                                ungetc(next_char, stdin);
                            }
                            vector_dispose(buffer);
                            return token;

                        case('<'):
                            token->question_mark = false;
                            token->type = TOKEN_LESS;

                            if((next_char = (char) getc(stdin)) == '='){
                                a_state = S_START;
                                token->question_mark = false;
                                token->type = TOKEN_LESS_EQ;
                                return token;
                            } else {
                                ungetc(next_char, stdin);
                            }
                            vector_dispose(buffer);
                            return token;

                        case('='):
                            token->question_mark = false;
                            token->type = TOKEN_EQ;

                            if((next_char = (char) getc(stdin)) == '='){
                                a_state = S_START;
                                token->question_mark = false;
                                token->type = TOKEN_EQEQ;
                                return token;
                            } else {
                                ungetc(next_char, stdin);
                            }
                            vector_dispose(buffer);
                            return token;

                        case('!'):
                            token->question_mark = false;
                            token->type = TOKEN_EXCLAM;

                            if((next_char = (char) getc(stdin)) == '='){
                                a_state = S_START;
                                token->question_mark = false;
                                token->type = TOKEN_EXCLAMEQ;
                                return token;
                            } else {
                                ungetc(next_char, stdin);
                            }
                            vector_dispose(buffer);
                            return token;

                        case('*'):
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_MULTIPLY;
                            vector_dispose(buffer);
                            return token;

                        case('+'):
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_PLUS;
                            vector_dispose(buffer);
                            return token;

                       case('-'):
                            token->question_mark = false;
                            token->type = TOKEN_MINUS;

                            if((next_char = (char) getc(stdin)) == '>'){
                                a_state = S_START;
                                token->question_mark = false;
                                token->type = TOKEN_RET_TYPE;
                                return token;
                            } else {
                                ungetc(next_char, stdin);
                            }
                            vector_dispose(buffer);
                            return token;

                        case(' '):
                            continue;

                        case('\t'):
                            continue;

                        case('\n'):
                            token->question_mark = false;
                            token->type = TOKEN_EOL;
                            vector_dispose(buffer);
                            return token;

                        default:
                            if((int) readchar == EOF){
                                token->type = TOKEN_EOF;
                                vector_dispose(buffer);
                                return token;
                            }
                            break;
                    }
            case(S_QM):
                if((next_char = (char) getc(stdin)) == '?'){
                    a_state = S_START;
                    token->question_mark = false;
                    token->type = TOKEN_DOUBLE_QM;
                    return token;
                } else {
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
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