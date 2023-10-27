/**
 * @file scanner.c
 * 
 * IFJ23 compiler
 * 
 * @brief Scanner implementation
 *
 * @author Samuel Hejnicek <xhejni00>
 * @author Dominik Horut <xhorut01>
 */
#include "scanner.h"
#include <ctype.h>


keyword_t compare_keyword(vector* v){
    if(vector_str_cmp(v, "Double")){
        return KW_DOUBLE;
    } else if (vector_str_cmp(v, "else")){
        return KW_ELSE;
    } else if (vector_str_cmp(v, "func")){
        return KW_FUNC;
    } else if (vector_str_cmp(v, "if")){
        return KW_IF;
    } else if (vector_str_cmp(v, "Int")){
        return KW_INT;
    } else if (vector_str_cmp(v, "let")){
        return KW_LET;
    } else if (vector_str_cmp(v, "nil")){
        return KW_NIL;
    } else if (vector_str_cmp(v, "return")){
        return KW_RETURN;
    } else if (vector_str_cmp(v, "String")){
        return KW_STRING;
    } else if (vector_str_cmp(v, "var")){
        return KW_VAR;
    } else if (vector_str_cmp(v, "while")){
        return KW_WHILE;

    } else if (vector_str_cmp(v, "readString")){
        return KW_RD_STR;
    } else if (vector_str_cmp(v, "readInt")){
        return KW_RD_INT;
    } else if (vector_str_cmp(v, "readDouble")){
        return KW_RD_DBL;
    } else if (vector_str_cmp(v, "write")){
        return KW_WRT;
    } else if (vector_str_cmp(v, "Int2Double")){
        return KW_INT_2_DBL;
    } else if (vector_str_cmp(v, "Double2Int")){
        return KW_DBL_2_INT;
    } else if (vector_str_cmp(v, "length")){
        return KW_LENGHT;
    } else if (vector_str_cmp(v, "substring")){
        return KW_SUBSTR;
    } else if (vector_str_cmp(v, "ord")){
        return KW_ORD;
    } else if (vector_str_cmp(v, "chr")){
        return KW_CHR;
    } else {
        return 1000;
    }
}

token_t* get_me_token(){
    token_t* token = malloc(sizeof(token_t));
    //push_alloc_ptr(token, TOKEN);
    char readchar, next_char;
    automat_state_t a_state = S_START;
    vector* buffer = vector_init();
    token->value.vector = NULL;

    while ((readchar = (char) getc(stdin))){

        switch(a_state)
        {
            case (S_START):

                    if(readchar == ':'){
                        token->question_mark = false;
                        token->type = TOKEN_COLON;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == ','){
                        token->question_mark = false;
                        token->type = TOKEN_COMMA;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '?'){
                        a_state = S_QM;
                        token->question_mark = false;
                        vector_dispose(buffer);
                        //break;

                    } else if(readchar == '('){
                        a_state = S_START;
                        token->question_mark = false;
                        token->type = TOKEN_LPAR;
                        vector_dispose(buffer);
                        
                        return token;
                    } else if(readchar == ')'){
                        a_state = S_START;
                        token->question_mark = false;
                        token->type = TOKEN_RPAR;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '>'){
                        token->question_mark = false;
                        token->type = TOKEN_GREAT;

                        if((next_char = (char) getc(stdin)) == '='){
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_GREAT_EQ;
                            vector_dispose(buffer);
                            return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '<'){
                        token->question_mark = false;
                        token->type = TOKEN_LESS;

                        if((next_char = (char) getc(stdin)) == '='){
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_LESS_EQ;
                            vector_dispose(buffer);
                            return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '='){
                        token->question_mark = false;
                        token->type = TOKEN_EQ;

                        if((next_char = (char) getc(stdin)) == '='){
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_EQEQ;
                            vector_dispose(buffer);
                            return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '!'){
                        token->question_mark = false;
                        token->type = TOKEN_EXCLAM;

                        if((next_char = (char) getc(stdin)) == '='){
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_EXCLAMEQ;
                            vector_dispose(buffer);
                             return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;    

                    } else if(readchar == '*'){
                        a_state = S_START;
                        token->question_mark = false;
                        token->type = TOKEN_MULTIPLY;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '+'){
                        a_state = S_START;
                        token->question_mark = false;
                        token->type = TOKEN_PLUS;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '_'){
                        a_state = S_START;
                        token->question_mark = false;
                        token->type = TOKEN_UNDERSCORE;

                        if((next_char = (char) getc(stdin)) == '_' || isalpha(next_char) || isdigit(next_char)){
                            a_state = S_ID;
                            token->question_mark = false;
                            vector_append(buffer, readchar);
                            vector_append(buffer, next_char);
                            break;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;     

                    } else if(readchar == '-'){
                        token->question_mark = false;
                        token->type = TOKEN_MINUS;

                        if((next_char = (char) getc(stdin)) == '>'){
                            a_state = S_START;
                            token->question_mark = false;
                            token->type = TOKEN_RET_TYPE;
                            vector_dispose(buffer);
                            return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == ' '){
                        continue;
                        
                    } else if(readchar == '\t'){
                        continue;
                        
                    } else if(readchar == '\n'){
                        token->question_mark = false;
                        token->type = TOKEN_EOL;
                        vector_dispose(buffer);
                        return token;

                    } else if((int)readchar == EOF){
                        token->type = TOKEN_EOF;
                        vector_dispose(buffer);
                        return token;

                    } else if(isalpha(readchar)){
                        a_state = S_ID;
                        vector_append(buffer, readchar);
                        break;
                    } else if(isdigit(readchar)){
                        a_state = S_NUM;
                        vector_append(buffer, readchar);
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
            case(S_ID):
                if(isalpha(readchar) || isdigit(readchar) || readchar == '_'){
                    vector_append(buffer, readchar);
                    break;

                } else if(readchar == '?'){
                    keyword_t key = compare_keyword(buffer);
                    if(key < 3){
                        vector_append(buffer, readchar);
                        token->type = TOKEN_KEYWORD_QM;
                        token->value.keyword = key;
                        token->value.vector = buffer;
                        return token;
                    } else if (key > 3 && key != 1000){
                        ungetc(readchar, stdin);
                        a_state = S_QM;
                        token->type = TOKEN_KEYWORD;
                        token->value.keyword = key;
                        token->value.vector = buffer;
                        return token;
                    } else  {
                        ungetc(readchar, stdin);
                        a_state = S_QM;
                        token->type = TOKEN_ID;
                        token->value.vector = buffer;
                        return token;
                    }
                } else {
                    ungetc(readchar, stdin);
                    keyword_t key = compare_keyword(buffer);
                    if(key != 1000){
                        token->type = TOKEN_KEYWORD;
                        token->value.keyword = key;
                        token->value.vector = buffer;
                        return token;
                    } else {
                        token->type = TOKEN_ID;
                        token->value.vector = buffer;
                        return token;
                    }
                }
            
            case(S_NUM):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    break;
                } else if(readchar == '.'){
                    vector_append(buffer, readchar);
                    a_state = S_NUM_DOT;
                    break;
                } else if(readchar == 'e' || readchar == 'E'){
                    vector_append(buffer, readchar);
                    a_state = S_NUM_E;
                    break;
                } else {
                    ungetc(readchar, stdin);
                    token->type = TOKEN_NUM;
                    sscanf(buffer->array, "%d", &token->value.integer);
                    vector_dispose(buffer);
                    return token;
                }

            case(S_NUM_DOT):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    a_state = S_DEC;
                    break;
                } else {
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            
            case(S_DEC):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    break;
                } else if(readchar == 'e' || readchar == 'E'){
                    vector_append(buffer, readchar);
                    a_state = S_NUM_E;
                    break;
                } else {
                    ungetc(readchar, stdin);
                    token->type = TOKEN_DEC;
                    sscanf(buffer->array, "%lf", &token->value.type_double);
                    vector_dispose(buffer);
                    return token;
                }
            case(S_NUM_E):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    a_state = S_EXP;
                    break;
                } else if(readchar == '+' || readchar == '-'){
                    vector_append(buffer, readchar);
                    a_state = S_NUM_E_SIGN;
                    break;
                } else {
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_NUM_E_SIGN):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    a_state = S_EXP;
                    break; 
                } else {
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_EXP):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    break;
                } else {
                    ungetc(readchar, stdin);
                    token->type = TOKEN_EXP;
                    sscanf(buffer->array, "%lf", &token->value.type_double);
                    vector_dispose(buffer);
                    return token;
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